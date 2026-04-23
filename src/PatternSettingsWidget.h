#pragma once
#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QStackedWidget>
#include <opencv2/opencv.hpp>

enum class PatternType { Checkerboard, Circles, AsymmetricCircles, AprilTag };

struct PatternConfig {
    PatternType type = PatternType::Checkerboard;

    // Checkerboard
    cv::Size innerSize = {9, 6};   // inner corners (cols, rows)
    double squareSize = 0.025;     // meters

    // Circles / Asymmetric
    cv::Size circleGridSize = {4, 11};
    bool symmetric = true;
    double circleSpacing = 0.01;   // meters

    // AprilTag
    int tagFamily = 0;             // 0=TAG_36h11, 1=TAG_25h9, 2=TAG_16h5
    cv::Size tagGridSize = {6, 8}; // cols x rows
    float tagSize = 0.04f;         // meters
};

class PatternSettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit PatternSettingsWidget(QWidget* parent = nullptr);

    PatternConfig config() const;
    void apply(const PatternConfig& cfg);

signals:
    void patternChanged(const PatternConfig& cfg);

private:
    QComboBox* patternCombo;
    QStackedWidget* stack;

    // Checkerboard page
    QSpinBox *cbCols, *cbRows;
    QDoubleSpinBox *cbSize;

    // Circles controls
    QSpinBox *cgCols, *cgRows;
    QDoubleSpinBox *cgSpacing;

    // Asymmetric circles controls
    QSpinBox *acCols, *acRows;
    QDoubleSpinBox *acSpacing;

    // AprilTag controls
    QComboBox* tagFamilyCombo;
    QSpinBox* tagCols;
    QSpinBox* tagRows;
    QDoubleSpinBox* tagSizeSpin;

    // UI builders
    QWidget* makeCheckerPage();
    QWidget* makeCirclesPage(bool asymmetric);
    QWidget* makeAprilTagPage();

    void emitChange();
};
