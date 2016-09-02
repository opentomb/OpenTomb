//
//  config-opentomb.h
//  OpenTomb
//
//  Created by Torsten Kammer on 20.08.16.
//
//

/*
 * This is a default version of config-opentomb.h for the people with workflows that don't include CMake, i.e. ones who
 * set up all their paths in their IDE of choice manually. CMake is definitely the recommended way in the future.
 *
 * CMake automatically generates a dynamic version of the file based on config-opentomb.h.in and then uses only that.
 * So if you feel the need to edit this file, please do the same changes to config-opentomb.h.in as well.
 */

#ifndef config_opentomb_h_in_h
#define config_opentomb_h_in_h

#define HAVE_ALC_H 1
#define HAVE_EFX_H 1
#define HAVE_EFX_PRESETS_H 1

#endif /* config_opentomb_h */
