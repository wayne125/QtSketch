cmake_minimum_required(VERSION 3.24)
project(sketchQmlDesktop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Qml Quick Concurrent)
qt_standard_project_setup(REQUIRES 6.6)
qt_policy(SET QTP0004 OLD)

# quickjs-ng: in-process JS engine, replacing the Node.js child-process worker.
# Vendored as plain source (third_party/quickjs-ng) since it has no prebuilt-binary
# toolchain mismatch to fight (unlike Indigo) -- CMake compiles it fresh for
# whichever compiler is active. Only the "qjs" static library target is wanted;
# the CLI/test/example executables their CMakeLists also defines are excluded
# from the default build below to keep this minimal.
add_subdirectory(third_party/quickjs-ng)
foreach(_qjs_extra_target IN ITEMS qjsc qjs_exe qjs-libc run-test262 api-test lre-test function_source)
    if (TARGET ${_qjs_extra_target})
        set_target_properties(${_qjs_extra_target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif()
endforeach()

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(StyleSheets.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_executable(sketch src/app/main.cpp src/app/IndigoService.h src/app/IndigoService.cpp src/app/ImagoService.h src/app/ImagoService.cpp src/v8_process.h src/v8_process.cpp src/qjs_engine.h src/qjs_engine.cpp src/app/FileIO.h src/app/PlacementEngines.h src/app/PlacementEngines.cpp src/app/PlacementPreviewManager.h src/app/PlacementPreviewManager.cpp src/app/AppController.h src/app/AppController.cpp src/app/DocumentManager.h src/app/DocumentManager.cpp)

# set_target_properties( PROPERTIES WIN32_EXECUTABLE TRUE)

qt_add_qml_module(sketch
    URI Sketch.App
    VERSION 1.0
    QML_FILES
        MainWindow.qml
        AppMenus.qml
        MainToolbar.qml
        dialogs/FileDialogs.qml
        dialogs/TaskDialogs.qml
        dialogs/MessageDialogs.qml
        ChemCanvas.qml
        MoleculeLayer.qml
        SelectionLayer.qml
        LabelLayer.qml
        ToolPanel.qml
        PropertyPanel.qml
        ToolOverlay.qml
        Theme.qml
        StyleSheets.qml
        PeriodicTablePopup.qml
        BiopolymerDialog.qml
        AtomPropertiesDialog.qml
        RGroupPanel.qml
        SdfRecordPicker.qml
        SubstructureSearchPopup.qml

        components/FocusRing.qml
        components/IconCell.qml
        components/ThumbnailGridItem.qml
        components/StatusPlaceholder.qml
        components/PopupHeader.qml
        components/AnchoredPicker.qml
        components/SegmentedControl.qml
        components/AppMenuBar.qml
        components/AppMenuBarItem.qml
        components/MenuItemRow.qml
        components/TaskDialog.qml

        chem-core.js
        js/Selection.js
        js/ToolLabels.js
        js/ThumbnailPainter.js
    RESOURCES
        fonts/materialdesignicons-webfont.ttf
        icons/select.svg
        icons/erase.svg
        icons/single_bond.svg
        icons/double_bond.svg
        icons/triple_bond.svg
        icons/up_bond.svg
        icons/down_bond.svg
        icons/updown_bond.svg
        icons/benzene.svg
        icons/clear.svg
        icons/undo.svg
        icons/redo.svg
        icons/copy.svg
        icons/cut.svg
        icons/paste.svg
        icons/save.svg
        icons/open.svg
        icons/fit.svg
        icons/copy_image.svg
        icons/smiles_in.svg
        icons/biopolymer.svg
        icons/select-fragment.svg
        icons/template-0.svg
        icons/template-2.svg
        icons/template-3.svg
        icons/template-4.svg
        icons/template-5.svg
        icons/charge-plus.svg
        icons/charge-minus.svg
        icons/period-table.svg
        icons/generic-groups.svg
        icons/template-lib.svg
        icons/reaction-arrow-open-angle.svg
        icons/reaction-arrow-multitail.svg
        icons/reaction-plus.svg
        icons/reaction-map.svg
        icons/reaction-arrow-filled-triangle.svg
        icons/reaction-arrow-retrosynthetic-arrow.svg
        icons/reaction-arrow-equilibrium-filled-triangle.svg
        icons/reaction-arrow-equilibrium-filled-half-bow.svg
        icons/reaction-arrow-elliptical-arc-arrow-filled-triangle.svg
        icons/chain.svg
        icons/text.svg
        icons/template-6.svg
        icons/template-7.svg
        icons/transform-rotate.svg
        icons/transform-flip-h.svg
        icons/transform-flip-v.svg
        icons/layout.svg
        icons/arom.svg
        icons/dearom.svg
        icons/explicit-hydrogens.svg
        icons/clean.svg
        icons/analyse.svg
        icons/check.svg
        icons/rgroup-label.svg
        icons/zoom-in.svg
        icons/zoom-out.svg
        icons/file-thumbnail.svg
        icons/hand.svg
        icons/select-lasso.svg
        icons/add-image.svg
        icons/search.svg
)

target_link_libraries(sketch PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Concurrent qjs
)


# Indigo C++ integration
target_include_directories(sketch PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app
    ${CMAKE_CURRENT_SOURCE_DIR}/build/vcpkg_installed/x64-mingw-dynamic/include
    ${CMAKE_CURRENT_SOURCE_DIR}/indigo/api/c/indigo
    ${CMAKE_CURRENT_SOURCE_DIR}/indigo/api/c/indigo-inchi
    ${CMAKE_CURRENT_SOURCE_DIR}/indigo/api/c/indigo-renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/imago/include
)

if (MSVC)
    # Official EPAM MSVC build (indigo-libs-windows-x86_64, v1.43.0) — the vendored
    # indigo/lib/indigo.dll is MinGW-built (Itanium-mangled exports) and cannot be
    # linked directly by link.exe. indigo.lib/indigo-inchi.lib here are generated import
    # libraries (dumpbin /exports -> .def -> lib.exe /def:), since EPAM's MSVC zip ships
    # only DLLs.
    set(INDIGO_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/indigo-libs-windows-x86_64/lib/windows-x86_64)
    target_link_libraries(sketch PRIVATE ${INDIGO_LIB_DIR}/indigo.lib ${INDIGO_LIB_DIR}/indigo-inchi.lib ${INDIGO_LIB_DIR}/indigo-renderer.lib)
    set(INDIGO_RUNTIME_FILES
        ${INDIGO_LIB_DIR}/indigo.dll
        ${INDIGO_LIB_DIR}/indigo-inchi.dll
        ${INDIGO_LIB_DIR}/indigo-renderer.dll
        ${INDIGO_LIB_DIR}/msvcp140.dll
        ${INDIGO_LIB_DIR}/vcruntime140.dll
        ${INDIGO_LIB_DIR}/vcruntime140_1.dll
    )
else()
    set(INDIGO_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/indigo/lib)
    set(IMAGO_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imago/lib)
    target_link_libraries(sketch PRIVATE ${INDIGO_LIB_DIR}/indigo.dll ${INDIGO_LIB_DIR}/indigo-inchi.dll ${INDIGO_LIB_DIR}/indigo-renderer.dll ${IMAGO_LIB_DIR}/imago.dll)
    set(INDIGO_RUNTIME_FILES ${INDIGO_LIB_DIR}/indigo.dll ${INDIGO_LIB_DIR}/indigo-inchi.dll ${INDIGO_LIB_DIR}/indigo-renderer.dll ${IMAGO_LIB_DIR}/imago.dll)
endif()

add_custom_command(TARGET sketch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${INDIGO_RUNTIME_FILES}
        $<TARGET_FILE_DIR:sketch>
)

