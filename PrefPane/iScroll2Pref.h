//
//  iScroll2Pref.h
//  iScroll2
//
//  Created by Daniel Becker on 2005-02-13.
//  Copyright (c) 2005 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <PreferencePanes/PreferencePanes.h>


@interface iScroll2Pref : NSPreferencePane 
{
    IBOutlet NSButton *_enable;
    IBOutlet NSButton *_CEnable;
    IBOutlet NSButton *_CInvert;
    IBOutlet NSSlider *_CResetTime;
    IBOutlet NSSlider *_CScale;
    IBOutlet NSSlider *_CThresh;
    IBOutlet NSMatrix *_twoFingerModClick;
    IBOutlet NSButton *_HEnable;
    IBOutlet NSButton *_HInvert;
    IBOutlet NSSlider *_HScale;
    IBOutlet NSSlider *_HThresh;
    IBOutlet NSButton *_ignoreWeakerAxis;
    IBOutlet NSSlider *_minDelay;
    IBOutlet NSButton *_VEnable;
    IBOutlet NSButton *_VInvert;
    IBOutlet NSSlider *_VScale;
    IBOutlet NSSlider *_VThresh;
	NSDictionary * _buttons;
	NSDictionary * _sliders;
	NSDictionary * _defaults;
}

- (void)mainViewDidLoad;
- (IBAction)saveSettings:(id)sender;

@end
