/***************************************************************************/
/*                                                                         */
/*  autofit.c                                                              */
/*                                                                         */
/*    Auto-fitter module (body).                                           */
/*                                                                         */
/*  Copyright 2003, 2004, 2005, 2006, 2007 by                              */
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
#include "afpic.hpp"
#include "afangles.hpp"
#include "afglobal.hpp"
#include "afhints.hpp"

#include "afdummy.hpp"
#include "aflatin.hpp"
#ifdef FT_OPTION_AUTOFIT2
#include "aflatin2.hpp"
#endif
#include "afcjk.hpp"
#include "afindic.hpp"

#include "afloader.hpp"
#include "afmodule.hpp"

#ifdef AF_USE_WARPER
#include "afwarp.hpp"
#endif

/* END */
