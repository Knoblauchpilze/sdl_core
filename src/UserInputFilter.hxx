#ifndef    USER_INPUT_FILTER_HXX
# define   USER_INPUT_FILTER_HXX

# include <algorithm>
# include "UserInputFilter.hh"

namespace sdl {
  namespace core {

    inline
    UserInputFilter::~UserInputFilter() {}

    inline
    bool
    UserInputFilter::filterEvent(engine::EventListener* /*watched*/,
                                 engine::EventShPtr e)
    {
      // Do not filter null event (even though it is a bit weird to get some).
      if (e == nullptr) {
        return false;
      }

      // If the event is valid, check its type against the internal mask.
      return isFiltered(e->getType());
    }

    inline
    engine::EventListenerShPtr
    UserInputFilter::createFilterFromMask(const Interaction::Mask& mask) {
      return std::make_shared<UserInputFilter>(mask);
    }

    inline
    engine::EventListenerShPtr
    UserInputFilter::createExclusionFilterFromMask(const Interaction::Mask& mask) {
      return std::make_shared<UserInputFilter>(mask, true);
    }

    inline
    bool
    UserInputFilter::isFiltered(const engine::Event::Type& type) const noexcept {
      // Assume the event is not filtered.
      bool filtered = false;

      // Distinguish based on the type of the event.
      switch (type) {
        case engine::Event::Type::KeyPress:
          filtered = filtering(Interaction::KeyPressed);
          break;
        case engine::Event::Type::KeyRelease:
          filtered = filtering(Interaction::KeyReleased);
          break;
        case engine::Event::Type::MouseButtonPress:
          filtered = filtering(Interaction::MouseButtonPressed);
          break;
        case engine::Event::Type::MouseButtonRelease:
          filtered = filtering(Interaction::MouseButtonReleased);
          break;
        case engine::Event::Type::MouseMove:
          filtered = filtering(Interaction::MouseMotion);
          break;
        case engine::Event::Type::MouseWheel:
          filtered = filtering(Interaction::MouseWheel);
          break;
        case engine::Event::Type::Quit:
          filtered = filtering(Interaction::Quit);
          break;
        default:
          // Unhandled event type, do not filter it.
          break;
      }

      // Return the filtered status.
      return filtered;
    }

    inline
    bool
    UserInputFilter::filtering(const Interaction::Mask& mask) const noexcept {
      // If the input `mask` can be matched in the internal `m_mask` we should filter the
      // event.
      // This semantic is inversed if the `m_exclusion` flag is `true`.
      return
        (!m_exclusion && m_mask & mask) ||
        (m_exclusion && !(m_mask& mask))
      ;
    }

  }
}

#endif    /* USER_INPUT_FILTER_HXX */
