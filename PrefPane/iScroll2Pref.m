//
//  iScroll2Pref.m
//  iScroll2
//
//  Created by Daniel Becker on 2005-02-13.
//  Copyright (c) 2005 __MyCompanyName__. All rights reserved.
//

#import "iScroll2Pref.h"

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
	key = @kTrackpadTwoFingerModClick;
	value = [parameters objectForKey:key];
	if(![value isKindOfClass:[NSNumber class]])
	{
		value = [_defaults objectForKey:key];
	}
	[(NSMatrix *)_twoFingerModClick selectCellWithTag:[(NSNumber *)value intValue]];
	[parameters release];
}

- (void)mainViewDidLoad
{
	_buttons = [[NSDictionary alloc] initWithObjectsAndKeys:
		_enable, @kTrackpadScroll,
		_CEnable, @kTrackpadCircScroll,
		_CInvert, @kTrackpadCircScrollInvert,
		_HEnable, @kTrackpadHorizScroll,
		_HInvert, @kTrackpadHorizScrollInvert,
		_ignoreWeakerAxis, @kTrackpadScrollDominantAxisOnly,
		_VEnable, @kTrackpadVertScroll,
		_VInvert, @kTrackpadVertScrollInvert,
		0];
	_sliders = [[NSDictionary alloc] initWithObjectsAndKeys:
		_CResetTime, @kTrackpadCircScrollMaxDelay,
		_CScale, @kTrackpadCircScrollScale,
		_CThresh, @kTrackpadCircScrollThreshold,
		_HScale, @kTrackpadHorizScrollScale,
		_HThresh, @kTrackpadHorizScrollThreshold,
		_minDelay, @kTrackpadScrollMinDelay,
		_VScale, @kTrackpadVertScrollScale,
		_VThresh, @kTrackpadVertScrollThreshold,
		0];
	_defaults = [[NSDictionary alloc] initWithObjectsAndKeys:
		[NSNumber numberWithInt: kTrackpadScrollDefault], @kTrackpadScroll,
		[NSNumber numberWithInt: kTrackpadHorizScrollDefault], @kTrackpadHorizScroll,
		[NSNumber numberWithInt: kTrackpadHorizScrollThresholdDefault], @kTrackpadHorizScrollThreshold,
		[NSNumber numberWithInt: kTrackpadHorizScrollScaleDefault], @kTrackpadHorizScrollScale,
		[NSNumber numberWithInt: kTrackpadHorizScrollInvertDefault], @kTrackpadHorizScrollInvert,
		[NSNumber numberWithInt: kTrackpadVertScrollDefault], @kTrackpadVertScroll,
		[NSNumber numberWithInt: kTrackpadVertScrollThresholdDefault], @kTrackpadVertScrollThreshold,
		[NSNumber numberWithInt: kTrackpadVertScrollScaleDefault], @kTrackpadVertScrollScale,
		[NSNumber numberWithInt: kTrackpadVertScrollInvertDefault], @kTrackpadVertScrollInvert,
		[NSNumber numberWithInt: kTrackpadCircScrollDefault], @kTrackpadCircScroll,
		[NSNumber numberWithInt: kTrackpadCircScrollThresholdDefault], @kTrackpadCircScrollThreshold,
		[NSNumber numberWithInt: kTrackpadCircScrollScaleDefault], @kTrackpadCircScrollScale,
		[NSNumber numberWithInt: kTrackpadCircScrollInvertDefault], @kTrackpadCircScrollInvert,
		[NSNumber numberWithInt: kTrackpadScrollDominantAxisOnlyDefault], @kTrackpadScrollDominantAxisOnly,
		[NSNumber numberWithInt: kTrackpadTwoFingerModClickDefault], @kTrackpadTwoFingerModClick,
		[NSNumber numberWithInt: kTrackpadScrollMinDelayDefault], @kTrackpadScrollMinDelay,
		[NSNumber numberWithInt: kTrackpadCircScrollMaxDelayDefault], @kTrackpadCircScrollMaxDelay,
		0];
}

- (void)willSelect
{
	[self loadSettings];
}

- (IBAction)saveSettings:(id)sender
{
	NSEnumerator * control;
	NSDictionary * parameters;
	NSObject * key;
	if([sender isKindOfClass:[NSButton class]])
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
	else if([sender isKindOfClass:[NSSlider class]])
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
	else if([sender isKindOfClass:[NSMatrix class]])
	{
		if(_twoFingerModClick == sender)
		{
			key = @kTrackpadTwoFingerModClick;
			parameters = [[NSDictionary alloc] initWithObjectsAndKeys:[NSNumber numberWithInt:[[sender selectedCell] tag]], key, 0];
		}
	}
	applySettingsToHIDSystem((CFDictionaryRef)parameters);
	[parameters release];
}

@end
