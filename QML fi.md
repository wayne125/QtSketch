After reading every relevant QML file raw — `MainWindow.qml` (1912 lines), `ToolPanel.qml`, `PropertyPanel.qml`, `Theme.qml`, `IconCell.qml`, `StyleSheets.qml`, `AnchoredPicker.qml`, `ToolOverlay.qml`, and the full icons directory — here is a concrete, code-level visual modernization plan.

***

## The Core Visual Problem

The current layout stacks **five horizontal chrome bands** before you even reach the canvas: [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/MainWindow.qml)

```
[Custom title bar — 32px]
[Toolbar row 1: File · Edit · Structure ops]
[Toolbar row 2: Reaction · Query · Style · Align · Page]
[Tab bar]
[ToolPanel LEFT | Canvas + rulers | PropertyPanel RIGHT]
[Atom quick-bar]
[Status bar]
```

This means roughly **130–160 px of vertical chrome** is consumed before a single molecule appears. Combined with a 136 px left panel and 240 px right panel always open, the canvas is boxed in from all four sides. Professional editors like Maestro, ChemDraw, and MarvinSketch all solve this the same way: **a single compact toolbar, a collapsible tool rail, and a context-driven inspector**. [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/Theme.qml)

***

## 1. Consolidate the Two-Toolbar Rows into One

**Current state:** Two `RowLayout` rows inside a `ColumnLayout` — Row 1 has File/Edit/Structure icons, Row 2 has Reaction/Query dropdowns, style selectors, align/distribute, page settings. [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/MainWindow.qml)

**Problem:** Row 2 tools are low-frequency (alignment, page size, chiral settings) but they permanently steal 36–40 px of vertical height.

**Fix — Single toolbar with right-anchored overflow:**

```
[≡ App menu] | [New · Open · Save] | [sep] | [Undo · Redo] | [sep] | [Clean 2D · Fit · Layout Selected] | ←→ spacer | [Align group — hidden until multi-select] | [Style picker] | [Dark/Light toggle]
```

- Height: **36 px** total (down from ~72 px)
- Align/distribute buttons: bind `visible` to `multiSelectActive` — they appear only when ≥2 atoms are selected [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/PropertyPanel.qml)
- Style preset: replace the status-bar toggle with a compact `SegmentedControl` ("Def · ACS96 · ACS09 · Nat · Lg") tucked into the right side of the single toolbar [github](https://github.com/wayne125/QtSketch/blob/test/StyleSheets.qml)
- Reaction/query controls: move into a **right-click context menu** on reaction arrows, or a dedicated Reaction menu item — they are not drawing-loop tools

***

## 2. ToolPanel — Vertical Icon Rail with Flyout Groups

**Current state:** 136 px wide, always visible, 4 collapsible `SectionHeader` sections (Edit / Bonds / Rings / Templates), `GridLayout columns: 3` per section, `toolCellSize: 36`, `toolGridGap: 2`. [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/ToolPanel.qml)

**Visual problem:** The 3-column grid with section headers makes the panel look like a settings accordion, not a tool rail. Section titles ("EDIT", "BONDS", "RINGS", "TEMPLATES") add 24 px each but communicate what is already obvious from the icons. The 4 expanded sections simultaneously means the panel is always tall and requires scrolling.

### Proposed visual layout — Flat single-column rail with group dividers

Replace the 3-column grid + section headers with a **single-column vertical rail** of 40 px cells, separated by 1 px divider lines. Total width drops from 136 px → **52 px collapsed / 160 px pinned-expanded**.

```
┌────────┐
│ [PIN]  │  ← thumbtack, 28px, top
├────────┤
│ ╌╌╌╌ selection group ╌╌╌╌
│ [▣  ] SELECT           ← active tool: left accent bar 3px
│ [⬡  ] SELECT_FRAGMENT
│ [◌  ] SELECT_LASSO
├─ 1px ─┤
│ ╌╌╌╌ bond group ╌╌╌╌
│ [─  ] BOND_1
│ [═  ] BOND_2
│ [≡  ] BOND_3
│ [↗  ] BOND_UP
│ [↘  ] BOND_DOWN
│ [↕  ] BOND_UPDOWN
│ [∿  ] CHAIN
├─ 1px ─┤
│ ╌╌╌╌ ring group ╌╌╌╌
│ [⬡  ] BENZENE
│ [⬡  ] PYRIDINE
│ [⬠  ] RING_6 … etc via flyout ▶
├─ 1px ─┤
│ ╌╌╌╌ utility group ╌╌╌╌
│ [T  ] TEXT
│ [🖼] IMAGE
│ [✋] HAND
│ [✕  ] ERASE
├─ 1px ─┤
│ [FG ] Func Groups  ← open AnchoredPicker
│ [S&S] Salts/Solv
│ [📚] Template Lib
└────────┘
```

The QML change is minimal — remove `SectionHeader` components and replace each `GridLayout` with a `ColumnLayout` plus `Rectangle { height: 1; color: Theme.outline; opacity: 0.5 }` dividers: [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/ToolPanel.qml)

```qml
// Replace SectionHeader + GridLayout block with:
ColumnLayout {
    spacing: 0
    Repeater {
        model: selectionTools   // ["SELECT","SELECT_FRAGMENT","SELECT_LASSO"]
        delegate: IconCell {
            toolId: modelData
            cellWidth: 52; cellHeight: 40
            // Active-tool left accent bar (see §3)
        }
    }
}
Rectangle { Layout.fillWidth: true; height: 1; color: Theme.outline; opacity: 0.45 }
// ... repeat for each group
```

### Ring flyout instead of 8 ring icons
Showing all 8 ring variants (3–8 membered + benzene + pyridine) in the panel is wasteful. The last-used ring should be the visible panel button; clicking-and-holding or right-clicking opens a small **2×4 flyout grid** of all variants — identical to how ChemDraw and Ketcher handle this. This saves 5–6 cells of vertical space.

***

## 3. IconCell — Add Left Accent Bar for Active Tool

**Current state:** Active tool gets `background.color: Theme.selected` (#1B3A52 dark) — a filled background rectangle. This is subtle and blends into the panel surface. [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/components/IconCell.qml)

**Fix — Left edge accent bar (3px):**

```qml
// Inside IconCell.qml background Rectangle — add:
Rectangle {
    id: activeBar
    visible: root.selected
    width: 3
    height: parent.height * 0.65
    anchors { left: parent.left; verticalCenter: parent.verticalCenter }
    radius: 1.5
    color: Theme.accent    // #5DADE2 — Prussian blue

    Behavior on opacity { NumberAnimation { duration: 120 } }
    opacity: root.selected ? 1.0 : 0.0
}
```

This is the visual language used by VS Code, Figma, and every professional tool sidebar. The filled background (`Theme.selected`) becomes lighter or transparent; the bar carries the identity of the active state. Much more legible at a glance.

Also fix `restOpacity: 0.6` → **`0.78`**. At 60% opacity every icon in the panel looks half-disabled. 78% preserves the "calm at rest" feel without making icons look greyed out. [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/Theme.qml)

***

## 4. Bottom Atom Quick-Bar → Inline in Canvas Context Bar

**Current state:** A Ketcher-style bottom `RowLayout` with atom buttons (C, N, O, S, F, Cl) + charge modifiers always visible at the bottom of the window. [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/MainWindow.qml)

**Problem:** These buttons are redundant with the periodic table popup and the atom label field in `PropertyPanel`. They consume 36 px of permanent bottom chrome and look visually disconnected from the canvas.

**Fix — Floating context bar above canvas bottom-left:**

Move the atom quick-bar to a floating `Rectangle` that appears **only when the bond or select tool is active and nothing is selected** — i.e., when placing a new atom is the most likely next action. Position it as a pill/toolbar floating 12 px above the canvas bottom edge, left-aligned.

```qml
// In MainWindow.qml — replace the bottom bar Rectangle with:
Rectangle {
    id: atomBar
    visible: currentTool === "BOND_1" || currentTool === "SELECT"
    anchors { bottom: canvas.bottom; bottomMargin: 16; left: canvas.left; leftMargin: 16 }
    height: 32; radius: 16
    color: Theme.surface
    border.color: Theme.outline; border.width: 1

    // Drop shadow
    layer.enabled: true
    layer.effect: DropShadow { radius: 8; color: "#33000000" }

    Row {
        anchors.centerIn: parent
        spacing: 2
        // C · N · O · S · F · Cl · | · +· −
    }

    Behavior on opacity { NumberAnimation { duration: 150 } }
}
```

This frees 36 px of vertical space permanently, and the bar only appears when it is actually useful.

***

## 5. PropertyPanel — Differentiate Label/Value Typography More Strongly

**Current state:** Labels use `Theme.textSecondary` + `fontSizeCaption (10)`, values use `Theme.textPrimary` + `fontSizeLabel (11)` + `fontMono`. The size difference (10 vs 11 px) is too small to create a clear visual hierarchy. [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/PropertyPanel.qml)

**Fix — Use a two-column card layout with better contrast:**

```qml
// Replace inline Grid { columns: 2 } with labelled card rows:
component PropRow: RowLayout {
    property string label
    property string value
    spacing: 0

    Text {
        text: label
        font.pixelSize: Theme.fontSizeCaption    // 10px
        color: Theme.textSecondary
        Layout.preferredWidth: 72                 // fixed label column
        elide: Text.ElideRight
    }
    Text {
        text: value
        font.pixelSize: Theme.fontSizeBody       // 12px — was 11, now +1
        font.family: Theme.fontMono
        color: Theme.textPrimary
        font.weight: Font.Medium
        Layout.fillWidth: true
    }
}
```

Also add **section header pills** for the Molecular vs Drug property groups — small `Rectangle` chips with rounded corners and `Theme.accentLight` background, not just a plain text label in `textSecondary`. This gives the panel a clear visual scanline.

***

## 6. Tab Bar — Professional Document Tabs

**Current state:** Standard QQC2 `TabBar` + `TabButton` per document. [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/MainWindow.qml)

**Missing visual elements:**
- No dirty/modified indicator on unsaved tabs
- No close button visible on tabs (only on hover presumably)
- Tab bar background same as toolbar — no visual separation

**Fix:**

```qml
TabButton {
    id: tabBtn
    contentItem: RowLayout {
        spacing: 4

        // Dirty dot
        Rectangle {
            width: 6; height: 6; radius: 3
            color: Theme.accent
            visible: DocumentManager.isDirty(modelData)
        }

        Text {
            text: DocumentManager.displayName(modelData)
            elide: Text.ElideMiddle
            Layout.preferredWidth: 80
            color: tabBtn.checked ? Theme.textPrimary : Theme.textSecondary
            font.pixelSize: Theme.fontSizeLabel
        }

        // Close button — always visible on active tab, hover on others
        IconCell {
            iconSource: "../icons/close.svg"
            cellWidth: 16; cellHeight: 16; iconSize: 10
            opacity: tabBtn.checked || tabBtn.hovered ? 0.7 : 0
            onClicked: DocumentManager.closeDoc(modelData)
        }
    }

    background: Rectangle {
        color: tabBtn.checked ? Theme.background : "transparent"
        // Bottom accent line on active tab
        Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width; height: 2
            color: Theme.accent
            visible: tabBtn.checked
        }
    }
}
```

The bottom accent line (2 px, `Theme.accent`) is the standard modern tab indicator — cleaner than a filled background.

***

## 7. StatusBar — Structured Information Architecture

**Current state:** Single `RowLayout` with text items for tool state, atom/bond counts, zoom slider. [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/MainWindow.qml)

**Fix — Three-zone layout:**

```
[LEFT: active tool · selection summary] ←→ spacer → [CENTER: formula · MW on hover] ←→ spacer → [RIGHT: async spinner · zoom −  85%  +]
```

```qml
Rectangle {
    height: 26     // down from implied 32px
    color: Theme.background
    // Top border only — 1px separator from canvas
    Rectangle { anchors.top: parent.top; width: parent.width; height: 1; color: Theme.outline }

    RowLayout {
        // LEFT zone — tool + selection
        Text { text: toolLabel; font.pixelSize: Theme.fontSizeCaption; color: Theme.textSecondary }
        Text { text: selectionSummary; color: Theme.textPrimary; font.pixelSize: Theme.fontSizeCaption }

        Item { Layout.fillWidth: true }

        // CENTER — formula (only if molAtoms > 0)
        Text {
            visible: molAtoms > 0
            text: molFormula + "  " + molMW.toFixed(2) + " Da"
            font.family: Theme.fontMono
            font.pixelSize: Theme.fontSizeCaption
            color: Theme.textSecondary
        }

        Item { Layout.fillWidth: true }

        // RIGHT — async indicator + zoom
        BusyIndicator { running: indigoWorking; width: 14; height: 14 }
        Text { text: Math.round(zoomLevel * 100) + "%"; font.pixelSize: Theme.fontSizeCaption; color: Theme.textSecondary; width: 36; horizontalAlignment: Text.AlignHCenter }
        IconCell { iconSource: "icons/zoom-out.svg"; cellWidth: 24; onClicked: zoomOut() }
        Slider { value: zoomLevel; from: 0.1; to: 4.0; Layout.preferredWidth: 80 }
        IconCell { iconSource: "icons/zoom-in.svg"; cellWidth: 24; onClicked: zoomIn() }
    }
}
```

***

## 8. Popup Dialogs (AnchoredPicker) — Add Visual Polish

**Current state:** `AnchoredPicker` uses `Popup` centered on screen, `padding: Theme.marginSmall`, spacing `4`, `PopupHeader` at top. [raw.githubusercontent](https://raw.githubusercontent.com/wayne125/QtSketch/refs/heads/test/components/AnchoredPicker.qml)

**Missing:**
- No drop shadow (critical for floating panels — they look flat without it)
- No border radius on popup container
- GridView cells use `cellWidth: 110, cellHeight: 48` — too tall for dense template grids

**Fix:**

```qml
Popup {
    background: Rectangle {
        color: Theme.surface
        border.color: Theme.