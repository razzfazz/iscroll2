#!/bin/sh

clear
echo
echo "Checking if your trackpad supports two-finger scrolling..."
if ! kextstat | grep com.apple.driver.AppleADBMouse > /dev/null
then 
    echo "Apple trackpad driver not loaded."
    echo "Your machine does not seem to have a compatible trackpad, or is using an alternative driver already."
    exit
fi
if ! ioreg -n AppleADBMouseType4 | grep "W Enhanced Trackpad" > /dev/null
then
    echo "Your trackpad does not seem to support W-enhanced mode."
    echo "W-enhanced mode is necessary for two-finger scrolling to work."
    exit
fi
echo "Your machine should support two-finger scrolling."
echo