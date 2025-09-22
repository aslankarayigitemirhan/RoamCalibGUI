#include "ResultsDialog.h"
#include "Utils.h"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPainter>

ResultsDialog::ResultsDialog(const CalibrationResult& res,
                             const std::vector<std::vector<cv::Point2f>>& undCorners,
                             cv::Size patternSize,
                             QWidget* parent) : QDialog(parent) {
    setWindowTitle("Calibration Results & Analysis");
    resize(800, 600);

    auto *lay = new QVBoxLayout(this);

    // Text summary
    auto *text = new QTextEdit(this);
    text->setReadOnly(true);
    text->append("<b>RMS reprojection error:</b> " + QString::number(res.rms, 'f', 4) + " px");
    text->append("<b>Camera Matrix K</b>:");
    for (int r=0; r<3; ++r) {
        QString row;
        for (int c=0; c<3; ++c)
            row += QString::number(res.K.at<double>(r,c), 'f', 6) + " ";
        text->append(row);
    }
    text->append("<b>Distortion Coeffs D</b>:");
    {
        QString row;
        for (int i=0; i<res.D.total(); ++i)
            row += QString::number(res.D.at<double>(i), 'f', 6) + " ";
        text->append(row);
    }

    // Per-view error chart
    auto *series = new QLineSeries();
    for (int i=0; i<(int)res.perViewError.size(); ++i)
        series->append(i, res.perViewError[i]);

    auto *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);

    auto *axX = new QValueAxis();
    axX->setTitleText("Frame index");
    auto *axY = new QValueAxis();
    axY->setTitleText("Per-view reproj error (px)");
    chart->addAxis(axX, Qt::AlignBottom);
    chart->addAxis(axY, Qt::AlignLeft);
    series->attachAxis(axX);
    series->attachAxis(axY);

    auto *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Linearity metric
    double meanRow = 0.0, meanCol = 0.0; int cnt = 0;
    for (const auto& v : undCorners) {
        if ((int)v.size() == patternSize.width * patternSize.height) {
            auto lr = utils::computeLinearityMetric(v, patternSize);
            meanRow += lr.meanRowDev;
            meanCol += lr.meanColDev;
            ++cnt;
        }
    }
    if (cnt > 0) {
        meanRow /= cnt;
        meanCol /= cnt;
    }
    text->append(QString("<b>Linearity metric</b> (mean row dev, mean col dev) in px: %1, %2")
                 .arg(QString::number(meanRow,'f',4))
                 .arg(QString::number(meanCol,'f',4)));
text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    lay->addWidget(text);
    lay->addWidget(chartView, 1);
}