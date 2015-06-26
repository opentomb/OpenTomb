#pragma once

#include <cstdint>
#include <memory>

struct BSPTree2DNode {
    std::unique_ptr<BSPTree2DNode> left;
    std::unique_ptr<BSPTree2DNode> right;

    bool isFilled = false;
    size_t x = 0;
    size_t y = 0;
    size_t width = 0;
    size_t height = 0;

    BSPTree2DNode() = default;
    BSPTree2DNode(size_t x_, size_t y_, size_t w, size_t h)
        : x(x_)
        , y(y_)
        , width(w)
        , height(h)
    {
    }

    bool isEmpty() const {
        return !left && !right;
    }

    bool hasChildren() const {
        return left && right;
    }

    void splitHorizontally(size_t splitLocation) {
        left.reset( new BSPTree2DNode(x, y, splitLocation, height) );
        right.reset( new BSPTree2DNode(x + splitLocation, y, width - splitLocation, height) );
    }

    void splitVertically(size_t splitLocation) {
        left.reset(new BSPTree2DNode(x, y, width, splitLocation));
        right.reset(new BSPTree2DNode(x, y + splitLocation, width, height-splitLocation));
    }

    bool findSpaceFor(size_t needleWidth, size_t needleHeight, size_t *destX, size_t *destY) {
        // Could this possibly fit?
        if (width < needleWidth || height < needleHeight)
            return false;

        if (isFilled)
        {
            // Already occupied
            return false;
        }
        else if (hasChildren())
        {
            // Recurse!
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
                left->isFilled = false;
                right->isFilled;
            }

            return found;
        }
        else if (isEmpty())
        {
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

        // Can't ever reach this
        return false;
    }
};
