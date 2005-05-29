#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <unistd.h>
#include <syslog.h>

#include "../Driver/ioregkeys.h"
#include "../PrefPane/preferences.h"

#define kProgramName "iScroll2Daemon"
#define kPIDFile "/var/run/" kProgramName ".pid"

void ConsoleUserChangedCallBackFunction(SCDynamicStoreRef store, 
										CFArrayRef changedKeys, void * context)
{
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
		if(name != NULL) syslog(1, "Console user changed to '%s'.", name);
		else syslog(1, "Console user changed to UID %d.", uid);
		settings = CFPreferencesCopyValue(CFSTR(kCurrentParametersKey), 
										  CFSTR(kDriverAppID), username, 
										  kCFPreferencesAnyHost);
		if(settings != NULL)
		{
			if(name != NULL) syslog(1, "%d settings loaded for user '%s'.\n", 
				CFDictionaryGetCount(settings), name);
			else syslog(1, "%d settings loaded for UID %d.\n", 
				CFDictionaryGetCount(settings), uid);
		}
		else
		{
			if(name != NULL) syslog(1, "No custom settings found for user '%s'; loading defaults.\n", 
				name);
			else syslog(1, "No custom settings found for UID %d; loading defaults.\n", uid);
			settings = CFPreferencesCopyValue(CFSTR(kDefaultParametersKey), 
											  CFSTR(kDriverAppID), kCFPreferencesAnyUser, 
											  kCFPreferencesCurrentHost);
		}
        CFRelease(username);
    }
    else
    {
		syslog(1, "No console user currently logged in.\n");
		settings = CFPreferencesCopyValue(CFSTR(kDefaultParametersKey), 
										  CFSTR(kDriverAppID), kCFPreferencesAnyUser, 
										  kCFPreferencesCurrentHost);
    }
	if(IOServiceGetMatchingServices(kIOMasterPortDefault, 
									IOServiceMatching(kIOHIDSystemClass),
									&iter) != KERN_SUCCESS) {
		iter = 0;
		syslog(1, "Error locating HIDSystem service!\n");
		goto EXIT;
	}
	if((regEntry = IOIteratorNext(iter)) == 0) {
		syslog(1, "No HIDSystem service found!\n");
		goto EXIT;
	}
	if(IORegistryEntrySetCFProperty(regEntry, CFSTR(kiScroll2SettingsKey), 
		settings) != KERN_SUCCESS) {
		syslog(1, "Error setting HIDSystem properties!\n");
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
	if(pid_file != NULL)
	{
		fprintf(pid_file, "%d\n", getpid());
		fclose(pid_file);
	}
	else
	{
		syslog(1, "Couldn't write PID to " kPIDFile "!\n");
	}
	unlink(kPIDFile);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
    CFRelease(rls);
    CFRunLoopRun();
	CFRelease(session);
    return 0;
}

