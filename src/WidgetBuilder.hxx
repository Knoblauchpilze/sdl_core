#ifndef    WIDGET_BUILDER_HXX
# define   WIDGET_BUILDER_HXX

# include "WidgetBuilder.hh"

namespace sdl {
  namespace core {

    inline
    WidgetBuilder::WidgetBuilder(utils::core::LoggerShPtr logger):
      m_logger(logger),
      m_palette(),
      m_sizeHint(),
      m_parent(nullptr)
    {}

    inline
    WidgetBuilder&
    WidgetBuilder::setLogger(utils::core::LoggerShPtr logger) noexcept {
      m_logger = logger;
      return *this;
    }

    inline
    WidgetBuilder&
    WidgetBuilder::setPalette(const Palette& palette) noexcept {
      m_palette = palette;
      return *this;
    }

    inline
    WidgetBuilder&
    WidgetBuilder::setSizeHint(const utils::maths::Sizef& hint) noexcept {
      m_sizeHint = hint;
      return *this;
    }

    inline
    WidgetBuilder&
    WidgetBuilder::setParent(SdlWidget* parent) noexcept {
      m_parent = parent;
      return *this;
    }

  }
}

#endif    /* WIDGET_BUILDER_HXX */
