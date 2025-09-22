#pragma once
#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <opencv2/opencv.hpp>

enum class CornerMethod { Harris, ShiTomasi, FAST, ORB_ }; // ORB for visualization

struct CornerSettings {
    CornerMethod method = CornerMethod::ShiTomasi;
    int maxCorners = 500;
    double quality = 0.01; // for ShiTomasi
    double minDistance = 5.0;
    int blockSize = 3; // Harris/Shi
    bool useHarris = false; 
    double harrisK = 0.04;
    int fastThreshold = 20; 
    bool fastNonmax = true;

    // Termination criteria for cornerSubPix
    int termMaxIters = 30; 
    double termEps = 0.01; 
    int winSize = 5;
};

class CornerSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit CornerSettingsWidget(QWidget* parent=nullptr);

    CornerSettings settings() const;
    /// 🔑 Yeni: parametreleri dışarıdan uygula
    void apply(const CornerSettings& s);

signals:
    void settingsChanged(const CornerSettings& s);

private:
    QComboBox* methodCombo;
    // common
    QSpinBox *maxCorners;
    QDoubleSpinBox *quality, *minDist;
    QSpinBox *blockSize; 
    QDoubleSpinBox *harrisK;
    QSpinBox *fastThresh; 
    QComboBox *fastNonmax;
    QSpinBox *termIters, *winSize;
    QDoubleSpinBox *termEps;

    void emitChange();
};
