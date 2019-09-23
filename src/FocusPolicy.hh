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
        Hover,
        Click,
        Tab,
        Wheel
      };

      constexpr int count = 4;

      /**
       * @brief - Retrieves a human readable name from the input focus type.
       * @param modifiers - a flag containing several modifiers for which a
       *                    name should be provided.
       * @return - a string describing the names of the modifiers registered
       *           in the input argument.
       */
      std::string
      getNameFromType(const Type& type) noexcept;

    }

    class FocusPolicy: public utils::CoreFlag<focus::count> {
      public:

        /**
         * @brief - Convenience enumeration allowing to create a focus policy
         *          from a group of individual flags aliased under a human
         *          readable name rather than a grouping of flags.
         */
        enum class Name {
          StrongFocus
        };

      public:

        /**
         * @brief - Creates a default focus policy with no active type.
         */
        FocusPolicy();

        /**
         * @brief - Creates a focus policy with the specified type intialized
         *          to `true`.
         * @param type - the focus type which should be activated.
         */
        FocusPolicy(const focus::Type& type);

        /**
         * @brief - Creates a focus policy gathering all the individual flags
         *          defined for the specified policy name.
         * @param name - the name of the policy to use to create this object.
         */
        FocusPolicy(const Name& name);

        /**
         * @brief - Destruction of the object.
         */
        ~FocusPolicy() = default;

        /**
         * @brief - Returns `true` if this focus policy can grab hover focus
         *          and `false` otherwise.
         * @return - `true` if the hover over focus can be handled, `false`
         *           otherwise.
         */
        bool
        canGrabHoverFocus() const noexcept;

        /**
         * @brief - Returns `true` if this focus policy can grab click focus
         *          and `false` otherwise.
         * @return - `true` if the click focus can be handled, `false` otherwise.
         */
        bool
        canGrabClickFocus() const noexcept;

        /**
         * @brief - Returns `true` if this focus policy can grab tab focus
         *          and `false` otherwise.
         * @return - `true` if the tab focus can be handled, `false` otherwise.
         */
        bool
        canGrabTabFocus() const noexcept;

        /**
         * @brief - Returns `true` if this focus policy can grab mouse wheel
         *          focus and `false` otherwise.
         * @return - `true` if the mouse wheel focus can be handled, `false`
         *           otherwise.
         */
        bool
        canGrabWheelFocus() const noexcept;

      private:

        /**
         * @brief - Used to initialize the key modifier bits in order to be
         *          able to easily use this flag. Basically registers each
         *          individual flag value in the base class.
         */
        void
        init();

        /**
         * @brief - Attemps to retrieve the bit index provided for the input
         *          focus type when registereing it through the `addNamedBit`
         *          interface. If no such information is available an error is
         *          raised.
         * @param type - the focus type for which the bit index should be retrieved.
         * 
         * @return - the index of the bit in the base class as returned by the
         *           `addNamedBit` method.
         */
        int
        getBitID(const focus::Type& type) const;

        /**
         * @brief - Used to register the input `type` enumeration value in the
         *          parent class through the `addNamedBit` interface. Also insert
         *          the returned bit index in the internal `m_typesToIDs`.
         *          Note that the focus type is registered with an initial value
         *          and a default value of `false`.
         * @param type - the focus type to register to the base class.
         */
        void
        registerFocusType(const focus::Type& type);

      private:

        using FocusTypesTable = std::unordered_map<focus::Type, int>;

        /**
         * @brief - Describes the association between a given focus type to its
         *          identifier in the base class bits array.
         *          This map is populated by the `init` function upon building
         *          any `FocusPolicy` object.
         */
        FocusTypesTable m_typesToIDs;
    };

    using FocusPolicyShPtr = std::shared_ptr<FocusPolicy>;
  }
}

# include "FocusPolicy.hxx"

#endif    /* FOCUS_POLICY_HH */
