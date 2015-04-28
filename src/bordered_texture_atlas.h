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

class bordered_texture_atlas
{
    /*!
     * @abstract Identifies a corner.
     * @discussion This is used for mapping a corner in a file object texture to the corresponding corner in the canonical object texture.
     */
    enum corner_location
    {
        TOP_LEFT,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT
    };
    
    /*!
     * An internal representation of a file texture. Note that this only stores a reference to the canonical texture and how the corners of the canonical texture map to this.
     */
    struct file_object_texture
    {
        unsigned long canonical_texture_index;
        enum corner_location corner_locations[4];
    };
    
    /*!
     * The canonical texture. In TR, a lot of textures can refer to the same rectangle of pixels, only in different winding orders. It is not practical to treat these as different textures, so they are all mapped to one canonical object texture. This structure consists of two parts: Describing the original location, and describing the new final location. The latter is only valid after the data in the texture atlas has been laid out.
     */
    struct canonical_object_texture
    {
        // The unadjusted size
        uint8_t width;
        uint8_t height;
        
        // Original origin
        uint16_t original_page;
        uint8_t original_x;
        uint8_t original_y;
        
        // New origin
        unsigned long new_page;
        unsigned new_x_with_border; // Where the adjusted data starts. The start of the actual data is this plus the atlas's border size.
        unsigned new_y_with_border; // See above.
    };
    
    // How much border to add.
    int border_width;
    
    // Store whether the data has been laid out. Adding more object textures after that is illegal.
    bool data_has_been_laid_out = false;
    
    // Whether non-power-of-two textures are supported.
    //int supports_npot;
    
    // Result pages
    // Note: No capacity here, this is handled internally by the layout method. Also, all result pages have the same width, which will always be less than or equal to the height.
    unsigned long number_result_pages = 0;
    unsigned result_page_width = 0;
    unsigned *result_page_height = nullptr;
    
    // Original data
    unsigned long number_original_pages = 0;
    unsigned long capacity_original_pages = 0;
    void **original_pages = nullptr;
    
    // Object textures in the file.
    unsigned long number_file_object_textures = 0;
    unsigned long capacity_file_object_textures = 0;
    struct file_object_texture *file_object_textures = nullptr;
    
    // Sprite texture in the file.
    // Note: No data is saved for them, they get mapped directly to canonical textures.
    unsigned long number_sprite_textures = 0;
    unsigned long capacity_sprite_textures = 0;
    unsigned long *canonical_textures_for_sprite_textures = nullptr;
    
    // Canonical object textures
    unsigned long number_canonical_object_textures = 0;
    unsigned long capacity_canonical_object_textures = 0;
    struct canonical_object_texture *canonical_object_textures = nullptr;
    
    /*! Lays out the texture data and switches the atlas to laid out mode. */
    void layOutTextures();
    
    /*! For sorting: Compares two different textures and sorts them by size. */
    static int compareCanonicalTextureSizes(const void *parameter1, const void *parameter2);
    
public:
    /*!
     * Create a new Bordered texture atlas with the specified border width. This
     * width cannot be changed later.
     * @param border The border width around each texture.
     */
    bordered_texture_atlas(int border);
    
    /*!
     * Destroy all contents of a bordered texture atlas. Using the atlas afterwards
     * is an error and undefined. If textures have been uploaded, then the OpenGL
     * texture objects will not be destroyed.
     */
    ~bordered_texture_atlas();
    
    /*!
     * Add a 32-bit 256x256 texture page. The atlas will use it to get the texture
     * data later. It does not copy the data, so this pointer must remain valid at
     * least until @see BorderedTextureAtlas_CreateTextures gets called.
     * @param trpage The data. Must have a size of 256x256x4 = 256 kB, and be stored in RGBA format.
     */
    void addPage(void *trpage);
    
    /*!
     * Add the contents of a Tomb Raider object texture. An object texture is how
     * Tomb Raider stores its texture coordinates. As simplification, this method
     * assumes/requires that this region is always an axis aligned rectangle, which
     * is true for all known levels. Only textures that got added through this
     * method will appear in the final result. They must be added in the original
     * order for @see BorderedTextureAtlas_GetCoordinates to work as expected.
     * @param texture The object texture.
     */
    void addObjectTexture(const tr4_object_texture_t &texture);
    
    /*!
     * Same as above, but for sprite textures.
     */
    void addSpriteTexture(const tr_sprite_texture_t &texture);
    
    /*!
     * Returns the texture coordinates of the specified texture. This must only be
     * called after all pages and object texture coordinates have been added.
     * Otherwise the size calculation code won't work.
     * @param texture The texture.
     * @param numCoordinates The number of coordinates you want. Typically 3 (for a triangle) or 4 (for a rectangle). Must be smaller than 5.
     * @param reverse Whether to reverse the order of texture coordinates on output.
     * @param outPage The page in the resulting texture atlas.
     * @param coordiantes The coordinates of the specified texture.
     * you don't think you'll need them all.
     */
    void getCoordinates(unsigned long texture,
                        bool reverse,
                        polygon_p poly,
                        signed shift = 0,
                        bool split = false);
    
    /*!
     * Same as above, but for sprite textures. This always returns four coordinates (eight float values), in the order top right, top left, bottom left, bottom right.
     */
    void getSpriteCoordinates(unsigned long sprite_texture,
                              uint32_t &outPage,
                              GLfloat *coordinates);
    
    /*!
     * Returns the number of texture atlas pages that have been created. Triggers a
     * layout if none has happened so far.
     */
    unsigned long getNumAtlasPages();
    
    /*!
     * Returns height of specified file object texture.
     */
    unsigned long getTextureHeight(unsigned long texture) const;
    
    /*!
     * Uploads the current data to OpenGL, as one or more texture pages.
     * textureNames has to have a length of at least GetNumAtlasPages and will
     * contain the names of the pages on return.
     * @param atlas The atlas.
     * @param textureNames The names of the textures.
     * @param additionalTextureNames How many texture names to create in addition to the needed ones.
     */
    void createTextures(GLuint *textureNames, GLuint additionalTextureNames);

};

#endif /* BORDERED_TEXTURE_ATLAS_H */
