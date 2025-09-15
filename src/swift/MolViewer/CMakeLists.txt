set(TARGET MolViewer)

add_executable(${TARGET} MACOSX_BUNDLE)
target_sources(
  ${TARGET}
  PRIVATE RealityKitTypeAliases.swift
          CGPoint+Extension.swift
          RKPosition+Extension.swift
          ContentView.swift
          ErrorsView.swift
          MolSceneBuilder.swift
          Lights.swift
          Camera.swift
          ArcBall.swift
          ViewTransform.swift
          SingleMolView.swift
          SingleMolSnapshotView.swift
          MolOverlayView.swift
          MolListView.swift
          MolSelectionMgr.swift
          SDFUTType.swift
          MolViewerAppDocument.swift
          MolViewerApp.swift)

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
target_link_libraries(${TARGET} PRIVATE MesaacSwift)

install(TARGETS ${TARGET} BUNDLE DESTINATION Applications)
