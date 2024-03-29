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
 * 18 June 1998 	  Start IOKit version.
 * 18 Nov  1998 suurballe port to C++
 *  4 Oct  1999 decesare  Revised for Type 4 support and sub-classed drivers.
 *  1 Feb  2000 tsherman  Added extended mouse functionality (implemented in setParamProperties)
 *  7 Feb  2005 dub       Added two-finger scrolling.
 *  9 Feb  2005 dub       Code cleanup, some improvements.
 * 27 Feb  2005 dub       Added click remapping.
 *  1 Mar  2005 dub       Lots of maintenance for two-finger scrolling.
 */

#include "AppleADBMouse.h"
#include "ioregkeys.h"
#include <IOKit/hidsystem/IOHIDTypes.h>
#include <IOKit/hidsystem/IOHIDParameter.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOPlatformExpert.h>

//Need the following to compile with GCC3 
static inline int
my_abs(int x)
{
	return x < 0 ? -x : x;
}

// modified dub:
// signum function (returns -1|0|+1 for x<0|x=0|x>0)
static inline int
my_sgn(int x)
{
	return x < 0 ? -1 : (x > 0 ? +1 : 0);
}
// end modifications

// modified dub:
// this is somehow missing from IOHIDParameters.h on my system
#ifndef kIOHIDVirtualHIDevice
#define kIOHIDVirtualHIDevice                   "HIDVirtualDevice"
#endif
#ifndef kIOHIDScrollAccelerationTypeKey
#define kIOHIDScrollAccelerationTypeKey			"HIDScrollAccelerationType"
#endif
#ifndef kIOHIDTrackpadScrollAccelerationType
#define kIOHIDTrackpadScrollAccelerationType	"HIDTrackpadScrollAcceleration"
#endif
#ifndef kHIDScrollAccelerationTableKey
#define kHIDScrollAccelerationTableKey			"HIDScrollAccelerationTable"
#endif
#ifndef kTrackpadScrollKey
#define kTrackpadScrollKey						"TrackpadScroll"
#endif
#ifndef kTrackpadHorizScrollKey
#define kTrackpadHorizScrollKey					"TrackpadHorizScroll"
#endif
#ifndef kHIDPointerAccelerationTypeKey
#define kHIDPointerAccelerationTypeKey			"HIDPointerAccelerationType"
#endif
#ifndef kHIDScrollAccelerationTypeKey
#define kHIDScrollAccelerationTypeKey			"HIDScrollAccelerationType"
#endif
#ifndef kHIDScrollResolutionKey
#define kHIDScrollResolutionKey					"HIDScrollResolution"
#endif

// end modifications

static bool add_usb_mouse(OSObject *, void *, IOService * );
static bool remove_usb_mouse(OSObject *, void *, IOService * );


// ****************************************************************************
// NewMouseData
//
// ****************************************************************************
static void NewMouseData(IOService * target, UInt8 adbCommand, IOByteCount length, UInt8 * data)
{
	((AppleADBMouse *)target)->packet(adbCommand, length, data);
}


// ****************************************************************************

#undef super
#define super IOHIPointing

OSDefineMetaClassAndStructors(AppleADBMouse, IOHIPointing);


// ****************************************************************************
// probe
//
// ****************************************************************************
IOService * AppleADBMouse::probe(IOService * provider, SInt32 * score)
{
	//kprintf("ADB generic probe called\n");
	adbDevice = (IOADBDevice *)provider;
	return this;
}



// ****************************************************************************
// start
//
// ****************************************************************************
bool AppleADBMouse::start(IOService * provider)
{
	if(!super::start(provider)) return false;
	
	if(!adbDevice->seizeForClient(this, NewMouseData)) {
		IOLog("%s: Seize failed\n", getName());
		return false;
	}
	return true;
}


// ****************************************************************************
// interfaceID
//
// ****************************************************************************
UInt32 AppleADBMouse::interfaceID(void)
{
	return NX_EVS_DEVICE_INTERFACE_ADB;
}


// ****************************************************************************
// deviceType
//
// ****************************************************************************
UInt32 AppleADBMouse::deviceType ( void )
{
	return adbDevice->handlerID();
}


// ****************************************************************************
// resolution
//
// ****************************************************************************
IOFixed AppleADBMouse::resolution(void)
{
	return _resolution;
}


// ****************************************************************************
// buttonCount
//
// ****************************************************************************
IOItemCount AppleADBMouse::buttonCount(void)
{
	return _buttonCount;
}


// ****************************************************************************
// packet
//
// ****************************************************************************
void AppleADBMouse::packet(UInt8 /*adbCommand*/,
IOByteCount /*length*/, UInt8 * data)
{
	int		  dx, dy;
	UInt32          buttonState = 0;
	AbsoluteTime    now;
	
	dy = data[0] & 0x7f;
	dx = data[1] & 0x7f;
	
	if (dy & 0x40) dy |= 0xffffffc0;
	if (dx & 0x40) dx |= 0xffffffc0;
	
	if ((data[0] & 0x80) == 0) buttonState |= 1;
	
	clock_get_uptime(&now);
	dispatchRelativePointerEvent(dx, dy, buttonState, now);
}


#if 0
// ****************************************************************************

#undef super
#define super AppleADBMouse

OSDefineMetaClassAndStructors(AppleADBMouseType1, AppleADBMouse);

IOService * AppleADBMouseType1::probe(IOService * provider, SInt32 * score)
{
	if (!super::probe(provider, score)) return 0;
	
	return this;
}

bool AppleADBMouseType1::start(IOService * provider)
{
	OSNumber 	*dpi;
	
	if (adbDevice->setHandlerID(1) != kIOReturnSuccess) return false;
	
	dpi = OSDynamicCast( OSNumber, getProperty("dpi"));
	if (dpi)
	{
		_resolution = dpi->unsigned16BitValue() << 16;
	}
	else
	{
		_resolution = 100 << 16;
	}
	_buttonCount = 1;
	
	return super::start(provider);
}


// ****************************************************************************

#undef super
#define super AppleADBMouse

OSDefineMetaClassAndStructors(AppleADBMouseType2, AppleADBMouse);

IOService * AppleADBMouseType2::probe(IOService * provider, SInt32 * score)
{
	if (!super::probe(provider, score)) return 0;
	
	if (adbDevice->setHandlerID(2) != kIOReturnSuccess) return 0;
	return this;
}

bool AppleADBMouseType2::start(IOService * provider)
{
	OSNumber 	*dpi;
	
	if (adbDevice->setHandlerID(2) != kIOReturnSuccess) return false;
	
	dpi = OSDynamicCast( OSNumber, getProperty("dpi"));
	if (dpi)
	{
		_resolution = dpi->unsigned16BitValue() << 16;
	}
	else
	{
		_resolution = 200 << 16;
	}
	_buttonCount = 1;
	
	return super::start(provider);
}


// ****************************************************************************
#endif

#undef super
#define super AppleADBMouse

OSDefineMetaClassAndStructors(iScroll2, AppleADBMouse);

IOService * iScroll2::probe(IOService * provider, SInt32 * score)
{
	UInt8       data[8];
	IOByteCount length = 8;
	
	if (!super::probe(provider, score)) return 0;
	
	if (adbDevice->setHandlerID(4) != kIOReturnSuccess) {
		adbDevice->setHandlerID(adbDevice->defaultHandlerID());
		return 0;
	}
	
	// To be a Type 4 Extended Mouse, register 1 must return 8 bytes.
	if (adbDevice->readRegister(1, data, &length) != kIOReturnSuccess) return 0;
	if (length != 8) return 0;
	
// modified dub:

	// check signature
	if( (data[0] != 't') || (data[1] != 'p') || (data[2] != 'a') || (data[3] != 'd') ) return 0;

	// W Enhanced trackpads should report 4 buttons!
	if(data[7] != 4) return 0;

// end modifications

	// Save the device's Extended Mouse Info.
	deviceSignature  = ((UInt32 *)data)[0];
	deviceResolution = ((UInt16 *)data)[2];
	deviceClass      = data[6];
	deviceNumButtons = data[7];
	
	return this;
}

bool iScroll2::start(IOService * provider)
{
// modified dub:
//	UInt8       adbdata[8];
//	IOByteCount adblength = 8;
// end modifications
	OSNumber 	*dpi;
	
// mopdified dub:
	IOLog("%s: starting up driver.\n", getName());
// end modifications
	
	typeTrackpad = FALSE;
	
	if (adbDevice->setHandlerID(4) != kIOReturnSuccess) return false;
	
	dpi = OSDynamicCast( OSNumber, getProperty("dpi"));
	if (dpi)
	{
		_resolution = dpi->unsigned16BitValue() << 16;
	}
	else
	{
		_resolution = deviceResolution << 16;
	}
	
	_buttonCount = deviceNumButtons;
	_notifierA = _notifierT = NULL;  //Only used by trackpad, but inspected by all type 4 mice
	_gettime = OSSymbol::withCString("get_last_keydown");
	_mouseLock = IOLockAlloc(); 
	
// modified dub:
//	adbDevice->readRegister(1, adbdata, &adblength);
//	if( (adbdata[0] == 't') && (adbdata[1] = 'p') && (adbdata[2] == 'a') && (adbdata[3] == 'd') )
// fix missing '=' in second comparison (for 'p'):
//	if( (adbdata[0] == 't') && (adbdata[1] == 'p') && (adbdata[2] == 'a') && (adbdata[3] == 'd') )
//	{
// end modifications
		mach_timespec_t     t;
		OSNumber 		*datan;
		
		t.tv_sec = 1; //Wait for keyboard driver for up to 1 second
		t.tv_nsec = 0;
// modified dub:
//		typeTrackpad = TRUE;
//		enableEnhancedMode();
		if(!enableEnhancedMode()) IOLog("%s: enableEnhancedMode failed.\n", getName());
		_pADBKeyboard = waitForService(serviceMatching("AppleADBKeyboard"), &t);
		if(!_pADBKeyboard) IOLog("%s: Error waiting for AppleADBKeyboard.\n", getName());
// end modifications
		datan = OSDynamicCast( OSNumber, getProperty("Trackpad Jitter Milliseconds"));
		if (datan)
		{
			_jitterclicktime64 = datan->unsigned16BitValue() * 1000 * 1000;  // in nanoseconds;
			nanoseconds_to_absolutetime(  _jitterclicktime64, &_jitterclicktimeAB);
		}
		else
		{
			_jitterclicktime64 = 750 * 1000 * 1000;  // in nanoseconds;
			nanoseconds_to_absolutetime(  _jitterclicktime64, &_jitterclicktimeAB);
		}
		datan = OSDynamicCast( OSNumber, getProperty("Trackpad Jitter Max delta"));
		if (datan)
		{
			_jitterdelta = datan->unsigned16BitValue();
			if (_jitterdelta == 0)
				_jittermove = false;
		}
		else
		{
			_jitterdelta = 16;  // pixels;
		}
// modified dub:
		setProperty(kIOHIDPointerAccelerationTypeKey, kIOHIDTrackpadAccelerationType); 
		setProperty(kIOHIDScrollAccelerationTypeKey, kIOHIDTrackpadScrollAccelerationType);
//		setProperty(kIOHIDScrollResolutionKey, kDefaultScrollFixedResolution, 32);
// end modifications

		//This is the only way to find out if we have new trackpad with W info passed in relative mode
		if (_buttonCount == 4)
		{
// modified dub:
//			_buttonCount = 2;	//Assume Apple trackpads will always have 
								//only 1 button and 1 gesture click
			_buttonCount = 6;
// end modifications
			setProperty("W Enhanced Trackpad", (unsigned long long)true, sizeof(Clicking)*8);
// modified dub:
//			_isWEnhanced = true;
//			_usePantherSettings = false;
// end modifications
			_jitterclick = _jittermove;	//Necessary since Mouse Pref fix isn't available
		}  
		else
		{
// modified dub:
//			_isWEnhanced = false;
// end modifications
			//Don't even bother to set the property.  Its ABSENCE will be just as informative.
		}
// modified dub:
//	} //end of trackpad processing
// end modifications
	
    if ( ! _notifierA)
        _notifierA = addNotification( gIOFirstMatchNotification, serviceMatching( "IOHIPointing" ), 
									  (IOServiceNotificationHandler)add_usb_mouse, this, 0 ); 
    if (! _notifierT)
        _notifierT = addNotification( gIOTerminatedNotification, serviceMatching( "IOHIPointing" ), 
									  (IOServiceNotificationHandler)remove_usb_mouse, this, 0 ); 
    //The same C function can handle both firstmatch and termination notifications
	
	return super::start(provider);
}


void iScroll2::free( void )
{
    if (_notifierA)
    {
		_notifierA->remove();
		_notifierA = NULL;
    }
    if (_notifierT)
    {
		_notifierT->remove();
		_notifierT = NULL;
    }
    _ignoreTrackpad = false;
    _sticky2finger = false;
// modified dub:
	_stickyRotating = false;
// end modifications
    _zonepeckingtimeAB = _keyboardTimeAB;  //unsticks trackpad when sleeping
    
    if (_gettime)
		_gettime->release();
    
    if (_mouseLock)
    {
		IOLock * lock;
		
		IOLockLock(_mouseLock);
		
		lock = _mouseLock;
		_mouseLock = NULL;
		
		IOLockUnlock(lock);
		IOLockFree(lock);
    }
    
    if (_externalMice) 
    {
        _externalMice->release();
        _externalMice = 0;
    }
	
    super::free();
}

bool add_usb_mouse(OSObject * us, void *, IOService * yourDevice)
{
    if (us)
    {
		((iScroll2 *)us)->_check_usb_mouse(yourDevice, true);
    }
    return true;
}

bool remove_usb_mouse(OSObject * us, void *, IOService * yourDevice)
{
	if (us)
    {
		((iScroll2 *)us)->_check_usb_mouse(yourDevice, false);
    }
    return true;
}

/*
 *  If any extra  mouse is found, then disable the trackpad.
 */
void iScroll2::_check_usb_mouse( IOService * service, bool added ) 
{
    // Check to see if we are interested in this service
    if ( (service == this)                      ||
         !OSDynamicCast(IOHIPointing, service)  || 
         (service->getProperty(kIOHIDVirtualHIDevice) == kOSBooleanTrue))
        return;
	
    IOLockLock(_mouseLock);
	
    if ( !_externalMice && 
         !(_externalMice = OSSet::withCapacity(4)))
    {
        IOLockUnlock(_mouseLock);
        return;
    }
	
    if (added) 
    {
        _externalMice->setObject(service);
    }
    else
    {
        _externalMice->removeObject(service);
    }
    IOLockUnlock(_mouseLock);
	
}


#if 0
/*
 *  The following ::packet() has not been changed at all.  Enhanced trackpad processing
 *  will automatically go to ::packetW().
 */
void iScroll2::packet(UInt8 /*adbCommand*/, IOByteCount length, UInt8 * data)
{
	int		  dx, dy, cnt, numExtraBytes;
	UInt32          buttonState = 0;
	AbsoluteTime	  now;
	
	IOLockLock(_mouseLock);
	bool shouldIgnore = (_ignoreTrackpad && (_externalMice && (_externalMice->getCount() > 0 )));  
	IOLockUnlock(_mouseLock);
	
	
	if (typeTrackpad && shouldIgnore) 
		return;
	
	
	if(_isWEnhanced)  //This also implies typeTrackpad == true
	{
		packetW(0, length, data);
		return;
	}
	
	numExtraBytes = length - 2;
	dy = data[0] & 0x7f;
	dx = data[1] & 0x7f;
	
	if ((data[0] & 0x80) == 0) 
	{
		buttonState |= 1;
	}
	
	if ((deviceNumButtons > 1) && ((data[1] & 0x80) == 0))
	{
		if(typeTrackpad)
		{
			if ((_jitterclick) && (_pADBKeyboard))
			{
				AbsoluteTime	keyboardtime; 
				UInt64		nowtime64, keytime64;
				
				keyboardtime.hi = 0;
				keyboardtime.lo = 0;
				_pADBKeyboard->callPlatformFunction(_gettime, false, 
													(void *)&keyboardtime, 0, 0, 0);
				clock_get_uptime(&now);
				absolutetime_to_nanoseconds(now, &nowtime64);
				absolutetime_to_nanoseconds(keyboardtime, &keytime64);
				if (nowtime64 - keytime64 > _jitterclicktime64)
				{
					buttonState |= 1;
				}
			}
			else
				buttonState |= 1;
		}
		else
		{
			buttonState |= 2;
		}
	}
	
	for (cnt = 0; cnt < numExtraBytes; cnt++) {
		dy |= ((data[2 + cnt] >> 4) & 7) << (7 + (cnt * 3));
		dx |= ((data[2 + cnt])      & 7) << (7 + (cnt * 3));
		
		if ((deviceNumButtons > (cnt + 2)) && ((data[2 + cnt] & 0x80) == 0))
			buttonState |= 4 << (cnt * 2);
		if ((deviceNumButtons > (cnt + 2 + 1)) && ((data[2 + cnt] & 0x08) == 0))
			buttonState |= 4 << (cnt * 2 + 1);
	}
	
	if (dy & (0x40 << (numExtraBytes * 3)))
		dy |= (0xffffffc0 << (numExtraBytes * 3));
	if (dx & (0x40 << (numExtraBytes * 3)))
		dx |= (0xffffffc0 << (numExtraBytes * 3));
	
	clock_get_uptime(&now);
	if(typeTrackpad)
	{
		if (_jittermove)
		{
			if (_pADBKeyboard)
			{
				AbsoluteTime	keyboardtime; 
				UInt64		nowtime64, keytime64;
				
				keyboardtime.hi = 0;
				keyboardtime.lo = 0;
				_pADBKeyboard->callPlatformFunction(_gettime, false, (void *)&keyboardtime, 0, 0, 0);
				absolutetime_to_nanoseconds(now, &nowtime64);
				absolutetime_to_nanoseconds(keyboardtime, &keytime64);
				if (nowtime64 - keytime64 < _jitterclicktime64)
				{
					if ((my_abs(dx) < _jitterdelta) && (my_abs(dy) < _jitterdelta))
					{
						if (!buttonState)
						{
							//simulate no mouse move.  Keeps cursor invisible.
							return;
						}
						else
						{
							dx = 0;
							dy = 0;
						}
					}
				}
			}
		} //jittermove
	}
	dispatchRelativePointerEvent(dx, dy, buttonState, now);
}
#endif

/*
 * The following ::packetW() assumes two flags:  
 * _isWEnhanced is true and typeTrackpad is true.
 */
// modified dub:
//void iScroll2::packetW(UInt8 /*adbCommand*/, IOByteCount length, UInt8 * data)
void iScroll2::packet(UInt8 adbCommand, IOByteCount length, UInt8 * data)
{
	#pragma unused (adbCommand)
// end modifications
	int				dx, dy, cnt, numExtraBytes;
	bool			palm = false, outzone = false, has2fingers = false;
	UInt32          buttonState = 0;
	AbsoluteTime	now;
	bool			shouldIgnore;
	
// modified dub:
/*
	if (_usePantherSettings)
	{
		packetWP(0, length, data);
		return;
	}
*/
	IOLockLock(_mouseLock);
	shouldIgnore = (_ignoreTrackpad && (_externalMice && (_externalMice->getCount() > 0 )));  
	IOLockUnlock(_mouseLock);
	if(shouldIgnore) return;
// end modifications
	
	numExtraBytes = length - 2;
	dy = data[0] & 0x7f;
	dx = data[1] & 0x7f;
	
// modified dub:
// do this later on
//	if ((data[0] & 0x80) == 0) 
//	{
//		buttonState |= 1;
//	}
// end modifications
	
	//Here we check for inadvertent palm presses and outside-zone clicks.  In theory I 
	//  should check for gesture clicks when outzone below is true, but the
	//  trackpad never picks up on that.  Can't make gesture clicks using edge of
	//  my palm either.
	//if(_isWEnhanced)  //This also implies typeTrackpad == true
	{
// modified dub:
		// it appears this never actually gets set by the trackpad?
// end modifications
		if ((data[2] & 0x80) == 0)
		{
//			IOLog("%s: OutZone detected!\n", getName());
			if (_zonenomove)
				outzone = true;
		}
		if ((data[2] & 0x08) == 0)
		{
			if (_palmnomove)
				palm = true;
		}
		if ((data[3] & 0x80) == 0)
		{
			// modified dub:
			if (_2fingernoaction || _enableScrollX || _enableScrollY || _enableScrollRot)
			// end modifications
			{
			
				has2fingers = true;
				_sticky2finger = true;	    
				//I need to log when it became sticky for timeout purposes
				clock_get_uptime(&_sticky2fingerTimeAB);
			
			}
		}
		if ((data[3] & 0x08) != 0)
		{
			//All fingers are off the trackpad now, so clear sticky bit
			_sticky2finger = false;
// modified dub:
			_stickyRotating = false;
			_oldScrollX = 0;
			_oldScrollY = 0;
// end modifications
		}
		//Fake a 2 finger contact if sticky bit is still on.  But calculate time delta first.
		if (_sticky2finger)
		{
			AbsoluteTime	nowtimeAB;
			
			clock_get_uptime(&nowtimeAB);
			//if (nowtime64 - _sticky2fingerTime64 < _timeoutSticky64)
			SUB_ABSOLUTETIME(&nowtimeAB, &_sticky2fingerTimeAB);
			if ( CMP_ABSOLUTETIME(&nowtimeAB, &_timeoutStickyAB) == -1)
			{
				has2fingers = true;
			}
			else
			{
				_sticky2finger = false;  //Make it more efficient
			}
		}
		
		//Apple trackpads should not have more than two buttons.  Non-Apple trackpads
		// should never match the "tpad" signature anyways.
		data[2] |= 0x88;  //clear two button bits, both negative logic
		data[3] |= 0x88;	//Assume Apple trackpads will never have multiple buttons for ADB
	}
	
// modified dub:
	if ((data[0] & 0x80) == 0) 
	{
		buttonState |= has2fingers ? (1 << _twoFingerClickMapTo) : (1 << _clickMapTo);
	}
//	if ((deviceNumButtons > 1) && ((data[1] & 0x80) == 0))
	if ((data[1] & 0x80) == 0)
// end modifications
	{
		//if(typeTrackpad)
    {
// modified dub:
//		if(has2fingers) IOLog("%s: Two-finger tap detected.\n", getName());
// end modifications
		if ((_jitterclick) && (_pADBKeyboard))
		{
			_keyboardTimeAB.hi = 0;
			_keyboardTimeAB.lo = 0;
			_pADBKeyboard->callPlatformFunction(_gettime, false, 
												(void *)&_keyboardTimeAB, 0, 0, 0);
			clock_get_uptime(&now);
			SUB_ABSOLUTETIME(&now, &_keyboardTimeAB);
			if ( CMP_ABSOLUTETIME(&now, &_jitterclicktimeAB) == 1)	    
				//if (nowtime64 - keytime64 > _jitterclicktime64)
			{
// modified dub:
//				buttonState |= 1;
				buttonState |= (1 << _tapMapTo);
// end modifications
				//This part automatically takes care of narrow objects along outside zones.
				//It is accepted if no recent typing, it is filtered if recent typing
				//The rejection is taken care of by code below, not here
			}
			//This is new for WEnhanced trackpads: ANYTHING in center of pad is ALWAYS accepted.
			//  That means even when typing.... this differs from old behavior 
			if (!outzone)
			{
// modifed dub:
//				buttonState |= 1;  // EB .. this could come first
				buttonState |= (1 << _tapMapTo);  // EB .. this could come first
// end modifications
			}
			if (CMP_ABSOLUTETIME( &_zonepeckingtimeAB, &_keyboardTimeAB) != 0)
			{
				buttonState = 0;
			}
		}
		else
// modified dub:
//			buttonState |= 1;
			buttonState |= (1 << _tapMapTo);
// end modifications
    }
	}
	
	for (cnt = 0; cnt < numExtraBytes; cnt++) {
		dy |= ((data[2 + cnt] >> 4) & 7) << (7 + (cnt * 3));
		dx |= ((data[2 + cnt])      & 7) << (7 + (cnt * 3));
		
// modified dub:
//		if ((deviceNumButtons > (cnt + 2)) && ((data[2 + cnt] & 0x80) == 0))
//			buttonState |= 4 << (cnt * 2);
//		if ((deviceNumButtons > (cnt + 2 + 1)) && ((data[2 + cnt] & 0x08) == 0))
//			buttonState |= 4 << (cnt * 2 + 1);
// end modifications
	}
	
	if (dy & (0x40 << (numExtraBytes * 3)))
		dy |= (0xffffffc0 << (numExtraBytes * 3));
	if (dx & (0x40 << (numExtraBytes * 3)))
		dx |= (0xffffffc0 << (numExtraBytes * 3));
	
	clock_get_uptime(&now);
	
    //If UI sets _jittermove, then we need special filtering for W Enhanced trackpads.
    //  That checkbox implies that ALL multi-finger presses are filtered.
    //  Also, if both outzone and palm are true, meaning palm on outside edges,
    //  then that is always filtered.
    if (_jittermove) 
    {
		
// modified dub:
//		if ((has2fingers) || (outzone && palm))
		if ((!(_enableScrollX || _enableScrollY || _enableScrollRot) && has2fingers) || (outzone && palm))
// end modifications
		
		{
			if (!buttonState)
			{
				//simulate no mouse move.  Keeps cursor invisible.
				return;
			}
			else if ((data[1] & 0x80) == 0) //filter out gesture clicks
			{
				return;
			}
			else
			{
				dx = 0;
				dy = 0;
			}
		}
		
		//This is new for WEnhanced trackpads: ANYTHING in center of pad is ALWAYS accepted
		// unless 2 finger (or sticky 2 finger)  is active above.  Note also that palm
		// input is rejected above ONLY if the palm is along the edge.  Palm contact in
		// center of pad will always be allowed.
		if (( !outzone) && (_pADBKeyboard))
		{	    
			_keyboardTimeAB.hi = 0;
			_keyboardTimeAB.lo = 0;
			_pADBKeyboard->callPlatformFunction(_gettime, false, (void *)&_zonepeckingtimeAB, 0, 0, 0);
		} else
			if (_pADBKeyboard)
			{
				AbsoluteTime	  sub_now;
				
				_keyboardTimeAB.hi = 0;
				_keyboardTimeAB.lo = 0;
				_pADBKeyboard->callPlatformFunction(_gettime, false, (void *)&_keyboardTimeAB, 0, 0, 0);
				nanoseconds_to_absolutetime( (unsigned long long)( (unsigned long long)(5 * 60 * 1000) 
					* (unsigned long long)(1000 * 1000) ), &_fake5minAB);
				sub_now = now;
				SUB_ABSOLUTETIME(&sub_now, &_keyboardTimeAB); 
				if (CMP_ABSOLUTETIME (&sub_now, &_fake5minAB) == -1) 
					//if (nowtime64 - keytime64 < _fake5min)
				{
					//This part rejects all movements along the edge when typing.  By
					//  logic that means narrow contact along edge.  Wide contact along
					//  edge (regardless of recent keyboarding) was already filtered out above.
					//if  (_zonepeckingtime64 != keytime64)
					if ( CMP_ABSOLUTETIME ( &_zonepeckingtimeAB, &_keyboardTimeAB) != 0)
					{
						if (!buttonState)
						{
							//simulate no mouse move.  Keeps cursor invisible.
							return;
						}
						else if ((data[1] & 0x80) == 0) 
						{
							return;
						}
						else
						{
							dx = 0;
							dy = 0;
						}
					}
				}
			}
    } //jittermove
	
	// modified dub:
	if((_enableScrollX || _enableScrollY || _enableScrollRot) && has2fingers)
	{
		bool skipLinear = _enableScrollRot && _scrollOnlyRot;
		AbsoluteTime sub_now = now;	 
		bool withhold;	 
		SUB_ABSOLUTETIME(&sub_now, &_scrollMinDelayAB);	 
		withhold = CMP_ABSOLUTETIME(&sub_now, &_lastScrollTimeAB) == -1;	 
		if(_enableScrollRot)
		{
			short angleSquared = _oldScrollY * dx - _oldScrollX * dy;
			angleSquared = 100 * my_abs(angleSquared) * angleSquared / 
				((_oldScrollX * _oldScrollX + _oldScrollY * _oldScrollY) * (dx * dx + dy * dy));
			_scrollRot += my_sgn(_scrollInvertRot ? -angleSquared : angleSquared) * (my_abs(dx) + my_abs(dy));
			if(_stickyRotating || (my_abs(_scrollRot) >= _scrollThreshRot))
			{
				if((_scrollRot / _scrollScaleRot) && !withhold)
				{
					dispatchScrollWheelEvent(_scrollRot / _scrollScaleRot, 0, 0, now);
					_scrollRot %= _scrollScaleRot;
					_lastScrollTimeAB = now;
				}
				skipLinear = true;
			}
		}
		if(!skipLinear)
		{
//			bool scrolled = false;
			if(_enableScrollX && ((my_abs(dx) >= _scrollThreshX) 
				&& (!_scrollDominantAxisOnly || (my_abs(dx) >= my_abs(dy)))))
				_scrollX += (_scrollInvertX ? dx : -dx);
			if(_enableScrollY && ((my_abs(dy) >= _scrollThreshY) 
				&& (!_scrollDominantAxisOnly || (my_abs(dy) >= my_abs(dx)))))
				_scrollY += (_scrollInvertY ? dy : -dy);
			if(((_scrollX / _scrollScaleX) || (_scrollY / _scrollScaleY)) && !withhold)
			{
				dispatchScrollWheelEvent(_scrollY / _scrollScaleY, _scrollX / _scrollScaleX, 0, now);
				_scrollX %= _scrollScaleX;
				_scrollY %= _scrollScaleY;
				_lastScrollTimeAB = now;
			}
/*			if((_scrollX / _scrollScaleX) && !withhold)
			{
				for(int i = 1; i <= _scrollX / _scrollScaleX; i++)
				{
					dispatchScrollWheelEvent(0, 1, 0, _lastScrollTimeAB + (i / (_scrollX / _scrollScaleX) * (now - _lastScrollTimeAB));
				}
				_scrollX = 0;
				scrolled = true;
			}
			if((_scrollY / _scrollScaleY)) && !withhold)
			{
				for(int i = 1; i <= _scrollY / _scrollScaleY; i++)
				{
					dispatchScrollWheelEvent(1, 0, 0, _lastScrollTimeAB + (i / (_scrollY / _scrollScaleY) * (now - _lastScrollTimeAB));
				}
				_scrollY = 0;
				scrolled = true;
			}
			if(scrolled) _lastScrollTimeAB = now;*/
		}
		if(buttonState != _oldButtonState)
		{
			dispatchRelativePointerEvent(0, 0, buttonState, now);
			_oldButtonState = buttonState;
		}
		_oldScrollX = dx;
		_oldScrollY = dy;
	}
	else
	{
		dispatchRelativePointerEvent(dx, dy, buttonState, now);
		_oldButtonState = buttonState;
	}
	// end modifications
	
}


#if 0
/*
 *  Next method is mostly for exploring future Panther filtering for trackpads.
 *  Since it is called by ::packetW(), it also assumes a W enhanced trackpad.
 */
void iScroll2::packetWP(UInt8 /*adbCommand*/, IOByteCount length, UInt8 * data)
{
	int		  dx, dy, cnt, numExtraBytes;
	bool		  palm = false, outzone = false, has2fingers = false;
	UInt32          buttonState = 0;
	AbsoluteTime	  now;
	
	numExtraBytes = length - 2;
	dy = data[0] & 0x7f;
	dx = data[1] & 0x7f;
	
	if ((data[0] & 0x80) == 0) 
	{
		buttonState |= 1;
	}
	
	//Here we check for inadvertent palm presses and outside-zone clicks.  In theory I 
	//  should check for gesture clicks when outzone below is true, but the
	//  trackpad never picks up on that.  Can't make gesture clicks using edge of
	//  my palm either.
	//if(typeTrackpad)
	{
		if ((data[2] & 0x80) == 0)
		{
			outzone = true;
		}
		if ((data[2] & 0x08) == 0)
		{
			palm = true;
		}
		if ((data[3] & 0x80) == 0)
		{
			if (_2fingernoaction)
			{
				has2fingers = true;
				_sticky2finger = true;
				//I need to log when it became sticky for timeout purposes
				clock_get_uptime(&now);
				absolutetime_to_nanoseconds(now, &_sticky2fingerTime64);	    
			}
		}
		if ((data[3] & 0x08) != 0)
		{
			//All fingers are off the trackpad now, so clear sticky bit
			_sticky2finger = false;	  
		}
		//Fake a 2 finger contact via palm bit if _sticky2finger is still on
		if (_sticky2finger)
		{
			UInt64	nowtime64;
			
			clock_get_uptime(&now);
			absolutetime_to_nanoseconds(now, &nowtime64);
			if (nowtime64 - _sticky2fingerTime64 < _timeoutSticky64)
			{
				palm = true;
			}
			else
			{
				_sticky2finger = false;  //Make it more efficient
			}
		}
		
		//Apple trackpads should not have more than two buttons.  Non-Apple trackpads
		// should never match the "tpad" signature anyways.
		data[2] |= 0x88;  //clear two button bits, both negative logic
		data[3] |= 0x88;	//Assume Apple trackpads will never have multiple buttons for ADB
	}
	
	if ((deviceNumButtons > 1) && ((data[1] & 0x80) == 0))
	{
		//if(typeTrackpad)
    {
		if ((_jitterclick) && (_pADBKeyboard))
		{
			AbsoluteTime	keyboardtime; 
			UInt64		nowtime64, keytime64;
			
			keyboardtime.hi = 0;
			keyboardtime.lo = 0;
			_pADBKeyboard->callPlatformFunction(_gettime, false, 
												(void *)&keyboardtime, 0, 0, 0);
			clock_get_uptime(&now);
			absolutetime_to_nanoseconds(now, &nowtime64);
			absolutetime_to_nanoseconds(keyboardtime, &keytime64);
			if (nowtime64 - keytime64 > _jitterclicktime64)
			{
				buttonState |= 1;
			}
		}
		else
			buttonState |= 1;
    }
	}
	
	for (cnt = 0; cnt < numExtraBytes; cnt++) {
		dy |= ((data[2 + cnt] >> 4) & 7) << (7 + (cnt * 3));
		dx |= ((data[2 + cnt])      & 7) << (7 + (cnt * 3));
		
		if ((deviceNumButtons > (cnt + 2)) && ((data[2 + cnt] & 0x80) == 0))
			buttonState |= 4 << (cnt * 2);
		if ((deviceNumButtons > (cnt + 2 + 1)) && ((data[2 + cnt] & 0x08) == 0))
			buttonState |= 4 << (cnt * 2 + 1);
	}
	
	if (dy & (0x40 << (numExtraBytes * 3)))
		dy |= (0xffffffc0 << (numExtraBytes * 3));
	if (dx & (0x40 << (numExtraBytes * 3)))
		dx |= (0xffffffc0 << (numExtraBytes * 3));
	
	clock_get_uptime(&now);
	if(typeTrackpad )
	{
		//The UI is expected to affect _palmnoaction but not _zonenoaction.  However,
		//  I am leaving _zonenoaction to a default of false.  Someone may find
		//  it useful some day and set it to true.
		if ((_2fingernoaction && has2fingers) || (_palmnoaction && palm) || 
			(_zonenoaction && outzone))
		{
			if (!buttonState)
			{
				//simulate no mouse move.  Keeps cursor invisible.
				return;
			}
			else
			{
				dx = 0;
				dy = 0;
			}
		}
		
		if (_jittermove)
		{
			if (_pADBKeyboard)
			{
				AbsoluteTime	keyboardtime; 
				UInt64		nowtime64, keytime64;
				
				keyboardtime.hi = 0;
				keyboardtime.lo = 0;
				_pADBKeyboard->callPlatformFunction(_gettime, false, (void *)&keyboardtime, 0, 0, 0);
				absolutetime_to_nanoseconds(now, &nowtime64);
				absolutetime_to_nanoseconds(keyboardtime, &keytime64);
				if (nowtime64 - keytime64 < _jitterclicktime64)
				{
					if ((my_abs(dx) < _jitterdelta) && (my_abs(dy) < _jitterdelta))
					{
						if (!buttonState)
						{
							//simulate no mouse move.  Keeps cursor invisible.
							return;
						}
						else
						{
							dx = 0;
							dy = 0;
						}
					}
				}
			}
		} //jittermove
		
		//I am assuming the UI eventually will make _zonenomove and _jittermove mutually
		//  exclusive, but for now I'll allow possibility of both being active.  Actions
		//  in _jitternomove code above take precedence, of course, and they affect the
		//  entire trackpad, not just certain zones.  The UI should not make _zonenomove
		//  available for older PowerBooks
		if (_zonenomove)
		{
			if (_pADBKeyboard)
			{
				AbsoluteTime	keyboardtime; 
				UInt64		nowtime64, keytime64;
				
				keyboardtime.hi = 0;
				keyboardtime.lo = 0;
				_pADBKeyboard->callPlatformFunction(_gettime, false, (void *)&keyboardtime, 0, 0, 0);
				absolutetime_to_nanoseconds(now, &nowtime64);
				absolutetime_to_nanoseconds(keyboardtime, &keytime64);
				if (nowtime64 - keytime64 < _jitterclicktime64)
				{
					//_palmnomove will not be in UI and it should default to off, but I
					//  am leaving this property here for now in case someone finds it useful.
					//  _palmnoaction is intended for the main palm check feature and
					//  supercedes _palmnomove.  Note also that I am rejecting even valid
					//  clicks from the physical button (not just gestures) because the
					//  entire palm may be on the trackpad
					if (_palmnomove && palm)
					{
						return;
					}
					
					//If it has been a while since the user last touched the ADB keyboard,
					//  then movements should be allowed anywhere on trackpad, but 
					//  then we would not be in this part of the driver anyways.
					if (outzone)
					{
						// _ignorezone is set below if user touches middle (major) zone of trackpad.
						// _ignorezone is set to false initially in ::enableEnhancedMode()
						if (!_ignorezone)
						{
							if (!buttonState)
							{
								return;
							}
							else if ((data[1] & 0x80) == 0) //filter out gesture clicks
							{
								return;
							}
							else
							{
								dx = 0;
								dy = 0;
							}
						}
					}
					else
					{
						//If user touches within center of trackpad and not outside edges, 
						//  assume the touch is intentional so then set a flag so that even
						//  if the user strays to edges again within our time period, those
						//  moves will not be filtered out
						_ignorezone = true;
					}
				}
				else //if long time has passed, reset zone checking for subsequent keyboard clicks
				{
					_ignorezone = false;
				}
			}
		} //_zonenomove
		
		
		//Again I am assuming the UI will make following _zonepecknomove mutually exclusive from 
		// _zonenomove and _jittermove above, meaning only ONE of the three would be active.
		// However, to keep our options flexible for now I will allow all three to be active
		// simultaneously.
		if (_zonepecknomove)
		{
			if (_pADBKeyboard)
			{
				AbsoluteTime	keyboardtime; 
				UInt64		keytime64;
				
				keyboardtime.hi = 0;
				keyboardtime.lo = 0;
				_pADBKeyboard->callPlatformFunction(_gettime, false, (void *)&keyboardtime, 0, 0, 0);
				absolutetime_to_nanoseconds(keyboardtime, &keytime64);
				if (outzone)
				{
					//if user had pressed a new keyboard key since the last time the trackpad
					//  was touched in the center zone, we need to cut that trackpad movement out.
					//  This is intended as a PERMANENT filtering for as long as the user does 
					//  not touch the center zone of the trackpad
					if  (_zonepeckingtime64 != keytime64)
					{
						if (!buttonState)
						{
							return;
						}
						else if ((data[1] & 0x80) == 0) //filter out gesture clicks
						{
							return;
						}
						else
						{
							dx = 0;
							dy = 0;
						}
					}
				}
				else
				{
					//Save time of last keypress since user is moving withing center zone now
					_zonepeckingtime64 = keytime64;
				}
				
			}
		} // _zonepecknomove
		
	}
	dispatchRelativePointerEvent(dx, dy, buttonState, now);
}
#endif


OSData * iScroll2::copyAccelerationTable()
{
    char keyName[10];
	
    strcpy( keyName, "accl" );
    keyName[4] = (deviceSignature >> 24);
    keyName[5] = (deviceSignature >> 16);
    keyName[6] = (deviceSignature >> 8);
    keyName[7] = (deviceSignature >> 0);
    keyName[8] = 0;
	
    IOLockLock( _mouseLock);
	
    OSData * data = OSDynamicCast( OSData,
								   getProperty( keyName ));
    IOLockUnlock( _mouseLock);
	
    if( data)
    {
        data->retain();
    }
    else
    {
        data = super::copyAccelerationTable();
    }
	
    return( data );
}

// ****************************************************************************
// enableEnhancedMode
//
// ****************************************************************************

bool iScroll2::enableEnhancedMode()
{
    UInt8		adbdata[8];
    IOByteCount	adblength = 8;
    OSNumber	*plistnum;
    
    IOLog("%s: enableEnhancedMode called.\n", getName());
    adbDevice->readRegister(1, adbdata, &adblength);
	
    if((adbdata[6] != 0x0D))
    {
        adbdata[6] = 0xD;
        if (adbDevice->writeRegister(1, adbdata, &adblength) != 0)
            return FALSE;
        if (adbDevice->readRegister(1, adbdata, &adblength) != 0)
            return FALSE;
        if (adbdata[6] != 0x0D)
        {
// modified dub:
            IOLog("%s: deviceClass = 0x%x (non-Extended Mode)\n", getName(), adbdata[6]);
// end modifications
            return FALSE;
        }
// modified dub:
        IOLog("%s: deviceClass = 0x%x (Extended Mode, scrolling supported)\n", getName(), adbdata[6]);
// end modifications
		
        // Set ADB Extended Features to default values.
        adbdata[0] = 0x19;  //MSB=Use Soft click (gesture).  7 bits for DownTime.
        adbdata[1] = 0x14;
        adbdata[2] = 0x19;
        adbdata[3] = 0xB2;  //MSB=Use sticky drag.  7 bits for StickyDragTime.
        adbdata[4] = 0xB2;
        adbdata[5] = 0x8A;
        adbdata[6] = 0x1B;
		//WARNING... next one may have a conflict
        //adbdata[7] = 0x50;
        adbdata[7] = 0x57;  //bits 3:0 for W mode
        adblength = 8;
		
        adbDevice->writeRegister(2, adbdata, &adblength);
		
        /* Add IORegistry entries for Enhanced mode */
        Clicking = FALSE;
        Dragging = FALSE;
        DragLock = FALSE;
        
        setProperty("Clicking", (unsigned long long)Clicking, sizeof(Clicking)*8);
        setProperty("Dragging", (unsigned long long)Dragging, sizeof(Dragging)*8);
        setProperty("DragLock", (unsigned long long)DragLock, sizeof(DragLock)*8);
		
		/* check for jitter correction initially before Mouse Preferences
			calls setParamProperties*/
		plistnum = OSDynamicCast( OSNumber, getProperty("JitterNoClick"));
		if (plistnum)
		{
			_jitterclick = (bool) plistnum->unsigned16BitValue();
		}
		else
		{
			_jitterclick = false;
		}
		
		plistnum = OSDynamicCast( OSNumber, getProperty("JitterNoMove"));
		if (plistnum)
		{
			_jittermove = (bool) plistnum->unsigned16BitValue();
		}
		else
		{
			_jittermove = false;
		}	
		
// modified dub:
//		if (_isWEnhanced)  //W enhanced trackpads should get filtering on by default
//		{
			setProperty("JitterNoClick", true, sizeof(UInt32));
			setProperty("JitterNoMove", true, sizeof(UInt32));
			_jitterclick = _jittermove = true;
//		}
// end modifications
		
		plistnum = OSDynamicCast( OSNumber, getProperty("W sticky input timeout"));
		if (plistnum)
		{
			_timeoutSticky64 = plistnum->unsigned32BitValue() * 1000 * 1000;  // in nanoseconds;
			nanoseconds_to_absolutetime(_timeoutSticky64, &_timeoutStickyAB);
		}
		else
		{
			_timeoutSticky64 = 500 * 1000 * 1000;  // 500 millisecond sticky timeout
			nanoseconds_to_absolutetime(_timeoutSticky64, &_timeoutStickyAB);
		}	
		
		
		plistnum = OSDynamicCast( OSNumber, getProperty("W threshold"));
		if (plistnum)
		{
			_WThreshold = (UInt8) plistnum->unsigned16BitValue();
		}
		else
		{
			_WThreshold = 7;  //Arbitrary value for medium size thumbs
		}	
		
		plistnum = OSDynamicCast( OSNumber, getProperty("TwofingerNoAction"));
		if (plistnum)
		{
			_2fingernoaction = (bool) plistnum->unsigned16BitValue();
		}
		else
		{
			_2fingernoaction = false;
		}	
		
		plistnum = OSDynamicCast( OSNumber, getProperty("PalmNoAction Permanent"));
		if (plistnum)
		{
			_palmnoaction = (bool) plistnum->unsigned16BitValue();
		}
		else
		{
			_palmnoaction = false;
		}	
		
		plistnum = OSDynamicCast( OSNumber, getProperty("PalmNoAction When Typing"));
		if (plistnum)
		{
			_palmnomove = (bool) plistnum->unsigned16BitValue();
		}
		else
		{
			_palmnomove = false;
		}	
		
		plistnum = OSDynamicCast( OSNumber, getProperty("OutsidezoneNoAction Permanent"));
		if (plistnum)
		{
			_zonenoaction = (bool) plistnum->unsigned16BitValue();
		}
		else
		{
			_zonenoaction = false;
		}	
		
		plistnum = OSDynamicCast( OSNumber, getProperty("OutsidezoneNoAction When Typing"));
		if (plistnum)
		{
			_zonenomove = (bool) plistnum->unsigned16BitValue();
		}
		else
		{
			_zonenomove = false;
		}
		
		plistnum = OSDynamicCast( OSNumber, getProperty("OutsidezoneNoAction When Pecking"));
		if (plistnum)
		{
			_zonepecknomove = (bool) plistnum->unsigned16BitValue();
		}
		else
		{
			_zonepecknomove = false;
		}
		
		_ignorezone = false;
		_zonepeckingtime64 = 0;	
		nanoseconds_to_absolutetime(0, &_zonepeckingtimeAB);
		
// modified dub:
		nanoseconds_to_absolutetime(0, &_lastScrollTimeAB);	 
// end modifications
		
		return TRUE;
    }
	
// modified dub:
//    return FALSE;
	return TRUE;
// end modifications
}

// ****************************************************************************
// setParamProperties
//
// ****************************************************************************
IOReturn iScroll2::setParamProperties( OSDictionary * dict )
{
    OSNumber 	*datan;
// modified dub:
	OSBoolean	*datab;
	OSDictionary*datad;
	OSData		*data;
	OSString	*datas;
// end modifcations
    IOReturn	err = kIOReturnSuccess;
    UInt8       adbdata[8];
    IOByteCount adblength;
    UInt16	settrue = 0;
	
// modified dub:
//	if (typeTrackpad == TRUE) 
//    {  
// end modifications
		IOLockLock( _mouseLock); 
		if (datan = OSDynamicCast(OSNumber, dict->getObject("Clicking"))) 
		{	  
			settrue = 0;
			if (datan->unsigned32BitValue())
			{
				settrue = 1;	//Guard against values that are neither 0 nor 1
			}
			adblength = sizeof(adbdata);
			adbDevice->readRegister(2, adbdata, &adblength);
			adbdata[0] = (adbdata[0] & 0x7F) | ( settrue <<7);
			setProperty("Clicking", (unsigned long long)((adbdata[0]&0x80)>>7), sizeof(adbdata[0])*8);
			adbDevice->writeRegister(2, adbdata, &adblength);
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("Dragging"))) 
		{
			settrue = 0;
			if (datan->unsigned32BitValue())
			{
				settrue = 1;
			}
			adblength = sizeof(adbdata);
			adbDevice->readRegister(2, adbdata, &adblength);
			adbdata[1] = (adbdata[1] & 0x7F) | (settrue <<7);
			setProperty("Dragging", (unsigned long long)((adbdata[1]&0x80)>>7), sizeof(adbdata[1])*8);
			adbDevice->writeRegister(2, adbdata, &adblength);
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("DragLock"))) 
		{
			adblength = sizeof(adbdata);
			adbDevice->readRegister(2, adbdata, &adblength);
			adbdata[3] = datan->unsigned32BitValue();
			
			if(adbdata[3])
			{
				setProperty("DragLock", (unsigned long long)adbdata[3], sizeof(adbdata[3])*8);
				adbdata[3] = 0xFF;
				adblength = sizeof(adbdata);
				adbDevice->writeRegister(2, adbdata, &adblength);
			}
			else
			{
				setProperty("DragLock", (unsigned long long)adbdata[3], sizeof(adbdata[3])*8);
				adbdata[3] = 0xB2;
				adblength = sizeof(adbdata);
				adbDevice->writeRegister(2, adbdata, &adblength);
			}
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("JitterNoClick"))) 
		{
			_jitterclick = (bool) datan->unsigned32BitValue();
			setProperty("JitterNoClick", _jitterclick, sizeof(UInt32));
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("JitterNoMove"))) 
		{
			_jittermove = (bool) datan->unsigned32BitValue();
			setProperty("JitterNoMove", _jittermove, sizeof(UInt32));
			_jitterclick = _jittermove;
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("Trackpad Jitter Milliseconds"))) 
		{
			_jitterclicktime64 = datan->unsigned32BitValue() * 1000 * 1000;  // in nanoseconds;
			setProperty("Trackpad Jitter Milliseconds", datan->unsigned32BitValue(), 
						8 * sizeof(adbdata[1]));
			nanoseconds_to_absolutetime (_jitterclicktime64, &_jitterclicktimeAB);
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("W sticky input timeout"))) 
		{
			_timeoutSticky64 = datan->unsigned32BitValue() * 1000 * 1000;  // in nanoseconds;
			setProperty("W sticky input timeout", datan->unsigned32BitValue(), 
						8 * sizeof(adbdata[1]));
			nanoseconds_to_absolutetime(_timeoutSticky64, &_timeoutStickyAB);
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("W threshold"))) 
		{
			_WThreshold = (UInt8) datan->unsigned32BitValue();
			_WThreshold &= 0xf;	//f is max threshold
			setProperty("W threshold", _WThreshold, sizeof((adbdata[2]) * 8));
			adblength = sizeof(adbdata);
			adbDevice->readRegister(2, adbdata, &adblength);
			adbdata[7] = adbdata[7] & 0xf0;  //only set bits 3..0
			adbdata[7] = adbdata[7] | _WThreshold;
			
			adblength = sizeof(adbdata);
			adbDevice->writeRegister(2, adbdata, &adblength);
			
#if 0	    
			//Now test it, and log problems
			adblength = sizeof(adbdata);
			adbDevice->readRegister(1, adbdata, &adblength);
			if((adbdata[6] != 0x0D))
			{
				IOLog("Gestures are not in mode 0x0D\n");
			}
			adblength = sizeof(adbdata);
			adbdata[7] = 0;  //sanity check
			adbDevice->readRegister(2, adbdata, &adblength);
#endif
			
		}
        
		
// modified dub:
/*		if (datan = OSDynamicCast(OSNumber, dict->getObject("Use Panther Settings for W"))) 
		{
			_usePantherSettings = (bool) datan->unsigned32BitValue();
			setProperty("Use Panther Settings for W", _usePantherSettings, sizeof(UInt32));
		}*/
// end modifications
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("PalmNoAction Permanent"))) 
		{
			_palmnoaction = (bool) datan->unsigned32BitValue();
			setProperty("PalmNoAction Permanent", _palmnoaction, sizeof(UInt32));
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("PalmNoAction When Typing"))) 
		{
			_palmnomove = (bool) datan->unsigned32BitValue();
			setProperty("PalmNoAction When Typing", _palmnomove, sizeof(UInt32));
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("TwofingerNoAction"))) 
		{
			_2fingernoaction = (bool) datan->unsigned32BitValue();
			setProperty("TwofingerNoAction", _2fingernoaction, sizeof(UInt32));
		}
		
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("OutsidezoneNoAction Permanent"))) 
		{
			_zonenoaction = (bool) datan->unsigned32BitValue();
			setProperty("OutsidezoneNoAction Permanent", _zonenoaction, sizeof(UInt32));
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("OutsidezoneNoAction When Typing"))) 
		{
			_zonenomove = (bool) datan->unsigned32BitValue();
			setProperty("OutsidezoneNoAction When Typing", _zonenomove, sizeof(UInt32));
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("OutsidezoneNoAction When Pecking"))) 
		{
			_zonepecknomove = (bool) datan->unsigned32BitValue();
			setProperty("OutsidezoneNoAction When Pecking", _zonepecknomove, sizeof(UInt32));
		}
		
		if (datan = OSDynamicCast(OSNumber, dict->getObject("USBMouseStopsTrackpad"))) 
		{
			UInt8		mode;
			
			mode = datan->unsigned32BitValue();	
			setProperty("USBMouseStopsTrackpad", (unsigned long long)(mode), sizeof(mode)*8);
			if (mode)
			{                
                _ignoreTrackpad = true;
			}
			else
			{
				_ignoreTrackpad = false;
			}
			
		}
		
// modified dub:
		if(datan = OSDynamicCast(OSNumber, dict->getObject(kTrackpadScrollKey)))
		{
			setProperty(kTrackpadScrollKey, datan->unsigned32BitValue(), 32);
		}
		if(datan = OSDynamicCast(OSNumber, dict->getObject(kTrackpadHorizScrollKey)))
		{
			setProperty(kTrackpadHorizScrollKey, datan->unsigned32BitValue(), 32);
		}
		if(data = OSDynamicCast(OSData, dict->getObject(kHIDScrollAccelerationTableKey)))
		{
			setProperty(kHIDScrollAccelerationTableKey, data->getBytesNoCopy());
		}
		if(datas = OSDynamicCast(OSString, dict->getObject(kHIDPointerAccelerationTypeKey)))
		{
			setProperty(kHIDPointerAccelerationTypeKey, datas->getCStringNoCopy());
		}
		if(datas = OSDynamicCast(OSString, dict->getObject(kHIDScrollAccelerationTypeKey)))
		{
			setProperty(kHIDScrollAccelerationTypeKey, datas->getCStringNoCopy());
		}
		if(datan = OSDynamicCast(OSNumber, dict->getObject(kHIDScrollResolutionKey)))
		{
			setProperty(kHIDScrollResolutionKey, datan->unsigned32BitValue(), 32);
		}
		if(datad = OSDynamicCast(OSDictionary, dict->getObject(kiScroll2SettingsKey)))
		{
			if(datab = OSDynamicCast(OSBoolean, datad->getObject(kTrackpadHScrollKey)))
			{
				_enableScrollX = datab->isTrue();
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadHScrollThresholdKey))) 
			{
				_scrollThreshX = datan->unsigned16BitValue();
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadHScrollScaleKey)))
			{
				_scrollScaleX = datan->unsigned16BitValue();
			}
			if(datab = OSDynamicCast(OSBoolean, datad->getObject(kTrackpadHScrollInvertKey))) 
			{
				_scrollInvertX = datab->isTrue();
			}
			if(datab = OSDynamicCast(OSBoolean, datad->getObject(kTrackpadVScrollKey))) 
			{
				_enableScrollY = datab->isTrue();
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadVScrollThresholdKey))) 
			{
				_scrollThreshY = datan->unsigned16BitValue();
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadVScrollScaleKey)))
			{
				_scrollScaleY = datan->unsigned16BitValue();
			}
			if(datab = OSDynamicCast(OSBoolean, datad->getObject(kTrackpadVScrollInvertKey))) 
			{
				_scrollInvertY = datab->isTrue();
			}
			if(datab = OSDynamicCast(OSBoolean, datad->getObject(kTrackpadCScrollKey))) 
			{
				_enableScrollRot = datab->isTrue();
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadCScrollThresholdKey))) 
			{
				_scrollThreshRot = datan->unsigned16BitValue();
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadCScrollScaleKey))) 
			{
				_scrollScaleRot = datan->unsigned16BitValue();
			}
			if(datab = OSDynamicCast(OSBoolean, datad->getObject(kTrackpadCScrollInvertKey))) 
			{
				_scrollInvertRot = datab->isTrue();
			}
			if(datab = OSDynamicCast(OSBoolean, datad->getObject(kTrackpadCScrollOnlyKey))) 
			{
				_scrollOnlyRot = datab->isTrue();
			}
			if(datab = OSDynamicCast(OSBoolean, datad->getObject(kTrackpadScrollIgnoreWeakAxisKey))) 
			{
				_scrollDominantAxisOnly = datab->isTrue();
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadClickMapToKey))) 
			{
				_clickMapTo = datan->unsigned8BitValue();
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadTapMapToKey))) 
			{
				_tapMapTo = datan->unsigned8BitValue();
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadTwoFingerClickMapToKey))) 
			{
				_twoFingerClickMapTo = datan->unsigned8BitValue();
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadScrollResolutionKey)))
			{
				setProperty(kIOHIDScrollResolutionKey, datan->unsigned32BitValue(), 32);
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadTapDownTimeKey)))
			{
				adblength = sizeof(adbdata);
				adbDevice->readRegister(2, adbdata, &adblength);
				adbdata[0] = (adbdata[0] & (1 << 7) | (datan->unsigned8BitValue() & 0x7f));
				adbDevice->writeRegister(2, adbdata, &adblength);
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadStickyDragTimeKey)))
			{
				adblength = sizeof(adbdata);
				adbDevice->readRegister(2, adbdata, &adblength);
				adbdata[3] = (adbdata[3] & (1 << 7) | (datan->unsigned8BitValue() & 0x7f));
				adbDevice->writeRegister(2, adbdata, &adblength);
			}
			if(datan = OSDynamicCast(OSNumber, datad->getObject(kTrackpadScrollMinDelayKey)))
			{
				UInt16 minDelay = datan->unsigned16BitValue();
				nanoseconds_to_absolutetime(minDelay * 1000 * 1000, &_scrollMinDelayAB);
			}
 			setProperty(kiScroll2SettingsKey, datad);
		}
// end modifications

		IOLockUnlock( _mouseLock);
// modified dub:
//    }  // end of typeTrackpad
// end modifications
#if 0
    // For debugging purposes
    adblength = 8;
    adbDevice->readRegister(2, adbdata, &adblength);
    IOLog("adbdata[0] = 0x%x\n", adbdata[0]);
    IOLog("adbdata[1] = 0x%x\n", adbdata[1]);
    IOLog("adbdata[2] = 0x%x\n", adbdata[2]);
    IOLog("adbdata[3] = 0x%x\n", adbdata[3]);
    IOLog("adbdata[4] = 0x%x\n", adbdata[4]);
    IOLog("adbdata[5] = 0x%x\n", adbdata[5]);
    IOLog("adbdata[6] = 0x%x\n", adbdata[6]);
    IOLog("adbdata[7] = 0x%x\n", adbdata[7]);
#endif
	
    if (err == kIOReturnSuccess)
    {
        return super::setParamProperties(dict);
    }
    
    IOLog("iScroll2::setParamProperties failing here\n");
    return( err );
}
