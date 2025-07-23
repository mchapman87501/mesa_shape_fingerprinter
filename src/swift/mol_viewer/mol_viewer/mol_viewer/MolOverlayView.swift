import MesaacSwift
import RealityKit
import SwiftUI
import simd

struct MolOverlayView: View {
  @Binding var mols: [Mol]
  @Binding var selectedItems: MolSelectionMgr

  @State private var molContainerEntity = Entity()
  @State private var anchor = AnchorEntity(world: .zero)
  @State private var lights = Lights()

  @State private var camera = Camera()
  @State private var cameraState = CameraState()

  private func updateMolContainerEntity() {
    let container = molContainerEntity
    while !container.children.isEmpty {
      container.removeChild(container.children.first!)
    }

    for mol in selectedItems.selection() {
      let entity = MolSceneBuilder(mol: mol).molEntity()
      container.addChild(entity)
    }
  }

  private func maybeResetCamera(patchSize: CGSize) {
    // Adjust the camera, but only when transitioning from
    // nothing to view to something to view.
    guard selectedItems.justChangedFromNoneToSome() else { return }

    let bounds = molContainerEntity.visualBounds(relativeTo: nil)
    if bounds.isEmpty {
      return
    }

    let targetPos = bounds.center
    let boundingRadius = bounds.boundingRadius

    let fovDegrees = camera.component.fieldOfViewInDegrees
    let fovRads = (fovDegrees / 180.0) * .pi

    let distance = boundingRadius / tan(fovRads / 2)

    cameraState.reset(
      targetPos: targetPos, distance: CGFloat(distance), patchSize: patchSize)
  }

  var body: some View {
    ZStack {
      Color.black.edgesIgnoringSafeArea(.all)
      // Need geometry for the trackball radius.
      GeometryReader { proxy in
        RealityView { content in

          // This is based on Xcode 26 beta 3 documentation
          content.camera = .virtual

          anchor.addChild(lights.entity)
          anchor.addChild(molContainerEntity)

          content.add(anchor)
          content.add(camera.camEntity)
          cameraState.reposition(camEntity: camera.camEntity)

        } update: { content in
          cameraState.reposition(camEntity: camera.camEntity)
        }
        .onChange(of: selectedItems.selected) {
          updateMolContainerEntity()
          maybeResetCamera(patchSize: proxy.size)
          cameraState.reposition(camEntity: camera.camEntity)
        }
        .gesture(
          DragGesture()
            .modifiers(.command)
            .onChanged { value in
              cameraState.zoomDrag(from: value.startLocation, to: value.location)
            }
            .onEnded { value in
              cameraState.completeZoomDrag(from: value.startLocation, to: value.location)
            }
        )
        .gesture(
          DragGesture()
            .onChanged { value in
              cameraState.drag(from: value.startLocation, to: value.location)
            }
            .onEnded { value in
              cameraState.completeDrag(from: value.startLocation, to: value.location)
            }
        )
      }
    }
  }
}
