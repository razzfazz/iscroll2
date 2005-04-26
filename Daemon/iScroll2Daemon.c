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
		syslog(1, "Console user logged out.\n");
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
		syslog(1, "Couldn't get HIDSystem properties!\n");
		goto EXIT;
	}
EXIT:
	if(settings) CFRelease(settings);
	if(regEntry) IOObjectRelease(regEntry);
	if(iter) IOObjectRelease(iter);
}

Boolean InstallConsoleUserChangedNotifier(CFStringRef appID, 
	CFRunLoopSourceRef * rls)
{
	SCDynamicStoreContext context = { 0, NULL, NULL, NULL, NULL };
	SCDynamicStoreRef session = NULL;
	CFStringRef consoleUserNameChangeKey = NULL;
	CFArrayRef notificationKeys;
	Boolean result;
	
	*rls = NULL;
	if (appID == NULL)
	{
		appID = CFSTR("");
    }
    else
    {
        CFRetain(appID);
    }
	context.info = (void*) NULL;
	session = SCDynamicStoreCreate(NULL, appID, ConsoleUserChangedCallBackFunction, 
		&context);  
	CFRelease(appID);
    if (session == NULL) return FALSE;
//	ConsoleUserChangedCallBackFunction(session, 0, &context);
    consoleUserNameChangeKey = SCDynamicStoreKeyCreateConsoleUser(NULL);
    if (consoleUserNameChangeKey == NULL) 
    {
        CFRelease(session);
		syslog(1, "Couldn't create ConsoleUser key!\n"); 
        return FALSE;
    }
    notificationKeys = CFArrayCreate(NULL, (void*)&consoleUserNameChangeKey, 
		(CFIndex)1, &kCFTypeArrayCallBacks);
    if (notificationKeys == NULL)
    {
        CFRelease(session); 
        CFRelease(consoleUserNameChangeKey);
		syslog(1, "Couldn't create notification key array!\n"); 
        return FALSE;
    }
	result = SCDynamicStoreSetNotificationKeys(session, notificationKeys, NULL);
	CFRelease(notificationKeys); 
	CFRelease(consoleUserNameChangeKey); 
	if (result == FALSE)
	{
        CFRelease(session); 
		syslog(1, "Couldn't set up dynamic store notification!\n"); 
        return FALSE;
	}
    *rls = SCDynamicStoreCreateRunLoopSource(NULL, session, (CFIndex)0);
    return TRUE;
}

int main (int argc, const char * argv[]) 
{
    Boolean				result;
    CFRunLoopSourceRef	rls = NULL;
    FILE				* pid_file;
    daemon(0,0);
	syslog(1, "Daemon starting up with PID %d.\n", getpid());
    result = InstallConsoleUserChangedNotifier(CFSTR(kProgramName), &rls);
    if (result != TRUE)
    {
		syslog(1, "Unable to register for login notification.\n");
        return 1;
    }
    pid_file = fopen(kPIDFile, "w");
    fprintf(pid_file, "%d\n", getpid());
    fclose(pid_file);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
    CFRelease(rls);
    CFRunLoopRun();
	unlink(kPIDFile);
    return 0;
}

