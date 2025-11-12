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

    // Set the delegate first on the main thread
    dispatch_async(dispatch_get_main_queue(), ^{
        UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
        center.delegate = m_delegate;

        // Log bundle identifier for debugging
        NSBundle *bundle = [NSBundle mainBundle];
        NSString *bundleId = [bundle bundleIdentifier];
        NSLog(@"QtNotifications: Bundle identifier: %@", bundleId ? bundleId : @"(nil - app may not be properly bundled)");

        // Check current authorization status
        [center getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings *settings) {
            NSLog(@"QtNotifications: Current authorization status: %ld", (long)settings.authorizationStatus);

            // Only request authorization if not already determined
            if (settings.authorizationStatus == UNAuthorizationStatusNotDetermined) {
                NSLog(@"QtNotifications: Requesting notification authorization...");
                [center requestAuthorizationWithOptions:(UNAuthorizationOptionAlert | UNAuthorizationOptionSound | UNAuthorizationOptionBadge)
                                      completionHandler:^(BOOL granted, NSError * _Nullable error) {
                    if (granted) {
                        NSLog(@"QtNotifications: Notification permission granted.");
                    } else {
                        NSLog(@"QtNotifications: Notification permission denied.");
                    }
                    if (error) {
                        NSLog(@"QtNotifications: Error requesting notification permission: %@ (code: %ld, domain: %@)",
                              error.localizedDescription ?: @"(no description)",
                              (long)error.code,
                              error.domain);
                        if (error.code == 1) {
                            NSLog(@"QtNotifications: Error code 1 (UNErrorCodeNotificationsNotAllowed) - This usually means:");
                            NSLog(@"QtNotifications:   1. The app is not properly bundled (must be run as .app bundle, not command line)");
                            NSLog(@"QtNotifications:   2. The app is not properly signed");
                            NSLog(@"QtNotifications:   3. The Info.plist is missing or incorrect");
                        }
                    }
                }];
            } else if (settings.authorizationStatus == UNAuthorizationStatusAuthorized) {
                NSLog(@"QtNotifications: Notification permission already authorized.");
            } else {
                NSLog(@"QtNotifications: Notification permission denied or restricted. Status: %ld", (long)settings.authorizationStatus);
            }
        }];
    });
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
    Q_UNUSED(type)

    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];

    // Check authorization status - use a semaphore but only if not on main thread
    __block BOOL isAuthorized = NO;
    __block BOOL checkComplete = NO;
    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

    [center getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings *settings) {
        isAuthorized = (settings.authorizationStatus == UNAuthorizationStatusAuthorized);
        checkComplete = YES;
        dispatch_semaphore_signal(semaphore);
    }];

    // Wait for the authorization check (with timeout)
    // Use a short timeout to avoid blocking too long
    dispatch_time_t timeout = dispatch_time(DISPATCH_TIME_NOW, 0.5 * NSEC_PER_SEC);
    if (dispatch_semaphore_wait(semaphore, timeout) != 0) {
        NSLog(@"QtNotifications: Timeout waiting for notification authorization status check");
        // Continue anyway - the async check will complete eventually
    }

    if (!isAuthorized) {
        NSLog(@"QtNotifications: Cannot send notification - app does not have notification authorization (status check complete: %@)", checkComplete ? @"YES" : @"NO");
        NSLog(@"QtNotifications: Make sure the app is run as a proper .app bundle and has requested permissions");
        return false;
    }

    UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc] init];
    content.title = summary.toNSString();
    content.body = body.toNSString();

    // Add icon/image attachment if provided
    if (!icon.isEmpty()) {
        NSString *iconPath = icon.toNSString();
        NSURL *iconURL = [NSURL fileURLWithPath:iconPath];

        // Check if file exists
        if ([[NSFileManager defaultManager] fileExistsAtPath:iconPath]) {
            NSError *attachmentError = nil;
            UNNotificationAttachment *attachment = [UNNotificationAttachment attachmentWithIdentifier:@"notification-icon"
                                                                                                  URL:iconURL
                                                                                              options:nil
                                                                                                error:&attachmentError];
            if (attachment && !attachmentError) {
                content.attachments = @[attachment];
            } else {
                NSLog(@"QtNotifications: Failed to create notification attachment from icon: %@, error: %@", iconPath, attachmentError.localizedDescription);
            }
        } else {
            NSLog(@"QtNotifications: Icon file not found: %@", iconPath);
        }
    }

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
    [center setNotificationCategories:categories];
    content.categoryIdentifier = categoryId;
    UNTimeIntervalNotificationTrigger *trigger = [UNTimeIntervalNotificationTrigger triggerWithTimeInterval:1 repeats:NO];
    NSString *identifier = [[NSUUID UUID] UUIDString];
    UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier:identifier content:content trigger:trigger];
    [center addNotificationRequest:request withCompletionHandler:^(NSError * _Nullable error) {
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
