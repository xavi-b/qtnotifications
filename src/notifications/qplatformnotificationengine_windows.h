#ifndef QPLATFORMNOTIFICATIONENGINE_WINDOWS_H
#define QPLATFORMNOTIFICATIONENGINE_WINDOWS_H

#include <QtNotifications/qplatformnotificationengine.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <windows.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.UI.Notifications.h>

QT_BEGIN_NAMESPACE

class QPlatformNotificationEngineWindows : public QPlatformNotificationEngine
{
    Q_OBJECT
public:
    explicit QPlatformNotificationEngineWindows(QObject *parent = nullptr);
    ~QPlatformNotificationEngineWindows() = default;

    bool isSupported() const override;
    uint sendNotification(const QString &title,
                         const QString &message,
                         const QVariantMap &parameters,
                         const QMap<QString, QString> &actions) override;

private:
    void ensureComInitialized() const;
    void setAppUserModelID();

    // Event handler methods for toast events
    void onToastActivated(winrt::Windows::UI::Notifications::ToastNotification const& sender, winrt::Windows::Foundation::IInspectable const& args);
    void onToastDismissed(winrt::Windows::UI::Notifications::ToastNotification const& sender, winrt::Windows::UI::Notifications::ToastDismissedEventArgs const& args);

    QString m_appUserModelID;
    // Map from ToastNotification pointer to notification ID
    QHash<const void*, uint> m_notificationIdMap;
};

QPlatformNotificationEngine *qt_create_notification_engine_windows();

QT_END_NAMESPACE

#endif // QPLATFORMNOTIFICATIONENGINE_WINDOWS_H
