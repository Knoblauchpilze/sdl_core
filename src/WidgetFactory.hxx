#ifndef    WIDGET_FACTORY_HXX
# define   WIDGET_FACTORY_HXX

# include "WidgetFactory.hh"

namespace sdl {
  namespace core {

    inline
    WidgetFactory::WidgetFactory(utils::core::LoggerShPtr logger):
      m_logger(logger)
    {}

    inline
    void
    WidgetFactory::setLogger(utils::core::LoggerShPtr logger) noexcept {
      m_logger = logger;
    }

    inline
    void
    WidgetFactory::setPalette(const Palette& palette) noexcept {
      m_palette = palette;
    }

    inline
    SdlWidgetShPtr
    WidgetFactory::createRootWidget(const std::string& name,
                                    const utils::maths::Sizef& sizeHint,
                                    const Color& bgColor) const
    {
      return std::make_shared<SdlWidget>(
        name,
        sizeHint,
        nullptr,
        false,
        Palette::fromBackgroundColor(bgColor),
        m_logger,
        this
      );
    }

    inline
    SdlWidget*
    WidgetFactory::createWidget(const std::string& name,
                                const utils::maths::Sizef& sizeHint,
                                const Color& bgColor,
                                SdlWidget* parent) const
    {
      return new SdlWidget(
        name,
        sizeHint,
        parent,
        false,
        Palette::fromBackgroundColor(bgColor),
        m_logger,
        this
      );
    }
  }
}

#endif    /* WIDGET_FACTORY_HXX */
