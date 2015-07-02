/***************************************************************************/
/*                                                                         */
/*  cff.c                                                                  */
/*                                                                         */
/*    FreeType OpenType driver component (body only).                      */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2013 by                                     */
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

#include "cffpic.hpp"
#include "cffdrivr.hpp"
#include "cffparse.hpp"
#include "cffload.hpp"
#include "cffobjs.hpp"
#include "cffgload.hpp"
#include "cffcmap.hpp"

#include "cf2arrst.hpp"
#include "cf2blues.hpp"
#include "cf2error.hpp"
#include "cf2font.hpp"
#include "cf2ft.hpp"
#include "cf2hints.hpp"
#include "cf2intrp.hpp"
#include "cf2read.hpp"
#include "cf2stack.hpp"

/* END */
