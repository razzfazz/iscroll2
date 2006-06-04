/*
 *  ioregistry.c
 *  iScroll2Pref
 *
 *  Created by Daniel Becker on 2005-02-19.
 *  Copyright 2005 Daniel Becker. All rights reserved.
 *
 */

#include "ioregistry.h"

#include <CoreServices/CoreServices.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDShared.h>

CFDictionaryRef getHIDSystemParameters()
{
	io_iterator_t			iter = 0;
	io_registry_entry_t		regEntry = 0;
	bool					success = TRUE;
	CFMutableDictionaryRef	properties = NULL;
	CFDictionaryRef			parameters = NULL;
	
	IOServiceGetMatchingServices(kIOMasterPortDefault, 
								 IOServiceMatching(kIOHIDSystemClass), &iter);
	if(iter == 0) {
		success = FALSE;
		goto EXIT;
	}
	
	regEntry = IOIteratorNext(iter);
	if(regEntry == 0) {
		success = FALSE;
		goto EXIT;
	}
	
	if((IORegistryEntryCreateCFProperties(regEntry, &properties, 
										  kCFAllocatorDefault, 0) != 
		KERN_SUCCESS) || (properties == NULL)) {		
		success = FALSE;
		goto EXIT;
	}
	
	parameters = CFDictionaryGetValue(properties, CFSTR("HIDParameters"));
	if(!parameters || CFGetTypeID(parameters) != CFDictionaryGetTypeID()) {
		success = FALSE;
		goto EXIT;
	}
	CFRetain(parameters);
	
EXIT:
	if(properties != NULL)
		CFRelease(properties);
	if(regEntry != 0)
		IOObjectRelease(regEntry);
	if(iter != 0)
		IOObjectRelease(iter);
	return success ? parameters : 0;
}

bool applySettingsToHIDSystem(CFDictionaryRef settings)
{
	io_iterator_t		iter = 0;
	io_registry_entry_t	regEntry = 0;
	bool				success = TRUE;
	
	IOServiceGetMatchingServices(kIOMasterPortDefault, 
								 IOServiceMatching(kIOHIDSystemClass), &iter);
	if(iter == 0) {
		success = FALSE;
		goto EXIT;
	}
	
	regEntry = IOIteratorNext(iter);
	if(regEntry == 0) {
		success = FALSE;
		goto EXIT;
	}
	
	if(IORegistryEntrySetCFProperties(regEntry, settings) != KERN_SUCCESS) {
		success = FALSE;
		goto EXIT;
	}
	
EXIT:
	if(regEntry != 0)
		IOObjectRelease(regEntry);
	if(iter != 0)
		IOObjectRelease(iter);
	return success;
}