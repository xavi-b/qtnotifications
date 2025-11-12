#include "qplatformnotificationengine_windows.h"
#include <windows.h>
#include <shobjidl.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <winrt/Windows.UI.Notifications.h>
#include <combaseapi.h>
#include <propvarutil.h>
#include <winternl.h>
#include <QtCore/qstring.h>
#include <QtCore/qdebug.h>
#include <QtCore/qmap.h>
#include <winrt/impl/Windows.Foundation.1.h>
#include <winrt/impl/Windows.UI.Notifications.1.h>
#include <winrt/Windows.Foundation.h>
#include <QtCore/QRegularExpression>

QT_BEGIN_NAMESPACE

using namespace winrt;
using namespace winrt::Windows::Data::Xml::Dom;
using namespace winrt::Windows::UI::Notifications;

// Helper to sanitize AppUserModelID parts
static QString sanitizeAppUserModelID(const QString &input)
{
    QString sanitized = input;
    sanitized.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9.-]")), QStringLiteral("_"));
    return sanitized;
}

QPlatformNotificationEngineWindows::QPlatformNotificationEngineWindows(QObject *parent)
  : QPlatformNotificationEngine(parent)
{
    // Use organizationDomain and applicationName for AppUserModelID, sanitized
    QString org = qApp ? qApp->organizationDomain() : QStringLiteral("qt");
    QString app = qApp ? qApp->applicationName() : QStringLiteral("QtNotifications");
    org = sanitizeAppUserModelID(org);
    app = sanitizeAppUserModelID(app);
    m_appUserModelID = org + QLatin1Char('.') + app;
    setAppUserModelID();
}

void QPlatformNotificationEngineWindows::handleActionInvoked(uint notificationId, const QString &actionKey)
{
    emit actionInvoked(notificationId, actionKey);
}

void QPlatformNotificationEngineWindows::handleNotificationClosed(uint notificationId, uint reason)
{
    emit notificationClosed(notificationId, reason);
}

void QPlatformNotificationEngineWindows::handleNotificationClicked(uint notificationId)
{
    emit notificationClicked(notificationId);
}

bool QPlatformNotificationEngineWindows::isSupported() const
{
    RTL_OSVERSIONINFOW osvi;
    ZeroMemory(&osvi, sizeof(RTL_OSVERSIONINFOW));
    osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod) {
        RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (fxPtr != nullptr) {
            fxPtr(&osvi);
        }
    }
    return osvi.dwMajorVersion >= 10;
}

bool QPlatformNotificationEngineWindows::sendNotification(const QString &summary, const QString &body, const QString &icon, const QMap<QString, QString> &actions, int type)
{
    ensureComInitialized();
    static uint s_notificationId = 1;
    uint notificationId = s_notificationId++;
    QString xml = QStringLiteral(
        "<toast>"
        "<visual>"
        "<binding template=\"ToastGeneric\">"
    );

    // Add icon if provided
    if (!icon.isEmpty()) {
        // Support different icon formats and placements
        // You can use: "appLogoOverride", "hero", "inline"
        QString placement = QStringLiteral("appLogoOverride");

        // If icon path contains "hero" or "inline", use that placement
        if (icon.contains(QStringLiteral("hero"), Qt::CaseInsensitive)) {
            placement = QStringLiteral("hero");
        } else if (icon.contains(QStringLiteral("inline"), Qt::CaseInsensitive)) {
            placement = QStringLiteral("inline");
        }

        xml += QStringLiteral("<image placement=\"%1\" src=\"%2\"/>").arg(placement, icon);
    }

    xml += QStringLiteral(
        "<text>%1</text>"
        "<text>%2</text>"
        "</binding>"
        "</visual>"
    ).arg(summary, body);
    if (!actions.isEmpty()) {
        xml += "<actions>";
        for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
            xml += QString("<action content=\"%1\" arguments=\"%2\" activationType=\"foreground\"/>").arg(it.value(), it.key());
        }
        xml += "</actions>";
    }
    xml += "</toast>";

    try {
        auto toastXml = winrt::Windows::Data::Xml::Dom::XmlDocument();
        toastXml.LoadXml(winrt::hstring(xml.toStdWString()));
        auto toast = winrt::Windows::UI::Notifications::ToastNotification(toastXml);
        // Use TypedEventHandler for event handlers
        using ActivatedHandler = winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::UI::Notifications::ToastNotification, winrt::Windows::Foundation::IInspectable>;
        using DismissedHandler = winrt::Windows::Foundation::TypedEventHandler<winrt::Windows::UI::Notifications::ToastNotification, winrt::Windows::UI::Notifications::ToastDismissedEventArgs>;
        toast.Activated(ActivatedHandler{this, &QPlatformNotificationEngineWindows::onToastActivated});
        toast.Dismissed(DismissedHandler{this, &QPlatformNotificationEngineWindows::onToastDismissed});
        // Store notificationId for use in handlers
        m_lastNotificationId = notificationId;
        auto notifier = winrt::Windows::UI::Notifications::ToastNotificationManager::CreateToastNotifier(winrt::hstring(m_appUserModelID.toStdWString()));
        notifier.Show(toast);
        return true;
    } catch (const winrt::hresult_error &e) {
        qWarning() << "WinRT error:" << QString::fromWCharArray(e.message().c_str());
        return false;
    }
}

void QPlatformNotificationEngineWindows::ensureComInitialized()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (hr != S_OK && hr != S_FALSE && hr != RPC_E_CHANGED_MODE) {
        qWarning() << "CoInitializeEx failed";
    }
}

void QPlatformNotificationEngineWindows::setAppUserModelID()
{
    HRESULT hr = S_OK;
    typedef HRESULT (WINAPI *SetCurrentProcessExplicitAppUserModelID_t)(PCWSTR);
    HMODULE shell32 = LoadLibraryW(L"shell32.dll");
    if (shell32) {
        auto setID = (SetCurrentProcessExplicitAppUserModelID_t)GetProcAddress(shell32, "SetCurrentProcessExplicitAppUserModelID");
        if (setID) {
            hr = setID(reinterpret_cast<const wchar_t *>(m_appUserModelID.utf16()));
        }
        FreeLibrary(shell32);
    }
    if (FAILED(hr)) {
        qWarning() << "Failed to set AppUserModelID";
    }
}

QPlatformNotificationEngine *qt_create_notification_engine_windows()
{
    static QPlatformNotificationEngineWindows engine;
    return &engine;
}

// Add these private methods to the class implementation
void QPlatformNotificationEngineWindows::onToastActivated(winrt::Windows::UI::Notifications::ToastNotification const& sender, winrt::Windows::Foundation::IInspectable const& args)
{
    QString actionKey = "default";
    auto activatedArgs = args.try_as<winrt::Windows::UI::Notifications::ToastActivatedEventArgs>();
    if (activatedArgs) {
        actionKey = QString::fromWCharArray(activatedArgs.Arguments().c_str());
    }

    // If no specific action was invoked (empty arguments), emit notificationClicked
    if (actionKey.isEmpty() || actionKey == "default") {
        handleNotificationClicked(m_lastNotificationId);
    } else {
        handleActionInvoked(m_lastNotificationId, actionKey);
    }
}

void QPlatformNotificationEngineWindows::onToastDismissed(winrt::Windows::UI::Notifications::ToastNotification const& sender, winrt::Windows::UI::Notifications::ToastDismissedEventArgs const& args)
{
    handleNotificationClosed(m_lastNotificationId, 0);
}

QT_END_NAMESPACE
