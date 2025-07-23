import RealityKit
@preconcurrency import SwiftUI

// From https://developer.apple.com/forums/thread/770404

@MainActor
@Observable
class EntityToImage {
  private let renderer: RealityRenderer?
  private let cameraEntity = PerspectiveCamera()

  init() throws {
    do {
      renderer = try RealityRenderer()
    } catch {
      renderer = nil
      throw error
    }
  }

  private func configureCameraEntity(cameraEntity: PerspectiveCamera, rootEntity: Entity) {
    cameraEntity.camera.fieldOfViewOrientation = .horizontal
    let fovDegrees = Float(60.0)
    cameraEntity.camera.fieldOfViewInDegrees = fovDegrees

    // Adjust the camera, if there is anything to view.
    let bounds = rootEntity.visualBounds(relativeTo: nil)
    guard !bounds.isEmpty else { return }

    let center = bounds.center
    let boundingRadius = bounds.boundingRadius
    let fovRads = (fovDegrees / 180.0) * .pi

    // fovDegrees is horizontal fov.  Vertical may be clipped depending on aspect
    // ratio.  Lacking aspect ratio info, apply a fudge factor - "margin"
    let margin = Float(1.25)
    let distance = (boundingRadius / tan(fovRads / 2)) * margin
    let cameraPosition = center + SIMD3<Float>(0, 0, distance)
    cameraEntity.camera.near = 0.01
    cameraEntity.camera.far = distance + boundingRadius
    cameraEntity.look(at: center, from: cameraPosition, relativeTo: nil)
  }

  private func keyLight() -> Entity {
    let result = DirectionalLight()
    result.light.intensity = 5000
    result.light.color = .white
    result.look(at: [0, 0, 0], from: [1, 1, 1], relativeTo: nil)
    return result
  }

  private func backLight() -> Entity {
    let result = DirectionalLight()
    result.light.intensity = 1000
    result.light.color = .white
    result.look(at: [0, 0, 0], from: [-1, -1, -1], relativeTo: nil)
    return result

  }

  private func configureRenderer(root: Entity) {
    guard let renderer = renderer else { return }
    renderer.entities.removeAll()

    renderer.entities.append(root)
    renderer.entities.append(cameraEntity)

    // Try (fail) to better mimic RealityView lighting.  How do I define an ambient light?
    renderer.entities.append(keyLight())
    renderer.entities.append(backLight())
    configureCameraEntity(cameraEntity: cameraEntity, rootEntity: root)

    renderer.activeCamera = cameraEntity
    renderer.cameraSettings.colorBackground = .color(.init(gray: 0.0, alpha: 0.0))
    renderer.cameraSettings.antialiasing = .none
  }

  private func textureImage(from texture: MTLTexture) -> Image? {
    let componentCount = 4
    let bitmapInfo =
      CGImageByteOrderInfo.order32Big.rawValue | CGImageAlphaInfo.premultipliedLast.rawValue
    let bitsPerComponent = 8
    let colorSpace = CGColorSpace(name: CGColorSpace.sRGB)!

    let bytesPerRow = texture.width * componentCount
    guard let pixelBuffer = malloc(texture.height * bytesPerRow) else {
      return nil
    }

    defer {
      free(pixelBuffer)
    }

    let region = MTLRegionMake2D(0, 0, texture.width, texture.height)
    texture.getBytes(pixelBuffer, bytesPerRow: bytesPerRow, from: region, mipmapLevel: 0)
    let ctx = CGContext(
      data: pixelBuffer,
      width: texture.width,
      height: texture.height,
      bitsPerComponent: bitsPerComponent,
      bytesPerRow: bytesPerRow,
      space: colorSpace,
      bitmapInfo: bitmapInfo)

    guard let cgImage = ctx?.makeImage() else {
      return nil
    }
    let ciImage = CIImage(cgImage: cgImage)
    let context = CIContext(options: nil)
    guard let cgImage = context.createCGImage(ciImage, from: ciImage.extent) else { return nil }
    return Image(cgImage, scale: 1.0, label: Text("Snapshot"))
  }

  private func createTexture(width imageWidth: Double, height imageHeight: Double) -> MTLTexture? {
    let contentSize = CGSize(width: imageWidth, height: imageHeight)
    let descriptor = MTLTextureDescriptor()
    descriptor.width = Int(contentSize.width)
    descriptor.height = Int(contentSize.height)
    descriptor.pixelFormat = .rgba8Unorm_srgb
    descriptor.sampleCount = 1
    descriptor.usage = [.renderTarget, .shaderRead, .shaderWrite]

    return MTLCreateSystemDefaultDevice()?.makeTexture(descriptor: descriptor)
  }

  private func createImage(from texture: MTLTexture, with renderer: RealityRenderer) async
    -> Image?
  {
    await withCheckedContinuation { (continuation: CheckedContinuation<Image?, Never>) in
      do {
        let output = try RealityRenderer.CameraOutput(
          RealityRenderer.CameraOutput.Descriptor.singleProjection(colorTexture: texture))
        try renderer.updateAndRender(
          deltaTime: 0.1, cameraOutput: output,
          onComplete: { _ in
            Task { @MainActor in
              let textureImage = self.textureImage(from: texture)
              continuation.resume(returning: textureImage)
            }
          })
      } catch {
        continuation.resume(returning: nil)
      }
    }
  }

  public func renderImage(content root: Entity, w imageWidth: Double, h imageHeight: Double)
    async throws -> Image?
  {
    guard let renderer = renderer else { return nil }
    configureRenderer(root: root)
    configureCameraEntity(cameraEntity: cameraEntity, rootEntity: root)

    // If you use a lit material you'll need an ibl
    //        renderer.lighting.resource = try await EnvironmentResource(named: "ImageBasedLighting")
    guard let texture = createTexture(width: imageWidth, height: imageHeight) else { return nil }

    return await createImage(from: texture, with: renderer)
  }
}

// Demo of a view that relies on EntityToImage for its displayed content:
@MainActor
struct ETISingleton {
  static let entityToImage = try? EntityToImage()

}

struct SnapshotView: View {
  @State private var image: Image?
  @State private var sphere: Entity

  init() {

    // Build the initial scene
    let sphere = Entity()
    var wireframeMaterial = UnlitMaterial()
    wireframeMaterial.triangleFillMode = .lines
    sphere.position = [0, 0, -1]
    let modelComponent = ModelComponent(
      mesh: .generateSphere(radius: 0.2),
      // Note, if you use a lit material be sure to
      // add an IBL to EntityToImage.
      materials: [wireframeMaterial]
    )
    sphere.components.set(modelComponent)

    self.sphere = sphere
  }

  private func renderImage() async throws {
    let root = Entity()
    root.addChild(sphere)
    image = try? await ETISingleton.entityToImage?.renderImage(content: root, w: 1920, h: 1080)
  }

  var body: some View {
    VStack {
      if let image {
        image
          .resizable()
          .scaledToFit()
      } else {
        Text("Unable to generate image.")
      }

      Button("Move Sphere") {
        Task { @MainActor in
          // Update the entity
          sphere.position.x += 0.2

          // Re-render the image
          try? await renderImage()
        }
      }
    }
    .task {
      // render the initial scene
      try? await renderImage()
    }
  }
}
