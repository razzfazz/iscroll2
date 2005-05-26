#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <unistd.h>
#include <stdio.h>

#include "../Driver/ioregkeys.h"
#include "../PrefPane/preferences.h"

#define kProgramName "iScroll2Daemon"
#define kPIDFile "/var/run/" kProgramName ".pid"

void ConsoleUserChangedCallBackFunction(SCDynamicStoreRef store, 
										CFArrayRef changedKeys, void * context)
{
#pragma unused (changedKeys)
    CFStringRef			username;
    uid_t				uid;
    gid_t				gid;
	io_iterator_t		iter = 0;
	io_registry_entry_t	regEntry = 0;
	CFDictionaryRef		settings = 0;
	username = SCDynamicStoreCopyConsoleUser(store, &uid, &gid);
	if (username != NULL)
    {
		const char * name;
		name = CFStringGetCStringPtr(username, kCFStringEncodingMacRoman);
		if(name) syslog(1, "Console user changed to '%s'.", name);
		else syslog(1, "Console user changed to UID %d.", uid);
		settings = CFPreferencesCopyValue(CFSTR(kCurrentParametersKey), 
										  CFSTR(kDriverAppID), username, 
										  kCFPreferencesAnyHost);
        CFRelease(username);
		if(!settings)
		{
			if(name) syslog(1, "No custom settings found for user '%s'; loading defaults.\n", 
				name);
			else syslog(1, "No custom settings found for UID %d; loading defaults.\n", uid);
			settings = CFPreferencesCopyValue(CFSTR(kDefaultParametersKey), 
											  CFSTR(kDriverAppID), kCFPreferencesAnyUser, 
											  kCFPreferencesCurrentHost);
		}
		else
		{
			if(name) syslog(1, "%d settings loaded for user '%s'.\n", 
				CFDictionaryGetCount(settings), name);
			else syslog(1, "%d settings loaded for UID %d.\n", 
				CFDictionaryGetCount(settings), uid);
		}
    }
    else
    {
		syslog(1, "No console user currently logged in.\n");
		settings = CFPreferencesCopyValue(CFSTR(kDefaultParametersKey), 
										  CFSTR(kDriverAppID), kCFPreferencesAnyUser, 
										  kCFPreferencesCurrentHost);
    }
	IOServiceGetMatchingServices(kIOMasterPortDefault, 
								 IOServiceMatching(kIOHIDSystemClass),
								 &iter);
	if (iter == 0) {
		syslog(1, "Couldn't find HIDSystem service!\n");
		goto EXIT;
	}
	regEntry = IOIteratorNext(iter);
	if(regEntry == 0) {
		syslog(1, "Couldn't find HIDSystem service!\n");
		goto EXIT;
	}
	if(IORegistryEntrySetCFProperty(regEntry, CFSTR(kiScroll2SettingsKey), 
		settings) != KERN_SUCCESS) {
		syslog(1, "Couldn't set HIDSystem properties!\n");
		goto EXIT;
	}
EXIT:
	if(settings) CFRelease(settings);
	if(regEntry) IOObjectRelease(regEntry);
	if(iter) IOObjectRelease(iter);
}

int main (int argc, const char * argv[]) 
{
	SCDynamicStoreContext	context = {0, NULL, NULL, NULL, NULL};
	SCDynamicStoreRef		session = NULL;
	CFStringRef				consoleUserNameChangeKey = NULL;
	CFArrayRef				notificationKeys;
    CFRunLoopSourceRef		rls = NULL;
    FILE					* pid_file;
    daemon(0,0);
	syslog(1, "Daemon starting up with PID %d.\n", getpid());
	session = SCDynamicStoreCreate(NULL, CFSTR(kProgramName), 
		ConsoleUserChangedCallBackFunction, &context);  
    if (session == NULL)
	{
		syslog(1, "Couldn't create DynamicStore session!\n");
		return 1;
	}
    consoleUserNameChangeKey = SCDynamicStoreKeyCreateConsoleUser(NULL);
    if (consoleUserNameChangeKey == NULL) 
    {
        CFRelease(session);
		syslog(1, "Couldn't create ConsoleUser key!\n"); 
        return 1;
    }
    notificationKeys = CFArrayCreate(NULL, (void*)&consoleUserNameChangeKey, 
		(CFIndex)1, &kCFTypeArrayCallBacks);
	CFRelease(consoleUserNameChangeKey);
    if (notificationKeys == NULL)
    {
        CFRelease(session); 
		syslog(1, "Couldn't create notification key array!\n"); 
        return 1;
    }
	if (SCDynamicStoreSetNotificationKeys(session, notificationKeys, NULL) == FALSE)
	{
		CFRelease(notificationKeys); 
        CFRelease(session); 
		syslog(1, "Couldn't set up dynamic store notification!\n"); 
        return 1;
	}
	CFRelease(notificationKeys); 
    rls = SCDynamicStoreCreateRunLoopSource(NULL, session, (CFIndex)0);
    pid_file = fopen(kPIDFile, "w");
    fprintf(pid_file, "%d\n", getpid());
    fclose(pid_file);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
    CFRelease(rls);
    CFRunLoopRun();
	unlink(kPIDFile);
    return 0;
}

