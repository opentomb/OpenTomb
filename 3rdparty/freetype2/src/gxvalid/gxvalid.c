/***************************************************************************/
/*                                                                         */
/*  gxvalid.c                                                              */
/*                                                                         */
/*    FreeType validator for TrueTypeGX/AAT tables (body only).            */
/*                                                                         */
/*  Copyright 2005 by suzuki toshiya, Masatake YAMATO, Red Hat K.K.,       */
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

#include "gxvfeat.hpp"
#include "gxvcommn.hpp"
#include "gxvbsln.hpp"
#include "gxvtrak.hpp"
#include "gxvjust.hpp"
#include "gxvmort.hpp"
#include "gxvmort0.hpp"
#include "gxvmort1.hpp"
#include "gxvmort2.hpp"
#include "gxvmort4.hpp"
#include "gxvmort5.hpp"
#include "gxvmorx.hpp"
#include "gxvmorx0.hpp"
#include "gxvmorx1.hpp"
#include "gxvmorx2.hpp"
#include "gxvmorx4.hpp"
#include "gxvmorx5.hpp"
#include "gxvkern.hpp"
#include "gxvopbd.hpp"
#include "gxvprop.hpp"
#include "gxvlcar.hpp"
#include "gxvmod.hpp"


/* END */
