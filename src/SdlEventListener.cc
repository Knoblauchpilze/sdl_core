
# include "SdlEventListener.hh"

namespace sdl {
  namespace core {

    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::NoInteraction;

    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::KeyPressed;
    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::KeyReleased;
    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::Key;

    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::MouseButtonPressed;
    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::MouseButtonReleased;
    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::MouseButton;

    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::MouseMotion;

    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::MouseWheelDown;
    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::MouseWheelUp;
    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::MouseWheel;

    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::Mouse;

    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::Quit;

    constexpr SdlEventListener::Interaction::Mask SdlEventListener::Interaction::FullInteraction;

    SdlEventListener::SdlEventListener(const Interaction::Mask& mask):
      m_mask(mask)
    {
      // Nothing to do.
    }

  }
}
