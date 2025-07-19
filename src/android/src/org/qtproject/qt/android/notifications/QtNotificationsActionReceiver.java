package org.qtproject.qt.android.notifications;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class QtNotificationsActionReceiver extends BroadcastReceiver {
    private static final String TAG = "QtNotificationsActionReceiver";
    public static final String ACTION_NOTIFICATION = "org.qtproject.qt.android.notifications.ACTION_NOTIFICATION";
    public static final String ACTION_NOTIFICATION_CLOSED = "org.qtproject.qt.android.notifications.ACTION_NOTIFICATION_CLOSED";

    // Native callback functions
    native void notifyNotificationClosed(long id);
    native void notifyActionInvoked(long id, String actionKey);

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        int notificationId = intent.getIntExtra("notification_id", 0);

        Log.d(TAG, "Received action: " + action + " for notification: " + notificationId);

        if (ACTION_NOTIFICATION_CLOSED.equals(action)) {
            // Notification was dismissed
            notifyNotificationClosed(notificationId);
        } else if (ACTION_NOTIFICATION.equals(action)) {
            // Action button was clicked
            String actionKey = intent.getStringExtra("action_key");
            if (actionKey != null) {
                Log.d(TAG, "Action invoked: " + actionKey);
                notifyActionInvoked(notificationId, actionKey);
            }
        }
    }
}