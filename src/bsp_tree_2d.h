#ifndef BSP_TREE_2D_H
#define BSP_TREE_2D_H

/*!
 * @header bsp_tree_2d
 * @abstract Manage the fill state of a 2D rectangle where new rectangles may be added at any time.
 * @discussion This class is used internally by the bordered texture atlas. It is used for laying out texture tiles in the big texture atlas. The external interface is deliberately small and hopefully easy to understand.
 *
 * Internally, this uses a 2D BSP tree, as outlined in <TODO: Find that website>, which also inspires the name. This is not a requirement for proper working of the bordered texture atlas. All that it requires is an interface pretty much like this one, which observes the contract of FindSpaceFor, the key method.
 * @see bordered_textured_atlas_t
 */

#ifdef __cplusplus
extern "C" {
#endif
    
/*!
 * The struct that defines this type. Its contents are not relevant for or accessible to clients.
 */
typedef struct bsp_tree_2d_s *bsp_tree_2d_p;

/*!
 * Creates a new tree with the given dimensions.
 */
bsp_tree_2d_p BSPTree2D_Create(unsigned width, unsigned height);
    
/*!
 * Destroys a tree and releases all allocated resources. Note: Do not use free() on a tree; that would cause leaks.
 */
void BSPTree2D_Destroy(bsp_tree_2d_p tree);

/*!
 * @abstract Find space for a given rectangle within the tree's area.
 * @discussion Produces the start of an area that has the passed in size, and does not overlap any area returned by previous calls to this method. If no such area can be found, it returns 0 and leaves the internal state untouched.
 *
 * This is the main and only interesting method of this class.
 *
 * @param tree The tree.
 * @param width The width of the area.
 * @param height The height of the area.
 * @param x On return, the x coordinate of an area with the given size. The value on return is undefined if no such area was not found. Must never bebe NULL.
 * @param x On return, the y coordinate of an area with the given size. The value on return is undefined if no such area was not found. Must never bebe NULL.
 * @result 1 if such an area was found, or 0 if no area was found.
 */
int BSPTree2D_FindSpaceFor(bsp_tree_2d_p tree, unsigned width, unsigned height, unsigned *x, unsigned *y);
    
#ifdef __cplusplus
}
#endif

#endif /* BSP_TREE_2D_H */
