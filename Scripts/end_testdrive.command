#!/bin/sh

origbundleid="com.apple.driver.AppleADBMouse"
modbundleid="name.razzfazz.driver.iScroll2"

clear
echo
echo "Checking if trackpad driver is loaded..."
if ! kextstat | grep $modbundleid > /dev/null
then 
    echo "ERROR: Modified trackpad driver not loaded."
    echo "Your machine does not seem to have a compatible trackpad, or is using an alternative driver already."
    exit
fi
echo "Driver found."
echo
echo "This will unload the modified trackpad driver and load the original one."
echo
echo "Are you sure you want to continue?"
echo "Press CTRL-C or close this window to abort, RETURN to continue."
read
echo
echo "When prompted for a password, enter your admin password."
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
    echo "DO NOT TOUCH YOUR TRACKPAD NOW OR YOUR COMPUTER WILL CRASH!"
    exit
fi
echo "Driver replaced successfully."
echo