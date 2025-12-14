import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts

import "components"

Window {
	id: root
	width: 800
	height: 600
	visible: false // Keep false so C++ handles the show()
	color: "transparent"
	flags: Qt.FramelessWindowHint

	Shortcut {
		sequence: "Escape"
		onActivated: exitSequence.start()
	}

	Rectangle {
		id: dimOverlay
		anchors.fill: parent
		color: "#000315"
		opacity: 0 // Start invisible

		// Click outside to close
		MouseArea {
			anchors.fill: parent
			onClicked: exitSequence.start()
		}
	}

	Item {
		id: launcherContainer
		width: 800 // Your desired total width
		height: 600 // Your desired total height

		// Position it where you want on the screen
		// anchors.top: parent.top
		// anchors.topMargin: 100 // Move down 100px (replaces C++ margins)
		anchors.horizontalCenter: parent.horizontalCenter // Optional: Center it?
		anchors.verticalCenter: parent.verticalCenter

		Rectangle {
			id: leftStrip
			width: 34
			height: 0
			color: "transparent"
			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter

			Image {
				source: "qrc:/assets/launcher_left_strip.png"
				anchors.fill: parent
				fillMode: Image.Stretch
				visible: status === Image.Ready
			}
		}

		Rectangle {
			id: contentArea
			x: 37
			y: 0
			height: parent.height
			width: 0
			clip: true
			color: "transparent"

			// This forces the GPU to render this entire box as a texture during animation.
			// It prevents the "stutter" caused by the layout engine waking up at the end.
			layer.enabled: true
			layer.smooth: true

			Image {
				source: "qrc:/assets/launcher_bg.png"
				anchors.fill: parent
				fillMode: Image.Stretch
				opacity: 0.9
			}

			// Layout Container
			ColumnLayout {
				// Don't use anchors.fill: parent. It causes circular dependency lag.
				// Explicitly bind dimensions.
				x: 0
				y: 0
				width: 763 // Fixed width of the final content
				height: parent.height

				anchors.margins: 0
				spacing: 0

				TextField {
					id: searchInput
					Layout.fillWidth: true
					leftPadding: 20
					Layout.margins: 20
					Layout.leftMargin: 14
					Layout.bottomMargin: 0
					// placeholderText: "Search applications..."
					placeholderTextColor: "#8341FF"
					font.family: "Monospace"

					font.pixelSize: 30
					color: "#8341FF"

					cursorDelegate: Item {
						id: cursorWrapper

						// 1. Define the actual cursor image inside the wrapper
						Image {
							id: cursor
							source: "qrc:/assets/launcher_text_box_cursor.png"
							width: 26

							// Anchor appropriately (e.g., center or bottom of the line)
							anchors.centerIn: parent

							// Start invisible so the animation controls the first appearance
							visible: false

							SequentialAnimation {
								id: cursorAnim
								// 2. Run whenever the box has focus (ignore system blink timer)
								running: searchInput.activeFocus
								loops: Animation.Infinite

								// Immediately make visible (Start of cycle)
								PropertyAction {
									target: cursor
									property: 'visible'
									value: true
								}

								PauseAnimation {
									duration: 600
								}

								PropertyAction {
									target: cursor
									property: 'visible'
									value: false
								}

								PauseAnimation {
									duration: 600
								}

								// Ensure it hides if animation stops (focus lost)
								onStopped: cursor.visible = false
							}
						}

						// 3. The Magic: Reset animation on any activity
						Connections {
							target: searchInput

							// Fires on typing OR arrow keys
							function onCursorPositionChanged() {
								// restart() resets the animation to the beginning
								// The beginning is "visible = true", so the cursor appears instantly.
								cursorAnim.restart()
							}
						}
					}

					background: Image {
						id: name
						source: "qrc:/assets/launcher_text_box.png"
					}
					onTextChanged: appView.search(text)

					onAccepted: {
						appView.launchCurrentItem()
					}

					Keys.onPressed: event => {
										if (event.key === Qt.Key_Down) {
											appView.notifyKeyboardMovement(
												) // <--- ADD THIS
											appView.incrementCurrentIndex()
											event.accepted = true
										} else if (event.key === Qt.Key_Up) {
											appView.notifyKeyboardMovement(
												) // <--- ADD THIS
											appView.decrementCurrentIndex()
											event.accepted = true
										}
									}

					Keys.onReleased: event => {
										 if (event.key === Qt.Key_Down) {
											 appView.notifyKeyboardMovement(
												 ) // <--- ADD THIS
											 // appView.incrementCurrentIndex()
											 event.accepted = true
										 } else if (event.key === Qt.Key_Up) {
											 appView.notifyKeyboardMovement(
												 ) // <--- ADD THIS
											 // appView.decrementCurrentIndex()
											 event.accepted = true
										 }
									 }
				}
				ListSearchResults {
					id: appView
					Layout.fillWidth: true
					Layout.fillHeight: true

					Layout.margins: 10

					onRequestLaunch: (name, cmd, queryText) => {
										 backend.launchApp(name, cmd, queryText)
										 exitSequence.start()
									 }
				}
			}
		}

		Timer {
			id: focusTimer
			interval: 10
			repeat: true
			running: root.visible // Run only when window is shown
			onTriggered: {
				if (searchInput.activeFocus) {
					// We got focus! Stop trying.
					running = false
				} else {
					// Try to grab focus again
					searchInput.forceActiveFocus()
				}
			}
		}

		ScriptAction {
			script: searchInput.forceActiveFocus()
		}

		ParallelAnimation {
			id: startSequence
			running: true

			NumberAnimation {
				target: dimOverlay
				property: "opacity"
				from: 0
				to: 0.6 // 60% Dark
				duration: 400
				easing.type: Easing.OutExpo
			}

			SequentialAnimation {

				NumberAnimation {
					target: leftStrip
					property: "height"
					from: 0
					to: 600
					duration: 125
					easing.type: Easing.Linear
				}

				NumberAnimation {
					target: contentArea
					property: "width"
					from: 1
					to: 763
					duration: 150
					easing.type: Easing.OutQuad
				}
			}

			// 4. Turn off layering after animation to save memory (Optional)
			ScriptAction {
				script: contentArea.layer.enabled = false
			}
		}

		// --- EXIT ANIMATION ---
		SequentialAnimation {
			id: exitSequence
			running: false // Do not run automatically

			// 1. Force the layer on again for smooth resizing
			ScriptAction {
				script: contentArea.layer.enabled = true
			}

			// 2. Shrink everything in parallel
			// Shrink Content
			NumberAnimation {
				target: contentArea
				property: "width"
				from: 763
				to: 0
				duration: 150 // Slightly faster than opening
				easing.type: Easing.OutQuad // Decelerate (Start fast, end slow)
			}

			// Shrink Strip
			NumberAnimation {
				target: leftStrip
				property: "height"
				from: 600
				to: 0
				duration: 125
				easing.type: Easing.Linear
			}

			// Fade out Background
			NumberAnimation {
				target: dimOverlay
				property: "opacity"
				from: 0.6
				to: 0
				duration: 200
			}

			// 3. THE END: Kill the C++ Process
			ScriptAction {
				script: backend.close()
			}
		}
	}
}
