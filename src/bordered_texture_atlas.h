#ifndef BORDERED_TEXTURE_ATLAS_H
#define BORDERED_TEXTURE_ATLAS_H

/*!
 * @header bordered_texture_atlas.h
 * @abstract Defines a texture atlas that adds a border around each tile.
 * @discussion To the outside, the bordered texture atlas shows a very similar
 * API as the normal texture atlas (which it will hopefully replace), and it
 * fulfills the same duty: Combine the millions of small Tomb Raider texture
 * pages (four-digit numbers have been observed) into few big pages. To avoid
 * issues with mipmapping and anisotropic filtering, it adds a border (you can
 * choose the width) around each tile, which is just that edge copied.
 *
 * Using it is fairly simple:
 * <ol>
 *   <li>Create it with @see BorderedTextureAtlas_Create.
 *   <li>Give it references to all original pages with
 *       @see BorderedTextureAtlas_AddPage.
 *   <li>Inform it about all Tomb Raider object textures with
 *       @see BorderedTextureAtlas_AddObjectTexture.
 *   <li>Create the texture data and upload it with
 *       @see BorderedTextureAtlas_CreateTextures
 *   <li>Get the new page and texture coordinates with
 *       @see BorderedTextureAtlas_GetCoordinates
 *   <li>Destroy it with @see BorderedTextureAtlas_Destroy.
 * </ol>
 */

#include <stdint.h>
#include <SDL2/SDL_platform.h>
#include <SDL2/SDL_opengl.h>
#include "vt/tr_types.h"
#include "polygon.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @struct bordered_texture_atlas_s
 * @abstract The bordered texture atlas.
 * @discussion The contents of this struct are private and subject to change. Do
 * not put an instance of it on the stack; only create it through @see BorderedTextureAtlas_Create.
 */
typedef struct bordered_texture_atlas_s *bordered_texture_atlas_p;

/*!
 * Create a new Bordered texture atlas with the specified border width. This
 * width cannot be changed later.
 * @param border The border width around each texture.
 */
bordered_texture_atlas_p BorderedTextureAtlas_Create(int border);
    
/*!
 * Destroy all contents of a bordered texture atlas. Using the atlas afterwards
 * is an error and undefined. If textures have been uploaded, then the OpenGL
 * texture objects will not be destroyed.
 */
void BorderedTextureAtlas_Destroy(bordered_texture_atlas_p atlas);

/*!
 * Add a 32-bit 256x256 texture page. The atlas will use it to get the texture
 * data later. It does not copy the data, so this pointer must remain valid at
 * least until @see BorderedTextureAtlas_CreateTextures gets called.
 * @param atlas The atlas
 * @param trpage The data. Must have a size of 256x256x4 = 256 kB, and be stored in RGBA format.
 */
void BorderedTextureAtlas_AddPage(bordered_texture_atlas_p atlas, void *trpage);

/*!
 * Add the contents of a Tomb Raider object texture. An object texture is how
 * Tomb Raider stores its texture coordinates. As simplification, this method
 * assumes/requires that this region is always an axis aligned rectangle, which
 * is true for all known levels. Only textures that got added through this
 * method will appear in the final result. They must be added in the original
 * order for @see BorderedTextureAtlas_GetCoordinates to work as expected.
 * @param atlas The texture atlas
 * @param texture The object texture.
 */
void BorderedTextureAtlas_AddObjectTexture(bordered_texture_atlas_p atlas,
                                           const tr4_object_texture_t *texture);

/*!
 * Same as above, but for sprite textures.
 */
void BorderedTextureAtlas_AddSpriteTexture(bordered_texture_atlas_p atlas,
                                           const tr_sprite_texture_t *texture);

/*!
 * Returns the texture coordinates of the specified texture. This must only be
 * called after all pages and object texture coordinates have been added.
 * Otherwise the size calculation code won't work.
 * @param atlas The texture atlas
 * @param texture The texture.
 * @param numCoordinates The number of coordinates you want. Typically 3 (for a triangle) or 4 (for a rectangle). Must be smaller than 5.
 * @param reverse Whether to reverse the order of texture coordinates on output.
 * @param outPage The page in the resulting texture atlas.
 * @param coordiantes The coordinates of the specified texture.
 * you don't think you'll need them all.
 */
void BorderedTextureAtlas_GetCoordinates(bordered_texture_atlas_p atlas,
                                         unsigned long texture,
                                         int reverse,
                                         polygon_p poly,
                                         signed shift = 0,
                                         bool split = false);

/*!
 * Same as above, but for sprite textures. This always returns four coordinates (eight float values), in the order top right, top left, bottom left, bottom right.
 */
void BorderedTextureAtlas_GetSpriteCoordinates(bordered_texture_atlas_p atlas, unsigned long sprite_texture, uint32_t *outPage, GLfloat *coordinates);

/*!
 * Returns the number of texture atlas pages that have been created. Triggers a
 * layout if none has happened so far.
 */
unsigned long BorderedTextureAtlas_GetNumAtlasPages(bordered_texture_atlas_p atlas);

/*!
 * Returns height of specified file object texture.
 */
unsigned long BorderedTextureAtlas_GetTextureHeight(bordered_texture_atlas_p atlas,
                                                    unsigned long texture);

/*!
 * Uploads the current data to OpenGL, as one or more texture pages.
 * textureNames has to have a length of at least GetNumAtlasPages and will
 * contain the names of the pages on return.
 * @param atlas The atlas.
 * @param textureNames The names of the textures.
 * @param additionalTextureNames How many texture names to create in addition to the needed ones.
 */
void BorderedTextureAtlas_CreateTextures(bordered_texture_atlas_p atlas, GLuint *textureNames, GLuint additionalTextureNames);

/*!
 * Returns height of desired TexInfo.
 * layout if none has happened so far.
 */


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* BORDERED_TEXTURE_ATLAS_H */
