
# include "WidgetBuilder.hh"
# include "SdlWidget.hh"

namespace sdl {
  namespace core {

    SdlWidget*
    WidgetBuilder::build(const std::string& name) const {
      return new SdlWidget(
        name,
        m_sizeHint,
        m_parent,
        false,
        m_palette,
        m_logger
      );
    }

  }
}
