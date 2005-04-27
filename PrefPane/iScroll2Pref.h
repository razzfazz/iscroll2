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
    IBOutlet NSSlider		*_CScale;
    IBOutlet NSSlider		*_CThresh;
	IBOutlet NSButton		*_COnly;
    IBOutlet NSPopUpButton	*_tapMapTo;
    IBOutlet NSPopUpButton	*_twoFingerClickMapTo;
    IBOutlet NSPopUpButton	*_clickMapTo;
    IBOutlet NSButton		*_HEnable;
    IBOutlet NSButton		*_HInvert;
    IBOutlet NSSlider		*_HScale;
    IBOutlet NSSlider		*_HThresh;
    IBOutlet NSButton		*_ignoreWeakerAxis;
    IBOutlet NSButton		*_VEnable;
    IBOutlet NSButton		*_VInvert;
    IBOutlet NSSlider		*_VScale;
    IBOutlet NSSlider		*_VThresh;
	IBOutlet NSSlider		*_scrollResolution;
	IBOutlet NSSlider		*_minDelay;
	IBOutlet NSSlider		*_tapDownTime;
	IBOutlet NSSlider		*_stickyDragTime;
	IBOutlet NSTextField	*_smallName;
	IBOutlet NSTextField	*_productName;
	IBOutlet NSTextField	*_productVersion;
	IBOutlet NSTextField	*_copyright;
	IBOutlet NSTextField	*_aboutText;
	NSDictionary			*_buttons;
	NSDictionary			*_sliders;
	NSDictionary			*_popupbuttons;
	NSMutableDictionary		*_current;
	BOOL					_trackpadScroll;
	BOOL					_trackpadHorizScroll;
}

- (void)getSettings;
- (void)displaySettings;
- (void)applySettings;
- (void)mainViewDidLoad;
- (IBAction)changeSetting:(id)sender;
- (IBAction)loadDefaultSettings:(id)sender;
- (IBAction)uninstall:(id)sender;

@end
