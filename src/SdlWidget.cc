
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

      // This custom blend mode is mainly used to be able to have additive alpha blending in children widget.
      // Basically if a widget has a transparency of 128 and one of its children has also a transparency of 128,
      // we would want the final pixel to have a transparency of 64 (i.e. one fourth of totally opaque).
      // This cannot be achieved using any of the proposed blend modes so we have to rely on custom blend mode.
      // The custom blend mode can be composed as follows:
      // dstRGB = colorOperation(srcRGB * srcColorFactor, dstRGB * dstColorFactor)
      // dstA = alphaOperation(srcA * srcAlphaFactor, dstA * dstAlphaFactor)
      m_blendMode(
        SDL_ComposeCustomBlendMode(
          SDL_BLENDFACTOR_SRC_ALPHA,           // srcColorFactor
          SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, // dstColorFactor
          SDL_BLENDOPERATION_ADD,              // colorOperation
          SDL_BLENDFACTOR_DST_ALPHA,           // srcAlphaFactor
          SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, // dstAlphaFactor
          SDL_BLENDOPERATION_ADD               // alphaOperation
        )
      ),

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
      else {
        clearContentPrivate(renderer, m_content);
        drawContentPrivate(renderer, m_content);
      }

      // Save the current state of the renderer.
      RendererState state(renderer);
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

      // Return the built-in texture.
      return m_content;
    }

  }
}
