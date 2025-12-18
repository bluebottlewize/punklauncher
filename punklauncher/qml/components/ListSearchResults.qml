import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import Qt5Compat.GraphicalEffects

ListView {
	id: rootList
	clip: true

	focus: true

	property string currentQuery: ""

	onCurrentIndexChanged: positionViewAtIndex(currentIndex, ListView.Contain)

	Component.onCompleted: {
		// Searching for "" (empty string) returns ALL apps,
		// sorted by your UsageRanker (Most used first)
		search("")

		mouseBlockerStart.restart()
		// Optional: Select the first item (your most used app)
		currentIndex = 0
	}

	// Optional: Smooth scrolling speed (milliseconds)
	// highlightMoveDuration: 100
	Timer {
		id: mouseBlockerStart
		interval: 350
		repeat: false
		running: false
	}

	Timer {
		id: mouseBlocker
		interval: 200
	}

	// Call this function when keys are pressed (from Main.qml)
	function notifyKeyboardMovement() {
		mouseBlocker.restart()
	}

	function search(query) {
		appModel.search(query)
	}

	function launchCurrentItem() {
		if (count > 0 && currentIndex >= 0) {
			var item = appModel.get(currentIndex)
			requestLaunch(item.name, item.exec, currentQuery, item.type)
		}
	}

	signal requestLaunch(string name, string cmd, string queryText, string type)

	model: ListModel {
		id: appModel

		function search(query) {
			currentQuery = query // Save it
			var results = backend.search(query) // Ask C++ for sorted results

			appModel.clear()
			for (var i = 0; i < results.length; i++) {
				appModel.append(results[i])
			}

			if (count > 0)
				currentIndex = 0
		}
	}

	delegate: ItemDelegate {
		id: delegateItem
		width: ListView.view.width
		height: 50

		hoverEnabled: true
		onHoveredChanged: {

			if (hovered && !mouseBlocker.running
					&& !mouseBlockerStart.running) {
				rootList.currentIndex = index
			}
			// if (hovered)
			//     ListView.view.currentIndex = index
		}

		background: Image {
			source: delegateItem.ListView.isCurrentItem ? "qrc:/assets/list_item_active.png" : "qrc:/assets/list_item.png"
			anchors.fill: parent
			fillMode: Image.Stretch
			anchors.leftMargin: 4
			anchors.rightMargin: 8
			anchors.topMargin: 4
			anchors.bottomMargin: 4
		}

		// background: Image {
		//     radius: 5

		//     // We only check 'highlighted' because hover now forces highlight!
		//     color: delegateItem.ListView.isCurrentItem ? "#ff0055" : "transparent"

		//     border.color: delegateItem.ListView.isCurrentItem ? "#ff0055" : "transparent"
		//     border.width: 1

		//     // Behavior on color {
		//     //     ColorAnimation {
		//     //         duration: 50
		//     //     }
		//     // } // Fast transition
		// }
		RowLayout {
			anchors.fill: parent
			anchors.margins: 5
			anchors.leftMargin: 20

			Item {
				Layout.preferredWidth: 32
				Layout.preferredHeight: 32

				// A. The Fallback: A colored box with the first letter
				Rectangle {
					anchors.fill: parent
					color: "transparent" // Dark grey background
					// border.color: "#8341FF"

					// Only show this box if the real image Failed or is Null
					visible: realIcon.status !== Image.Ready

					Text {
						anchors.centerIn: parent
						text: model.name ? model.name.charAt(
											   0).toUpperCase() : "?"
						color: "#8341FF"
						font.bold: true
						font.pixelSize: 20
					}
				}

				// B. The Real Icon
				Image {
					id: realIcon
					anchors.fill: parent
					source: model.icon.startsWith(
								"/") ? "file://" + model.icon : "image://appicon/" + model.icon
					fillMode: Image.PreserveAspectFit

					// If it fails, we simply Hide the image, revealing the Box behind it
					onStatusChanged: {
						if (status === Image.Error) {
							visible = false
						}
					}
				}
			}

			Text {
				text: model.name
				font.family: "Monospace"
				color: "#998EFF"
				font.pixelSize: 20
				Layout.fillWidth: true
			}
		}

		onClicked: rootList.requestLaunch(model.name, model.exec,
										  rootList.currentQuery, model.type)
	}
}
