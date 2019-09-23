#ifndef    SIZE_POLICY_HH
# define   SIZE_POLICY_HH

# include <memory>
# include <core_utils/CoreFlag.hh>

namespace sdl {
  namespace core {

    namespace size {

      /**
       * @brief - Policies available to represent a size management strategy.
       */
      enum class Policy {
        Grow,
        Expand,
        Shrink,
        Ignore,
      };

      constexpr int count = 4;

      /**
       * @brief - Retrieves a human readable name from the size policy.
       * @param policy - a flag containing the policy for which a name
       *                 should be provided.
       * @return - a string describing the names of the policy registered
       *           in the input argument.
       */
      std::string
      getNameFromPolicy(const Policy& policy) noexcept;

    }

    class SizePolicy {
      public:

        /**
         * @brief - Convenience name to group in a human readable way the individual
         *          flags used to describe somewhat complex size strategies.
         */
        enum class Name {
          Fixed,
          Minimum,
          Maximum,
          Preferred,
          Expanding,
          MinimumExpanding,
          Ignored
        };

      public:

        /**
         * @brief - Creates a size policy with the input strategies for both
         *          horizontal and vertical behavior. Each bit corresponding
         *          to the input policies will be set to `true`.
         *          Note that stretches values will be set to the default.
         * @param hPolicy - the horizontal size policy for this object.
         * @param vPolicy - the vertical size policy for this object.
         */
        SizePolicy(const Name& hPolicy = Name::Fixed,
                   const Name& vPolicy = Name::Fixed);

        /**
         * @brief - Destruction of the object.
         */
        ~SizePolicy() = default;

        /**
         * @brief - Assigns a new horizontal policy for this object.
         * @param policy - the name of the policy to assign as vertical strategy
         *                 to this object.
         */
        void
        setHorizontalPolicy(const Name& policy) noexcept;

        /**
         * @brief - Retrievs the horizontal stretch for this policy.
         * @return - the horizontal stretch value.
         */
        const float&
        getHorizontalStretch() const noexcept;

        /**
         * @brief - Used to assign a new value for the horizontal stretch.
         * @param stretch - the new horizontal stretch value to assign.
         */
        void
        setHorizontalStretch(const float& stretch) noexcept;

        /**
         * @brief - Assigns a new vertical policy for this object.
         * @param policy - the name of the policy to assign as horizontal strategy
         *                 to this object.
         */
        void
        setVerticalPolicy(const Name& policy) noexcept;

        /**
         * @brief - Retrievs the vertical stretch for this policy.
         * @return - the vertical stretch value.
         */
        const float&
        getVerticalStretch() const noexcept;

        /**
         * @brief - Used to assign a new value for the vertical stretch.
         * @param stretch - the new vertical stretch value to assign.
         */
        void
        setVerticalStretch(const float& stretch) noexcept;

        /**
         * @brief - Determines whether this size policy object is fixed
         *          along the horizontal axis, meaning that it can neither
         *          shrink nor grow.
         * @return - `true` if this policy does not allow for growth nor
         *           shrink along the horizontal axis.
         */
        bool
        isFixedHorizontally() const noexcept;

        /**
         * @brief - Determines whether this size policy object can shrink
         *          along the horizontal axis.
         * @return - `true` if the size policy can shrink horizontally and
         *           `false` otherwise.
         */
        bool
        canShrinkHorizontally() const noexcept;

        /**
         * @brief - Determines whether this size policy object can grow
         *          along the horizontal axis.
         * @return - `true` if the size policy can grow horizontally and
         *           `false` otherwise.
         */
        bool
        canGrowHorizontally() const noexcept;

        /**
         * @brief - Determines whether this size policy object can expand
         *          along the horizontal axis.
         * @return - `true` if the size policy can expand horizontally and
         *           `false` otherwise.
         */
        bool
        canExpandHorizontally() const noexcept;

        /**
         * @brief - Determines whether this size policy object can either
         *          grow or expand along the horizontal axis.
         * @return - `true` if the size policy can extend horizontally and
         *           `false` otherwise.
         */
        bool
        canExtendHorizontally() const noexcept;

        /**
         * @brief - Determines whether this size policy object is fixed
         *          along the vertical axis, meaning that it can neither
         *          shrink nor grow.
         * @return - `true` if this policy does not allow for growth nor
         *           shrink along the vertical axis.
         */
        bool
        isFixedVertically() const noexcept;

        /**
         * @brief - Determines whether this size policy object can shrink
         *          along the vertical axis.
         * @return - `true` if the size policy can shrink vertically and
         *           `false` otherwise.
         */
        bool
        canShrinkVertically() const noexcept;

        /**
         * @brief - Determines whether this size policy object can grow
         *          along the vertical axis.
         * @return - `true` if the size policy can grow vertically and
         *           `false` otherwise.
         */
        bool
        canGrowVertically() const noexcept;

        /**
         * @brief - Determines whether this size policy object can expand
         *          along the vertical axis.
         * @return - `true` if the size policy can expand vertically and
         *           `false` otherwise.
         */
        bool
        canExpandVertically() const noexcept;

        /**
         * @brief - Determines whether this size policy object can either
         *          grow or expand along the vertical axis.
         * @return - `true` if the size policy can extend vertically and
         *           `false` otherwise.
         */
        bool
        canExtendVertically() const noexcept;

      private:

        class SizePolicyFlag: public utils::CoreFlag<size::count> {
        public:

          /**
           * @brief - Creates a default size policy flag with no active growth strategy.
           *          This is basically equivalent to the `Fixed` named policy.
           */
          SizePolicyFlag();

          /**
           * @brief - Creates a size policy flag with the specified strategy intialized
           *          to `true`.
           * @param policy - the size policy which should be activated.
           */
          SizePolicyFlag(const size::Policy& policy);

          /**
           * @brief - Destruction of the object.
           */
          ~SizePolicyFlag() = default;

          /**
           * @brief - Determines whether this size policy object can shrink.
           * @return - `true` if the size policy can shrink and `false` otherwise.
           */
          bool
          canShrink() const noexcept;

          /**
           * @brief - Determines whether this size policy object can grow.
           * @return - `true` if the size policy can grow and `false` otherwise.
           */
          bool
          canGrow() const noexcept;

          /**
           * @brief - Determines whether this size policy object can expand.
           * @return - `true` if the size policy can expand and `false` otherwise.
           */
          bool
          canExpand() const noexcept;

          /**
           * @brief - Determines whether this size policy object can either
           *          grow or expand.
           * @return - `true` if the size policy can extend and `false` otherwise.
           */
          bool
          canExtend() const noexcept;

        private:

          /**
           * @brief - Used to initialize the size policy flag bits in order to be
           *          able to easily use this flag. Basically registers each individual
           *          flag value in the base class.
           */
          void
          init();

          /**
           * @brief - Attemps to retrieve the bit index provided for the input
           *          size strategy when registereing it through the `addNamedBit`
           *          interface. If no such information is available an error is
           *          raised.
           * @param policy - the size strategy for which the bit index should be retrieved.
           * 
           * @return - the index of the bit in the base class as returned by the
           *           `addNamedBit` method.
           */
          int
          getBitID(const size::Policy& policy) const;

          /**
           * @brief - Used to register the input `policy` enumeration value in the
           *          parent class through the `addNamedBit` interface. Also insert
           *          the returned bit index in the internal `m_strategiesToIDs`.
           *          Note that the size strategy is registered with an initial value
           *          and a default value of `false`.
           * @para m policy - the size policy to register to the base class.
           */
          void
          registerSizePolicy(const size::Policy& policy);

        private:

          using StrategiesTable = std::unordered_map<size::Policy, int>;

          /**
           * @brief - Describes the association between a given size strategy to its
           *          identifier in the base class bits array.
           *          This map is populated by the `init` function upon building any
           *          `SizePolicyFlag` object.
           */
          StrategiesTable m_strategiesToIDs;
        };

        /**
         * @brief - Used to create a size policy flag from the input size policy
         *          name Assign each needed individual bit to create the flag that
         *          does correspond to the input name.
         * @param name - the name of the policy which should be converted into a
         *               size policy flag.
         * @return - a size policy flag representing the human readable size policy
         *           name.
         */
        static
        SizePolicyFlag
        initFromName(const Name& name);

      private:

        using GrowthStrategyTable = std::unordered_map<size::Policy, int>;

        /**
         * @brief - Describes the flag containing the size strategy along the
         *          horizontal axis.
         */
        SizePolicyFlag m_hPolicy;

        /**
         * @brief - A floating point value describing some quantifications of
         *          the additional data to be received by this object compared
         *          to other elements in the same layer.
         *          The default value `0` indicates that this item should only
         *          receive additional space if no any other widget can make
         *          use of it. A larger value indicates a more pressant appetite
         *          for additional space.
         *          Note that a value of `0` means that the size policy will not
         *          claim any additional space when used in the same layer as
         *          another policy defining a non zero value.
         */
        float m_hStretch;

        /**
         * @brief - Describes the flag containing the size strategy along the
         *          vertical axis.
         */
        SizePolicyFlag m_vPolicy;

        /**
         * @brief - Same as `m_hStretch` but for vertical space.
         */
        float m_vStretch;
    };

    using SizePolicyShPtr = std::shared_ptr<SizePolicy>;
  }
}

# include "SizePolicy.hxx"

#endif    /* SIZE_POLICY_HH */
