#ifndef    LAYOUT_ITEM_HH
# define   LAYOUT_ITEM_HH

# include <memory>

# include <maths_utils/Box.hh>
# include <maths_utils/Size.hh>
# include <sdl_engine/EngineObject.hh>

# include "SizePolicy.hh"

namespace sdl {
  namespace core {

    class LayoutItem: public engine::EngineObject {
      public:

        /**
         * @brief - Creates a layout item with the specified name and properties.
         *          A layout item can be considered a `root item` if it is not
         *          embedded into another layer of layout. This has an impact on
         *          the final bboxes produced by this item which will not be
         *          converted if the layout is a top level item.
         *          This comes from the fact that the area provided to a top level
         *          layout item is already converted by the underlying api.
         *          The user can allow or disable various features using this
         *          constructor, and can also choose to turn this layout into a
         *          virtual layout, which means that no events will be generated
         *          upon inserting elements.
         *          This allows to use layouts inside other layouts and to trigger
         *          their recomputations through external means rather than making
         *          it internally handled by the item itself.
         * @param name - the name of the layout item. Used in the `log` calls as the
         *               last identifier in the log line.
         * @param sizeHint - the hint to use for this layout item: default value is
         *                   an invalid hint which indicates that this layout item
         *                   will be assigned the most relevant size by the layout
         *                   engine.
         * @param rootItem - true if this item is not nested into another layout,
         *                   false otherwise. When this value is true it assumes
         *                   that all areas given to the layout item, notably
         *                   through events won't be converted before using them.
         * @param virtualItem - true if this item is part of a virtual layout
         *                      hierarchy, which means that it is handled by
         *                      some other external sources which will trigger
         *                      the recomputations when needed. Thus no events
         *                      will be generated by this layout when inserting
         *                      or removing an item.
         * @param allowLog - true if the log messages should not be discarded,
         *                   false otherwise.
         */
        LayoutItem(const std::string& name,
                   const utils::Sizef& sizeHint = utils::Sizef(),
                   const bool rootItem = false,
                   const bool virtualItem = false,
                   const bool allowLog = false);

        virtual ~LayoutItem();

        utils::Sizef
        getMinSize() const noexcept;

        void
        setMinSize(const utils::Sizef& size) noexcept;

        utils::Sizef
        getSizeHint() const noexcept;

        void
        setSizeHint(const utils::Sizef& hint) noexcept;

        utils::Sizef
        getMaxSize() const noexcept;

        void
        setMaxSize(const utils::Sizef& size) noexcept;

        SizePolicy
        getSizePolicy() const noexcept;

        void
        setSizePolicy(const SizePolicy& policy) noexcept;

        virtual utils::Boxf
        getRenderingArea() const noexcept;

        bool
        isRootItem() const noexcept;

        bool
        isVirtual() const noexcept;

        bool
        isVisible() const noexcept;

        virtual void
        setVisible(bool visible) noexcept;

      protected:

        virtual void
        makeGeometryDirty();

        virtual bool
        hasGeometryChanged() const noexcept;

        virtual
        void
        geometryRecomputed() noexcept;

        virtual
        void
        setRoot(const bool isRoot);

        virtual void
        updatePrivate(const utils::Boxf& window);

        bool
        geometryUpdateEvent(const engine::Event& e) override;

        bool
        resizeEvent(const engine::ResizeEvent& e) override;

      private:

        /**
         * @brief - Describes the size policy for this widget. The policy is described using
         *          several sizes which roles are described below. In addition to that, the
         *          `m_sizePolicy` allows to determine the behavior of the widget when some
         *          extra space is allocated to it or if it should be srhunk for some reason.
         */
        /**
         * @brief - Holds the minimum size which can be assigned to a widget while still making
         *          the widget usable. Any area smaller than this would make the widget useless
         *          as the user would not be able to properly use it.
         */
        utils::Sizef m_minSize;

        /**
         * @brief - Holds a sensible size for the widget which should allow for best ergonomy.
         *          According to the policy one can determine whether some extra space can be
         *          used (or conversely if some missing space is a problem) but it should be
         *          the target size of the widget.
         */
        utils::Sizef m_sizeHint;

        /**
         * @brief - Holds a maximum size over which the widget starts being unusable. Up until
         *          this value the widget can make use of some extra space but not beyond. Such
         *          an area is the theoretical maximum bound for usability of this widget.
         */
        utils::Sizef m_maxSize;

        /**
         * @brief - Defines the strategy of the widget regarding space allocation. This allows
         *          for precise determination of the capability of the widget to use some extra
         *          space or to determine whether it's a problem if the widget has to be shrunk.
         *          This policy is best used in conjunction with the layout system, and is taken
         *          into consideration when computing the space to assign to the widget.
         */
        SizePolicy m_sizePolicy;

        /**
         * @brief - Used to determine whether the geometry information held by this widget is
         *          up to date. This is particularly useful to delay geometry computations to
         *          a later date, for example when an event of type `geometry update` is received.
         *          Ideally whenever a request to retrieve size information for this widget is
         *          received, it should be checked against this status to trigger a recomputation
         *          if it appears that the information is not up to date.
         */
        bool m_geometryDirty;

        /**
         * @brief - Describes the current rendering area assigned to this widget. Should always
         *          be greater than the `m_minSize`, smaller than the `m_maxSize` and as close
         *          to the `m_sizeHint` (if any is provided).
         *          This is used in computation to allocate and fill the internal visual textures
         *          used to represent the widget.
         */
        utils::Boxf m_area;

        /**
         * @brief - Used to determine whether this item is a root item, meaning it is not embedded
         *          into any other layout, or if it is a child item of some sort.
         */
        bool        m_rootItem;

        /**
         * @brief - Used to determine whether the item is visible or not. This is used by layouts
         *          for example where visibility can determine if an item will get some space or
         *          not.
         */
        bool        m_visible;

        /**
         * @brief - Indicates whether this layout item is part of a virtual hierarchy, which means
         *          that geometry update will be triggered at the most appropriate times by some
         *          external objects and thus no events should be generated upon inserting or
         *          removing a child item.
         */
        bool        m_virtual;
    };

    using LayoutItemShPtr = std::shared_ptr<LayoutItem>;
  }
}

# include "LayoutItem.hxx"

#endif    /* LAYOUT_ITEM_HH */
