
# include "SdlWidget.hh"
# include <core_utils/CoreWrapper.hh>

namespace sdl {
  namespace core {

    SdlWidget::SdlWidget(const std::string& name,
                         const utils::Sizef& sizeHint,
                         SdlWidget* parent,
                         const engine::Color& color):
      LayoutItem(name, sizeHint, false, false),

      m_names(),
      m_children(),
      m_layout(),
      m_zOrder(0),
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
      std::lock_guard<std::recursive_mutex> guard(m_drawingLocker);
      clearTexture();

      m_names.clear();

      for (WidgetsMap::const_iterator child = m_children.cbegin() ;
           child != m_children.cend() ;
           ++child)
      {
        if (child->widget != nullptr) {
          delete child->widget;
        }
      }
    }

    void
    SdlWidget::draw(const utils::Sizef& dims) {
      // Retrieve the drawing area for this widget before locking it,
      // as it also locks the widget.
      utils::Boxf drawing = getDrawingArea();

      // Lock this widget.
      std::lock_guard<std::recursive_mutex> guard(m_drawingLocker);

      // Clear the content and draw the new version: we NEED to do that
      // before rendering children elements so that we maintain some
      // kind of ordering in the depth of widgets.
      // Note that this is not optimal as we depend on the order
      // in which widgets are rendered.
      // TODO: This method should probably only use the cached content
      // without needing to redraw anything. The actual drawing is
      // performed in the `repaintEvent` method. This brings the question
      // about how children widgets will notify the parent to be redrawn.
      // Indeed imagine the following architecture:
      //
      // root_widget
      //  |
      //  +- child_1
      //  |    |
      //  |    + child_3
      //  |
      //  +- child_2
      //
      // The `child_3` gets updated, but if it does not tell its parent `child_2`
      // that it should be redrawn, the `draw` method of `child_1` will reuse the
      // cached data to display. Same problem with `child_1` and `root_widget`.
      // So we need to have some sort of mechanism to post a repaint event to the
      // parent with the area covered by this widget. It should also transmit to
      // siblings of the parent widget if the area is too big to fit inside the
      // parent's widget.
      clearContentPrivate(m_content);
      drawContentPrivate(m_content);

      // Convert the drawing area to output coordinate frame.
      drawing.x() += (dims.w() / 2.0f);
      drawing.y() = (dims.h() / 2.0f) - drawing.y();

      getEngine().drawTexture(
        m_content,
        nullptr,
        &drawing
      );

      // Proceed to update of children containers if any: at this point
      // the `m_children` array is already sorted by z order so we can
      // just iterate over it and we will process children in a valid
      // order.
      for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
        if (child->widget->isVisible()) {
          drawChild(*child->widget, dims);
        }
      }
    }

    void
    SdlWidget::trimEvents(std::vector<engine::EventShPtr>& events) {
      // Traverse the list of events and abalyze each one.
      bool prevWasHide = false;
      bool prevWasShow = false;

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
          // discard this event and move to the next one. We have to keep one
          // of them though, as the `isVisible` status is updated right away
          // even before queuing the event.
          if (prevWasHide) {
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
          if (prevWasShow) {
            event = events.erase(event);
          }
          else {
            ++event;
          }

          prevWasShow = true;
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
    SdlWidget::rebuildZOrdering() {
      // First we need to sort the internal `m_children` array.
      // Note that we want the items to be sorted in ascending
      // order of their z order.
      // Indeed as larger values of z order indicates widgets
      // in front of others, this is the correct behavior to
      // adopt. The sort should compare `lhs` less than `rhs`
      // based on their z order.
      std::sort(m_children.begin(), m_children.end(),
        [](const ChildWrapper& lhs, const ChildWrapper& rhs) {
          return lhs.zOrder < rhs.zOrder;
        }
      );

      // Now rebuild the internal `m_names` array.
      m_names.clear();

      for (int id = 0 ; id < getChildrenCount() ; ++id) {
        m_names[m_children[id].widget->getName()] = id;
      }
    }

    void
    SdlWidget::drawChild(SdlWidget& child,
                         const utils::Sizef& dims)
    {
      // Protect against errors.
      withSafetyNet(
        [&child, &dims]() {
          // Draw this object (caching is handled by the object itself).
          child.draw(dims);
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
    SdlWidget::resizeEvent(engine::ResizeEvent& e) {
      // Mark the content as dirty.
      makeContentDirty();

      // Use the base handler method to perform additional operations and
      // also to provide a return value.
      return LayoutItem::resizeEvent(e);
    }

    bool
    SdlWidget::zOrderChanged(const engine::Event& e) {
      // Traverse the children list and updtae the z order for each one.
      for (WidgetsMap::iterator child = m_children.begin() ; child != m_children.end() ; ++child) {
        child->zOrder = child->widget->getZOrder();
      }

      // Proceed to rebuild the z ordering.
      rebuildZOrdering();

      // Use the base handler method to provide a return value.
      return LayoutItem::zOrderChanged(e);
    }

  }
}
