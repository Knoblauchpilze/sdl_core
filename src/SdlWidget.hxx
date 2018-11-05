#ifndef    SDLWIDGET_HXX
# define   SDLWIDGET_HXX

# include "SdlWidget.hh"
# include "SdlException.hh"

namespace sdl {
  namespace core {

    inline
    SdlWidget::~SdlWidget() {
      
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      clearTexture();
      
    }

    inline
    const std::string&
    SdlWidget::getName() const noexcept {
      return m_name;
    }

    inline
    const Boxf&
    SdlWidget::getRenderingArea() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_area;
    }

    inline
    void
    SdlWidget::setRenderingArea(const Boxf& area) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_area = area;
      makeDirty();
    }

    inline
    void
    SdlWidget::setBackgroundColor(const SDL_Color& color) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_background = color;
      makeDirty();
    }

    inline
    void
    SdlWidget::addWidget(std::shared_ptr<SdlWidget> child) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      if (child == nullptr) {
        throw SdlException(std::string("Cannot add null child to \"") + getName() + "\"");
      }

      m_children[child->getName()] = child;
      child->setParent(this);

      if (m_layout != nullptr) {
        m_layout->addItem(child);
      }

      makeDirty();
    }

    inline
    void
    SdlWidget::addWidget(std::shared_ptr<SdlWidget> child,
                         const unsigned& x,
                         const unsigned& y,
                         const unsigned& w,
                         const unsigned& h)
    {
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      if (child == nullptr) {
        throw SdlException(std::string("Cannot add null child to \"") + getName() + "\"");
      }

      m_children[child->getName()] = child;
      child->setParent(this);

      if (m_layout != nullptr) {
        m_layout->addItem(child, x, y, w, h);
      }

      makeDirty();
    }

    inline
    void
    SdlWidget::removeWidget(std::shared_ptr<SdlWidget> child) {
      if (child == nullptr) {
        throw SdlException(std::string("Cannot remove null child from \"") + getName() + "\"");
      }
      removeWidget(child->getName());
    }

    inline
    void
    SdlWidget::removeWidget(const std::string& name) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);

      m_children.erase(name);

      makeDirty();
    }

    inline
    unsigned
    SdlWidget::getWidgetsCount() const noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      return m_children.size();
    }

    inline
    void
    SdlWidget::setLayout(std::shared_ptr<SdlLayout> layout) noexcept {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_layout = layout;
      makeDirty();
    }

    inline
    bool
    SdlWidget::hasChanged() const noexcept {
      return m_dirty;
    }

    inline
    SDL_Texture*
    SdlWidget::createContentPrivate(SDL_Renderer* renderer) const {
      const SDL_Rect areaAsRect = m_area.toSDLRect();

      SDL_Texture* content = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        areaAsRect.w,
        areaAsRect.h
      );

      if (content == nullptr) {
        throw SdlException(std::string("Could not create content for widget \"") + getName() + "\" (err: \"" + SDL_GetError() + "\")");
      }

      // Clear the content (i.e. fill with color).
      clearContentPrivate(renderer, content);
      
      return content;
    }

    inline
    void
    SdlWidget::clearContentPrivate(SDL_Renderer* renderer, SDL_Texture* texture) const noexcept {
      SDL_Texture* currentTarget = SDL_GetRenderTarget(renderer);
      SDL_Color currentDrawColor;
      SDL_GetRenderDrawColor(renderer, &currentDrawColor.r, &currentDrawColor.g, &currentDrawColor.b, &currentDrawColor.a);
      SDL_SetRenderTarget(renderer, texture);
      SDL_SetRenderDrawColor(renderer, m_background.r, m_background.g, m_background.b, m_background.a);
      SDL_RenderClear(renderer);
      SDL_SetRenderTarget(renderer, currentTarget);
      SDL_SetRenderDrawColor(renderer, currentDrawColor.r, currentDrawColor.g, currentDrawColor.b, currentDrawColor.a);
    }

    inline
    void
    SdlWidget::setParent(SdlWidget* parent) {
      std::lock_guard<std::mutex> guard(m_drawingLocker);
      m_parent = parent;
    }

    inline
    void
    SdlWidget::makeDirty() noexcept {
      m_dirty = true;
    }

    inline
    std::mutex&
    SdlWidget::getLocker() noexcept {
      return m_drawingLocker;
    }

    inline
    void
    SdlWidget::clearTexture() {
      if (m_content != nullptr) {
        SDL_DestroyTexture(m_content);
      }
    }

    inline
    void
    SdlWidget::drawChild(SDL_Renderer* renderer, SdlWidget& child) {
      // Draw this object (caching is handled by the object itself).
      SDL_Texture* picture = child.draw(renderer);

      // Draw the picture at the corresponding place.
      const Boxf& render = child.getRenderingArea();
      SDL_Rect dstArea = render.toSDLRect();

      if (picture != nullptr) {
        SDL_RenderCopy(renderer, picture, nullptr, &dstArea);
      }
    }

  }
}

#endif    /* SDLWIDGET_HXX */
