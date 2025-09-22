#pragma once
#include <QDialog>
#include <QSize>
#include "PatternSettingsWidget.h"
#include "CornerSettingsWidget.h"
#include "CalibrationController.h"

class QComboBox;
class QSpinBox;
class QPushButton;

class SettingsWindow : public QDialog {
    Q_OBJECT
public:
    explicit SettingsWindow(QWidget* parent=nullptr);

    void setInitial(const PatternConfig& p, const CornerSettings& c,
                    CameraModel m, const QSize& datasetSize);

signals:
    void parametersSaved(const PatternConfig& p, const CornerSettings& c,
                         CameraModel m, const QSize& datasetSize);

private slots:
    void onSave();

private:
    PatternSettingsWidget* patternW;
    CornerSettingsWidget* cornerW;
    QComboBox* modelCombo;
    QSpinBox* imgW;
    QSpinBox* imgH;
    QPushButton* btnSave;
    QPushButton* btnExit;
    QPushButton* btnBack;

    PatternConfig p_;
    CornerSettings c_;
    CameraModel m_ = CameraModel::Pinhole;
    QSize dsSize_;
};
