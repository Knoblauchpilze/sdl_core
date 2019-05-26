
# include "LayoutItem.hh"

namespace sdl {
  namespace core {

    LayoutItem::LayoutItem(const std::string& name,
                           const utils::Sizef& sizeHint,
                           const bool rootItem,
                           const bool allowLog):
      engine::EngineObject(name, allowLog),
      m_minSize(),
      m_sizeHint(sizeHint),
      m_maxSize(utils::Sizef::max()),
      m_sizePolicy(),
      m_geometryDirty(true),
      m_area(utils::Boxf(0.0f, 0.0f, sizeHint.w(), sizeHint.h())),
      m_rootItem(rootItem)
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
        log(std::string("Updating geometry for layout item"));

        updatePrivate(m_area);

        geometryRecomputed();
      }

      // Mark the event as accepted if it is directed only through this
      // object.
      if (isReceiver(e)) {
        e.accept();
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

      log(std::string("Area is now ") + m_area.toString());

      // Once the internal size has been updated, we need to both recompute
      // the geometry and then perform a repaint. Post both events.
      makeGeometryDirty();

      // Mark the event as accepted if it is directed only through this
      // object.
      if (isReceiver(e)) {
        e.accept();
      }

      // Use base handler to determine whether the event was recognized.
      return engine::EngineObject::resizeEvent(e);
    }

  }
}
