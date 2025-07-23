import MesaacSwift
import RealityKit
import SwiftUI

struct SingleMolView: View {
  let mol: Mol
  @State private var image: Image?

  private let width = 128.0
  private let height = 96.0

  private func renderImage(currMol: Mol) async throws {
    let rootEntity = AnchorEntity(world: .zero)
    let lights = Lights()
    rootEntity.addChild(lights.entity)
    let molEntity = MolSceneBuilder(mol: currMol).molEntity()
    rootEntity.addChild(molEntity)
    image = try? await ETISingleton.entityToImage?.renderImage(
      content: rootEntity, w: width * 4, h: height * 4)
  }

  var body: some View {
    VStack(alignment: .center) {
      Text(mol.name)
        .padding([.leading, .trailing], 4)
        .padding([.top, .bottom], 2)
        .frame(maxWidth: .infinity)
        .background(Color(red: 0.9, green: 0.9, blue: 0.9))

      if let image {
        image
          .resizable()
          .scaledToFit()
      } else {
        Text("...")
          .foregroundColor(.white)
          .frame(height: CGFloat(height))
      }
    }
    .background(.black)
    .border(.gray)
    .task {
      try? await renderImage(currMol: mol)
    }
  }
}
