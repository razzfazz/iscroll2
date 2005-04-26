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
    IBOutlet NSButton		*_CInvert;
	IBOutlet NSButton		*_COnly;
    IBOutlet NSSlider		*_CScale;
    IBOutlet NSSlider		*_CThresh;
    IBOutlet NSPopUpButton	*_tapMapTo;
    IBOutlet NSPopUpButton	*_twoFingerClickMapTo;
    IBOutlet NSPopUpButton	*_clickMapTo;
    IBOutlet NSButton		*_HInvert;
    IBOutlet NSSlider		*_HThresh;
    IBOutlet NSButton		*_ignoreWeakerAxis;
    IBOutlet NSButton		*_VInvert;
    IBOutlet NSSlider		*_VThresh;
	IBOutlet NSTextField	*_name;
	IBOutlet NSTextField	*_version;
	IBOutlet NSTextField	*_copyright;
	NSDictionary			*_buttons;
	NSDictionary			*_sliders;
	NSDictionary			*_popupbuttons;
	NSMutableDictionary		*_current;
}

- (void)getSettings;
- (void)displaySettings;
- (void)applySettings;
- (void)mainViewDidLoad;
- (IBAction)changeSetting:(id)sender;
- (IBAction)loadDefaultSettings:(id)sender;
- (IBAction)uninstall:(id)sender;

@end
