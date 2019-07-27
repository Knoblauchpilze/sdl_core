#ifndef    FOCUS_STATE_HH
# define   FOCUS_STATE_HH

# include <memory>
# include <sdl_engine/FocusEvent.hh>
# include <sdl_engine/Palette.hh>

namespace sdl {
  namespace core {

    class FocusState {
      public:

        /**
         * @brief - Creates a default focus state with no active focus.
         */
        FocusState();

        ~FocusState() = default;

        /**
         * @brief - Used to handle a focus in request with the specified reason. The internal
         *          state of this object will reflect the focused state expected after the
         *          provided reason. The return value indicates whether the internal state
         *          has been modified or not.
         *          If the return value is `true` the user is encouraged to retrieve the
         *          texture role associated to the new state using the `getColorRole` class
         *          method.
         *          The input reason is assumed to represent a focus in request: basically
         *          we try to raise the internal state to a higher focus value.
         * @param reason - the focus in reason to handle.
         * @return - `true` if the internal state has been modified by the focus in reason
         *           and `false` otherwise.
         */
        bool
        handleFocusIn(const engine::FocusEvent::Reason& reason);

        /**
         * @brief - Used to handle a focus out request with the specified reason. The internal
         *          state of this object will reflect the focused state expected after the
         *          provided reason if it is strong enough to override the current focus state
         *          of this element.
         *          We can only decrease the focus state of this object using this function.
         *          If the return value is `true` the user is encouraged to retrieve the
         *          texture role associated to the new state using the `getColorRole` class
         *          method.
         * @param reason - the focus out reason to handle.
         * @return - `true` if the internal state has been modified by the focus out reason
         *           and `false` otherwise.
         */
        bool
        handleFocusOut(const engine::FocusEvent::Reason& reason);

        /**
         * @brief - Returns the associated color role with the current focus state described
         *          by this object.
         * @return - a color role describing the role to associate to a texture supposedly
         *           representing this focus state.
         */
        engine::Palette::ColorRole
        getColorRole() const noexcept;

        /**
         * @brief - Transforms this state into a human readable string.
         * @return - a human readable string representing this state.
         */
        std::string
        toString() const noexcept;

      private:

        /**
         * @brief - Internal enumeration used to represent a focus state. Each value is sorted
         *          in order of importance and usually receiving a new action with a state
         *          higher than the current one will trigger an update of the internal state.
         */
        enum class State {
          None,       //<! - No active focus.
          Hover,      //<! - Focus through mouse hovering over only.
          Tab,        //<! - Focus through tab key activation.
          Click       //<! - Focus through mouse click.
        };

        /**
         * @brief - Used to retrieve a human readable name from the internal state describing
         *          this focus object.
         * @param state - the state from which a human readable name should be retrieved.
         * @return - a string representing the input state.
         */
        static
        std::string
        getNameFromState(const State& state) noexcept;

        /**
         * @brief - Retrieves the internal state to associate to a focus reason. This state
         *          can then be compared to the internal state and updated if needed.
         *          Not all focus reason have an associated state, in which case the default
         *          `None` value is used. Some reasons might be associated to the same state.
         * @param reason - the focus reason to transform into an internal state.
         * @return - the internal state associated to the input focus reason.
         */
        static
        State
        getStateFromFocusReason(const engine::FocusEvent::Reason& reason) noexcept;

        /**
         * @brief - Compares `lhs` with `rhs` and returns `true` if `lhs` is smaller to the
         *          `rhs` value. Basically returns `true` if `lhs < rhs` and `false` otherwise.
         * @param lhs - the first value to compare.
         * @param rhs - the second value to compare.
         * @return - `true` if `lhs < rhs`, `false` otherwise.
         */
        static
        bool
        isLessThan(const State& lhs, const State& rhs) noexcept;


      private:

        /**
         * @brief - The internal state used to represent the focus state of this object. This
         *          state is updated when a call to `handleFocus` provides a reason that can
         *          override the current set value.
         */
        State m_state;

    };

    using FocusStateShPtr = std::shared_ptr<FocusState>;
  }
}

std::ostream&
operator<<(const sdl::core::FocusState& state, std::ostream& out) noexcept;

std::ostream&
operator<<(std::ostream& out, const sdl::core::FocusState& state) noexcept;

# include "FocusState.hxx"

#endif    /* FOCUS_STATE_HH */
