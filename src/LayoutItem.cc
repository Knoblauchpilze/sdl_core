
# include "LayoutItem.hh"

namespace sdl {
  namespace core {

    LayoutItem::LayoutItem(const std::string& name,
                           const utils::Sizef& sizeHint,
                           const bool needsConvert,
                           const bool virtualItem):
      engine::EngineObject(name),
      m_minSize(),
      m_sizeHint(sizeHint),
      m_maxSize(utils::Sizef::max()),
      m_sizePolicy(),
      m_geometryDirty(true),
      m_area(utils::Boxf(0.0f, 0.0f, sizeHint.w(), sizeHint.h())),
      m_nested(false),
      m_needsConvert(needsConvert),
      m_visible(true),
      m_virtual(virtualItem)
    {
      setService(std::string("layout_item"));
    }

    bool
    LayoutItem::geometryUpdateEvent(const engine::Event& e) {
      // Perform the rebuild if the geometry has changed.
      // This check should not be really useful because
      // the `geometryUpdateEvent` should already be
      // triggered at the most appropriate time.
      if (hasGeometryChanged()) {
        log(std::string("Updating geometry for layout item"), utils::Level::Notice);

        updatePrivate(m_area);

        geometryRecomputed();
      }

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::geometryUpdateEvent(e);
    }

    bool
    LayoutItem::resizeEvent(const engine::ResizeEvent& e) {
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
