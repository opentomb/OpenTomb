/***************************************************************************/
/*                                                                         */
/*  ftbase.c                                                               */
/*                                                                         */
/*    Single object library component (body only).                         */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2006, 2007, 2008, 2009 by       */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <ft2build.h>

#define  FT_MAKE_OPTION_SINGLE_OBJECT

#include "ftpic.hpp"
#include "basepic.hpp"
#include "ftadvanc.hpp"
#include "ftcalc.hpp"
#include "ftdbgmem.hpp"
#include "ftgloadr.hpp"
#include "ftobjs.hpp"
#include "ftoutln.hpp"
#include "ftrfork.hpp"
#include "ftsnames.hpp"
#include "ftstream.hpp"
#include "fttrigon.hpp"
#include "ftutil.hpp"

#ifdef FT_MACINTOSH
#include "ftmac.hpp"
#endif

/* END */
