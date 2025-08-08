import SwiftUI

struct ErrorsViewData: Hashable, Codable {
  let filename: String
  let errors: [String]
}

struct ErrorsView: View {
  @Binding var data: ErrorsViewData?
  @Environment(\.dismiss) private var dismiss

  var body: some View {
    VStack {
      ScrollView {
        LazyVStack(alignment: .leading, spacing: 4) {
          if let data {
            ForEach(data.errors, id: \.self) {
              Text($0)
                .padding(.bottom, 2)
            }

          }
        }
        .padding([.leading, .trailing], 8)
      }
      .background(Color.white)
      .padding(8)

      HStack {
        Spacer()
        Button("Close") { dismiss() }
        Spacer()
      }
      .padding([.bottom], 8)
    }
  }
}
