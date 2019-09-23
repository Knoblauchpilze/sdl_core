#ifndef    FOCUS_POLICY_HXX
# define   FOCUS_POLICY_HXX

# include "FocusPolicy.hh"

namespace sdl {
  namespace core {

    namespace focus {

      inline
      std::string
      getNameFromType(const Type& type) noexcept {
        switch (type) {
          case Type::Hover:
            return std::string("Hover");
          case Type::Click:
            return std::string("Click");
          case Type::Tab:
            return std::string("Tab");
          case Type::Wheel:
            return std::string("Wheel");
          default:
            return std::string("Unknown");
        }
      }

    }

    inline
    FocusPolicy::FocusPolicy():
      utils::CoreFlag<focus::count>(std::string("focus_policy")),
      m_typesToIDs()
    {
      init();
    }

    inline
    FocusPolicy::FocusPolicy(const focus::Type& type):
      utils::CoreFlag<focus::count>(std::string("focus_policy")),
      m_typesToIDs()
    {
      init();

      set(getBitID(type));
    }

    inline
    FocusPolicy::FocusPolicy(const Name& name):
      utils::CoreFlag<focus::count>(std::string("focus_policy")),
      m_typesToIDs()
    {
      init();

      bool valid = true;
      switch (name) {
        case Name::StrongFocus:
          set(getBitID(focus::Type::Hover));
          set(getBitID(focus::Type::Click));
          set(getBitID(focus::Type::Tab));
          set(getBitID(focus::Type::Wheel));
          break;
        default:
          valid = false;
          break;
      }

      if (!valid) {
        throw utils::CoreException(
          std::string("Could not create focus policy with name ") + std::to_string(static_cast<int>(name)),
          std::string("Constructor"),
          std::string("FocusPolicy"),
          std::string("Unhandled focus policy name")
        );
      }
    }

    inline
    bool
    FocusPolicy::canGrabHoverFocus() const noexcept {
      return isSet(getBitID(focus::Type::Hover));
    }

    inline
    bool
    FocusPolicy::canGrabClickFocus() const noexcept {
      return isSet(getBitID(focus::Type::Click));
    }

    inline
    bool
    FocusPolicy::canGrabTabFocus() const noexcept {
      return isSet(getBitID(focus::Type::Tab));
    }

    inline
    bool
    FocusPolicy::canGrabWheelFocus() const noexcept {
      return isSet(getBitID(focus::Type::Wheel));
    }

    inline
    void
    FocusPolicy::init() {
      // Register all focus types.
      registerFocusType(focus::Type::Hover);
      registerFocusType(focus::Type::Click);
      registerFocusType(focus::Type::Tab);
      registerFocusType(focus::Type::Wheel);
    }

    inline
    int
    FocusPolicy::getBitID(const focus::Type& type) const {
      // Find the corresponding focus type in the internal table.
      FocusTypesTable::const_iterator it = m_typesToIDs.find(type);

      // Check for errors.
      if (it == m_typesToIDs.cend()) {
        throw utils::CoreException(
          std::string("Could not get bit index for \"") + focus::getNameFromType(type) + "\"",
          std::string("getBitID"),
          std::string("FocusPolicy"),
          std::string("No such bit registered")
        );
      }

      // Return the corresponding index.
      return it->second;
    }

    inline
    void
    FocusPolicy::registerFocusType(const focus::Type& type) {
      // Register the name corresponding to the input type with false value and default
      // value.
      int id = addNamedBit(focus::getNameFromType(type), false, false);

      // Register the returned index to easily retrieve its value later on.
      m_typesToIDs[type] = id;
    }

  }
}

#endif    /* FOCUS_POLICY_HXX */
