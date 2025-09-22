#pragma once
#include <QWidget>
#include <QLabel>
#include <opencv2/opencv.hpp>

class PreviewPane : public QWidget {
    Q_OBJECT
public:
    explicit PreviewPane(QWidget* parent=nullptr);
    void updateFrames(const cv::Mat& distorted, const cv::Mat& undistorted);

    QWidget* distortedWidget() const { return (QWidget*)distorted; }
    QWidget* undistortedWidget() const { return (QWidget*)undistorted; }

private:
    QLabel* distorted;
    QLabel* undistorted;
};
