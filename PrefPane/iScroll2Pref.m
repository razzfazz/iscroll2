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
#import "preferences.h"

@implementation iScroll2Pref

- (void)getSettings
{
	NSDictionary * settings;
	settings = (NSDictionary *)getHIDSystemParameters();
	_trackpadScroll = [(NSNumber *)[settings objectForKey:@kTrackpadScrollKey] boolValue];
	_trackpadHorizScroll = [(NSNumber *)[settings objectForKey:@kTrackpadHorizScrollKey] boolValue];
	[_current setDictionary:[settings objectForKey:@kiScroll2SettingsKey]];
	[settings release];
}

- (void)displaySettings
{
	NSObject * key;
	NSObject * object;
	NSEnumerator * control;
	control = [_buttons keyEnumerator];
	while(key = [control nextObject])
	{
		object = [_current objectForKey:key];
		[(NSButton *)[_buttons objectForKey:key] setState:[(NSNumber *)object boolValue] ? NSOnState : NSOffState];
	}
	control = [_sliders keyEnumerator];
	while(key = [control nextObject])
	{
		NSSlider *slider;
		object = [_current objectForKey:key];
		slider = (NSSlider *)[_sliders objectForKey:key];
		[slider setIntValue:[slider tag] == 1 ? (int)([slider maxValue] + [slider minValue]) - [(NSNumber *)object intValue] : [(NSNumber *)object intValue]];
	}
	control = [_popupbuttons keyEnumerator];
	while(key = [control nextObject])
	{
		object = [_current objectForKey:key];
		[(NSPopUpButton *)[_popupbuttons objectForKey:key] selectItemAtIndex:[(NSNumber *)object intValue]];
	}
	[_VEnable setState:_trackpadScroll ? NSOnState : NSOffState];
	[_HEnable setState:_trackpadHorizScroll ? NSOnState : NSOffState];
}

- (void)applySettings
{
	NSDictionary * settings;
	settings = [[NSDictionary alloc] initWithObjectsAndKeys:_current, @kiScroll2SettingsKey, 0];
	applySettingsToHIDSystem((CFDictionaryRef)settings);
	[settings release];
 	CFPreferencesSetValue((CFStringRef)@kCurrentParametersKey, (CFDictionaryRef)_current, 
		(CFStringRef)@kDriverAppID, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
	CFPreferencesSynchronize((CFStringRef)@kDriverAppID,
		kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
}

- (void)mainViewDidLoad
{
	NSDictionary * localizedInfoDict;
	_buttons = [[NSDictionary alloc] initWithObjectsAndKeys:
		_CEnable, @kTrackpadCScrollKey,
		_COnly, @kTrackpadCScrollOnlyKey,
		_CInvert, @kTrackpadCScrollInvertKey,
		_HInvert, @kTrackpadHScrollInvertKey,
		_VInvert, @kTrackpadVScrollInvertKey,
		_ignoreWeakerAxis, @kTrackpadScrollIgnoreWeakAxisKey,
		0];
	_sliders = [[NSDictionary alloc] initWithObjectsAndKeys:
		_CScale, @kTrackpadCScrollScaleKey,
		_CThresh, @kTrackpadCScrollThresholdKey,
		_HThresh, @kTrackpadHScrollThresholdKey,
		_VThresh, @kTrackpadVScrollThresholdKey,
		_scrollResolution, @kTrackpadScrollResolutionKey,
		_tapDownTime, @kTrackpadTapDownTimeKey,
		0];
	_popupbuttons = [[NSDictionary alloc] initWithObjectsAndKeys:
		_clickMapTo, @kTrackpadClickMapToKey,
		_tapMapTo, @kTrackpadTapMapToKey,
		_twoFingerClickMapTo, @kTrackpadTwoFingerClickMapToKey,
		0];
	_current = [[NSMutableDictionary alloc] init];
	localizedInfoDict = [[[self bundle] localizedInfoDictionary] retain];
	[_nameBottom setStringValue:[localizedInfoDict objectForKey:(NSString *)kCFBundleNameKey]];
	[_versionBottom setStringValue:[NSString stringWithFormat:@"Version %@, %@", 
		[[[self bundle] infoDictionary] objectForKey:(NSString *)kCFBundleVersionKey], 
		[localizedInfoDict objectForKey:(NSString *)@"NSHumanReadableCopyright"]]];
	[_nameAbout setStringValue:[localizedInfoDict objectForKey:(NSString *)kCFBundleNameKey]];
	[_versionAbout setStringValue:[NSString stringWithFormat:@"Version %@", 
		[[[self bundle] infoDictionary] objectForKey:(NSString *)kCFBundleVersionKey]]];
	[_copyrightAbout setStringValue:[localizedInfoDict objectForKey:(NSString *)@"NSHumanReadableCopyright"]];
	[localizedInfoDict release];
}

- (void)willSelect
{
	[self getSettings];
	[self displaySettings];
}

- (IBAction)changeSetting:(id)sender
{
	NSEnumerator * control;
	NSObject * key = nil;
	NSObject * object = nil;
	if([sender isMemberOfClass:[NSButton class]])
	{
		control = [_buttons keyEnumerator];
		while(key = [control nextObject])
		{
			if([_buttons objectForKey:key] == sender)
			{
				object = [NSNumber numberWithBool:[sender state] == NSOnState];
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
				object = [NSNumber numberWithInt:[sender tag] == 1 ? (int)([sender maxValue] + [sender minValue]) - [sender intValue] : [sender intValue]];
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
				object = [NSNumber numberWithInt:[sender indexOfSelectedItem]];
				break;
			}
		}
	}
	if(object != nil)
	{
		[_current setObject:object forKey:key];
		[self applySettings];
	}
}

- (IBAction)loadDefaultSettings:(id)sender
{
	NSDictionary * defaults;
	defaults = (NSDictionary *)CFPreferencesCopyValue((CFStringRef)@kDefaultParametersKey, 
		(CFStringRef)@kDriverAppID, kCFPreferencesAnyUser, kCFPreferencesCurrentHost);
	[_current setDictionary:defaults];
	[defaults release];
	[self displaySettings];
	[self applySettings];
}

- (IBAction)uninstall:(id)sender
{
	[[NSWorkspace sharedWorkspace] openFile:[NSString stringWithFormat:@"%@/%@/%@", 
		[[self bundle] bundlePath], @"Contents/Resources", @"uninstall.command"]];
}

@end
