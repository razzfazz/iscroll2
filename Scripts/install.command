#!/bin/sh

modkext="iScroll2.kext"
modbundleid="name.razzfazz.driver.iScroll2"
origbundleid="com.apple.driver.AppleADBMouse"
extdir="/System/Library/Extensions"

clear
cd `dirname $0`
echo
echo "Checking for other alternative trackpad drivers..."
if kextstat | grep com.ragingmenace.driver.SideTrack > /dev/null
then
    echo "ERROR: SideTrack seems to be installed on your machine."
    echo "iScroll2 is not compatible with SideTrack."
    exit
fi
if kextstat | grep uk.org.outpost.driver.OutpostADBMouse > /dev/null
then
    echo "ERROR: FingaMIDI seems to be installed on your machine."
    echo "iScroll2 is not compatible with FingaMIDI."
    exit
fi
echo "No alternative trackpad drivers detected."
echo
echo "Checking if Apple trackpad driver is loaded..."
if ! kextstat | grep $origbundleid > /dev/null
then 
    echo "ERROR: Apple trackpad driver not loaded."
    echo "Your machine does not seem to have a compatible trackpad, "
    echo "or is using an alternative driver already."
    exit
fi
echo "Driver found."
echo
echo "Checking if trackpad supports W-enhanced mode..."
if ! ioreg -n AppleADBMouseType4 | grep "W Enhanced Trackpad" > /dev/null
then
    echo "ERROR: Your trackpad doesn't seem to support W-enhanced mode."
    echo "W-enhanced mode is necessary for two-finger scrolling to work."
    exit
fi
echo "W-enhanced mode supported."
echo
if ! [ -d ./$modkext ]
then
    echo "ERROR: Modified driver kernel extension not found."
    echo "Please run this script from the directory you unpacked the driver to."
    exit
fi
echo "This will PERMANENTLY install the modified trackpad driver."
echo "If you want to revert to using the original driver, use the supplied uninstall script."
echo
echo "Are you sure you want to continue?"
echo "Press CTRL-C or close this window to abort, RETURN to continue."
read
echo
echo "When prompted for a password, enter your admin password."
echo
echo "Installing modified driver..."
if ! sudo cp -pR ./$modkext $extdir
then
    echo "ERROR: Unable to copy modified driver to $extdir."
    exit
fi
sudo chown -R root:wheel $extdir/$modkext
sudo chmod -R u=rwX $extdir/$modkext
sudo chmod -R go=rX $extdir/$modkext
echo "Modified driver installed to $extdir."
echo
echo "Do you want to activate the modified driver now?"
echo "Press CTRL-C or close this window to cancel (modified driver will be activated at next reboot), or press RETURN to activate the driver now."
read
echo
echo "Unloading the original driver..."
if ! sudo kextunload -b $origbundleid
then
    echo "ERROR: Unable to unload the original driver."
    exit
fi
echo "Original driver unloaded."
echo
echo "Loading the modified driver..."
if ! sudo kextload -b $modbundleid
then
    echo "ERROR: Unable to load the modified driver."
    echo "Reloading the original driver."
    if ! sudo kextload -b $origbundleid
    then
        echo "ERROR: Unable to reload the original driver."
        echo "DO NOT TOUCH YOUR TRACKPAD NOW OR YOUR COMPUTER WILL CRASH!"
    fi
    exit
fi
echo "Driver replaced successfully."
echo