#!/bin/sh

modkext="iScroll2.kext"
origbundleid="com.apple.driver.AppleADBMouse"
modbundleid="name.razzfazz.driver.iScroll2"
extdir="/System/Library/Extensions"

clear
echo
echo "This will uninstall the modified driver and restore the original Apple-supplied trackpad driver."
echo
echo "Are you sure you want to continue?"
echo "Press CTRL-C or close this window to abort, RETURN to continue."
read
echo
echo "When prompted for a password, enter your admin password."
echo
if [ -d $extdir/$modkext ]
then
    echo "Removing modified driver..."
    if ! sudo rm -R $extdir/$modkext
    then
        echo "ERROR: Unable to remove modified driver."
        exit
    fi
    echo "Modified driver removed."
    echo
else
    echo "Modified driver not found."
    exit
fi
echo
echo "Do you want to reactivate the original driver now?"
echo "Press CTRL-C or close this window to cancel (original driver will be activated at next reboot), or press RETURN to activate the driver now."
read
echo
echo "Unloading the modified driver..."
if ! sudo kextunload -b $modbundleid
then
    echo "ERROR: Unable to unload the modified driver."
    exit
fi
echo "Modified driver unloaded."
echo
echo "Loading the original driver..."
if ! sudo kextload -b $origbundleid
then
    echo "ERROR: Unable to load the original driver."
    exit
fi
echo "Driver replaced successfully."
echo