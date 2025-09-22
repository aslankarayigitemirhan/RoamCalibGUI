#include "DeviceManager.h"
#include <QDir>
#include <QDebug>

DeviceManager::DeviceManager(QObject* parent) : QObject(parent) {}

QStringList DeviceManager::listDevices() {
    QStringList result;
    QDir dev("/dev");

    // Linux video device filter
    QStringList filters;
    filters << "video*";

    for (auto &f : dev.entryList(filters, QDir::System | QDir::Files)) {
        QString path = "/dev/" + f;
        result << path;
    }

    if (result.isEmpty()) {
        qWarning() << "No /dev/video* devices found!";
    }
    return result;
}
