#ifndef    FOCUS_STATE_HXX
# define   FOCUS_STATE_HXX

# include "FocusState.hh"

namespace sdl {
  namespace core {

    inline
    FocusState::FocusState():
      m_state(State::None)
    {}

    inline
    bool
    FocusState::handleFocusIn(const engine::FocusEvent::Reason& reason) {
      // Convert the input focus reason to a valid state.
      State newState = getStateFromFocusReason(reason);

      // Check whether this state should be assigned as the new
      // state for this object.
      if (isLessThan(m_state, newState)) {
        // The new state is more specialized than the current one,
        // use it as the new internal state.
        m_state = newState;

        return true;
      }

      // The new state does not override the existing one.
      return false;
    }

    inline
    bool
    FocusState::handleFocusOut(const engine::FocusEvent::Reason& reason) {
      // We can only decrease the the focus state with an equivalent reason
      // to what put us in this state in the first place.

      // Retrieve the state which would result after applying this reason.
      State resultingState = getStateFromFocusReason(reason);

      // If the resulting state is the same as the current state or greater,
      // reset the focus.
      if (isLessThan(m_state, resultingState)) {
        // As the action is enough to completely override the current state
        // we can reset it to no focus.
        m_state = State::None;

        return true;
      }

      // The input reason is not strong enough to override the current focus
      // state: return `false` as no modifications were made to the internal
      // state.
      return false;
    }

    inline
    engine::Palette::ColorRole
    FocusState::getColorRole() const noexcept {}

    inline
    bool
    FocusState::hasFocus() const noexcept {
      return m_state != State::None;
    }

    inline
    std::string
    FocusState::toString() const noexcept {
      return std::string("[State: ") + getNameFromState(m_state) + "]";
    }

    inline
    std::string
    FocusState::getNameFromState(const State& state) noexcept {
      switch (state) {
        case State::None:
          return "None";
        case State::Hover:
          return "Hover";
        case State::Tab:
          return "Tab";
        case State::Click:
          return "Click";
        default:
          return "Unknown";
      }
    }

    inline
    FocusState::State
    FocusState::getStateFromFocusReason(const engine::FocusEvent::Reason& reason) noexcept {
      switch (reason) {
        case engine::FocusEvent::Reason::HoverFocus:
          return State::Hover;
        case engine::FocusEvent::Reason::MouseFocus:
          return State::Click;
        case engine::FocusEvent::Reason::TabFocus:
        case engine::FocusEvent::Reason::BacktabFocus:
          return State::Tab;
        case engine::FocusEvent::Reason::OtherFocus:
        default:
          return State::None;
      }
    }

    inline
    bool
    FocusState::isLessThan(const State& lhs, const State& rhs) noexcept {
      return static_cast<std::underlying_type_t<State>>(lhs) < static_cast<std::underlying_type_t<State>>(rhs);
    }

  }
}

inline
std::ostream&
operator<<(const sdl::core::FocusState& state, std::ostream& out) noexcept {
  return operator<<(out, state);
}

inline
std::ostream&
operator<<(std::ostream& out, const sdl::core::FocusState& state) noexcept {
  out << state.toString();
  return out;
}

#endif    /* FOCUS_STATE_HXX */
