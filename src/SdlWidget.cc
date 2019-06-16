
# include "SdlWidget.hh"
# include <core_utils/CoreWrapper.hh>

namespace sdl {
  namespace core {

    SdlWidget::SdlWidget(const std::string& name,
                         const utils::Sizef& sizeHint,
                         SdlWidget* parent,
                         const engine::Color& color):
      LayoutItem(name, sizeHint, false, false),

      m_children(),
      m_layout(),
      m_palette(engine::Palette::fromButtonColor(color)),
      m_engine(nullptr),

      m_parent(nullptr),

      m_contentDirty(true),

      m_content(),
      m_drawingLocker(),

      m_mouseInside(false),

      onClick()
    {
      // Assign the service for this widget.
      setService(std::string("widget"));

      // Assign the input `parent` to this widget: this will also share the engine
      // and events queue if any is defined in the parent widget.
      setParent(parent);
    }

    SdlWidget::~SdlWidget() {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      clearTexture();

      for (WidgetsMap::const_iterator widget = m_children.cbegin() ;
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
      for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
        if (child->second->isVisible()) {
          drawChild(*child->second);
        }
      }

      // Return the built-in texture.
      return m_content;
    }

    void
    SdlWidget::trimEvents(std::vector<engine::EventShPtr>& events) {
      // Traverse the list of events and abalyze each one.
      bool prevWasHide = false;

      std::vector<engine::EventShPtr>::iterator event = events.begin();

      while (event != events.cend()) {
        // We want to react to specific event which will trigger a
        // modification of the input events queue. These events are
        // the following:
        // 1. Null events will be trashed.
        // 2. Events with None type will be trashed.
        // 3. Hide events will clear the rest of the queue except
        //    Show events: in this case it will be collapsed based
        //    on the current state of the widget.
        // 4. Hide event in an already hidden widget will be trashed.
        // 5. Show event in an already visible widget will be trashed.
        // That'it for now.
        if ((*event) == nullptr || (*event)->getType() == engine::Event::Type::None) {
          event = events.erase(event);
        }
        else if ((*event)->getType() == engine::Event::Type::Hide) {
          // Check whether the item is already hidden: if this is the case we
          // discard this event and move to the next one.
          if (!isVisible()) {
            event = events.erase(event);
          }
          else {
            ++event;
          }

          // Mark the fact that we processed a Hide event.
          prevWasHide = true;
        }
        else if ((*event)->getType() == engine::Event::Type::Show) {
          // If this item is already visible, trash it.
          if (isVisible()) {
            event = events.erase(event);
          }
          else {
            ++event;
          }

          prevWasHide = false;
        }
        else {
          // Check whether we were processing a Hide event right before that.
          // If this is the case it means that there's a hide operation with
          // no Show operation: we just need to clear everything apart from
          // the Hide event.
          if (prevWasHide) {
            // Clear the list.
            while (event != events.cend()) {
              event = events.erase(event);
            }
          }
          else {
            // Move to the next event.
            ++event;
          }
        }
      }
    }

    void
    SdlWidget::drawChild(SdlWidget& child) {
      const utils::Uuid& uuid = m_content;
      engine::Engine& engine = getEngine();

      // Copy also the internal area in order to perform the coordinate
      // frame transform.
      utils::Boxf area = LayoutItem::getRenderingArea();
      utils::Sizef dims = area.toSize();

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
        std::string("draw_child(") + child.getName() + ")"
      );
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

      utils::Boxf area = LayoutItem::getRenderingArea();

      if (!area.valid()) {
        error(std::string("Could not repaint widget"), std::string("Invalid size"));
      }

      // Perform the repaint if the content has changed.
      // This check should not be really useful because
      // the `repaintEvent` should already be triggered
      // at the most appropriate time.
      if (hasContentChanged()) {
        clearTexture();
        m_content = createContentPrivate();
        m_contentDirty = false;
      }

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::repaintEvent(e);
    }

    bool
    SdlWidget::resizeEvent(const engine::ResizeEvent& e) {
      // Mark the content as dirty.
      makeContentDirty();

      // Use the base handler method to perform additional operations and
      // also to provide a return value.
      return LayoutItem::resizeEvent(e);
    }

  }
}
