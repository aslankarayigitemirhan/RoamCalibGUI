#pragma once
#include <QObject>
#include <opencv2/opencv.hpp>
#include <vector>
#include "PatternSettingsWidget.h"
#include "CornerSettingsWidget.h"

enum class CameraModel { Pinhole, Fisheye };

struct CalibrationResult {
    cv::Mat K, D;
    std::vector<cv::Mat> rvecs, tvecs;
    double rms = 0.0;
    std::vector<double> perViewError;
};

class CalibrationController : public QObject {
    Q_OBJECT
public:
    explicit CalibrationController(QObject* parent=nullptr);
    bool isCalibrated() const { return calibrated; }
void applyResult(const CalibrationResult& res) {
        K_ = res.K.clone();
        D_ = res.D.clone();
        calibrated = true;
    }
    void setPattern(const PatternConfig& cfg);
    void setCorner(const CornerSettings& s);
    void setModel(CameraModel m);

    PatternConfig pattern() const { return pCfg; }
    CornerSettings corner() const { return cSet; }
    CameraModel model() const { return model_; }   // ✅ artık model_ üzerinden

    void clear();
    bool detectPattern(const cv::Mat& gray, std::vector<cv::Point2f>& corners, cv::Size& patternSize);
    bool acceptFrame(const cv::Mat& frame);

    bool calibrate(CalibrationResult& out);
    cv::Mat undistortFrame(const cv::Mat& frame) const;

    const std::vector<std::vector<cv::Point2f>>& imagePoints() const { return imgPoints; }
    const std::vector<std::vector<cv::Point3f>>& objectPoints() const { return objPts; }
    cv::Size imageSize() const { return imgSize; }

    cv::Size patternSize() const;

private:
    PatternConfig pCfg;
    CornerSettings cSet;
    CameraModel model_ = CameraModel::Pinhole;   // ✅ model → model_

    std::vector<std::vector<cv::Point2f>> imgPoints;
    std::vector<std::vector<cv::Point3f>> objPts;
    cv::Size imgSize = {0,0};

    cv::Mat K_, D_;
    bool calibrated = false;
};
