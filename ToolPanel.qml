import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore

Rectangle {
    id: root

    property string currentTool: "SELECT"
    // Bound by MainWindow to the currently-active document's V8Process instance.
    property var sketch: null

    signal toolSelected(string toolId)

    color: Theme.surface

    // ── Collapsible section state (persisted across restarts) ───────────────
    property bool editOpen: true
    property bool bondsOpen: true
    property bool ringsOpen: true
    property bool templatesOpen: true

    Settings {
        category: "toolPanel"
        property alias editOpen: root.editOpen
        property alias bondsOpen: root.bondsOpen
        property alias ringsOpen: root.ringsOpen
        property alias templatesOpen: root.templatesOpen
    }

    // Auto-expand the section owning a tool activated by other means (single-key
    // shortcut, periodic-table pick). Only ever opens sections — never collapses.
    onCurrentToolChanged: {
        if (currentTool === "SELECT" || currentTool === "SELECT_FRAGMENT" || currentTool === "SELECT_LASSO" ||
            currentTool === "HAND" || currentTool === "ERASE" || currentTool === "TEXT")
            editOpen = true
        else if (currentTool.indexOf("BOND_") === 0 || currentTool === "CHAIN")
            bondsOpen = true
        else if (currentTool.indexOf("TEMPLATE_") === 0 || currentTool === "LIB_Pyridine")
            ringsOpen = true
        else if (currentTool.indexOf("FG_") === 0 || currentTool.indexOf("SS_") === 0 || currentTool.indexOf("LIB_") === 0)
            templatesOpen = true
    }

    // ── Reusable pieces ──────────────────────────────────────────────────────
    component SectionHeader: Rectangle {
        id: header
        property string title
        property bool expanded: true
        signal toggled()
        Layout.fillWidth: true
        implicitHeight: 22
        radius: 3
        color: headerMouse.containsMouse ? Theme.hover : "transparent"
        Accessible.role: Accessible.Button
        Accessible.name: header.title
        Behavior on color { ColorAnimation { duration: 150 } }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 4
            spacing: 4
            Text {
                text: header.expanded ? "▾" : "▸"
                color: Theme.textSecondary
                font.pixelSize: 10
            }
            Text {
                text: header.title
                color: Theme.textSecondary
                font { pixelSize: Theme.fontSizeCaption; bold: true; letterSpacing: 1.5; family: Theme.fontDisplay }
            }
            Item { Layout.fillWidth: true }
        }
        MouseArea {
            id: headerMouse
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: header.toggled()
        }
    }

    // Scrollable so every tool stays reachable when the window is shorter than
    // the stacked tool sections (~700px of content).
    ScrollView {
        id: toolScroll
        anchors.fill: parent
        padding: Theme.marginSmall
        clip: true
        contentWidth: availableWidth
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        ColumnLayout {
        width: toolScroll.availableWidth
        spacing: Theme.spacingSmall

        // ── Edit section ─────────────────────────────────────────────────────
        SectionHeader {
            title: "EDIT"
            expanded: root.editOpen
            onToggled: root.editOpen = !root.editOpen
        }
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.editOpen ? editGrid.implicitHeight : 0
            Behavior on Layout.preferredHeight { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
            clip: true
            opacity: root.editOpen ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: 200 } }

            GridLayout {
                id: editGrid
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                columns: 3
            columnSpacing: Theme.toolGridGap
            rowSpacing: Theme.toolGridGap
            Repeater {
                model: [
                    { id: "SELECT", icon: "select.svg", tip: "Selection tool (S)" },
                    { id: "SELECT_FRAGMENT", icon: "select-fragment.svg", tip: "Fragment Selection tool" },
                    { id: "SELECT_LASSO", icon: "select-lasso.svg", tip: "Lasso select tool - freeform selection" },
                    { id: "HAND", icon: "hand.svg", tip: "Hand tool - drag to pan canvas (H)" },
                    { id: "ERASE", icon: "erase.svg", tip: "Erase tool (E)" },
                    { id: "TEXT", icon: "text.svg", tip: "Text annotation (click canvas)" },
                    { id: "IMAGE", icon: "add-image.svg", tip: "Insert image" }
                ]
                delegate: IconCell {
                    required property var modelData
                    Layout.alignment: Qt.AlignHCenter
                    iconSource: modelData.icon.indexOf(".svg") !== -1 ? "icons/" + modelData.icon : ""
                    glyph: modelData.icon.indexOf(".svg") === -1 ? modelData.icon : ""
                    tip: modelData.tip
                    selected: root.currentTool === modelData.id
                    onClicked: root.toolSelected(modelData.id)
                }
            }
        }
        }

        // ── Bonds section ────────────────────────────────────────────────────
        SectionHeader {
            title: "BONDS"
            expanded: root.bondsOpen
            onToggled: root.bondsOpen = !root.bondsOpen
        }
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.bondsOpen ? bondsGrid.implicitHeight : 0
            Behavior on Layout.preferredHeight { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
            clip: true
            opacity: root.bondsOpen ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: 200 } }

            GridLayout {
                id: bondsGrid
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                columns: 3
            columnSpacing: Theme.toolGridGap
            rowSpacing: Theme.toolGridGap
            Repeater {
                model: [
                    { id: "BOND_1", icon: "single_bond.svg", tip: "Single bond (B)" },
                    { id: "BOND_2", icon: "double_bond.svg", tip: "Double bond" },
                    { id: "BOND_3", icon: "triple_bond.svg", tip: "Triple bond" },
                    { id: "BOND_UP", icon: "up_bond.svg", tip: "Wedge bond" },
                    { id: "BOND_DOWN", icon: "down_bond.svg", tip: "Dash bond" },
                    { id: "BOND_UPDOWN", icon: "updown_bond.svg", tip: "Wavy bond" },
                    { id: "CHAIN", icon: "chain.svg", tip: "Chain (drag to lay a carbon chain)" }
                ]
                delegate: IconCell {
                    required property var modelData
                    Layout.alignment: Qt.AlignHCenter
                    iconSource: modelData.icon.indexOf(".svg") !== -1 ? "icons/" + modelData.icon : ""
                    glyph: modelData.icon.indexOf(".svg") === -1 ? modelData.icon : ""
                    tip: modelData.tip
                    selected: root.currentTool === modelData.id
                    onClicked: root.toolSelected(modelData.id)
                }
            }
        }
        }

        // ── Rings section ────────────────────────────────────────────────────
        SectionHeader {
            title: "RINGS"
            expanded: root.ringsOpen
            onToggled: root.ringsOpen = !root.ringsOpen
        }
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.ringsOpen ? ringsGrid.implicitHeight : 0
            Behavior on Layout.preferredHeight { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
            clip: true
            opacity: root.ringsOpen ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: 200 } }

            GridLayout {
                id: ringsGrid
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                columns: 3
            columnSpacing: Theme.toolGridGap
            rowSpacing: Theme.toolGridGap
            Repeater {
                model: [
                    // template-N files are in Ketcher's palette order, not ring size:
                    // 0=benzene, 2=hexagon, 3=pentagon, 5=square, 4=triangle
                    { id: "TEMPLATE_BENZENE", icon: "template-0.svg", tip: "Benzene (R)" },
                    { id: "LIB_Pyridine", icon: "Pyr", tip: "Pyridine" },
                    { id: "TEMPLATE_6", icon: "template-2.svg", tip: "Cyclohexane" },
                    { id: "TEMPLATE_5", icon: "template-3.svg", tip: "Cyclopentane" },
                    { id: "TEMPLATE_4", icon: "template-5.svg", tip: "Cyclobutane" },
                    { id: "TEMPLATE_3", icon: "template-4.svg", tip: "Cyclopropane" },
                    { id: "TEMPLATE_7", icon: "template-6.svg", tip: "Cycloheptane" },
                    { id: "TEMPLATE_8", icon: "template-7.svg", tip: "Cyclooctane" }
                ]
                delegate: IconCell {
                    required property var modelData
                    Layout.alignment: Qt.AlignHCenter
                    iconSource: modelData.icon.indexOf(".svg") !== -1 ? "icons/" + modelData.icon : ""
                    glyph: modelData.icon.indexOf(".svg") === -1 ? modelData.icon : ""
                    tip: modelData.tip
                    selected: root.currentTool === modelData.id
                    onClicked: root.toolSelected(modelData.id)
                }
            }
        }
        }

        // ── Templates section (FG / Salts / Library launchers) ───────────────
        SectionHeader {
            title: "TEMPLATES"
            expanded: root.templatesOpen
            onToggled: root.templatesOpen = !root.templatesOpen
        }
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.templatesOpen ? templatesGrid.implicitHeight : 0
            Behavior on Layout.preferredHeight { NumberAnimation { duration: 200; easing.type: Easing.InOutQuad } }
            clip: true
            opacity: root.templatesOpen ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: 200 } }

            GridLayout {
                id: templatesGrid
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                columns: 3
            columnSpacing: Theme.toolGridGap
            rowSpacing: Theme.toolGridGap

        // Functional Groups Tool
        IconCell {
            Layout.alignment: Qt.AlignHCenter
            iconSource: "icons/generic-groups.svg"
            tip: "Functional Groups"
            selected: root.currentTool.startsWith("FG_")
            onClicked: {
                fgList.model = []
                fgPopup.open()
                sketch.requestFunctionalGroupsList()
            }
        }

        // Salts & Solvents Tool
        IconCell {
            Layout.alignment: Qt.AlignHCenter
            glyph: "S&S"
            tip: "Salts & Solvents"
            selected: root.currentTool.startsWith("SS_")
            onClicked: {
                saltsList.model = []
                ssPopup.open()
                sketch.requestSaltsAndSolventsList()
            }
        }

        // Template Library Tool
        IconCell {
            Layout.alignment: Qt.AlignHCenter
            iconSource: "icons/template-lib.svg"
            tip: "Template Library"
            selected: root.currentTool.startsWith("LIB_")
            onClicked: {
                root._libraryAllItems = []
                libGroupCombo.currentIndex = 0
                libSearchField.text = ""
                libItemList.model = []
                libPopup.open()
                sketch.requestTemplateLibraryList()
            }
        }
        }
        }

        Item { Layout.fillHeight: true }  // Spacer
        }
    }

    property var _saltsListData: []
    property var _libraryAllItems: []
    property var _libraryGroups: []
    property var _thumbnails: ({})

    Connections {
        target: root.sketch
        function onStructureReady(reqId, data) {
            if (reqId === "salts") {
                try {
                    var items = JSON.parse(data)
                    var labels = []
                    for (var i = 0; i < items.length; ++i) labels.push(items[i].label)
                    saltsList.model = labels
                    root._thumbnails = ({})
                    for (var s = 0; s < labels.length; ++s) {
                        sketch.requestTemplateThumbnail(labels[s], "thumb_ss_" + labels[s])
                    }
                } catch (e) {}
            } else if (reqId === "fg_list") {
                try {
                    var fgItems = JSON.parse(data)
                    var fgLabels = []
                    for (var j = 0; j < fgItems.length; ++j) fgLabels.push(fgItems[j].label)
                    fgList.model = fgLabels
                    // Request thumbnails for each FG
                    root._thumbnails = ({})
                    for (var t = 0; t < fgLabels.length; ++t) {
                        sketch.requestTemplateThumbnail(fgLabels[t], "thumb_fg_" + fgLabels[t])
                    }
                } catch (e) {}
            } else if (reqId.indexOf("thumb_fg_") === 0) {
                try {
                    var name = reqId.substring(9)
                    var thumb = JSON.parse(data)
                    var updated = Object.assign({}, root._thumbnails)
                    updated[name] = thumb
                    root._thumbnails = updated
                } catch (e) {}
            } else if (reqId.indexOf("thumb_ss_") === 0) {
                try {
                    var name = reqId.substring(9)
                    var thumb = JSON.parse(data)
                    var updated = Object.assign({}, root._thumbnails)
                    updated[name] = thumb
                    root._thumbnails = updated
                } catch (e) {}
            } else if (reqId.indexOf("thumb_lib_") === 0) {
                try {
                    var name = reqId.substring(10)
                    var thumb = JSON.parse(data)
                    var updated = Object.assign({}, root._thumbnails)
                    updated[name] = thumb
                    root._thumbnails = updated
                } catch (e) {}
            } else if (reqId === "library_list") {
                try {
                    var libItems = JSON.parse(data)
                    root._libraryAllItems = libItems
                    // Collect unique groups preserving order
                    var seen = {}
                    var groups = ["All"]
                    for (var k = 0; k < libItems.length; ++k) {
                        var g = libItems[k].group || "Other"
                        if (!seen[g]) { seen[g] = true; groups.push(g) }
                    }
                    root._libraryGroups = groups
                    libGroupCombo.model = groups
                    libGroupCombo.currentIndex = 0
                    root._applyLibraryFilter()
                    root._thumbnails = ({})
                    for (var l = 0; l < libItems.length; ++l) {
                        sketch.requestTemplateThumbnail(libItems[l].label, "thumb_lib_" + libItems[l].label)
                    }
                } catch (e) {}
            }
        }
    }

    function _applyLibraryFilter() {
        var group = libGroupCombo.currentText
        var search = libSearchField.text.trim().toLowerCase()
        var all = root._libraryAllItems
        var filtered = []
        for (var i = 0; i < all.length; ++i) {
            if (group !== "All" && all[i].group !== group) continue
            if (search !== "" && all[i].label.toLowerCase().indexOf(search) === -1) continue
            filtered.push(all[i].label)
        }
        libItemList.model = filtered
    }

    AnchoredPicker {
        id: ssPopup
        width: 240
        height: 320
        title: "Salts & Solvents"
        loading: saltsList.count === 0

        GridView {
            id: saltsList
            anchors.fill: parent
            clip: true
            cellWidth: 110
            cellHeight: 48
            model: []
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            delegate: ThumbnailGridItem {
                required property string modelData
                required property int index
                label: modelData
                thumbData: root._thumbnails[modelData] || null
                onClicked: {
                    root.toolSelected("SS_" + modelData)
                    ssPopup.close()
                }
            }
        }
    }

    AnchoredPicker {
        id: fgPopup
        width: 240
        height: 320
        title: "Functional Groups"
        loading: fgList.count === 0

        GridView {
            id: fgList
            anchors.fill: parent
            clip: true
            cellWidth: 110
            cellHeight: 48
            model: []
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            delegate: ThumbnailGridItem {
                required property string modelData
                required property int index
                label: modelData
                thumbData: root._thumbnails[modelData] || null
                onClicked: {
                    root.toolSelected("FG_" + modelData)
                    fgPopup.close()
                }
            }
        }
    }

    // Template Library popup — group filter + item list
    AnchoredPicker {
        id: libPopup
        width: 280
        height: 400
        title: "Template Library"
        loading: libItemList.count === 0

        ColumnLayout {
            anchors.fill: parent
            spacing: 4

            TextField {
                id: libSearchField
                Layout.fillWidth: true
                placeholderText: "Search templates…"
                font.pixelSize: Theme.fontSizeLabel
                onTextChanged: root._applyLibraryFilter()
            }

            ComboBox {
                id: libGroupCombo
                Layout.fillWidth: true
                model: ["All"]
                font.pixelSize: Theme.fontSizeLabel
                onCurrentTextChanged: root._applyLibraryFilter()
            }

            GridView {
                id: libItemList
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                cellWidth: 110
                cellHeight: 48
                model: []
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                delegate: ThumbnailGridItem {
                    required property string modelData
                    required property int index
                    label: modelData
                    thumbData: root._thumbnails[modelData] || null
                    onClicked: {
                        root.toolSelected("LIB_" + modelData)
                        libPopup.close()
                    }
                }
            }
        }
    }

}
