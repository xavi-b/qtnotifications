import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtNotifications

Window {
    id: window
    width: 600
    height: 700
    visible: true
    title: "Qt Notifications QML Example"

    Rectangle {
        anchors.fill: parent
        color: "salmon"
    }

    Notifications {
        id: notifications
    }

    FileDialog {
        id: iconFileDialog
        title: "Select Icon File"
        nameFilters: ["Image Files (*.png *.jpg *.jpeg *.bmp *.ico *.svg)", "All Files (*)"]
        onAccepted: {
            iconPathField.text = selectedFile.toString().replace("file://", "")
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Text {
            text: "Qt Notifications QML Example"
            font.pixelSize: 20
            font.bold: true
        }

        Column {
            Layout.fillWidth: true
            spacing: 15

            // Title input
            Row {
                width: parent.width
                spacing: 10
                Label {
                    text: "Title:"
                    width: 100
                    anchors.verticalCenter: parent.verticalCenter
                }
                TextField {
                    id: titleField
                    width: parent.width - 110
                    placeholderText: "Enter notification title..."
                }
            }

            // Message input
            Row {
                width: parent.width
                spacing: 10
                Label {
                    text: "Message:"
                    width: 100
                    anchors.verticalCenter: parent.verticalCenter
                }
                TextArea {
                    id: messageField
                    width: parent.width - 110
                    height: 80
                    placeholderText: "Enter notification message..."
                    wrapMode: TextArea.Wrap
                }
            }

        }

        Column {
            Layout.fillWidth: true
            spacing: 10

            Button {
                width: parent.width
                text: "Send Simple Notification"
                onClicked: {
                    if (notifications.isSupported()) {
                        var title = titleField.text || "QML Test"
                        var message = messageField.text || "This is a simple notification from QML!"
                        notifications.sendNotification(title, message)
                        logText.text += "✓ Sent simple notification\n"
                    } else {
                        logText.text += "⚠ Notifications not supported\n"
                    }
                }
            }

            Button {
                width: parent.width
                text: "Send Notification with Actions"
                onClicked: {
                    if (notifications.isSupported()) {
                        var title = titleField.text || "QML Test with Actions"
                        var message = messageField.text || "Choose an action below:"
                        var actions = {
                            "open": "Open",
                            "dismiss": "Dismiss",
                            "reply": "Reply"
                        }
                        notifications.sendNotification(title, message, {}, actions)
                        logText.text += "✓ Sent notification with actions\n"
                    } else {
                        logText.text += "⚠ Notifications not supported\n"
                    }
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
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
            Layout.fillWidth: true
            text: "Clear Log"
            onClicked: {
                logText.text = "Log cleared.\n"
            }
        }
    }

    Connections {
        target: notifications
        function onNotificationClosed(notificationId) {
            logText.text += "✓ Notification " + notificationId + " was closed!\n"
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
        function onNotificationClicked(notificationId) {
            logText.text += "👆 Notification " + notificationId + " was clicked!\n"
        }
    }

    Component.onCompleted: {
        if (!notifications.isSupported()) {
            logText.text += "⚠ Notifications are not supported on this platform\n"
        } else {
            logText.text += "✓ Notifications are supported on this platform\n"
        }
    }
}
