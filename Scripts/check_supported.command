#!/bin/sh

clear
echo
echo "Checking for other alternative trackpad drivers..."
if kextstat | grep com.ragingmenace.driver.SideTrack > /dev/null
then
    echo "SideTrack seems to be installed on your machine."
    echo "iScroll2 is not compatible with SideTrack."
    exit
fi
if kextstat | grep uk.org.outpost.driver.OutpostADBMouse > /dev/null
then
    echo "FingaMIDI seems to be installed on your machine."
    echo "iScroll2 is not compatible with FingaMIDI."
    exit
fi
echo "No alternative trackpad drivers detected."
echo
echo "Checking if supported trackpad driver is loaded..."
if ! (kextstat | grep com.apple.driver.AppleADBMouse > /dev/null || \
    kextstat | grep name.razzfazz.iScroll2 > /dev/null || \
    kextstat | grep name.razzfazz.driver.iScroll2 > /dev/null)
then 
    echo "Apple trackpad driver not loaded."
    echo "Your machine does not seem to have a compatible trackpad, or is using an unknown alternative driver already."
    exit
fi
echo "Supported trackpad driver found."
echo
echo "Checking if trackpad hardware supports scrolling..."
if ! (ioreg -n AppleADBMouseType4 | grep "W Enhanced Trackpad" > /dev/null || \
    $ioreg -n iScroll2 | grep "W Enhanced Trackpad" > /dev/null)
then
    echo "Your trackpad does not seem to support W-enhanced mode."
    echo "W-enhanced mode is necessary for two-finger scrolling to work."
    exit
fi
echo "Your machine should support two-finger scrolling."
echo