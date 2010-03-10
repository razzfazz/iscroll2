#!/bin/sh

launch_agent="name.razzfazz.daemon.iScroll2"
launch_agent_file="/Library/LaunchAgents/${launch_agent}.plist"
files="/System/Library/Extensions/iScroll2.kext \
       /Library/PreferencePanes/iScroll2.prefPane \
       /Library/StartupItems/iScroll2 \
       /usr/local/bin/iScroll2Daemon \
       /Library/Receipts/iScroll2.pkg \
       ${launch_agent_file}"

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

echo "Trying to unload LaunchAgent ${launch_agent}..."
if [ -e ${launch_agent_file} ]
then
    if ! sudo launchctl unload ${launch_agent_file}
    then
	echo "ERROR: Unable to unload LaunchAgent ${launch_agent_file}."
    else
	echo "LaunchAgent ${launch_agent_file} successfullt unloaded."
    fi
else
    "ERROR: LaunchAgent control file ${launch_agent_file} not found; trying to remove LaunchAgent by ID..."
    if ! sudo launchctl remove ${launch_agent}
    then
	echo "ERROR: Unable to remove LaunchAgent ${launch_agent}."
    else
	echo "LaunchAgent ${launch_agent} successfully removed."
    fi
fi
echo

for f in $files
do
    if [ -e "$f" ]
    then
    	echo "Trying to remove ${f}..."
	if ! sudo rm -R "${f}"
	then
            echo "ERROR: Unable to remove ${f}. Please remove manually."
	else
    	    echo "${f} successfully removed."
	fi
    else
    	echo "${f} not found, skipping."
    fi
    echo
done

sudo touch /System/Library/Extensions

echo "Uninstall completed. You should restart your system now."
echo
