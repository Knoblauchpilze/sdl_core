
# include "SdlLayout.hh"
# include "SdlWidget.hh"

namespace sdl {
  namespace core {

    SdlLayout::SdlLayout(SdlWidget* container):
      m_widget(container),
      m_items()
    {}

    SdlLayout::~SdlLayout() {
      if (m_widget != nullptr) {
        m_widget->setLayout(nullptr);
      }
    }

    void SdlLayout::update(const Boxf& area) {
      // Check if a container is assigned to this layout.
      if (m_widget == nullptr) {
        return;
      }

      // And if some items are managed by this layout.
      if (m_items.empty()) {
        return;
      }

      // Update with private handler.
      updatePrivate(area);
    }

  }
}
