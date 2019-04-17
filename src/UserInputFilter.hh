#ifndef    USER_INPUT_FILTER_HH
# define   USER_INPUT_FILTER_HH

# include <sdl_engine/Event.hh>
# include <sdl_engine/EngineObject.hh>

namespace sdl {
  namespace core {

    class UserInputFilter: public engine::EngineObject {
      public:

        struct Interaction {
          using Mask = unsigned char;

          static constexpr Mask NoInteraction = 0x00;

          static constexpr Mask KeyPressed = 0x01;
          static constexpr Mask KeyReleased = 0x02;
          static constexpr Mask Key = KeyPressed | KeyReleased;

          static constexpr Mask MouseButtonPressed = 0x04;
          static constexpr Mask MouseButtonReleased = 0x08;
          static constexpr Mask MouseButton = MouseButtonPressed | MouseButtonReleased;

          static constexpr Mask MouseMotion = 0x10;

          static constexpr Mask MouseWheelDown = 0x20;
          static constexpr Mask MouseWheelUp = 0x40;
          static constexpr Mask MouseWheel = MouseWheelDown | MouseWheelUp;

          static constexpr Mask Mouse = MouseButton | MouseMotion | MouseWheel;

          static constexpr Mask Quit = 0x80;

          static constexpr Mask FullInteraction = Key | Mouse | Quit;

        };

      public:

        UserInputFilter(const Interaction::Mask& mask,
                        const bool exclusion = false,
                        const std::string& name = std::string("UserInputFilter"));

        virtual ~UserInputFilter();

        bool
        filterEvent(engine::EngineObject* watched,
                    engine::EventShPtr e) override;

        // Create a filter which filters out the event related to the input mask.
        static
        engine::EngineObjectShPtr
        createFilterFromMask(const Interaction::Mask& mask);

        // Create a filter which filters out the event which ARE NOT related to
        // the input mask.
        static
        engine::EngineObjectShPtr
        createExclusionFilterFromMask(const Interaction::Mask& mask);

      private:

        bool
        isFiltered(const engine::Event::Type& type) const noexcept;

        bool
        filtering(const Interaction::Mask& mask) const noexcept;

      private:

        Interaction::Mask m_mask;
        bool m_exclusion;

    };

  }
}

# include "UserInputFilter.hxx"

#endif    /* USER_INPUT_FILTER_HH */
