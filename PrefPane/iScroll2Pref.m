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
		NSSlider *slider;
		value = [parameters objectForKey:key];
		if(![value isKindOfClass:[NSNumber class]])
		{
			value = [_defaults objectForKey:key];
		}
		slider = (NSSlider *)[_sliders objectForKey:key];
		[slider setIntValue:[slider tag] == 1 ? (int)([slider maxValue] + [slider minValue]) - [(NSNumber *)value intValue] : [(NSNumber *)value intValue]];
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
	NSDictionary * localizedInfoDict;
	_buttons = [[NSDictionary alloc] initWithObjectsAndKeys:
		_VEnable, @kTrackpadVScroll,
		_HEnable, @kTrackpadHScroll,
		_CEnable, @kTrackpadCScroll,
		_CInvert, @kTrackpadCScrollInvert,
		_HInvert, @kTrackpadHScrollInvert,
		_VInvert, @kTrackpadVScrollInvert,
		_ignoreWeakerAxis, @kTrackpadScrollIgnoreWeakAxis,
		0];
	_sliders = [[NSDictionary alloc] initWithObjectsAndKeys:
		_CScale, @kTrackpadCScrollScale,
		_CThresh, @kTrackpadCScrollThreshold,
		_HScale, @kTrackpadHScrollScale,
		_HThresh, @kTrackpadHScrollThreshold,
		_VScale, @kTrackpadVScrollScale,
		_VThresh, @kTrackpadVScrollThreshold,
		_minDelay, @kTrackpadScrollMinDelay,
		0];
	_popupbuttons = [[NSDictionary alloc] initWithObjectsAndKeys:
		_clickMapTo, @kTrackpadClickMapTo,
		_tapMapTo, @kTrackpadTapMapTo,
		_twoFingerClickMapTo, @kTrackpadTwoFingerClickMapTo,
		0];
	_defaults = [[NSDictionary alloc] initWithObjectsAndKeys:
		[NSNumber numberWithInt: kTrackpadVScrollDefault], @kTrackpadVScroll,
		[NSNumber numberWithInt: kTrackpadVScrollThresholdDefault], @kTrackpadVScrollThreshold,
		[NSNumber numberWithInt: kTrackpadVScrollScaleDefault], @kTrackpadVScrollScale,
		[NSNumber numberWithInt: kTrackpadVScrollInvertDefault], @kTrackpadVScrollInvert,
		[NSNumber numberWithInt: kTrackpadHScrollDefault], @kTrackpadHScroll,
		[NSNumber numberWithInt: kTrackpadHScrollThresholdDefault], @kTrackpadHScrollThreshold,
		[NSNumber numberWithInt: kTrackpadHScrollScaleDefault], @kTrackpadHScrollScale,
		[NSNumber numberWithInt: kTrackpadHScrollInvertDefault], @kTrackpadHScrollInvert,
		[NSNumber numberWithInt: kTrackpadCScrollDefault], @kTrackpadCScroll,
		[NSNumber numberWithInt: kTrackpadCScrollThresholdDefault], @kTrackpadCScrollThreshold,
		[NSNumber numberWithInt: kTrackpadCScrollScaleDefault], @kTrackpadCScrollScale,
		[NSNumber numberWithInt: kTrackpadCScrollInvertDefault], @kTrackpadCScrollInvert,
		[NSNumber numberWithInt: kTrackpadScrollIgnoreWeakAxisDefault], @kTrackpadScrollIgnoreWeakAxis,
		[NSNumber numberWithInt: kTrackpadClickMapToDefault], @kTrackpadClickMapTo,
		[NSNumber numberWithInt: kTrackpadTapMapToDefault], @kTrackpadTapMapTo,
		[NSNumber numberWithInt: kTrackpadTwoFingerClickMapToDefault], @kTrackpadTwoFingerClickMapTo,
		[NSNumber numberWithInt: kTrackpadScrollMinDelayDefault], @kTrackpadScrollMinDelay,
		0];
	[_version setStringValue:[NSString stringWithFormat:@"Version %@", [[[self bundle] infoDictionary] objectForKey:(NSString *)kCFBundleVersionKey]]];
	localizedInfoDict = [[self bundle] localizedInfoDictionary];
	[_name setStringValue:[localizedInfoDict objectForKey:(NSString *)kCFBundleNameKey]];
	[_copyright setStringValue:[localizedInfoDict objectForKey:(NSString *)@"NSHumanReadableCopyright"]];
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
				parameters = [[NSDictionary alloc] initWithObjectsAndKeys:
					[NSNumber numberWithInt:[sender state] == NSOnState ? 1 : 0], key,
					0];
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
				parameters = [[NSDictionary alloc] initWithObjectsAndKeys:
					[NSNumber numberWithInt:[sender tag] == 1 ? (int)([sender maxValue] + [sender minValue]) - [sender intValue] : [sender intValue]], key,
					0];
				break;
			}
		}
	}
	else if([sender isMemberOfClass:[NSPopUpButton class]])
	{
		if((sender == _clickMapTo || sender == _tapMapTo || sender == _twoFingerClickMapTo) 
			&& ([_clickMapTo indexOfSelectedItem] && [_tapMapTo indexOfSelectedItem] && [_twoFingerClickMapTo indexOfSelectedItem]))
			if(NSRunAlertPanel(@"Warning:", 
				@"This will leave no action mapped to a left click.\n"
				@"If you proceed, you will be unable to control your system with the trackpad.\n"
				@"You should only do this if you have an external mouse attached to your system.",
				@"Cancel", @"Proceed", nil) != NSAlertAlternateReturn)
			{
				[sender selectItemAtIndex:0];
				return;
			}
		control = [_popupbuttons keyEnumerator];
		while(key = [control nextObject])
		{
			if([_popupbuttons objectForKey:key] == sender)
			{
				parameters = [[NSDictionary alloc] initWithObjectsAndKeys:
					[NSNumber numberWithInt:[sender indexOfSelectedItem]], key,
					0];
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

- (IBAction)loadDefaults:(id)sender
{
	applySettingsToHIDSystem((CFDictionaryRef)_defaults);
	[self loadSettings];
}

@end
