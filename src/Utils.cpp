#include "Utils.h"
#include <QSysInfo>
#include <numeric>

namespace utils {

QString osName() {
#if defined(Q_OS_WIN)
    return "Windows";
#elif defined(Q_OS_MAC)
    return "macOS";
#elif defined(Q_OS_LINUX)
    return "Linux";
#else
    return "Unknown";
#endif
}

std::vector<int> probeCameraIndices(int maxIndex) {
    std::vector<int> out;
    for (int i = 0; i < maxIndex; ++i) {
        cv::VideoCapture cap;
        if (!cap.open(i, cv::CAP_ANY)) {
            cap.release();
            cap.open(i, cv::CAP_V4L2);
        }
        if (cap.isOpened()) { out.push_back(i); cap.release(); }
    }
    return out;
}
QImage matToQImage(const cv::Mat &m) {
    if (m.empty()) return QImage();

    cv::Mat src = m.isContinuous() ? m : m.clone();
    switch (src.type()) {
    case CV_8UC1:
        return QImage(src.data, src.cols, src.rows, (int)src.step,
                      QImage::Format_Grayscale8).copy();

    case CV_8UC3: {
        cv::Mat rgb;
        cv::cvtColor(src, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, (int)rgb.step,
                      QImage::Format_RGB888).copy();
    }

    case CV_8UC4: {
        cv::Mat rgba;
        cv::cvtColor(src, rgba, cv::COLOR_BGRA2RGBA);
        return QImage(rgba.data, rgba.cols, rgba.rows, (int)rgba.step,
                      QImage::Format_RGBA8888).copy();
    }

    default: {
        // Normalize to [0,255]
        double minv, maxv;
        cv::minMaxLoc(src, &minv, &maxv);
        cv::Mat norm8u;
        src.convertTo(norm8u, CV_8U,
                      255.0 / (maxv - minv + 1e-9), -minv);

        if (norm8u.channels() == 1) {
            return QImage(norm8u.data, norm8u.cols, norm8u.rows,
                          (int)norm8u.step, QImage::Format_Grayscale8).copy();
        } else if (norm8u.channels() == 3) {
            cv::Mat rgb;
            cv::cvtColor(norm8u, rgb, cv::COLOR_BGR2RGB);
            return QImage(rgb.data, rgb.cols, rgb.rows,
                          (int)rgb.step, QImage::Format_RGB888).copy();
        } else if (norm8u.channels() == 4) {
            cv::Mat rgba;
            cv::cvtColor(norm8u, rgba, cv::COLOR_BGRA2RGBA);
            return QImage(rgba.data, rgba.cols, rgba.rows,
                          (int)rgba.step, QImage::Format_RGBA8888).copy();
        }
        return QImage();
    }
    }
}

cv::Mat qimageToMat(const QImage &img) {
    if (img.isNull()) return cv::Mat();

    QImage conv = img.convertToFormat(QImage::Format_RGB888);
    cv::Mat rgb(conv.height(), conv.width(), CV_8UC3,
                const_cast<uchar*>(conv.constBits()),
                (size_t)conv.bytesPerLine());
    cv::Mat bgr;
    cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);
    return bgr.clone();  // clone for safety
}


bool isPatternWellFramed(const std::vector<cv::Point2f>& corners,
                         const cv::Size& frameSize, double marginRatio) {
    if (corners.empty()) return false;
    float minx=1e9f, miny=1e9f, maxx=-1e9f, maxy=-1e9f;
    for (const auto &p : corners) {
        minx = std::min(minx, p.x); maxx = std::max(maxx, p.x);
        miny = std::min(miny, p.y); maxy = std::max(maxy, p.y);
    }
    float mx = frameSize.width * marginRatio;
    float my = frameSize.height * marginRatio;
    return (minx > mx && miny > my && maxx < frameSize.width - mx && maxy < frameSize.height - my);
}

std::vector<cv::Point3f> makeChessboardObjectPoints(cv::Size innerCorners, float squareSize) {
    std::vector<cv::Point3f> obj; obj.reserve(innerCorners.width * innerCorners.height);
    for (int r=0; r<innerCorners.height; ++r)
        for (int c=0; c<innerCorners.width; ++c)
            obj.emplace_back(c*squareSize, r*squareSize, 0.0f);
    return obj;
}

static double pointLineDev(const cv::Point2f& p, const cv::Vec4f& line) {
    cv::Point2f v(line[0], line[1]); cv::Point2f p0(line[2], line[3]);
    cv::Point2f w = p - p0;
    double area = std::abs(v.x * w.y - v.y * w.x);
    double norm = std::sqrt(v.x*v.x + v.y*v.y);
    return area / (norm + 1e-9);
}

LinearityResult computeLinearityMetric(const std::vector<cv::Point2f>& corners, cv::Size patternSize) {
    LinearityResult res; if ((int)corners.size() != patternSize.width*patternSize.height) return res;
    double rowSum=0; int rowCnt=0;
    for (int r=0; r<patternSize.height; ++r) {
        std::vector<cv::Point2f> row; row.reserve(patternSize.width);
        for (int c=0; c<patternSize.width; ++c) row.push_back(corners[r*patternSize.width + c]);
        cv::Vec4f line; cv::fitLine(row, line, cv::DIST_L2, 0, 0.01, 0.01);
        for (auto &p : row) { rowSum += pointLineDev(p, line); ++rowCnt; }
    }
    double colSum=0; int colCnt=0;
    for (int c=0; c<patternSize.width; ++c) {
        std::vector<cv::Point2f> col; col.reserve(patternSize.height);
        for (int r=0; r<patternSize.height; ++r) col.push_back(corners[r*patternSize.width + r*0 + c]);
        cv::Vec4f line; cv::fitLine(col, line, cv::DIST_L2, 0, 0.01, 0.01);
        for (auto &p : col) { colSum += pointLineDev(p, line); ++colCnt; }
    }
    res.meanRowDev = (rowCnt>0) ? rowSum/rowCnt : 0.0;
    res.meanColDev = (colCnt>0) ? colSum/colCnt : 0.0;
    return res;
}

double meanReprojError(const std::vector<std::vector<cv::Point2f>>& imgPts,
                       const std::vector<std::vector<cv::Point3f>>& objPts,
                       const std::vector<cv::Mat>& rvecs,
                       const std::vector<cv::Mat>& tvecs,
                       const cv::Mat& K,
                       const cv::Mat& D,
                       bool fisheye) {
    size_t n = imgPts.size();
    double total = 0.0; size_t count = 0;
    for (size_t i = 0; i < n; ++i) {
        std::vector<cv::Point2f> proj;
        if (fisheye) cv::fisheye::projectPoints(objPts[i], proj, rvecs[i], tvecs[i], K, D);
        else cv::projectPoints(objPts[i], rvecs[i], tvecs[i], K, D, proj);
        for (size_t j=0; j<proj.size() && j<imgPts[i].size(); ++j) { total += cv::norm(imgPts[i][j] - proj[j]); ++count; }
    }
    return (count>0) ? total / count : 0.0;
}

} // namespace utils
