#!/bin/sh

files="/System/Library/Extensions/iScroll2.kext \
	/Library/PreferencePanes/iScroll2.prefPane \
	/Library/Preferences/name.razzfazz.driver.iScroll2.plist \
	~/Library/Preferences/name.razzfazz.driver.iScroll2.plist \
	/Library/StartupItems/iScroll2
	/usr/local/bin/iScroll2Daemon
	/Library/Receipts/iScroll2.pkg"

clear
echo
echo "This will remove iScroll2 from your system."
echo
echo "Are you sure you want to continue?"
echo "Press CTRL-C or close this window to abort, RETURN to continue."
read
echo
echo "When prompted for a password, enter your admin password."
echo

for f in $files
do
	if [ -e $f ]
	then
    		echo "Removing $f..."
		if ! echo "sudo rm -R $f"
		then
        		echo "ERROR: Unable to remove $f. Please remove manually."
		 fi
    		echo "$f successfully removed."
    		echo
	else
    		echo "$f not found, skipping."
		echo
	fi
done

echo "Uninstall completed. You should restart your system now."
echo
