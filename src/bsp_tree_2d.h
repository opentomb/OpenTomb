#pragma once

#include <cstdint>
#include <memory>

/**
 * @brief A Binary Space Partition Tree for 2D space.
 */
struct BSPTree2DNode {
    std::unique_ptr<BSPTree2DNode> left;
    std::unique_ptr<BSPTree2DNode> right;

    //! If @c true, denotes that there is no more free space in this node or its children.
    //! @note This is a pure caching mechanism to avoid unnecessary recursion.
    bool isFilled = false;

    //! The X coordinate of this node in 2D space.
    size_t x = 0;
    //! The Y coordinate of this node in 2D space.
    size_t y = 0;
    //! The width of this node in 2D space.
    size_t width = 0;
    //! The height of this node in 2D space.
    size_t height = 0;

    BSPTree2DNode() = default;
    BSPTree2DNode(size_t x_, size_t y_, size_t w, size_t h)
        : x(x_)
        , y(y_)
        , width(w)
        , height(h)
    {
    }

    bool isSplit() const {
        return left && right;
    }

    /**
     * @brief Split this node along its Y axis (X is split).
     * @param splitLocation Local X coordinate of the split point
     */
    void splitHorizontally(size_t splitLocation) {
        assert( splitLocation < width );
        left.reset( new BSPTree2DNode(x, y, splitLocation, height) );
        right.reset( new BSPTree2DNode(x + splitLocation, y, width - splitLocation, height) );
    }

    /**
     * @brief Split this node along its X axis (Y is split).
     * @param splitLocation Local Y coordinate of the split point
     */
    void splitVertically(size_t splitLocation) {
        assert( splitLocation < height );
        left.reset(new BSPTree2DNode(x, y, width, splitLocation));
        right.reset(new BSPTree2DNode(x, y + splitLocation, width, height-splitLocation));
    }

    bool fits(size_t w, size_t h) const noexcept
    {
        return !isFilled && w <= width && h <= height;
    }

    /**
     * @brief Find a free space in this node or its children
     * @param needleWidth
     * @param needleHeight
     * @param destX
     * @param destY
     * @return
     */
    bool findSpaceFor(size_t needleWidth, size_t needleHeight, size_t *destX, size_t *destY) {
        // Could this possibly fit?
        if(!fits(needleWidth, needleHeight))
            return false;

        if (isSplit())
        {
            // This node is already split => Recurse!
            bool found = false;
            if (needleWidth <= left->width && needleHeight <= left->height)
            {
                found = left->findSpaceFor(needleWidth, needleHeight, destX, destY);
            }
            if (!found && needleWidth <= right->width && needleHeight <= right->height)
            {
                found = right->findSpaceFor(needleWidth, needleHeight, destX, destY);
            }

            // If both children are filled, mark this as filled and discard the
            // children.
            if (left->isFilled && right->isFilled)
            {
                isFilled = true;
                left.reset();
                right.reset();
            }

            return found;
        }

        // We may split this node
        if (height == needleHeight && width == needleWidth)
        {
            // Perfect match
            isFilled = true;
            *destX = x;
            *destY = y;
            return true;
        }
        else if (height == needleHeight)
        {
            // Split horizontally
            splitHorizontally(needleWidth);

            // height already fits, width fits too now, so this is the result
            isFilled = true;
            *destX = left->x;
            *destY = left->y;
            return true;
        }
        else
        {
            // In case of doubt do a vertical split
            splitVertically(needleHeight);

            // Recurse, because the width may not match
            return left->findSpaceFor(needleWidth, needleHeight, destX, destY);
        }
    }
};
