#pragma once
#include <QDialog>
#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QGroupBox>
#include "CalibrationController.h"


class ResultsWindow : public QDialog
{
    Q_OBJECT
public:
    ResultsWindow(const CalibrationResult &res,
                  const std::vector<std::vector<cv::Point2f>> &imgPts,
                  const std::vector<std::vector<cv::Point3f>> &objPts,
                  const cv::Size &imgSize,
                  const cv::Size &patSize,
                  QWidget *parent = nullptr);

private slots:
    void onVizChanged(int idx);
    void onSaveEnv();

private:
    void buildParamsText();
    void clearPlotArea();
    void buildPlot_Reproj();
    void buildPlot_Linearity();

    // --- Data ---
    CalibrationResult res_;
    std::vector<std::vector<cv::Point2f>> imgPts_;
    std::vector<std::vector<cv::Point3f>> objPts_;
    cv::Size imgSize_;
    cv::Size patSize_;

    // --- UI ---
    QTabWidget *tabWidget;
    QLabel *lblParams = nullptr;

    // Plot areas
    QVBoxLayout *plotHost = nullptr;
    QVBoxLayout *plotHostLinear = nullptr;

    QChart *chart = nullptr;
    QChartView *chartView = nullptr;
};
