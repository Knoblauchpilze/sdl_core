
# include "SdlWidget.hh"

namespace sdl {
  namespace core {

    SdlWidget::SdlWidget(const std::string& name,
                         const utils::Sizef& sizeHint,
                         SdlWidget* parent,
                         const bool transparent,
                         const engine::Color& color):
      engine::EngineObject(name),

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

      m_mouseInside(false),

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

    bool
    SdlWidget::handleEvent(engine::EventShPtr e) {
      // Lock this widget to prevent concurrent modifications.
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Handle this event using the base handler.
      const bool recognized = core::engine::EngineObject::handleEvent(e);

      // Check whether the event has been accepted before dispatching to children.
      if (!e->isAccepted()) {
        // Dispatch to children.
        WidgetMap::const_iterator widget = m_children.cbegin();

        while (widget != m_children.cend() && !e->isAccepted()) {
          widget->second->event(e);
          ++widget;
        }
      }

      return recognized;
    }

  }
}
