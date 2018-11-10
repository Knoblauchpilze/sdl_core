#ifndef    EVENTLISTENER_HXX
# define   EVENTLISTENER_HXX

# include <algorithm>
# include "SdlEventListener.hh"

namespace sdl {
  namespace core {

    inline
    SdlEventListener::~SdlEventListener() {}

    inline
    const SdlEventListener::Interaction::Mask&
    SdlEventListener::getInteractionMask() const noexcept {
      return m_mask;
    }

    inline
    void
    SdlEventListener::onKeyPressedEvent(const SDL_KeyboardEvent& keyEvent) {}

    inline
    void
    SdlEventListener::onKeyReleasedEvent(const SDL_KeyboardEvent& keyEvent) {}

    inline
    void
    SdlEventListener::onMouseMotionEvent(const SDL_MouseMotionEvent& mouseMotionEvent) {}

    inline
    void
    SdlEventListener::onMouseButtonPressedEvent(const SDL_MouseButtonEvent& mouseButtonEvent) {}

    inline
    void
    SdlEventListener::onMouseButtonReleasedEvent(const SDL_MouseButtonEvent& mouseButtonEvent) {}

    inline
    void
    SdlEventListener::onMouseWheelEvent(const SDL_MouseWheelEvent& event) {}

    inline
    void
    SdlEventListener::onQuitEvent(const SDL_QuitEvent& event) {}

    inline
    bool
    SdlEventListener::isRelevant(const Interaction::Mask& event) const noexcept {
      return m_mask & event;
    }

  }
}

#endif    /* EVENTLISTENER_HXX */
