import QtQuick
import QtQuick.Controls
import QtNotifications

Window {
    id: window
    width: 500
    height: 500
    visible: true
    title: "Qt Notifications QML Example"

    Notifications {
        id: notifications
    }

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Text {
            text: "Qt Notifications QML Example"
            font.pixelSize: 18
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                text: "Send Simple Notification"
                onClicked: {
                    if (notifications.isSupported()) {
                        notifications.sendNotification("QML Test", "This is a simple notification from QML!")
                    } else {
                        logText.text += "Notifications not supported\n"
                    }
                }
            }

            Button {
                text: "Send Notification with Icon"
                onClicked: {
                    if (notifications.isSupported()) {
                        notifications.sendNotification("QML Test with Icon", "This notification includes an icon path", "/path/to/icon.png")
                    }
                }
            }
        }

        Row {
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                text: "Send Notification with Actions"
                onClicked: {
                    if (notifications.isSupported()) {
                        var actions = {
                            "open": "Open",
                            "dismiss": "Dismiss",
                            "reply": "Reply"
                        }
                        notifications.sendNotification("QML Test with Actions", "Choose an action below:", actions)
                    }
                }
            }

            Button {
                text: "Send Success Notification"
                onClicked: {
                    if (notifications.isSupported()) {
                        notifications.sendNotification("Success", "Operation completed successfully!", "", 3) // Success type
                    }
                }
            }
        }

        Text {
            text: "Notification Events:"
            font.pixelSize: 14
            font.bold: true
        }

        ScrollView {
            width: parent.width
            height: 200
            clip: true

            Text {
                id: logText
                text: "Waiting for notification events...\n"
                font.family: "monospace"
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                color: "black"
            }
        }

        Button {
            text: "Clear Log"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: {
                logText.text = "Log cleared.\n"
            }
        }
    }

    Connections {
        target: notifications
        function onNotificationClosed() {
            logText.text += "✓ Notification was closed!\n"
        }
        function onActionInvoked(notificationId, actionKey) {
            logText.text += "⚡ Action invoked for notification " + notificationId + " with key: " + actionKey + "\n"
            if (actionKey === "open") {
                logText.text += "  → Opening application...\n"
            } else if (actionKey === "dismiss") {
                logText.text += "  → Dismissing notification...\n"
            } else if (actionKey === "reply") {
                logText.text += "  → Opening reply dialog...\n"
            }
        }
    }

    Component.onCompleted: {
        if (!notifications.isSupported()) {
            logText.text += "⚠ Notifications are not supported on this platform\n"
        }
    }
}
