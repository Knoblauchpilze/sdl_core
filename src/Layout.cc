
# include "Layout.hh"
# include "SdlWidget.hh"

namespace sdl {
  namespace core {

    Layout::Layout(SdlWidget* container,
                   const std::string& name):
      utils::CoreObject(name),
      m_widget(container),
      m_items(),
      m_dirty(true)
    {
      setService(std::string("layout"));
    }

    Layout::~Layout() {
      if (m_widget != nullptr) {
        m_widget->setLayout(nullptr);
      }
    }

    void Layout::update() {
      // Check if a container is assigned to this layout.
      if (m_widget == nullptr) {
        return;
      }

      // Check if this layout is dirty.
      if (!m_dirty) {
        return;
      }

      // And if some items are managed by this layout.
      if (m_items.empty()) {
        return;
      }

      // Update with private handler.
      updatePrivate(m_widget->m_area);

      m_dirty = false;
    }

    int
    Layout::addItem(SdlWidget* item) {
      // Check for valid items.
      if (item != nullptr) {
        // Check for duplicated items.
        if (getContainerOrNull(item) != nullptr) {
          error(std::string("Cannot add widget \"") + item->getName() + "\" to layout for \"" + m_widget->getName() + "\", duplicated item");
        }

        // Insert the item into the layout.
        m_items.push_back(item);
        invalidate();

        // Assign the parent widhet for this item.
        item->setParent(m_widget);

        // Return the index of this item.
        return m_items.size() - 1;
      }

      // Invalid item to add.
      return -1;
    }

    std::vector<Layout::WidgetInfo>
    Layout::computeWidgetsInfo() const noexcept {
      // Create the return value.
      std::vector<Layout::WidgetInfo> info(m_items.size());

      // Fill each widget's info.
      for (unsigned index = 0u ; index < m_items.size() ; ++index) {
        info[index] = {
          m_items[index]->getSizePolicy(),
          m_items[index]->getMinSize(),
          m_items[index]->getSizeHint(),
          m_items[index]->getMaxSize(),
          m_items[index]->getRenderingArea()
        };
      }

      return info;
    }

    void
    Layout::assignRenderingAreas(const std::vector<utils::Boxf>& boxes) {
      // Assign the rendering area to widgets.
      for (unsigned index = 0u; index < boxes.size() ; ++index) {
        m_items[index]->setRenderingArea(boxes[index]);
      }
    }

  }
}
