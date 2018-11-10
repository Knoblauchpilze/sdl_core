
# include "SdlWidget.hh"

namespace sdl {
  namespace core {

    SdlWidget::SdlWidget(const std::string& name,
                         const Boxf& area,
                         SdlWidget* parent,
                         const SDL_Color& color):
      SdlEventListener(SdlEventListener::Interaction::MouseButtonReleased),

      m_name(name),
      m_parent(parent),
      m_area(area),
      m_background(color),

      m_dirty(true),
      m_isDrawable(true),
      m_content(nullptr),
      m_drawingLocker(),

      m_children(),

      m_layout()
    {
      // Add a child to the parent widget if any.
      if (parent != nullptr) {
        parent->addWidget(this);
      }
    }

    SDL_Texture*
    SdlWidget::draw(SDL_Renderer* renderer) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Repaint if needed.
      if (hasChanged()) {
        clearTexture();
        m_content = createContentPrivate(renderer);
        m_dirty = false;
      }

      SDL_Texture* currentTarget = SDL_GetRenderTarget(renderer);
      SDL_SetRenderTarget(renderer, m_content);

      // Update layout if any.
      if (m_layout != nullptr) {
        m_layout->update(m_area);
      }

      // Proceed to update of children containers if any.
      for (WidgetMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
        try {
          if (child->second->isDrawable()) {
            drawChild(renderer, *child->second);
          }
        }
        catch (const SdlException& e) {
          std::cerr << "[WIDGET] Caught internal exception while repainting child " << child->first
                    << " for container " << getName()
                    << std::endl << e.what()
                    << std::endl;
        }
      }

      SDL_SetRenderTarget(renderer, currentTarget);

      // Return the built-in texture.
      return m_content;
    }

  }
}
