//
//  iScroll2Pref.h
//  iScroll2
//
//  Created by Daniel Becker on 2005-02-13.
//  Copyright (c) 2005 Daniel Becker. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <PreferencePanes/PreferencePanes.h>


@interface iScroll2Pref : NSPreferencePane 
{
    IBOutlet NSButton		*_CEnable;
    IBOutlet NSButton		*_HEnable;
    IBOutlet NSButton		*_VEnable;
    IBOutlet NSButton		*_CInvert;
    IBOutlet NSSlider		*_CScale;
    IBOutlet NSSlider		*_CThresh;
    IBOutlet NSPopUpButton	*_tapMapTo;
    IBOutlet NSPopUpButton	*_twoFingerClickMapTo;
    IBOutlet NSPopUpButton	*_clickMapTo;
    IBOutlet NSButton		*_HInvert;
    IBOutlet NSSlider		*_HScale;
    IBOutlet NSSlider		*_HThresh;
    IBOutlet NSButton		*_ignoreWeakerAxis;
    IBOutlet NSSlider		*_minDelay;
    IBOutlet NSButton		*_VInvert;
    IBOutlet NSSlider		*_VScale;
    IBOutlet NSSlider		*_VThresh;
	IBOutlet NSTextField	*_version;
	NSDictionary			*_buttons;
	NSDictionary			*_sliders;
	NSDictionary			*_popupbuttons;
	NSDictionary			*_defaults;
}

- (void)mainViewDidLoad;
- (IBAction)saveSettings:(id)sender;
- (IBAction)loadDefaults:(id)sender;

@end
