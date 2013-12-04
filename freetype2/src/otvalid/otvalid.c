/***************************************************************************/
/*                                                                         */
/*  otvalid.c                                                              */
/*                                                                         */
/*    FreeType validator for OpenType tables (body only).                  */
/*                                                                         */
/*  Copyright 2004, 2007 by                                                */
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

#include "otvbase.hpp"
#include "otvcommn.hpp"
#include "otvgdef.hpp"
#include "otvgpos.hpp"
#include "otvgsub.hpp"
#include "otvjstf.hpp"
#include "otvmath.hpp"
#include "otvmod.hpp"

/* END */
