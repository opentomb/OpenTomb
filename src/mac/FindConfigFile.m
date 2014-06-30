//
//  FindConfigFile.c
//  OpenTomb
//
//  Created by Torsten Kammer on 02.12.13.
//  Copyright (c) 2013 Torsten Kammer. All rights reserved.
//

#include "FindConfigFile.h"

#include <Foundation/Foundation.h>

BOOL checkHasConfigAndChdir(NSString *directoryPath)
{
	NSString *configPath = [directoryPath stringByAppendingPathComponent:@"config.lua"];
	BOOL isDirectory = NO;
	if ([[NSFileManager defaultManager] fileExistsAtPath:configPath isDirectory:&isDirectory] && !isDirectory) {
		[[NSFileManager defaultManager] changeCurrentDirectoryPath:directoryPath];
		return YES;
	}
	return NO;
}

void FindConfigFile(void)
{
	// 1. Check directory the app is in
	NSString *mainBundleParent = [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent];
	if (checkHasConfigAndChdir(mainBundleParent))
		return;
	
	// 2. Check documents path explicitly
	// Note: This app runs before AppKit is started, so using NSSearchPathForDirectoriesInDomains does not work.
	NSString *documentsPath = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents"];
	NSString *documentsOpenTombPath = [documentsPath stringByAppendingPathComponent:@"Open Tomb"];
	if (checkHasConfigAndChdir(documentsOpenTombPath))
		return;
	
	// 3. Check explicitly "Downloads"
	NSString *downloadsPath = [NSHomeDirectory() stringByAppendingPathComponent:@"Downloads"];
	NSString *downloadsEnginePath = [downloadsPath stringByAppendingPathComponent:@"engine"];
	if (checkHasConfigAndChdir(downloadsEnginePath))
		return;
	
	fprintf(stderr, "No config.lua found");
	
	exit(12);
}
