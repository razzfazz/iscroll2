//
//  iScroll2Pref.m
//  iScroll2
//
//  Created by Daniel Becker on 2005-02-13.
//  Copyright (c) 2005 Daniel Becker. All rights reserved.
//

#import "iScroll2Pref.h"

#include <CoreFoundation/CFBundle.h>

#import "ioregistry.h"
#import "../Driver/ioregkeys.h"
#import "../Driver/defaults.h"

@implementation iScroll2Pref

- (void)loadSettings
{
	NSDictionary * parameters;
	NSObject * key;
	NSObject * value;
	NSEnumerator * control;
	parameters = (NSDictionary *)getHIDSystemParameters();
	control = [_buttons keyEnumerator];
	while(key = [control nextObject])
	{
		value = [parameters objectForKey:key];
		if(![value isKindOfClass:[NSNumber class]])
		{
			value = [_defaults objectForKey:key];
		}
		[(NSButton *)[_buttons objectForKey:key] setState:[(NSNumber *)value intValue] == 0 ? NSOffState : NSOnState];
	}
	control = [_sliders keyEnumerator];
	while(key = [control nextObject])
	{
		value = [parameters objectForKey:key];
		if(![value isKindOfClass:[NSNumber class]])
		{
			value = [_defaults objectForKey:key];
		}
		[(NSSlider *)[_sliders objectForKey:key] setIntValue:[(NSNumber *)value intValue]];
	}
	control = [_popupbuttons keyEnumerator];
	while(key = [control nextObject])
	{
		value = [parameters objectForKey:key];
		if(![value isKindOfClass:[NSNumber class]])
		{
			value = [_defaults objectForKey:key];
		}
		[(NSPopUpButton *)[_popupbuttons objectForKey:key] selectItemAtIndex:[(NSNumber *)value intValue]];
	}
	[parameters release];
}

- (void)mainViewDidLoad
{
	_buttons = [[NSDictionary alloc] initWithObjectsAndKeys:
		_CEnable, @kTrackpadCircScroll,
		_CInvert, @kTrackpadCircScrollInvert,
		_HInvert, @kTrackpadHorizScrollInvert,
		_VInvert, @kTrackpadVertScrollInvert,
		_ignoreWeakerAxis, @kTrackpadScrollIgnoreWeakAxis,
		0];
	_sliders = [[NSDictionary alloc] initWithObjectsAndKeys:
		_CResetTime, @kTrackpadCircScrollMaxDelay,
		_CScale, @kTrackpadCircScrollScale,
		_CThresh, @kTrackpadCircScrollThreshold,
		_HScale, @kTrackpadHorizScrollScale,
		_HThresh, @kTrackpadHorizScrollThreshold,
		_VScale, @kTrackpadVertScrollScale,
		_VThresh, @kTrackpadVertScrollThreshold,
		_minDelay, @kTrackpadScrollMinDelay,
		0];
	_popupbuttons = [[NSDictionary alloc] initWithObjectsAndKeys:
		_clickMapTo, @kTrackpadClickMapTo,
		_tapMapTo, @kTrackpadTapMapTo,
		_twoFingerClickMapTo, @kTrackpadTwoFingerClickMapTo,
		0];
	_defaults = [[NSDictionary alloc] initWithObjectsAndKeys:
		[NSNumber numberWithInt: kTrackpadScrollDefault], @kTrackpadScroll,
		[NSNumber numberWithInt: kTrackpadVertScrollThresholdDefault], @kTrackpadVertScrollThreshold,
		[NSNumber numberWithInt: kTrackpadVertScrollScaleDefault], @kTrackpadVertScrollScale,
		[NSNumber numberWithInt: kTrackpadVertScrollInvertDefault], @kTrackpadVertScrollInvert,
		[NSNumber numberWithInt: kTrackpadHorizScrollDefault], @kTrackpadHorizScroll,
		[NSNumber numberWithInt: kTrackpadHorizScrollThresholdDefault], @kTrackpadHorizScrollThreshold,
		[NSNumber numberWithInt: kTrackpadHorizScrollScaleDefault], @kTrackpadHorizScrollScale,
		[NSNumber numberWithInt: kTrackpadHorizScrollInvertDefault], @kTrackpadHorizScrollInvert,
		[NSNumber numberWithInt: kTrackpadCircScrollDefault], @kTrackpadCircScroll,
		[NSNumber numberWithInt: kTrackpadCircScrollThresholdDefault], @kTrackpadCircScrollThreshold,
		[NSNumber numberWithInt: kTrackpadCircScrollScaleDefault], @kTrackpadCircScrollScale,
		[NSNumber numberWithInt: kTrackpadCircScrollInvertDefault], @kTrackpadCircScrollInvert,
		[NSNumber numberWithInt: kTrackpadScrollIgnoreWeakAxisDefault], @kTrackpadScrollIgnoreWeakAxis,
		[NSNumber numberWithInt: kTrackpadClickMapToDefault], @kTrackpadClickMapTo,
		[NSNumber numberWithInt: kTrackpadTapMapToDefault], @kTrackpadTapMapTo,
		[NSNumber numberWithInt: kTrackpadTwoFingerClickMapToDefault], @kTrackpadTwoFingerClickMapTo,
		[NSNumber numberWithInt: kTrackpadScrollMinDelayDefault], @kTrackpadScrollMinDelay,
		[NSNumber numberWithInt: kTrackpadCircScrollMaxDelayDefault], @kTrackpadCircScrollMaxDelay,
		0];
	[_version setStringValue:[NSString stringWithFormat:@"Version %@", [[[self bundle] infoDictionary] objectForKey:(NSString *)kCFBundleVersionKey]]];
}

- (void)willSelect
{
	[self loadSettings];
}

- (IBAction)saveSettings:(id)sender
{
	NSEnumerator * control;
	NSDictionary * parameters = nil;
	NSObject * key;
	if([sender isMemberOfClass:[NSButton class]])
	{
		control = [_buttons keyEnumerator];
		while(key = [control nextObject])
		{
			if([_buttons objectForKey:key] == sender)
			{
				parameters = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithInt:[sender state] == NSOnState ? 1 : 0], key, 0];
				break;
			}
		}
	}
	else if([sender isMemberOfClass:[NSSlider class]])
	{
		control = [_sliders keyEnumerator];
		while(key = [control nextObject])
		{
			if([_sliders objectForKey:key] == sender)
			{
				parameters = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithInt:[sender intValue]], key, 0];
				break;
			}
		}
	}
	else if([sender isMemberOfClass:[NSPopUpButton class]])
	{
		control = [_popupbuttons keyEnumerator];
		while(key = [control nextObject])
		{
			if([_popupbuttons objectForKey:key] == sender)
			{
				parameters = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithInt:[sender indexOfSelectedItem]], key, 0];
				break;
			}
		}
	}
	if(parameters != nil)
	{
		applySettingsToHIDSystem((CFDictionaryRef)parameters);
		[parameters release];
	}
}

@end
