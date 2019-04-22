
# include "SdlWidget.hh"
# include <core_utils/CoreWrapper.hh>

namespace sdl {
  namespace core {

    SdlWidget::SdlWidget(const std::string& name,
                         const utils::Sizef& sizeHint,
                         SdlWidget* parent,
                         const engine::Color& color):
      engine::EngineObject(name),

      m_minSize(),
      m_sizeHint(sizeHint),
      m_maxSize(utils::Sizef::max()),
      m_sizePolicy(),
      m_geometryDirty(true),
      m_area(utils::Boxf(0.0f, 0.0f, sizeHint.w(), sizeHint.h())),

      m_isVisible(true),

      m_children(),
      m_layout(),
      m_palette(engine::Palette::fromButtonColor(color)),
      m_engine(nullptr),

      m_parent(nullptr),

      m_contentDirty(true),

      m_content(),
      m_drawingLocker(),

      m_mouseInside(false)
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

    SdlWidget::~SdlWidget() {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      clearTexture();

      for (WidgetMap::const_iterator widget = m_children.cbegin() ;
           widget != m_children.cend() ;
           ++widget)
      {
        if (widget->second != nullptr) {
          delete widget->second;
        }
      }
    }

    utils::Uuid
    SdlWidget::draw() {
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Clear the content and draw the new version.
      clearContentPrivate(m_content);
      drawContentPrivate(m_content);

      // Proceed to update of children containers if any.
      for (WidgetMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
        if (child->second->isVisible()) {
          drawChild(*child->second);
        }
      }

      // Return the built-in texture.
      return m_content;
    }

    void
    SdlWidget::drawChild(SdlWidget& child) {
      const utils::Uuid& uuid = m_content;
      engine::Engine& engine = getEngine();

      // Copy also the internal area in order to perform the coordinate
      // frame transform.
      utils::Sizef dims = m_area.toSize();

      // Protect against errors.
      withSafetyNet(
        [&child, &uuid, &engine, &dims]() {
          // Draw this object (caching is handled by the object itself).
          utils::Uuid picture = child.draw();

          // Draw the picture at the corresponding place. Note that the
          // coordinates of the box of each child is in local coordinates
          // relatively to this widget.
          // In order to obtain good results, we need to convert to an
          // intermediate coordinate frame not centered on the origin but
          // rather on the position of this widget.
          // This is because the SDL talks in terms of top left corner
          // and we talk in terms of center.
          // The conversion cannot happen without knowing the dimension
          // of the input texture, which is only known here.
          utils::Boxf render = child.getRenderingArea();

          // Account for the intermediate coordinate frame transformation.
          render.x() += (dims.w() / 2.0f);
          render.y() = (dims.h() / 2.0f) - render.y();

          engine.drawTexture(
            picture,
            &uuid,
            &render
          );
        },
        std::string("draw_child")
      );
    }

    bool
    SdlWidget::handleEvent(engine::EventShPtr e) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Handle this event using the base handler.
      const bool recognized = engine::EngineObject::handleEvent(e);

      // Check whether the event has been accepted before dispatching to children.
      if (!e->isAccepted()) {
        // Dispatch to visible children.
        WidgetMap::const_iterator widget = m_children.cbegin();

        while (widget != m_children.cend() && !e->isAccepted()) {
          if (widget->second->isVisible()) {
            widget->second->event(e);
          }
          ++widget;
        }
      }

      return recognized;
    }

    bool
    SdlWidget::geometryUpdateEvent(const engine::Event& e) {
      // Perform an update of the geometry of this widget.
      // This inludes updating the layout if any is assigned
      // to this widget.

      // Perform the rebuild if the geometry has changed.
      // This check should not be really useful because
      // the `geometryUpdateEvent` should already be
      // triggered at the most appropriate time.
      if (hasGeometryChanged()) {
        log(std::string("Updating layout for widget"));

        if (m_layout != nullptr) {
          m_layout->update();
        }
        m_geometryDirty = false;
      }

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::geometryUpdateEvent(e);
    }

    bool
    SdlWidget::repaintEvent(const engine::PaintEvent& e) {
      // In order to repaint the widget, a valid rendering area
      // must have been defined through another process (usually
      // by updating the layout of the parent widget).
      // If this is not the case, an error is raised.
      // Also the widget should be visible: if this is not the
      // case we know that the `setVIsible` method will trigger
      // a repaint when called with a `true` value (i.e. when the
      // widget is set back to visible). So no need to worry of
      // these events if the widget is not visible.
      if (!isVisible()) {
        // Use the base handler to determine the return value.
        return engine::EngineObject::repaintEvent(e);
      }

      if (!m_area.valid()) {
        error(std::string("Could not repaint widget"), std::string("Invalid size"));
      }

      // Perform the repaint if the content has changed.
      // This check should not be really useful because
      // the `repaintEvent` should already be triggered
      // at the most appropriate time.
      if (hasContentChanged()) {
        log(std::string("Updating content for widget"));

        clearTexture();
        m_content = createContentPrivate();
        m_contentDirty = false;
      }

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::repaintEvent(e);
    }

  }
}
