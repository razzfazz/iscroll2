#!/bin/sh

origbundleid="com.apple.driver.AppleADBMouse"
modkext="iScroll2.kext"

clear
cd `dirname $0`
echo
echo "Checking if Apple trackpad driver is loaded..."
if ! kextstat | grep $origbundleid > /dev/null
then 
    echo "ERROR: Apple trackpad driver not loaded."
    echo "Your machine does not seem to have a compatible trackpad, or is using an alternative driver already."
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
echo "This will TEMPORARILY load the modified trackpad driver."
echo "The system will revert to using the original driver automatically after the next reboot."
echo
echo "Are you sure you want to continue?"
echo "Press CTRL-C or close this window to abort, RETURN to continue."
read
echo
echo "When prompted for a password, enter your admin password."
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
sudo chmod -R u=rwX ./$modkext
sudo chmod -R go=rX ./$modkext
sudo chown -R root:wheel ./$modkext
if ! sudo kextload ./$modkext
then
    echo "ERROR: Unable to load the modified driver."
    echo "Reloading the original driver."
    if ! sudo kextload -b $origbundleid
    then
        echo "ERROR: Unable to reload the original driver."
        echo "DO NOT TOUCH YOUR TRACKPAD NOW OR YOUR COMPUTER WILL CRASH!"
    fi
    sudo chown -R `id -un`:`id -gn` ./$modkext
    exit
fi
sudo chown -R `id -un`:`id -gn` ./$modkext
echo "Driver replaced successfully."
echo