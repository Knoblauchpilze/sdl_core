
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
      m_area(utils::Boxf::fromSize(sizeHint, true)),

      m_visible(true),
      m_visibleLocker(),

      m_focusState(),
      m_manager(nullptr)
    {
      setService(std::string("layout_item"));

      // Filter window events: we do that by activating the events
      // processing and has we handle the window events using the
      // provided method we should be filtering them.
      activateEventsProcessing();
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
      // the `old` size should correspond to the current size of the
      // widget.

      // Assign the area.
      m_area = e.getNewSize();

      log(std::string("Area is now ") + m_area.toString(), utils::Level::Info);

      // Once the internal size has been updated, we need to both recompute
      // the geometry and then perform a repaint. Post both events.
      makeGeometryDirty();

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::resizeEvent(e);
    }

  }
}
