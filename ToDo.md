# Overview

A scrolling list of RealityView instances is a great way to get a whole bunch of allocation failures.

ChatGPT suggests a possible workaround.

## 1. Snapshot Generator ViewModel

```swift
@MainActor
class RealitySnapshotRenderer: ObservableObject {
    @Published var images: [CGImage] = []

    private(set) var content: RealityViewContent?
    private var hasRendered = false

    func setContent(_ content: RealityViewContent) {
        self.content = content

        // Only run once
        guard !hasRendered else { return }
        hasRendered = true

        Task {
            await generateSnapshots(for: makeModels())
        }
    }

    func generateSnapshots(for models: [Entity]) async {
        guard let content else { return }

        for model in models {
            content.entities.removeAll()

            let anchor = AnchorEntity(world: .zero)
            anchor.addChild(model)
            content.add(anchor)

            try? await Task.sleep(nanoseconds: 50_000_000)  // 50ms

            if let image = await RealityView.snapshot(for: content) {
                images.append(image)
            }
        }
    }

    /// Replace with your real model logic
    func makeModels() -> [Entity] {
        (0..<10).map { i in
            let entity = ModelEntity(mesh: .generateBox(size: 0.1))
            entity.transform.translation = [Float(i) * 0.05, 0, 0]
            return entity
        }
    }
}
```

## 2. Hidden RealityView for Rendering

```swift
struct HiddenRealityRendererView: View {
    @ObservedObject var renderer: RealitySnapshotRenderer

    var body: some View {
        RealityView { content in
            renderer.setContent(content)
        }
        .frame(width: 1, height: 1)  // Hidden
        .opacity(0)
        .allowsHitTesting(false)
    }
}
```

## 3. Main View That Triggers Snapshots

```swift
struct RealitySnapshotList: View {
    @StateObject private var renderer = RealitySnapshotRenderer()

    var body: some View {
        ScrollView {
            LazyVGrid(columns: [.init(.adaptive(minimum: 150))]) {
                ForEach(Array(renderer.images.enumerated()), id: \.offset) { _, img in
                    Image(img, scale: 1, label: Text("Snapshot"))
                        .resizable()
                        .frame(width: 150, height: 150)
                }
            }
        }
        .background(HiddenRealityRendererView(renderer: renderer))  // Invisible RealityView
        .padding()
    }
}
```
