#ifndef    RENDERERSTATE_HXX
# define   RENDERERSTATE_HXX

# include "RendererState.hh"

namespace sdl {
  namespace core {

    inline
    RendererState::RendererState(SDL_Renderer* renderer):
      m_renderer(renderer),
      m_color(),
      m_texture(SDL_GetRenderTarget(renderer))
    {
      SDL_GetRenderDrawColor(m_renderer, &m_color.r, &m_color.g, &m_color.b, &m_color.a);
    }

    inline
    RendererState::~RendererState() {
      // Restore old renderer state.
      SDL_SetRenderDrawColor(m_renderer, m_color.r, m_color.g, m_color.b, m_color.a);
      SDL_SetRenderTarget(m_renderer, m_texture);
    }

  }
}

#endif    /* RENDERERSTATE_HXX */