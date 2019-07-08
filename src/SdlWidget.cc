
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
      m_repaintOperation(nullptr),
      m_refreshOperation(nullptr),
      m_cachedContent(),
      m_cacheLocker(),

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

      std::lock_guard<std::mutex> cacheGuard(m_cacheLocker);
      clearCachedTexture();

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

    utils::Uuid
    SdlWidget::draw() {
      // Perform the lock to process oending repaint events.
      handleGraphicOperations();

      // We also need to traverse the list of children and
      // call the `draw` method on each one. This allows to
      // actually perform the pending graphic operations.
      // This will guarantee that repaint operations can
      // bubble up to the top level when needed.
      {
        std::lock_guard<std::recursive_mutex> guard(m_drawingLocker);

        utils::Sizef area = LayoutItem::getRenderingArea().toSize();

        for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
          if (child->widget->isVisible()) {
            child->widget->draw();
          }
        }
      }

      // Return the cached texture.
      std::lock_guard<std::mutex> guard(m_cacheLocker);
      return m_cachedContent;
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
      const utils::Uuid& uuid = m_content;
      engine::Engine& engine = getEngine();

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
            nullptr,
            &uuid,
            &render
          );
        },
        std::string("draw_child(") + child.getName() + ")"
      );
    }

    bool
    SdlWidget::refreshEvent(const engine::Event& e) {
      // The refresh event is meant to allow the update of the internal
      // cached content with the up-to-date actual content. However this
      // includes creating a new texture or destroying any existing
      // cached content and due to limitations of the engine is meant to
      // be done in the main thread.
      // As we cannot guarantee that this operation will be performed in
      // the main thread we chose to save internally such events to process
      // them later on, during a call to `draw` method which is surely
      // called from the main thread.
      // So in here we just have to save the event for further processing.

      // If no previous refresh operations were registered, we need to
      // create a new one.
      if (m_refreshOperation == nullptr) {
        m_refreshOperation = std::make_shared<engine::Event>(e);
      }
      else {
        // Should not happen: proceed to merge both elements.
        log(std::string("Merging duplicated refresh operation in widget"), utils::Level::Warning);

        m_refreshOperation->merge(e);
      }

      // Use base handler to determine whether the event was recognized.
      return LayoutItem::refreshEvent(e);

    }

    bool
    SdlWidget::repaintEvent(const engine::PaintEvent& e) {
      // Usually the paint event is meant to update the internal
      // visual representation of this widget. It is important so
      // that the display for this widget is always up-to-date
      // with its content.
      // Unfortunately some limitations in the engine we're using
      // make it impossible to create textures outside of the
      // main thread. This is particularly problematic because
      // the whole events' system is designed to handle easily
      // the repaint operation just like any operation.
      // We chose to use a workaround: we save the repaint events
      // in an internal array so that they can be processed later
      // on during a call to the `draw` method. This method is
      // called by the application into which the widget is used
      // and always from the main thread.
      // We handle caching and the repaint operation itself over
      // there.
      // So in here we just have to save the event for further
      // processing.

      // If no previous repaint operations were registered, we need to
      // create a new one.
      if (m_repaintOperation == nullptr) {
        m_repaintOperation = std::make_shared<engine::PaintEvent>(e);
      }
      else {
        // Should not happen: proceed to merge both elements.
        log(std::string("Merging duplicated repaint operation in widget"), utils::Level::Warning);

        m_repaintOperation->merge(e);
      }

      // Trigger a refresh.
      requestRefresh();

      // Use base handler to determine whether the event was recognized.
      return LayoutItem::repaintEvent(e);
    }

    bool
    SdlWidget::resizeEvent(engine::ResizeEvent& e) {
      // Use the base handler to handle the resize.
      const bool toReturn = LayoutItem::resizeEvent(e);

      // Mark the content as dirty.
      makeContentDirty();

      // Return the value provided by the base handler.
      return toReturn;
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

    void
    SdlWidget::refreshEventPrivate(const engine::Event& /*e*/) {
      // Replace the cached content.
      std::lock_guard<std::mutex> guard(m_cacheLocker);

      // Create a new cached texture if the size of the cached content is
      // different from the current size of the content.
      utils::Sizei old;
      if (m_cachedContent.valid()) {
        old = getEngine().queryTexture(m_cachedContent);
      }
      utils::Sizei cur = getEngine().queryTexture(m_content);

      if (!m_cachedContent.valid() || old != cur) {

        log("Recreating cached content, old size was " + old.toString() + ", new is " + cur.toString());

        // Clear existing cached texture.
        clearCachedTexture();

        // Create new one with required dimensions.
        m_cachedContent = createContentPrivate();
        
        // In order to make the texture valid for rendering we need to clear it
        // with a valid color.
        getEngine().fillTexture(m_cachedContent, getPalette());
      }
      else {
        // Clear content so that we do not get polluted by the remains of old
        // renderings.
        clearContentPrivate(m_cachedContent, utils::Boxf::fromSize(old, true));
      }

      // Copy the data of `m_content` onto `m_cachedContent`.
      // We can copy withtout specifying dimensions as both
      // textures should have similar sizes.
      getEngine().drawTexture(m_content, nullptr, &m_cachedContent);

      // Also, notify the parent if needed.
      if (hasParent()) {
        // As we will set a new emitter we need to create a new event.
        // The size associated to the paint event corresponds to the
        // largest size between the new and old size. Indeed the parent
        // needs to repaint areas which might not be covered by this
        // widget anymore.
        utils::Boxf thisArea = LayoutItem::getRenderingArea();

        const float w = old.w() > cur.w() ? old.w() : cur.w();
        const float h = old.h() > cur.h() ? old.h() : cur.h();

        // The paint event are supposed to express the coordinates using
        // global coordinate frame. So after computing the local values
        // we need to transform using the position of the parent.
        const utils::Vector2f local(
          0.0f + (w - thisArea.w()) / 2.0f,
          0.0f - (h - thisArea.h()) / 2.0f
        );
        const utils::Vector2f global = mapToGlobal(local);

        utils::Boxf toRepaint(global, w, h);

        log("Old area is " + old.toString() + " new is " + cur.toString(), utils::Level::Info);
        log("This area is " + thisArea.toString() + ", local is " + local.toString() + ", global is " + global.toString(), utils::Level::Info);
        log("To repaint for parent " + m_parent->getName() + " is " + toRepaint.toString(), utils::Level::Info);

        // Once we have the coordinates, create and post the paint event.
        engine::PaintEventShPtr pe = std::make_shared<engine::PaintEvent>(toRepaint, m_parent);
        pe->setEmitter(this);

        m_parent->postEvent(pe);
      }
    }

    void
    SdlWidget::repaintEventPrivate(const engine::PaintEvent& e) {
      // When calling this method we should be in the main thread.
      // This means that it is ok to create a texture, it will be
      // usable in the main thread for display purposes.
      // So in order to repaint the widget, a valid rendering area
      // must have been defined through another process (usually
      // by updating the layout of the parent widget). If this is
      // not the case, an error is raised. Also the widget should
      // be visible: if this is not the case we know that the
      // `setVisible` method will trigger a repaint when called
      // with a `true` value (i.e. when the widget is set back to
      // visible). So no need to worry of these events if the
      // widget is not visible.
      // We also need to handle caching of the data so that it can
      // be reused later on withtout modifications and need to
      // redraw everything.

      // So first check that the widget is visible.
      if (!isVisible()) {
        // Return early.
        return;
      }

      // Retrieve and check the rendering area for this widget.
      utils::Boxf area = LayoutItem::getRenderingArea();

      if (!area.valid()) {
        error(std::string("Could not repaint widget"), std::string("Invalid size"));
      }

      // We are certain that the repaint operation is valid. In order
      // to perform the repaint we need to either completely recreate
      // the content or only update part of it.
      // The information about the area to repaint is available in the
      // input event but it's not enough. Indeed we are able to determine
      // whether only a part of the widget should be updated but not
      // whether the widget needs to be recreated. This information is
      // describes internally with the `m_contentDirty` boolean.
      // Checking it will allow us to precisely determine whether a global
      // paint event should also be paired with the creation of a new
      // texture for this widget or the content can only be redrawn.
      const bool redraw = m_contentDirty;

      if (m_contentDirty) {
        // Clear the internal texture.
        clearTexture();

        // Create the new content.
        m_content = createContentPrivate();

        // Until further notice the content is up-to-date.
        m_contentDirty = false;
      }

      // Perform the update of the area described by the input
      // paint event.
      const std::vector<utils::Boxf> regions = e.getUpdateRegions();

      for (int id = 0 ; id < static_cast<int>(regions.size()) ; ++id) {
        const utils::Boxf region = convertToLocal(regions[id], area);

        log("Updating region " + region.toString() + " from " + regions[id].toString() + " (ref: " + area.toString() + ")");

        clearContentPrivate(m_content, region);
        drawContentPrivate(m_content, region);
      }

      // Copy also the internal area in order to perform the coordinate
      // frame transform.
      utils::Sizef dims = area.toSize();

      // Proceed to update of children containers if any: at this point
      // the `m_children` array is already sorted by z order so we can
      // just iterate over it and we will process children in a valid
      // order.
      // In addition to that we only need to repaint children which have
      // a non empty intersection with any of the regions to update.
      // This behavior might be overriden by the `redraw` operation as
      // obviously if the whole widget has been recreated we need to
      // repaint chidlren.
      for (WidgetsMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
        // The chidlren needs to be repainted if:
        // 1. It is visible.
        // 2. It needs a repaint from the input `event`.
        // 3. It needs a repaint because the widget has been recreated.
        // The only tricky part is determining whether the widget has
        // some intersection with any of the update regions.

        bool intersectWithRepaint = false;
        int id = 0;
        const utils::Boxf childBox = child->widget->getRenderingArea();

        while (id < static_cast<int>(regions.size()) && !intersectWithRepaint) {
          // Convert region from global to local.
          const utils::Boxf region = convertToLocal(regions[id], area);

          // Determine whether the region has an intersection with the child.
          // TODO: In the case of the `gany` widget the box is relative to the parent (i.e. the `tabwidget_selector` but
          // does not include the translation of the `right_tab_widget`). This is a problem.
          // Log extract:
          // // Supposedly local box:
          // [Box: pos: 267.500000x0.000000, dims: 94.000000x235.000000]
          // // Update from gany, relative to global coordinate frame:
          // from [Box: pos: 267.500000x-117.500000, dims: 94.000000x235.000000]
          // // Ref (i.e. box of `tabwidget_selector`) does not account for position of `right_tabwidget`.
          // (ref: [Box: pos: 0.000000x-117.500000, dims: 94.500000x235.000000])
          intersectWithRepaint = region.intersect(childBox).valid();

          // Move to the next one.
          ++id;
        }

        if (!intersectWithRepaint) {
          log("Child " + child->widget->getName() + " does not intersect with any of the repaint areas (" + std::to_string(regions.size()) + ")");
        }

        // Check whether we should repaint this child.
        if (child->widget->isVisible() && (intersectWithRepaint || redraw)) {
          drawChild(*child->widget, dims);
        }
      }
    }

  }
}
