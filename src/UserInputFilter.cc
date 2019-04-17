
# include "UserInputFilter.hh"

namespace sdl {
  namespace core {

    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::NoInteraction;

    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::KeyPressed;
    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::KeyReleased;
    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::Key;

    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::MouseButtonPressed;
    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::MouseButtonReleased;
    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::MouseButton;

    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::MouseMotion;

    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::MouseWheelDown;
    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::MouseWheelUp;
    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::MouseWheel;

    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::Mouse;

    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::Quit;

    constexpr UserInputFilter::Interaction::Mask UserInputFilter::Interaction::FullInteraction;

    UserInputFilter::UserInputFilter(const Interaction::Mask& mask,
                                     const bool exclusion,
                                     const std::string& name):
      engine::EngineObject(name),
      m_mask(mask),
      m_exclusion(exclusion)
    {}

  }
}
