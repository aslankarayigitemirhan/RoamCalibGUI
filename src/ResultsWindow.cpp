#include "ResultsWindow.h"
#include "Utils.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QGroupBox>
QT_USE_NAMESPACE

ResultsWindow::ResultsWindow(const CalibrationResult& res,
                             const std::vector<std::vector<cv::Point2f>>& imgPts,
                             const std::vector<std::vector<cv::Point3f>>& objPts,
                             const cv::Size& imgSize,
                             const cv::Size& patSize,
                             QWidget* parent)
    : QDialog(parent), res_(res), imgPts_(imgPts), objPts_(objPts), imgSize_(imgSize), patSize_(patSize) 
{
    setWindowTitle("Calibration Results");
    resize(1000, 700);

    auto *mainLayout = new QVBoxLayout(this);

    // 🔹 Kamera matrisi ve parametreler için subpanel
    QGroupBox *paramBox = new QGroupBox("Calibration Parameters", this);
    auto *paramLay = new QVBoxLayout(paramBox);
    lblParams = new QLabel(this);
    lblParams->setTextInteractionFlags(Qt::TextSelectableByMouse); // kopyalanabilir olsun
    lblParams->setFont(QFont("Courier", 10)); // monospace daha düzenli
    paramLay->addWidget(lblParams);
    buildParamsText();

    // 🔹 Tab widget (grafikler)
    tabWidget = new QTabWidget(this);

    // Reprojection plot tab
    auto *tabReproj = new QWidget();
    auto *layReproj = new QVBoxLayout(tabReproj);
    plotHost = new QVBoxLayout();
    layReproj->addLayout(plotHost);
    buildPlot_Reproj();
    tabWidget->addTab(tabReproj, "Reprojection Error");

    // Linearity plot tab
    auto *tabLinear = new QWidget();
    auto *layLinear = new QVBoxLayout(tabLinear);
    plotHostLinear = new QVBoxLayout();
    layLinear->addLayout(plotHostLinear);
    buildPlot_Linearity();
    tabWidget->addTab(tabLinear, "Linearity");

    // 🔹 Hepsini ana layout’a ekle
    mainLayout->addWidget(paramBox, 1);   // üst panel
    mainLayout->addWidget(tabWidget, 3);  // alt panel
    setLayout(mainLayout);

    setSizeGripEnabled(true);
}



void ResultsWindow::buildParamsText() {
    QString s;
    s += "📏 RMS Error: " + QString::number(res_.rms, 'f', 4) + "\n";
    s += "🖼️ Image Size: " + QString::number(imgSize_.width) + " x " + QString::number(imgSize_.height) + "\n\n";

    s += "📐 Camera Matrix K:\n";
    if (!res_.K.empty()) {
        for (int r=0;r<res_.K.rows;++r) {
            for (int c=0;c<res_.K.cols;++c) {
                s += QString::number(res_.K.at<double>(r,c), 'f', 6) + (c+1<res_.K.cols?"\t":"");
            }
            s += "\n";
        }
    }

    s += "\n🎯 Distortion Coeffs D: ";
    if (!res_.D.empty()) {
        for (int i=0;i<res_.D.total();++i)
            s += QString::number(res_.D.at<double>(i), 'f', 6) + " ";
    }

    lblParams->setText(s);
}

void ResultsWindow::clearPlotArea() {
    QLayoutItem* child;
    while ((child = plotHost->takeAt(0)) != nullptr) {
        if (child->widget()) child->widget()->deleteLater();
        delete child;
    }
    chartView = nullptr;
    chart = nullptr;
}

void ResultsWindow::onVizChanged(int idx) {
    clearPlotArea();
    if (idx==0) buildPlot_Reproj();
    else buildPlot_Linearity();
}

void ResultsWindow::buildPlot_Reproj() {

    // Bar data
    QBarSet *set = new QBarSet("Reproj Error");
    QStringList categories;
    for (int i = 0; i < (int)res_.perViewError.size(); ++i) {
        *set << res_.perViewError[i];
        categories << QString::number(i+1);
    }

    // Bar series
    QBarSeries *series = new QBarSeries();
    series->append(set);

    chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);

    // X axis (categories)
    QBarCategoryAxis *axX = new QBarCategoryAxis();
    axX->append(categories);
    axX->setTitleText("Frame index");

    // Y axis
    QValueAxis *axY = new QValueAxis();
    axY->setTitleText("Mean reprojection error (px)");

    chart->addAxis(axX, Qt::AlignBottom);
    chart->addAxis(axY, Qt::AlignLeft);
    series->attachAxis(axX);
    series->attachAxis(axY);

    // 🔹 RMS line (benchmark)
    auto *rmsLine = new QLineSeries();
    rmsLine->setName("RMS error");
    rmsLine->setColor(Qt::red);

    double rms = res_.rms;
    for (int i = 0; i < (int)res_.perViewError.size(); ++i)
        rmsLine->append(i+1, rms);

    chart->addSeries(rmsLine);
    rmsLine->attachAxis(axX);
    rmsLine->attachAxis(axY);

    // Chart view
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    plotHost->addWidget(chartView);
}

void ResultsWindow::buildPlot_Linearity() {
    auto *seriesR = new QLineSeries(); seriesR->setName("Row mean abs dev");
    auto *seriesC = new QLineSeries(); seriesC->setName("Col mean abs dev");

    double sumR = 0.0, sumC = 0.0;
    int N = (int)imgPts_.size();

    for (int i = 0; i < N; ++i) {
        auto lr = utils::computeLinearityMetric(imgPts_[i], patSize_);
        seriesR->append(i+1, lr.meanRowDev);
        seriesC->append(i+1, lr.meanColDev);
        sumR += lr.meanRowDev;
        sumC += lr.meanColDev;
    }

    double meanR = (N > 0) ? sumR / N : 0.0;
    double meanC = (N > 0) ? sumC / N : 0.0;

    chart = new QChart();
    chart->addSeries(seriesR);
    chart->addSeries(seriesC);

    auto *axX = new QValueAxis();
    axX->setTitleText("Frame index");
    axX->setRange(1, N);

    auto *axY = new QValueAxis();
    axY->setTitleText("Linearity deviation (px)");

    chart->addAxis(axX, Qt::AlignBottom);
    chart->addAxis(axY, Qt::AlignLeft);
    seriesR->attachAxis(axX); seriesR->attachAxis(axY);
    seriesC->attachAxis(axX); seriesC->attachAxis(axY);

    // Ortalama çizgiler
    auto *lineR = new QLineSeries();
    lineR->setName("Row avg");
    lineR->setColor(Qt::red);
    lineR->append(1, meanR);
    lineR->append(N, meanR);

    auto *lineC = new QLineSeries();
    lineC->setName("Col avg");
    lineC->setColor(Qt::darkGreen);
    lineC->append(1, meanC);
    lineC->append(N, meanC);

    chart->addSeries(lineR);
    chart->addSeries(lineC);
    lineR->attachAxis(axX); lineR->attachAxis(axY);
    lineC->attachAxis(axX); lineC->attachAxis(axY);

    chart->setTitle("Linearity metric (lower is better)");

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    plotHost->addWidget(chartView);
}



void ResultsWindow::onSaveEnv() {
    QString file = QFileDialog::getSaveFileName(this, "Save Calibration Environment", "", "JSON (*.json)");
    if (file.isEmpty()) return;

    QJsonObject root;

    // RMS
    root["rms"] = res_.rms;

    // K
    QJsonArray Karr;
    for (int r=0;r<res_.K.rows;++r) {
        QJsonArray row;
        for (int c=0;c<res_.K.cols;++c) row.append(res_.K.at<double>(r,c));
        Karr.append(row);
    }
    root["K"] = Karr;

    // D
    QJsonArray Darr;
    for (int i=0;i<res_.D.total();++i) Darr.append(res_.D.at<double>(i));
    root["D"] = Darr;

    // Per-view errors
    QJsonArray perView;
    for (double v: res_.perViewError) perView.append(v);
    root["per_view_error"] = perView;

    // Meta info
    root["image_w"] = imgSize_.width;
    root["image_h"] = imgSize_.height;
    root["pattern_w"] = patSize_.width;
    root["pattern_h"] = patSize_.height;

    QFile f(file);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
        f.close();
    }
}
