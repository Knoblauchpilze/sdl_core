#ifndef    WIDGET_FACTORY_HH
# define   WIDGET_FACTORY_HH

# include <memory>
# include <core_utils/CoreLogger.hh>
# include <maths_utils/Size.hh>

// # include "SdlWidget.hh"
# include "Palette.hh"
# include "Color.hh"

namespace sdl {
  namespace core {

    class SdlWidget;

    class WidgetFactory {
      public:

        WidgetFactory(utils::core::LoggerShPtr logger = nullptr);

        ~WidgetFactory() = default;

        void
        setLogger(utils::core::LoggerShPtr logger) noexcept;

        void
        setPalette(const Palette& palette) noexcept;

        std::shared_ptr<SdlWidget>
        createRootWidget(const std::string& name,
                         const utils::maths::Sizef& sizeHint = utils::maths::Sizef(),
                         const Color& bgColor = Color()) const;

        SdlWidget*
        createWidget(const std::string& name,
                     const utils::maths::Sizef& sizeHint = utils::maths::Sizef(),
                     const Color& bgColor = Color(),
                     SdlWidget* parent = nullptr) const;

      private:

        utils::core::LoggerShPtr m_logger;
        Palette m_palette;

    };

    using WidgetFactoryShPtr = std::shared_ptr<WidgetFactory>;
  }
}

# include "WidgetFactory.hxx"

#endif    /* WIDGET_FACTORY_HH */
