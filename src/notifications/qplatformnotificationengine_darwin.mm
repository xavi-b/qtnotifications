#include "qplatformnotificationengine_darwin.h"
#include <QtCore/qglobal.h>
#import <Foundation/Foundation.h>
#import <UserNotifications/UserNotifications.h>

@interface DarwinNotificationDelegate : NSObject <UNUserNotificationCenterDelegate>
@property (nonatomic, assign) QPlatformNotificationEngineDarwin *engine;
@end

@implementation DarwinNotificationDelegate
- (void)userNotificationCenter:(UNUserNotificationCenter *)center
       didReceiveNotificationResponse:(UNNotificationResponse *)response
                withCompletionHandler:(void (^)(void))completionHandler {
    NSString *actionIdentifier = response.actionIdentifier;
    if (![actionIdentifier isEqualToString:UNNotificationDefaultActionIdentifier] &&
        ![actionIdentifier isEqualToString:UNNotificationDismissActionIdentifier]) {
        QString actionKey = QString::fromNSString(actionIdentifier);
        self.engine->handleActionInvoked(1, actionKey);
        self.engine->handleNotificationClosed(1, QNotifications::Closed);
    } else if ([actionIdentifier isEqualToString:UNNotificationDefaultActionIdentifier]) {
        // Handle notification clicked (user tapped on notification body)
        self.engine->handleNotificationClicked(1);
        self.engine->handleNotificationClosed(1, QNotifications::Closed);
    } else if ([actionIdentifier isEqualToString:UNNotificationDismissActionIdentifier]) {
        // Handle notification dismissed
        self.engine->handleNotificationClosed(1, QNotifications::Dismissed);
    }
    completionHandler();
}

- (void)userNotificationCenter:(UNUserNotificationCenter *)center
       willPresentNotification:(UNNotification *)notification
         withCompletionHandler:(void (^)(UNNotificationPresentationOptions options))completionHandler {
    completionHandler(UNNotificationPresentationOptionList | UNNotificationPresentationOptionBanner | UNNotificationPresentationOptionSound);
}
@end

QT_BEGIN_NAMESPACE

QPlatformNotificationEngineDarwin::QPlatformNotificationEngineDarwin(QObject *parent)
  : QPlatformNotificationEngine(parent)
{
    m_delegate = [[DarwinNotificationDelegate alloc] init];
    m_delegate.engine = this;
    // Request notification permissions before setting the delegate
    [[UNUserNotificationCenter currentNotificationCenter] requestAuthorizationWithOptions:(UNAuthorizationOptionAlert | UNAuthorizationOptionSound | UNAuthorizationOptionBadge)
        completionHandler:^(BOOL granted, NSError * _Nullable error) {
            if (granted) {
                NSLog(@"Notification permission granted.");            // Set the delegate on the main thread after requesting permission
                dispatch_async(dispatch_get_main_queue(), ^{
                    [UNUserNotificationCenter currentNotificationCenter].delegate = m_delegate;
                });
            } else {
                NSLog(@"Notification permission denied.");
            }
            if (error) {
                NSLog(@"Error requesting notification permission: %@", error);
            }
        }];
}

QPlatformNotificationEngineDarwin::~QPlatformNotificationEngineDarwin()
{
    if (m_delegate) {
        [m_delegate release];
    }
}

void QPlatformNotificationEngineDarwin::handleActionInvoked(uint notificationId, const QString &actionKey)
{
    emit actionInvoked(notificationId, actionKey);
}

void QPlatformNotificationEngineDarwin::handleNotificationClosed(uint notificationId, QNotifications::ClosedReason reason)
{
    emit notificationClosed(notificationId, reason);
}

void QPlatformNotificationEngineDarwin::handleNotificationClicked(uint notificationId)
{
    emit notificationClicked(notificationId);
}

bool QPlatformNotificationEngineDarwin::isSupported() const
{
    return true;
}

bool QPlatformNotificationEngineDarwin::sendNotification(const QString &summary, const QString &body, const QString &icon, const QMap<QString, QString> &actions, QNotifications::NotificationType type)
{
    Q_UNUSED(icon)
    Q_UNUSED(type)
    UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
    content.title = summary.toNSString();
    content.body = body.toNSString();
    NSMutableArray *actionArray = [NSMutableArray array];
    for (auto it = actions.constBegin(); it != actions.constEnd(); ++it) {
        NSString *actionKey = it.key().toNSString();
        NSString *actionLabel = it.value().toNSString();
        UNNotificationAction *action = [UNNotificationAction actionWithIdentifier:actionKey title:actionLabel options:UNNotificationActionOptionForeground];
        [actionArray addObject:action];
    }
    NSString *categoryId = @"qt_notification_category";
    UNNotificationCategory *category = [UNNotificationCategory categoryWithIdentifier:categoryId actions:actionArray intentIdentifiers:@[] options:UNNotificationCategoryOptionCustomDismissAction];
    NSSet *categories = [NSSet setWithObject:category];
    [[UNUserNotificationCenter currentNotificationCenter] setNotificationCategories:categories];
    content.categoryIdentifier = categoryId;
    UNTimeIntervalNotificationTrigger *trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:1 repeats:NO];
    NSString *identifier = [[NSUUID UUID] UUIDString];
    UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier:identifier content:content trigger:trigger];
    [[UNUserNotificationCenter currentNotificationCenter] addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
        if (error) {
            NSLog(@"Failed to schedule notification: %@ (code: %ld, description: %@)", error, (long)error.code, error.localizedDescription);
        }
    }];
    return true;
}

QPlatformNotificationEngine *qt_create_notification_engine_darwin()
{
    static QPlatformNotificationEngineDarwin engine;
    return &engine;
}

QT_END_NAMESPACE
