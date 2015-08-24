//
//  bsp_tree_2d.c
//  tr1engine
//
//  Created by Torsten Kammer on 27.07.13.
//  Copyright (c) 2013 Torsten Kammer. All rights reserved.
//

#include "bsp_tree_2d.h"

#include <assert.h>
#include <stdlib.h>

/*!
 * @abstract A 2D BSP tree.
 * @discussion This is mostly a list of nodes and some bookkeeping references. All nodes in a tree belong to the same array, to avoid allocating too much.
 * @param nodes The list of nodes.
 * @param 
 */
struct bsp_tree_2d_s {
    struct bsp_tree_2d_node_s *nodes;
    
    // How many nodes can be stored in the nodes array at most
    unsigned nodeCapacity;
    
    // The last node that data was written to.
    unsigned lastUsedNode;
    
    // Are there unoccupied nodes in the range from 0 to lastUsedNode (inclusive)? These are marked with a fill state of UNUSED.
    unsigned numUnusedNodes;
};

/** How much to grow the initial array whenever it turns out that it is too small */
#define BSP_TREE_CAPACITY_GROWTH 20

/*!
 * @abstract 2D BSP Tree node.
 * @discussion The tree consists of nodes, which can be filled already, empty, or have children. Each node stores its own size and location. This is not strictly speaking necessary; all these value could be generated on the fly while traversing the tree. However, that requires a lot more arguments for everything and makes the code ugly. Because width and size are stored, it is not necessary to store for each splitting node whether it is horizontal or vertical.
 * @field children The node's children, as indices into the tree's array. These are only valid if the fill grade is NODE, otherwise the contents are undefined. The first node will be either the one with the lower x or lower y coordinate. Both children together will take up the entire area of this node. The children will not overlap.
 * @field x The x coordinate of this node's location
 * @field y The y coordinate of this node's location
 * @field width The node's width
 * @field height The node's height
 * @field fillGrade Stores whether this node is already completely full (FILLED), not full at all and perhaps subject to further splitting (EMPTY), or already split, and all attempts to add new objects should recurse (NODE). If this is UNUSED, then this is not a node at all, and all fields are invalid; this can happen due to internal bookkeeping.
 */
struct bsp_tree_2d_node_s {
    unsigned children[2];
    
    unsigned x;
    unsigned y;
    unsigned width;
    unsigned height;
    
    enum bsp_tree_2d_node_fill_grade_e { FILLED, EMPTY, NODE, UNUSED } fillGrade;
};

bsp_tree_2d_p BSPTree2D_Create(unsigned width, unsigned height)
{
    bsp_tree_2d_p result = malloc(sizeof(struct bsp_tree_2d_s));
    result->nodeCapacity = BSP_TREE_CAPACITY_GROWTH;
    
    result->nodes = malloc(result->nodeCapacity * sizeof(struct bsp_tree_2d_node_s));
    result->lastUsedNode = 0;
    result->numUnusedNodes = 0;
    
    result->nodes[0].fillGrade = EMPTY;
    result->nodes[0].x = 0;
    result->nodes[0].y = 0;
    result->nodes[0].width = width;
    result->nodes[0].height = height;
    
    return result;
}

void BSPTree2D_Destroy(bsp_tree_2d_p tree)
{
    free(tree->nodes);
    free(tree);
}

/*!
 * Find an index of a node that is not used, or creates an unused node.
 */
static unsigned bspTree2D_NewNode(bsp_tree_2d_p tree)
{
    // Ensure internal consistency (or rather fail if that is not possible)
    assert(tree->lastUsedNode < tree->nodeCapacity);
    
    // Are there unused nodes in the array? If yes, find them.
    if (tree->numUnusedNodes > 0)
    {
        unsigned i;
        for (i = 0; i <= tree->lastUsedNode; i++)
        {
            if (tree->nodes[i].fillGrade == UNUSED)
            {
                tree->numUnusedNodes -= 1;
                tree->nodes[i].fillGrade = EMPTY;
                return i;
            }
        }
    }
    
    // Is there still capacity in the array? If yes, use it.
    if (tree->lastUsedNode + 1 < tree->nodeCapacity) {
        tree->lastUsedNode += 1;
        tree->nodes[tree->lastUsedNode].fillGrade = EMPTY;
        return tree->lastUsedNode;
    }
    
    // Grow the node capacity
    tree->nodeCapacity += BSP_TREE_CAPACITY_GROWTH;
    tree->nodes = realloc(tree->nodes, tree->nodeCapacity * sizeof(tree->nodes[0]));
    
    // Return the last index, which now should exist
    tree->lastUsedNode += 1;
    tree->nodes[tree->lastUsedNode].fillGrade = EMPTY;
    return tree->lastUsedNode;
};

static void bspTree2D_SplitNodeHorizontally(bsp_tree_2d_p tree, unsigned parentNodeIndex, unsigned splitLocation)
{
    unsigned childrenIndices[2] = {
        bspTree2D_NewNode(tree),
        bspTree2D_NewNode(tree)
    };
    
    struct bsp_tree_2d_node_s *parent = tree->nodes + parentNodeIndex;
    
    parent->fillGrade = NODE;
    
    parent->children[0] = childrenIndices[0];
    tree->nodes[childrenIndices[0]].x = parent->x;
    tree->nodes[childrenIndices[0]].y = parent->y;
    tree->nodes[childrenIndices[0]].width = splitLocation;
    tree->nodes[childrenIndices[0]].height = parent->height;
    tree->nodes[childrenIndices[0]].fillGrade = EMPTY;
    
    parent->children[1] = childrenIndices[1];
    tree->nodes[childrenIndices[1]].x = parent->x + splitLocation;
    tree->nodes[childrenIndices[1]].y = parent->y;
    tree->nodes[childrenIndices[1]].width = parent->width - splitLocation;
    tree->nodes[childrenIndices[1]].height = parent->height;
    tree->nodes[childrenIndices[1]].fillGrade = EMPTY;
}

static void bspTree2D_SplitNodeVertically(bsp_tree_2d_p tree, unsigned parentNodeIndex, unsigned splitLocation)
{
    unsigned childrenIndices[2] = {
        bspTree2D_NewNode(tree),
        bspTree2D_NewNode(tree)
    };
    
    struct bsp_tree_2d_node_s *parent = tree->nodes + parentNodeIndex;
    
    parent->fillGrade = NODE;
    
    parent->children[0] = childrenIndices[0];
    tree->nodes[childrenIndices[0]].x = parent->x;
    tree->nodes[childrenIndices[0]].y = parent->y;
    tree->nodes[childrenIndices[0]].width = parent->width;
    tree->nodes[childrenIndices[0]].height = splitLocation;
    tree->nodes[childrenIndices[0]].fillGrade = EMPTY;
    
    parent->children[1] = childrenIndices[1];
    tree->nodes[childrenIndices[1]].x = parent->x;
    tree->nodes[childrenIndices[1]].y = parent->y + splitLocation;
    tree->nodes[childrenIndices[1]].width = parent->width;
    tree->nodes[childrenIndices[1]].height = parent->height - splitLocation;
    tree->nodes[childrenIndices[1]].fillGrade = EMPTY;
}

static int bspTree2D_RecursiveFindSpaceFor(bsp_tree_2d_p tree, unsigned width, unsigned height, unsigned *x, unsigned *y, unsigned nodeIndex)
{
    assert(nodeIndex <= tree->lastUsedNode);
    struct bsp_tree_2d_node_s *node = tree->nodes + nodeIndex;
    
    // To use TeslaRus's expression: Paranoia!
    assert(node->fillGrade < UNUSED);
    
    // Could this possibly fit?
    if (node->width < width || node->height < height)
        return 0;
    
    if (node->fillGrade == FILLED)
    {
        // Already occupied
        return 0;
    }
    else if (node->fillGrade == NODE)
    {
        // Recurse!
        int found = 0;
        if (width <= tree->nodes[node->children[0]].width && height <= tree->nodes[node->children[0]].height)
        {
            found = bspTree2D_RecursiveFindSpaceFor(tree, width, height, x, y, node->children[0]);
        }
        if (!found && width <= tree->nodes[node->children[1]].width && height <= tree->nodes[node->children[1]].height)
        {
            found = bspTree2D_RecursiveFindSpaceFor(tree, width, height, x, y, node->children[1]);
        }
        
        // Node pointer may have changed due to splits
        struct bsp_tree_2d_node_s *node = tree->nodes + nodeIndex;
        
        // If both children are filled, mark this as filled and discard the
        // children.
        if (tree->nodes[node->children[0]].fillGrade == FILLED && tree->nodes[node->children[1]].fillGrade == FILLED)
        {
            node->fillGrade = FILLED;
            tree->nodes[node->children[0]].fillGrade = UNUSED;
            tree->nodes[node->children[1]].fillGrade = UNUSED;
            tree->numUnusedNodes += 2;
        }
        
        return found;
    }
    else if (node->fillGrade == EMPTY)
    {
        if (node->height == height && node->width == width)
        {
            // Perfect match
            node->fillGrade = FILLED;
            *x = node->x;
            *y = node->y;
            return 1;
        }
        else if (node->height == height)
        {
            // Split horizontally
            bspTree2D_SplitNodeHorizontally(tree, nodeIndex, width);
            
            // Node pointer may have changed due to split
            struct bsp_tree_2d_node_s *node = tree->nodes + nodeIndex;
            
            // height already fits, width fits too now, so this is the result
            tree->nodes[node->children[0]].fillGrade = FILLED;
            *x = tree->nodes[node->children[0]].x;
            *y = tree->nodes[node->children[0]].y;
            return 1;
        }
        else
        {
            // In case of doubt do a vertical split
            bspTree2D_SplitNodeVertically(tree, nodeIndex, height);
            
            // Node pointer may have changed due to split
            struct bsp_tree_2d_node_s *node = tree->nodes + nodeIndex;
            
            // Recurse, because the width may not match
            return bspTree2D_RecursiveFindSpaceFor(tree, width, height, x, y, node->children[0]);
        }
    }
    
    // Can't ever reach this
    return 0;
}

int BSPTree2D_FindSpaceFor(bsp_tree_2d_p tree, unsigned width, unsigned height, unsigned *x, unsigned *y)
{
    return bspTree2D_RecursiveFindSpaceFor(tree, width, height, x, y, 0);
}
