#pragma once

#include <QObject>
#include <QSettings>
#include <QDir>

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    static SettingsManager& instance() {
        static SettingsManager inst;
        return inst;
    }

    bool showHiddenItems() const {
        return m_settings.value("showHiddenItems", false).toBool();
    }

    void setShowHiddenItems(bool show) {
        m_settings.setValue("showHiddenItems", show);
        emit settingsChanged();
    }

    QString defaultDirectory() const {
        return m_settings.value("defaultDirectory", QDir::rootPath()).toString();
    }

    void setDefaultDirectory(const QString &dir) {
        m_settings.setValue("defaultDirectory", dir);
        emit settingsChanged();
    }

signals:
    void settingsChanged();

private:
    SettingsManager() : m_settings("Unfold", "UnfoldApp") {}
    QSettings m_settings;

    // Prevent copying
    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;
};
