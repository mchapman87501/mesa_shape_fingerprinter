set(TARGET mol_viewer)

set(SRC
    mol_viewer/ContentView.swift
    mol_viewer/mol_viewerApp.swift
    mol_viewer/CGPoint+Extension.swift
    mol_viewer/RKPosition+Extension.swift
    mol_viewer/MolSceneBuilder.swift
    mol_viewer/RealityKitTypeAliases.swift
    mol_viewer/Lights.swift
    mol_viewer/Camera.swift
    mol_viewer/SingleMolView.swift
    mol_viewer/SingleMolSnapshotView.swift
    mol_viewer/MolOverlayView.swift
    mol_viewer/MolListView.swift
    mol_viewer/MolSelectionMgr.swift
    mol_viewer/mol_viewerAppDocument.swift
    mol_viewer/SDFUTType.swift
    mol_viewer/ArcBall.swift
    mol_viewer/CameraState.swift)

add_executable(${TARGET} MACOSX_BUNDLE ${SRC})

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
  target_compile_options(
    ${TARGET} PUBLIC "$<$<COMPILE_LANGUAGE:Swift>:-profile-generate>"
                     "$<$<COMPILE_LANGUAGE:Swift>:-profile-coverage-mapping>")
  target_link_options(
    ${TARGET} PUBLIC "$<$<COMPILE_LANGUAGE:Swift>:-profile-generate>"
    "$<$<COMPILE_LANGUAGE:Swift>:-profile-coverage-mapping>")
endif()

set_target_properties(
  ${TARGET} PROPERTIES MACOSX_BUNDLE_INFO_PLIST
                       "${CMAKE_CURRENT_SOURCE_DIR}/mol-viewer-Info.plist.in")
target_link_libraries(${TARGET} PRIVATE mesaac_swift)

install(TARGETS ${TARGET} BUNDLE DESTINATION Applications)
