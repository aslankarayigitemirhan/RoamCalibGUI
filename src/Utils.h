#pragma once
#include <QImage>
#include <QString>
#include <opencv2/opencv.hpp>
#include <vector>

namespace utils {

QString osName();
std::vector<int> probeCameraIndices(int maxIndex = 10);

QImage matToQImage(const cv::Mat& m);
cv::Mat qimageToMat(const QImage& img);

bool isPatternWellFramed(const std::vector<cv::Point2f>& corners,
                         const cv::Size& frameSize,
                         double marginRatio = 0.06);

std::vector<cv::Point3f> makeChessboardObjectPoints(cv::Size innerCorners, float squareSize);

struct LinearityResult {
    double meanRowDev = 0.0;
    double meanColDev = 0.0;
};
LinearityResult computeLinearityMetric(const std::vector<cv::Point2f>& corners,
                                       cv::Size patternSize);

double meanReprojError(const std::vector<std::vector<cv::Point2f>>& imgPts,
                       const std::vector<std::vector<cv::Point3f>>& objPts,
                       const std::vector<cv::Mat>& rvecs,
                       const std::vector<cv::Mat>& tvecs,
                       const cv::Mat& K,
                       const cv::Mat& D,
                       bool fisheye);

} // namespace utils
