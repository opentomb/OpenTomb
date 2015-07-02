/***************************************************************************/
/*                                                                         */
/*  ftcache.c                                                              */
/*                                                                         */
/*    The FreeType Caching sub-system (body only).                         */
/*                                                                         */
/*  Copyright 2000-2001, 2003 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#define FT_MAKE_OPTION_SINGLE_OBJECT

#include <ft2build.h>
#include "ftcmru.hpp"
#include "ftcmanag.hpp"
#include "ftccache.hpp"
#include "ftccmap.hpp"
#include "ftcglyph.hpp"
#include "ftcimage.hpp"
#include "ftcsbits.hpp"
#include "ftcbasic.hpp"

/* END */
