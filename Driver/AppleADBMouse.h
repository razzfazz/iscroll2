// 
// This is a hack to activate two-finger scrolling on supported pre-2005 
// PowerBooks and iBooks on OS X 10.3 (tested on 10.3.7).
//
// It is based on Apple's AppleADBMouse-209.0.10 driver from 10.3.7 that 
// is available as part of the publicly released Darwin source code.
//
// Modified by Daniel Becker (razzfazz@web.de) on Feb. 6/7, 2005.
//

/*
 * Copyright (c) 1998-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * 18 June 1998 sdouglas  Start IOKit version.
 * 18 Nov  1998 suurballe port to C++
 *  4 Oct  1999 decesare  Revised for Type 4 support and sub-classed drivers.
 *  7 Feb  2005 dub       Added two-finger scrolling.
 *  9 Feb  2005 dub       Code cleanup, some improvements.
 * 27 Feb  2005 dub       Added click remapping.
 *  1 Mar  2005 dub       Lots of maintenance for two-finger scrolling.
 */

#include <IOKit/adb/IOADBDevice.h>
#include <IOKit/hidsystem/IOHIPointing.h>
#include <IOKit/hid/IOHIDDevice.h>

#define TRUE	1
#define FALSE	0

class AppleADBMouse: public IOHIPointing
{
	OSDeclareDefaultStructors(AppleADBMouse);
	
protected:
	IOADBDevice		* adbDevice;
	IOFixed			_resolution;
	IOItemCount		_buttonCount;
	
public:
	virtual IOService * probe(IOService * provider, SInt32 * score);
	virtual bool start(IOService * provider);
	virtual UInt32 interfaceID(void);
	virtual UInt32 deviceType(void);
	virtual IOFixed resolution(void);
	virtual IOItemCount buttonCount(void);
	virtual void packet(UInt8 adbCommand, IOByteCount length, UInt8 * data);
};


class AppleADBMouseType1 : public AppleADBMouse
{
	OSDeclareDefaultStructors(AppleADBMouseType1);
	
public:
	virtual IOService * probe(IOService * provider, SInt32 * score);
	virtual bool start(IOService * provider);
};


class AppleADBMouseType2 : public AppleADBMouse
{
	OSDeclareDefaultStructors(AppleADBMouseType2);
	
public:
	virtual IOService * probe(IOService * provider, SInt32 * score);
	virtual bool start(IOService * provider);
};


class AppleADBMouseType4 : public AppleADBMouse
{
	OSDeclareDefaultStructors(AppleADBMouseType4);
	
private:
    bool Clicking, Dragging, DragLock, typeTrackpad;
    bool _jitterclick, _jittermove, _ignoreTrackpad;
    bool _palmnomove, _palmnoaction, _2fingernoaction, _sticky2finger, _zonenomove, 
		_zonenoaction, _zonepecknomove, _ignorezone, _isWEnhanced, _usePantherSettings;
	
	// modified dub:
	short			_oldScrollX;
	short			_oldScrollY;
	bool			_enableScroll;
	bool			_enableScrollX;
	UInt16			_scrollThreshX;
	UInt16			_scrollScaleX;
	bool			_scrollInvertX;
	bool			_enableScrollY;
	UInt16			_scrollThreshY;
	UInt16			_scrollScaleY;
	bool			_scrollInvertY;
	bool			_enableScrollRot;
	UInt16			_scrollThreshRot;
	UInt16			_scrollScaleRot;
	bool			_scrollInvertRot;
	UInt8			_clickMapTo;
	UInt8			_tapMapTo;
	UInt8			_twoFingerClickMapTo;
	AbsoluteTime	_scrollMinDelayAB;
    AbsoluteTime	_lastScrollTimeAB;
	bool			_stickyRotating;
	bool			_scrollDominantAxisOnly;
	UInt8			_oldButtonState;
	short			_scrollX;
	short			_scrollY;
	short			_scrollRot;
	// end modifications
	
    UInt8		_WThreshold;
    UInt16		_jitterdelta;
    UInt64		_jitterclicktime64, _zonepeckingtime64, _timeoutSticky64, _sticky2fingerTime64,
				_fake5min;
	
	AbsoluteTime	_jitterclicktimeAB, _zonepeckingtimeAB, _timeoutStickyAB, _sticky2fingerTimeAB,
					_fake5minAB, _keyboardTimeAB;

    bool enableEnhancedMode();
    IOService		* _pADBKeyboard;
    IONotifier		* _notifierA, * _notifierT;
    const OSSymbol	* _gettime;
    IOLock			* _mouseLock;
    OSSet			* _externalMice;
	
protected:
	UInt32	deviceSignature;
	UInt16	deviceResolution;
	UInt8	deviceClass;
	UInt8	deviceNumButtons;
	
public:
    virtual IOReturn setParamProperties( OSDictionary * dict );
	virtual IOService * probe(IOService * provider, SInt32 * score);
	virtual bool start(IOService * provider);
	virtual void packet(UInt8 adbCommand, IOByteCount length, UInt8 * data);
	virtual void packetW(UInt8 adbCommand, IOByteCount length, UInt8 * data);
	virtual void packetWP(UInt8 adbCommand, IOByteCount length, UInt8 * data);
	virtual OSData * copyAccelerationTable();
	virtual void _check_usb_mouse(IOService * service, bool added); 
	virtual void free();
	
};
