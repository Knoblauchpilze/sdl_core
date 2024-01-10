
# include "LayoutItem.hh"

namespace sdl {
  namespace core {

    LayoutItem::LayoutItem(const std::string& name,
                           const utils::Sizef& sizeHint):
      engine::EngineObject(name),

      m_minSize(),
      m_sizeHint(sizeHint),
      m_maxSize(utils::Sizef::max()),

      m_sizePolicy(),
      m_focusPolicy(),

      m_geometryDirty(true),
      m_area(),

      m_visible(true),
      m_visibleLocker(),

      m_focusState(),

      m_zOrder(0),
      m_keyboardFocus(false),

      m_manager(nullptr)
    {
      setService(std::string("layout_item"));

      // Filter window events: we do that by activating the events
      // processing and has we handle the window events using the
      // provided method we should be filtering them.
      activateEventsProcessing();
    }

    bool
    LayoutItem::filterMouseEvents(const engine::EngineObject* watched,
                                  const engine::MouseEventShPtr e) const noexcept
    {
      // What is important here is to detect mouse events which should be send to
      // another object than `watched`. This can happen if one of the other items
      // managed by this layout item block the path.
      // We have two steps here:
      // - check whether any of the first-level child managed by this layout can
      //   block the way.
      // - check whether any of the deeper child of each item can block the path.
      // Of course depending on the hierarchy of the `UI` we might have several items
      // on the way of the mouse. Ordering them in a correct way so that only the
      // most relevant one gets the focus is as important as determining the complete
      // list in the first place.

      // In order to get the list of items spanning the input position referenced by
      // the event we need to retrieve a position for the input mouse event. This can
      // happen for all but the mouse wheel event where there's no real meaning of
      // position.
      // Let's handle this first and move on to building the list.
      if (e->getType() == engine::Event::Type::MouseWheel) {
        // No filtering performed at this step.
        return false;
      }

      // Retrieve the item at this position: either it corresponds to the input object
      // in which case it means that given all the registered item the provided one is
      // the most relevant one to pass the event to so we don't filter it. Otherwise
      // we have to filter the event so that probably the item returned by `getItemAt`
      // method gets it.
      const LayoutItem* bestFit = getItemAt(e->getMousePosition());
      if (bestFit == nullptr) {
        return true;
      }

      if (bestFit == watched) {
        return false;
      }

      // The input `watched` event is not directly under the current position of the
      // mouse. We might still be okay in the case of drag events, where we want to
      // transmit the events to the items where the event started.

      if (e->getType() != engine::Event::Type::MouseDrag) {
        // Not a drag event, we're done, the event should be filtered.
        return true;
      }

      // Check whether the `watched` event was located where any of the drag motion
      // started (i.e. any of the button).
      engine::mouse::Buttons bs = e->getButtons();

      engine::mouse::Button b = engine::mouse::Button::Left;
      if (bs.isSet(b)) {
        const LayoutItem* best = getItemAt(e->getInitMousePosition(b));

        // The item is located where the button started to be dragged, this is enough
        // to send it this event.
        if (best == watched) {
          return false;
        }
      }

      b = engine::mouse::Button::Middle;
      if (bs.isSet(b)) {
        const LayoutItem* best = getItemAt(e->getInitMousePosition(b));

        if (best == watched) {
          return false;
        }
      }

      b = engine::mouse::Button::Right;
      if (bs.isSet(b)) {
        const LayoutItem* best = getItemAt(e->getInitMousePosition(b));

        if (best == watched) {
          return false;
        }
      }

      // The `watched` item was not located at the beginning of any of the current
      // drag events, the event should be filtered.
      return true;
    }

    bool
    LayoutItem::filterDragAndDropEvents(const engine::EngineObject* watched,
                                        const engine::DropEventShPtr e) const noexcept
    {
      // A drag and drop events usually include moving the mouse from a position to
      // another while keeping one or several buttons pressed. Elements might want
      // to react to such events in various ways.
      // We we want to achieve through this base behavior of filtering is to transmit
      // the drag and drop event only to the source and destination of the drag and
      // drop action. It could very well be the same element.
      // We will follow a similar process to what is done to filter mouse event:
      //  - determine the best item which is both at the source and at the destination
      //    of the event.
      //  - compare both source and destination with the `watched` object.
      //  - filter if there's no correspondance.

      // Retrieve the best fit for both the start position of the drag and drop operation
      // and also for the end of it.
      const LayoutItem* bestFitForStart = getItemAt(e->getStartPosition());
      const LayoutItem* bestFitForStop = getItemAt(e->getEndPosition());

      // The event is filtered if the input `watched` object is neither the start or the
      // destination of the event.
      return watched != bestFitForStart && watched != bestFitForStop;
    }

    bool
    LayoutItem::geometryUpdateEvent(const engine::Event& e) {
      // Perform the rebuild if the geometry has changed.
      // This check should not be really useful because
      // the `geometryUpdateEvent` should already be
      // triggered at the most appropriate time.
      if (hasGeometryChanged()) {
        updatePrivate(m_area);

        geometryRecomputed();
      }

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::geometryUpdateEvent(e);
    }

    bool
    LayoutItem::resizeEvent(engine::ResizeEvent& e) {
      // We need to assign the area for this layout item based on the size
      // provided in the event The required size is the `new` size and
      // the `old` size should correspond to the current size of the item.
      // Also to prevent production of too many events we will only launch
      // the update process if the new size described by the input event
      // is different from the existing one.
      if (e.getNewSize() == m_area) {
        return engine::EngineObject::resizeEvent(e);
      }

      // Assign the area.
      m_area = e.getNewSize();

      info(std::string("Area is now ") + m_area.toString());

      // Once the internal size has been updated, we need to both recompute
      // the geometry and then perform a repaint. Post both events.
      makeGeometryDirty();

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::resizeEvent(e);
    }

  }
}
