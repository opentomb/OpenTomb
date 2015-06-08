/***************************************************************************/
/*                                                                         */
/*  psaux.c                                                                */
/*                                                                         */
/*    FreeType auxiliary PostScript driver component (body only).          */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2006 by                                     */
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
#include "psobjs.hpp"
#include "psauxmod.hpp"
#include "t1decode.hpp"
#include "t1cmap.hpp"

#ifndef T1_CONFIG_OPTION_NO_AFM
#include "afmparse.hpp"
#endif

#include "psconv.hpp"


/* END */
