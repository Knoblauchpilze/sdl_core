#ifndef    EVENTLISTENER_HXX
# define   EVENTLISTENER_HXX

# include <algorithm>
# include "EventListener.hh"

namespace sdl {
  namespace core {

    inline
    EventListener::~EventListener() {}

    inline
    const EventListener::Interaction::Mask&
    EventListener::getInteractionMask() const noexcept {
      return m_mask;
    }

    inline
    void
    EventListener::onKeyPressedEvent(const SDL_KeyboardEvent& keyEvent) {}

    inline
    void
    EventListener::onKeyReleasedEvent(const SDL_KeyboardEvent& keyEvent) {}

    inline
    void
    EventListener::onMouseMotionEvent(const SDL_MouseMotionEvent& mouseMotionEvent) {}

    inline
    void
    EventListener::onMouseButtonPressedEvent(const SDL_MouseButtonEvent& mouseButtonEvent) {}

    inline
    void
    EventListener::onMouseButtonReleasedEvent(const SDL_MouseButtonEvent& mouseButtonEvent) {}

    inline
    void
    EventListener::onMouseWheelEvent(const SDL_MouseWheelEvent& event) {}

    inline
    void
    EventListener::onQuitEvent(const SDL_QuitEvent& event) {}

    inline
    bool
    EventListener::isRelevant(const Interaction::Mask& event) const noexcept {
      return m_mask & event;
    }

  }
}

#endif    /* EVENTLISTENER_HXX */
