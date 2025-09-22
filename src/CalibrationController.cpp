#include "CalibrationController.h"
#include "Utils.h"
#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>
CalibrationController::CalibrationController(QObject* parent): QObject(parent) {}

void CalibrationController::setPattern(const PatternConfig& cfg) { pCfg = cfg; }
void CalibrationController::setCorner(const CornerSettings& s) { cSet = s; }
void CalibrationController::setModel(CameraModel m) { model_ = m; }   // ✅ model_ kullanılıyor

void CalibrationController::clear() {
    imgPoints.clear();
    objPts.clear();
    calibrated = false;
    K_.release();
    D_.release();
    imgSize = {0,0};
}

#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>

bool CalibrationController::detectPattern(const cv::Mat& gray,
                                          std::vector<cv::Point2f>& corners,
                                          cv::Size& patSize) {
    bool found = false;
    corners.clear();

    if (pCfg.type == PatternType::Checkerboard) {
        patSize = pCfg.innerSize;
        int flags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE;
        std::vector<cv::Point2f> tmp;
        found = cv::findChessboardCorners(gray, patSize, tmp, flags);
        if (found) {
            cv::cornerSubPix(gray, tmp,
                             cv::Size(cSet.winSize, cSet.winSize),
                             cv::Size(-1, -1),
                             cv::TermCriteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER,
                                              cSet.termMaxIters, cSet.termEps));
            corners = tmp;
        }
    } 
    else if (pCfg.type == PatternType::Circles) {
        patSize = pCfg.circleGridSize;
        found = cv::findCirclesGrid(gray, patSize, corners, cv::CALIB_CB_SYMMETRIC_GRID);
    } 
    else if (pCfg.type == PatternType::AsymmetricCircles) {
        patSize = pCfg.circleGridSize;
        found = cv::findCirclesGrid(gray, patSize, corners, cv::CALIB_CB_ASYMMETRIC_GRID);
    } 
    else if (pCfg.type == PatternType::AprilTag) {
        // ✅ AprilTag için aruco dictionary kullanılır
        cv::Ptr<cv::aruco::Dictionary> dict;
        if (pCfg.tagFamily == 0)
            dict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_APRILTAG_36h11);
        else if (pCfg.tagFamily == 1)
            dict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_APRILTAG_25h9);
        else
            dict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_APRILTAG_16h5);

        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> cornersVec;

        cv::aruco::detectMarkers(gray, dict, cornersVec, ids);

        if (!ids.empty()) {
            // Şimdilik ilk tag’in köşeleri
            corners = cornersVec[0];
            patSize = cv::Size(2, 2); // 4 corner
            found = true;
        }
    }
    return found;
}


bool CalibrationController::acceptFrame(const cv::Mat& frame) {
    if (frame.empty()) return false;
    if (imgSize.width==0) imgSize = frame.size();

    cv::Mat gray;
    if (frame.channels()==3) cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    else gray = frame;

    std::vector<cv::Point2f> corners; cv::Size patSize;
    if (!detectPattern(gray, corners, patSize)) return false;
    if (!utils::isPatternWellFramed(corners, gray.size())) return false;

    if (pCfg.type == PatternType::Checkerboard) {
        objPts.push_back(utils::makeChessboardObjectPoints(pCfg.innerSize, (float)pCfg.squareSize));
    } else {
        std::vector<cv::Point3f> obj;
        obj.reserve(pCfg.circleGridSize.width * pCfg.circleGridSize.height);
        for (int r=0; r<pCfg.circleGridSize.height; ++r) {
            for (int c=0; c<pCfg.circleGridSize.width; ++c) {
                float x = (float)c * (float)pCfg.circleSpacing;
                float y = (float)r * (float)pCfg.circleSpacing;
                if (!pCfg.symmetric && (r%2==1)) x += 0.5f*(float)pCfg.circleSpacing;
                obj.emplace_back(x,y,0.0f);
            }
        }
        objPts.push_back(obj);
    }
    imgPoints.push_back(corners);
    return true;
}

bool CalibrationController::calibrate(CalibrationResult& out) {
    if (imgPoints.size() < 5) return false;

    cv::Mat K, D;
    std::vector<cv::Mat> rvecs, tvecs;
    double rms = 0.0;

    if (model_ == CameraModel::Fisheye) {   // ✅ model_ kullanılıyor
        K = cv::Mat::eye(3,3,CV_64F);
        D = cv::Mat::zeros(4,1,CV_64F);
        std::vector<cv::Mat> rvecs_, tvecs_;
        cv::fisheye::calibrate(objPts, imgPoints, imgSize, K, D, rvecs_, tvecs_,
                               cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC);
        rvecs = rvecs_; tvecs = tvecs_;
        rms = utils::meanReprojError(imgPoints, objPts, rvecs, tvecs, K, D, true);
    } else {
        D = cv::Mat();
        rms = cv::calibrateCamera(objPts, imgPoints, imgSize,
                                  K, D, rvecs, tvecs,
                                  cv::CALIB_RATIONAL_MODEL | cv::CALIB_THIN_PRISM_MODEL);
    }

    out.K = K; out.D = D;
    out.rvecs = rvecs; out.tvecs = tvecs;
    out.rms = rms;

    out.perViewError.resize(imgPoints.size());
    for (size_t i=0; i<imgPoints.size(); ++i) {
        std::vector<cv::Point2f> proj;
        if (model_==CameraModel::Fisheye)   // ✅ model_ kullanılıyor
            cv::fisheye::projectPoints(objPts[i], proj, rvecs[i], tvecs[i], K, D);
        else
            cv::projectPoints(objPts[i], rvecs[i], tvecs[i], K, D, proj);
        double sum=0;
        for (size_t j=0;j<proj.size();++j)
            sum += cv::norm(proj[j]-imgPoints[i][j]);
        out.perViewError[i] = (proj.empty()?0.0:sum/proj.size());
    }

    K_ = K.clone();
    D_ = D.clone();
    calibrated = true;
    return true;
}

cv::Mat CalibrationController::undistortFrame(const cv::Mat& frame) const {
    if (!calibrated || frame.empty()) return cv::Mat();

    cv::Mat und;
    cv::Mat newK = cv::getOptimalNewCameraMatrix(
        K_, D_, frame.size(), 1.0, frame.size(), 0);

    if (model_ == CameraModel::Fisheye) {
        cv::fisheye::undistortImage(frame, und, K_, D_, newK);
    } else {
        cv::undistort(frame, und, K_, D_, newK);
    }

    return und;
}



cv::Size CalibrationController::patternSize() const {
    return (pCfg.type == PatternType::Checkerboard) ? pCfg.innerSize : pCfg.circleGridSize;
}
