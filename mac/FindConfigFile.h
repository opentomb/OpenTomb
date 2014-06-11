//
//  FindConfigFile.h
//  OpenTomb
//
//  Created by Torsten Kammer on 02.12.13.
//  Copyright (c) 2013 Torsten Kammer. All rights reserved.
//

#ifndef OpenTomb_FindConfigFile_h
#define OpenTomb_FindConfigFile_h

#ifdef __cplusplus
extern "C" {
#endif

	
/*!
 * This function looks for the config file in various locations, and switches to
 * the first one where it is found.
 *
 * Locations that are checked include, in order:
 * <ul>
 * <li>The directory the app is in.
 * <li>~/Documents/Open Tomb
 * <li>~/Downloads/engine
 * </ul>
 * More are possible, if you have good ideas.
 */
void FindConfigFile(void);
	
	
#ifdef __cplusplus
}
#endif

#endif
