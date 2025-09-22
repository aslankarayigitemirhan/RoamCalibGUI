#pragma once
#include <QDialog>
#include <QTextEdit>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <opencv2/opencv.hpp>
#include "CalibrationController.h"
#include "Utils.h"


QT_USE_NAMESPACE


class ResultsDialog : public QDialog {
Q_OBJECT
public:
ResultsDialog(const CalibrationResult& res,
const std::vector<std::vector<cv::Point2f>>& undistortedCornersPerView,
cv::Size patternSize,
QWidget* parent=nullptr);
};