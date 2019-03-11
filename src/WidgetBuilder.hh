#ifndef    WIDGET_BUILDER_HH
# define   WIDGET_BUILDER_HH

# include <memory>
# include <core_utils/CoreLogger.hh>
# include <maths_utils/Size.hh>

# include "Palette.hh"

namespace sdl {
  namespace core {

    class SdlWidget;

    class WidgetBuilder {
      public:

        WidgetBuilder(utils::core::LoggerShPtr logger = nullptr);

        ~WidgetBuilder() = default;

        WidgetBuilder&
        setLogger(utils::core::LoggerShPtr logger) noexcept;

        WidgetBuilder&
        setPalette(const Palette& palette) noexcept;

        WidgetBuilder&
        setSizeHint(const utils::maths::Sizef& hint) noexcept;

        WidgetBuilder&
        setParent(SdlWidget* parent) noexcept;

        SdlWidget*
        build(const std::string& name) const;

      private:

        utils::core::LoggerShPtr m_logger;
        Palette m_palette;
        utils::maths::Sizef m_sizeHint;
        SdlWidget* m_parent;

    };

    using WidgetBuilderShPtr = std::shared_ptr<WidgetBuilder>;
  }
}

# include "WidgetBuilder.hxx"

#endif    /* WIDGET_BUILDER_HH */
