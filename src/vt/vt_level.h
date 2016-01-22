#ifndef _VT_LEVEL_H_
#define _VT_LEVEL_H_

#include "l_main.h"

#define TR_TEXTURE_INDEX_MASK_TR4   (0x7FFF)          // in some custom levels we need to use 0x7FFF flag
#define TR_TEXTURE_INDEX_MASK       (0x0FFF)
//#define TR_TEXTURE_SHAPE_MASK     (0x7000)          // still not used
#define TR_TEXTURE_FLIPPED_MASK     (0x8000)


void WriteTGAfile(const char *filename, const uint8_t *data, const int width, const int height, char invY);

class VT_Level : public TR_Level 
{
    public:
    static int get_level_format(const char *name);
    static int get_PC_level_version(const char *name);
    void prepare_level();
    void dump_textures();
    tr_staticmesh_t *find_staticmesh_id(uint32_t object_id);
    tr2_item_t *find_item_id(int32_t object_id);
    tr_moveable_t *find_moveable_id(uint32_t object_id);

    protected:
    void convert_textile8_to_textile32(tr_textile8_t & tex, tr2_palette_t & pal, tr4_textile32_t & dst);
    void convert_textile16_to_textile32(tr2_textile16_t & tex, tr4_textile32_t & dst);
};

#endif // _VT_LEVEL_H_