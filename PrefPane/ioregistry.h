/*
 *  ioregistry.h
 *  iScroll2Pref
 *
 *  Created by Daniel Becker on 2005-02-19.
 *  Copyright 2005 Daniel Becker. All rights reserved.
 *
 */

#ifndef _ISCROLL2_IOREGISTRY_H_
#define _ISCROLL2_IOREGISTRY_H_

#include <CoreFoundation/CoreFoundation.h>

CFDictionaryRef getHIDSystemParameters();
bool applySettingsToHIDSystem(CFDictionaryRef settings);

#endif