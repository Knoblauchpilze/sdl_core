
# include "SdlWidget.hh"

# include <core_utils/CoreLogger.hh>

namespace sdl {
  namespace core {

    SdlWidget::SdlWidget(const std::string& name,
                         const utils::maths::Sizef& sizeHint,
                         SdlWidget* parent,
                         const bool transparent,
                         const Palette& palette):
      EventListener(EventListener::Interaction::MouseButtonReleased),

      m_name(name),
      m_parent(nullptr),
      m_minSize(),
      m_sizeHint(sizeHint),
      m_maxSize(utils::maths::Sizef::max()),
      m_area(utils::maths::Boxf(0.0f, 0.0f, sizeHint.w(), sizeHint.h())),
      m_palette(palette),

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
          SDL_BLENDFACTOR_ZERO,                // srcAlphaFactor
          SDL_BLENDFACTOR_ONE,                 // dstAlphaFactor
          SDL_BLENDOPERATION_ADD               // alphaOperation
        )
      ),

      m_contentDirty(true),
      m_geometryDirty(true),
      m_isVisible(true),
      m_transparent(transparent),
      m_content(nullptr),
      m_drawingLocker(),

      m_children(),

      m_sizePolicy(),
      m_layout()
    {
      // Register the parent widget: if a layout is registered in the parent widget
      // we can use this, otherwise use the regular method.
      if (parent != nullptr && parent->m_layout != nullptr) {
        parent->m_layout->addItem(this);
      }
      else {
        setParent(parent);
      }
    }

    SDL_Texture*
    SdlWidget::draw(SDL_Renderer* renderer) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      // Check whether a valid size is provided for this widget.
      if (!m_area.valid()) {
        throw SdlException(std::string("Could not repaint widget \"") + getName() + "\", invalid size");
      }

      // Repaint if needed.
      if (hasContentChanged()) {
        utils::core::Logger::getInstance().logDebug(
          std::string("Updating content for widget \"") + getName() + "\"",
          std::string("widget")
        );

        clearTexture();
        m_content = createContentPrivate(renderer);
        m_contentDirty = false;
      }

      // Clear the content and draw the new version.
      clearContentPrivate(renderer, m_content);
      drawContentPrivate(renderer, m_content);

      // Save the current state of the renderer.
      RendererState state(renderer);

      // The rendering target is now set to 'm_content'.
      SDL_SetRenderTarget(renderer, m_content);

      // Update layout if any.
      if (hasGeometryChanged()) {
        utils::core::Logger::getInstance().logDebug(
          std::string("Updating layout for widget \"") + getName() + "\"",
          std::string("widget")
        );

        if (m_layout != nullptr) {
          m_layout->update();
        }
        m_geometryDirty = false;
      }

      // Proceed to update of children containers if any.
      for (WidgetMap::const_iterator child = m_children.cbegin() ; child != m_children.cend() ; ++child) {
        if (child->second->isVisible()) {
          drawChild(renderer, *child->second);
        }
      }

      // Return the built-in texture.
      return m_content;
    }

  }
}
