import MesaacSwift
import RealityKit
import SwiftUI
import simd

struct MolOverlayView: View {
  @Binding var mols: [Mol]
  @Binding var selectedItems: MolSelectionMgr

  @State private var molContainer = {
    var result = Entity()
    result.name = "Mol container"
    return result
  }()

  @State private var lights = Lights().entity
  @State private var camera = Camera()
  @State private var viewTransform = ViewTransform()

  private func addMolDepictions(_ itemsToAdd: [Mol], startIndex: Int, numToAdd: Int) {
    let endIndex = min(startIndex + numToAdd, itemsToAdd.count)
    for i in startIndex..<endIndex {
      molContainer.addChild(MolSceneBuilder(mol: itemsToAdd[i]).molEntity())
    }
  }

  private func updateDisplayedEntities() async {
    molContainer.children.removeAll()

    let itemsToAdd = selectedItems.selection()
    let chunkSize = 20
    for startIndex in stride(from: 0, to: itemsToAdd.count, by: chunkSize) {
      addMolDepictions(itemsToAdd, startIndex: startIndex, numToAdd: chunkSize)
      try? await Task.sleep(nanoseconds: 1_000_000)
    }
  }

  private func centerOnOrigin(entity: Entity) {
    let bounds = entity.visualBounds(relativeTo: nil)
    guard !bounds.isEmpty else { return }

    let offset = bounds.center
    entity.transform.translation = -offset
  }

  @MainActor
  private func recompose(controlWindowSize: CGSize) async {
    let wasEmpty = molContainer.children.isEmpty

    await updateDisplayedEntities()

    if wasEmpty && !molContainer.children.isEmpty {
      centerOnOrigin(entity: molContainer)
      camera.reposition(toSee: molContainer)
      viewTransform.configure(managing: molContainer, controlWindowSize: controlWindowSize)
    }
  }

  var body: some View {
    ZStack {
      Color.black.edgesIgnoringSafeArea(.all)
      GeometryReader { proxy in
        RealityView { content in
          content.camera = .virtual
          content.add(camera.camEntity)
          content.add(lights)
          content.add(molContainer)

        } update: { content in
          // Nothing to do?
        }
        .onChange(of: selectedItems.selected) {
          Task { @MainActor in
            await recompose(controlWindowSize: proxy.size)
          }
        }
        .gesture(
          DragGesture()
            .modifiers(.command)
            .onChanged { value in
              viewTransform.zoomDrag(from: value.startLocation, to: value.location)
            }
            .onEnded { value in
              viewTransform.completeZoomDrag(from: value.startLocation, to: value.location)
            }
        )
        .gesture(
          DragGesture()
            .modifiers(.shift)
            .onChanged { value in
              viewTransform.panDrag(from: value.startLocation, to: value.location)
            }
            .onEnded { value in
              viewTransform.completePanDrag(from: value.startLocation, to: value.location)
            }
        )
        .gesture(
          DragGesture()
            .onChanged { value in
              viewTransform.rotateDrag(from: value.startLocation, to: value.location)
            }
            .onEnded { value in
              viewTransform.completeRotateDrag(from: value.startLocation, to: value.location)
            }
        )
      }
    }
  }
}
