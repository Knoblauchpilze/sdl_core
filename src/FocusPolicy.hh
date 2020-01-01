#ifndef    FOCUS_POLICY_HH
# define   FOCUS_POLICY_HH

# include <memory>
# include <core_utils/CoreFlag.hh>

namespace sdl {
  namespace core {

    namespace focus {

      /**
       * @brief - Types of focus available.
       */
      enum class Type {
        Hover       = 0,
        Click       = 1,
        Tab         = 2,
        Wheel       = 3,
        ValuesCount = 4
      };

      /**
       * @brief - Retrieves a human readable name from the input focus type.
       * @param type - a flag containing the focus type for which a name
       *               should be provided.
       * @return - a string describing the names of the focus type registered
       *           in the input argument.
       */
      std::string
      getNameFromType(const Type& type) noexcept;

      /**
       * @brief - Convenience enumeration allowing to create a focus policy
       *          from a group of individual flags aliased under a human
       *          readable name rather than a grouping of flags.
       */
      enum class Name {
        StrongFocus
      };

    }

    using FocusPolicy = utils::CoreFlag<focus::Type>;

    /**
     * @brief - Used to create a focus policy flag with flags activated corresponding
     *          to the input name. Each name describes a general property to have for
     *          focus types and the user can easily create some policy with this helper
     *          function.
     * @param name - the name describing the focus policy.
     * @return - a flag with all focus reasons activated.
     */
    FocusPolicy
    createFocusFromName(const focus::Name& name) noexcept;

    /**
     * @brief - Returns `true` if the input focus policy can grab hover focus
     *          and `false` otherwise.
     * @param policy - the focus policy to check for hover acceptance.
     * @return - `true` if the hover over focus can be handled, `false`
     *           otherwise.
     */
    bool
    canGrabHoverFocus(const FocusPolicy& policy) noexcept;

    /**
     * @brief - Returns `true` if the input focus policy can grab click focus
     *          and `false` otherwise.
     * @param policy - the focus policy to check for click acceptance.
     * @return - `true` if the click focus can be handled, `false` otherwise.
     */
    bool
    canGrabClickFocus(const FocusPolicy& policy) noexcept;

    /**
     * @brief - Returns `true` if this focus policy can grab tab focus
     *          and `false` otherwise.
     * @param policy - the focus policy to check for tab acceptance.
     * @return - `true` if the tab focus can be handled, `false` otherwise.
     */
    bool
    canGrabTabFocus(const FocusPolicy& policy) noexcept;

    /**
     * @brief - Returns `true` if the input focus policy can grab mouse wheel
     *          focus and `false` otherwise.
     * @param policy - the focus policy to check for wheel acceptance.
     * @return - `true` if the mouse wheel focus can be handled, `false`
     *           otherwise.
     */
    bool
    canGrabWheelFocus(const FocusPolicy& policy) noexcept;
  }
}

# include "FocusPolicy.hxx"
# include "FocusPolicy_specialization.hxx"

#endif    /* FOCUS_POLICY_HH */
