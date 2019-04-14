
# include "SdlWidget.hh"

namespace sdl {
  namespace core {

    SdlWidget::SdlWidget(const std::string& name,
                         const utils::Sizef& sizeHint,
                         SdlWidget* parent,
                         const bool transparent,
                         const engine::Color& color):
      engine::EventListener(name),

      m_parent(nullptr),
      m_minSize(),
      m_sizeHint(sizeHint),
      m_maxSize(utils::Sizef::max()),
      m_area(utils::Boxf(0.0f, 0.0f, sizeHint.w(), sizeHint.h())),
      m_palette(engine::Palette::fromButtonColor(color)),

      m_engine(nullptr),

      m_contentDirty(true),
      m_geometryDirty(true),
      m_isVisible(true),
      m_transparent(transparent),
      m_content(),
      m_drawingLocker(),

      m_children(),

      m_sizePolicy(),
      m_layout()
    {
      // Assign the service for this widget.
      setService(std::string("widget"));

      // Register the parent widget: if a layout is registered in the parent widget
      // we can use this, otherwise use the regular method.
      if (parent != nullptr && parent->m_layout != nullptr) {
        parent->m_layout->addItem(this);
      }
      else {
        setParent(parent);
      }
    }

    utils::Uuid
    SdlWidget::draw() {
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Check whether a valid size is provided for this widget.
      if (!m_area.valid()) {
        error(std::string("Could not repaint widget"), std::string("Invalid size"));
      }

      // Repaint if needed.
      if (hasContentChanged()) {
        log(std::string("Updating content for widget"));

        clearTexture();
        m_content = createContentPrivate();
        m_contentDirty = false;
      }

      // Clear the content and draw the new version.
      clearContentPrivate(m_content);
      drawContentPrivate(m_content);

      // Update layout if any.
      if (hasGeometryChanged()) {
        log(std::string("Updating layout for widget"));

        if (m_layout != nullptr) {
          m_layout->update();
        }
        m_geometryDirty = false;
      }

      // Proceed to update of children containers if any.
      for (WidgetMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
        if (child->second->isVisible()) {
          drawChild(*child->second);
        }
      }

      // Return the built-in texture.
      return m_content;
    }

  }
}
