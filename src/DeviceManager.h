#pragma once
#include <QObject>
#include <QStringList>

class DeviceManager : public QObject {
    Q_OBJECT
public:
    explicit DeviceManager(QObject* parent=nullptr);
    QStringList listDevices();
};
