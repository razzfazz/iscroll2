#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include "../Driver/ioregkeys.h"
#include "../PrefPane/preferences.h"

#define kProgramName		"iScroll2Daemon"
#define kPIDFile			"/var/run/" kProgramName ".pid"
#define kDriverClassName	"iScroll2"
#define kSleepDelay			30
#define	LOG_DEFAULT			LOG_NOTICE

SCDynamicStoreRef		dsSession = NULL;
SCDynamicStoreContext	dsContext = {0, NULL, NULL, NULL, NULL};
CFRunLoopSourceRef		dsRunLoopSource = NULL;
IONotificationPortRef	ioKitNotificationPort = NULL;
CFRunLoopSourceRef		ioRunLoopSource = NULL;
CFStringRef				consoleUser = NULL;


static Boolean LoadSettingsForUser(CFStringRef username) {
	io_iterator_t		iter = 0;
	io_registry_entry_t	regEntry = 0;
	CFDictionaryRef		newSettings = NULL;
	const char			* name;
	Boolean				success = TRUE;
	
	name = CFStringGetCStringPtr(username, kCFStringEncodingMacRoman);
	syslog(LOG_DEFAULT, "Loading settings for user '%s'.", name);
	
	CFPreferencesSynchronize(CFSTR(kDriverAppID), username, 
							 kCFPreferencesAnyHost);
	newSettings = CFPreferencesCopyValue(CFSTR(kCurrentParametersKey), 
										 CFSTR(kDriverAppID), username, 
										 kCFPreferencesAnyHost);
	if(newSettings == NULL) {
		syslog(LOG_DEFAULT, "No custom settings found.");
		syslog(LOG_DEFAULT, "Loading default settings.");
	
		CFPreferencesSynchronize(CFSTR(kDriverAppID), 
								kCFPreferencesAnyUser, 
								kCFPreferencesCurrentHost);
		newSettings = CFPreferencesCopyValue(CFSTR(kDefaultParametersKey), 
											 CFSTR(kDriverAppID), 
											 kCFPreferencesAnyUser, 
											 kCFPreferencesCurrentHost);
		if(newSettings == NULL)
			syslog(LOG_WARNING, "No default settings found.");
	}
	
	if(newSettings == NULL) {
		syslog(LOG_DEFAULT, "No settings found.");
		success = FALSE;
		goto EXIT;
	}
	
	if(IOServiceGetMatchingServices(kIOMasterPortDefault, 
									IOServiceMatching(kIOHIDSystemClass),
									&iter) != KERN_SUCCESS) {
		iter = 0;
		syslog(LOG_ERR, "Error locating HIDSystem service!");
		success = FALSE;
		goto EXIT;
	}
	
	regEntry = IOIteratorNext(iter);
	if(regEntry == 0) {
		syslog(LOG_ERR, "No HIDSystem service found!");
		success = FALSE;
		goto EXIT;
	}
	
	if(IORegistryEntrySetCFProperty(regEntry, CFSTR(kiScroll2SettingsKey), 
									newSettings) != KERN_SUCCESS) {
		syslog(LOG_ERR, "Error setting HIDSystem properties!");
		success = FALSE;
		goto EXIT;
	}
	
EXIT:
	if(newSettings != NULL)
		CFRelease(newSettings);
	if(regEntry != 0)
		IOObjectRelease(regEntry);
	if(iter != 0)
		IOObjectRelease(iter);
	return success;
}


static void ConsoleUserChangedCallbackFunction(SCDynamicStoreRef store, 
											   CFArrayRef changedKeys, 
											   void * context) {
	const char			* name;
    uid_t				uid;
    gid_t				gid;

	if(consoleUser != NULL)
		CFRelease(consoleUser);
		
	consoleUser = SCDynamicStoreCopyConsoleUser(store, &uid, &gid);

	if(consoleUser != NULL) {
		name = CFStringGetCStringPtr(consoleUser, kCFStringEncodingMacRoman);
		syslog(LOG_DEFAULT, "Console user changed to '%s'.", name);
		
		LoadSettingsForUser(consoleUser);
	} else
		syslog(LOG_DEFAULT, "No console user logged in.");
}


static void ServiceMatchedCallbackFunction(void * context, 
											io_iterator_t iter) {
	io_object_t		ioObject;
	Boolean			anyMatched = FALSE;
	
	if((ioObject = IOIteratorNext(iter)) != 0) {
		anyMatched = TRUE;
		IOObjectRelease(ioObject);
	}
	
	while((ioObject = IOIteratorNext(iter)) != 0)
		IOObjectRelease(ioObject);

	if(anyMatched == TRUE) {
		syslog(LOG_DEFAULT, "Service '%s' matched.", kDriverClassName);

		if(consoleUser != NULL)
			LoadSettingsForUser(consoleUser);
	}
}


//static void ServiceTerminatedCallbackFunction(void * context, 
//											  io_iterator_t iter) {
//	io_object_t	ioObject;
//	Boolean		anyTerminated = FALSE;
//	
//	if((ioObject = IOIteratorNext(iter)) != NULL) {
//		anyTermainted = TRUE;
//		IOObjectRelease(ioObject);
//	}
//	
//	while((ioObject = IOIteratorNext(iter)) != NULL)
//		IOObjectRelease(ioObject);
//	
//	if(anyTerminated == TRUE)
//		syslog(LOG_DEFAULT, "Service '" kDriverClassName "' terminated.");
//}


static void SignalHandlerFunction(int signo) {
	syslog(LOG_DEFAULT, "Received signal %d.", signo);
	CFRunLoopStop(CFRunLoopGetCurrent());
}


static Boolean InitDynamicStore() {
	Boolean				success = TRUE;
	
	dsSession = SCDynamicStoreCreate(NULL, CFSTR(kProgramName), 
									 ConsoleUserChangedCallbackFunction, 
									 &dsContext);  
    if(dsSession == NULL) {
		syslog(LOG_ERR, "Couldn't create DynamicStore session!");
		success = FALSE;
		goto EXIT;
	}
	
    dsRunLoopSource = SCDynamicStoreCreateRunLoopSource(NULL, dsSession, 
														(CFIndex)0);
	if(dsRunLoopSource == NULL) {
		syslog(LOG_ERR, "Couldn't create Dynamic Store RunLoop source!");
		success = FALSE;
		goto EXIT;
	}
	
    CFRunLoopAddSource(CFRunLoopGetCurrent(), dsRunLoopSource, 
					   kCFRunLoopDefaultMode);
	
EXIT:
	if(dsRunLoopSource != NULL)
		CFRelease(dsRunLoopSource);
	return success;
}


static Boolean InitIOKit() {
	Boolean				success = TRUE;
	
    ioKitNotificationPort = IONotificationPortCreate(kIOMasterPortDefault);
	
    if(ioKitNotificationPort == NULL) {
		syslog(LOG_ERR, "Couldn't create IOKit notification port!");
		success = FALSE;
		goto EXIT;
	}
	
	ioRunLoopSource = IONotificationPortGetRunLoopSource(ioKitNotificationPort);
	if(ioRunLoopSource == NULL) {
		syslog(LOG_ERR, "Couldn't create IOKit RunLoop source!");
		success = FALSE;
		goto EXIT;
	}
	
    CFRunLoopAddSource(CFRunLoopGetCurrent(), ioRunLoopSource, 
					   kCFRunLoopDefaultMode);
	
EXIT:
	if(ioRunLoopSource != NULL)
		CFRelease(ioRunLoopSource);
	return success;
}


static Boolean RegisterConsoleUserChangeCallback() {
	CFStringRef	consoleUserNameChangeKey = NULL;
	CFArrayRef	notificationKeys = NULL;
	Boolean		success = TRUE;
	
	consoleUserNameChangeKey = SCDynamicStoreKeyCreateConsoleUser(NULL);
    if(consoleUserNameChangeKey == NULL) {
		syslog(LOG_ERR, "Couldn't create ConsoleUser key!");
		success = FALSE;
        goto EXIT;
    }
	
    notificationKeys = CFArrayCreate(NULL, (void*)&consoleUserNameChangeKey, 
									 (CFIndex)1, &kCFTypeArrayCallBacks);
    if(notificationKeys == NULL) {
		syslog(LOG_ERR, "Couldn't create notification key array!"); 
		success = FALSE;
        goto EXIT;
    }
	
	if(SCDynamicStoreSetNotificationKeys(dsSession, notificationKeys, NULL) == 
	   FALSE) {
		syslog(LOG_ERR, "Couldn't set up dynamic store notification!"); 
		success = FALSE;
        goto EXIT;
	}
	
	ConsoleUserChangedCallbackFunction(dsSession, NULL, &dsContext);
	
EXIT:
	if(notificationKeys != NULL)
		CFRelease(notificationKeys); 
	if(consoleUserNameChangeKey != NULL)
		CFRelease(consoleUserNameChangeKey);
	return success;
}


static Boolean RegisterServiceMatchedCallback() {
	io_iterator_t	iter = 0;
	CFDictionaryRef	matchDictionary = NULL;
	Boolean			success = TRUE;
	
	matchDictionary = IOServiceMatching(kDriverClassName);
	if(matchDictionary == NULL) {
		syslog(LOG_ERR, "Couldn't create matching dictionary for class '"
			   kDriverClassName "'!");
		success = FALSE;
		goto EXIT;
	}				
	
	if(IOServiceAddMatchingNotification(ioKitNotificationPort,
										kIOMatchedNotification,	matchDictionary,
										ServiceMatchedCallbackFunction, NULL, 
										&iter) != kIOReturnSuccess) {
		syslog(LOG_ERR, "Couldn't register service matched notification!");
		success = FALSE;
		goto EXIT;
	}
	
	ServiceMatchedCallbackFunction(NULL, iter);
	
EXIT:
	return success;
}


//static Boolean RegisterServiceTerminatedCallback() {
//	io_iterator_t	iter = 0;
//	CFDictionaryRef	matchDictionary = NULL;
//	Boolean			success = TRUE;
//	
//	matchDictionary = IOServiceMatching(kDriverClassName);
//	if(matchDictionary == NULL) {
//		syslog(LOG_ERR, "Couldn't create matching dictionary for class '"
//			   kDriverClassName "'!");
//		success = FALSE;
//		goto EXIT;
//	}				
//	
//	if(IOServiceAddMatchingNotification(ioKitNotificationPort,
//										kIOTerminatedNotification,
//										matchDictionary,
//										ServiceTerminatedCallbackFunction, NULL, 
//										&iter) != kIOReturnSuccess) {
//		syslog(LOG_ERR, "Couldn't register service terminated notification!");
//		success = FALSE;
//		goto EXIT;
//	}
//	
//	ServiceTerminatedCallbackFunction(NULL, iter);
//	
//EXIT:
//	return success;
//}


static void CleanupDynamicStore() {
//	if(dsRunLoopSource != NULL)
//		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), dsRunLoopSource, 
//							  kCFRunLoopDefaultMode);
	if(dsSession != NULL)
		CFRelease(dsSession);
}


static void CleanupIOKit() {
//	if(ioRunLoopSource != NULL)
//		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), ioRunLoopSource, 
//							  kCFRunLoopDefaultMode);
	if(ioKitNotificationPort != NULL)
		IONotificationPortDestroy(ioKitNotificationPort);
}


int main(int argc, const char * argv[]) {
	pid_t	pid;
	int		exitval = EXIT_SUCCESS;
	Boolean	daemonize;
    FILE	* pid_file;

	daemonize = (argc < 2) || (strncmp(argv[1], "-foreground", 2) != 0);
	if(daemonize) {
		syslog(LOG_DEFAULT, "Spawning daemon.");
		if(daemon(0, 0) != 0) {
			syslog(LOG_ERR, "Unable to daemonize!");
			exitval = EXIT_FAILURE;
			goto EXIT;
		}
	} else
		syslog(LOG_DEFAULT, "Running in forreground.");
	
	pid = getpid();
	syslog(LOG_NOTICE, "PID is %d.", pid);
	
    pid_file = fopen(kPIDFile, "w");
	if(pid_file == NULL) {
		syslog(1, "Couldn't write PID to " kPIDFile "!\n");
		exitval = EXIT_FAILURE;
		goto EXIT;
	}
	fprintf(pid_file, "%d\n", pid);
	fclose(pid_file);
	
	if(daemonize && ((argc < 2) || (strncmp(argv[1], "-nosleep", 2) != 0))) {
		syslog(LOG_DEFAULT, "Sleeping for %d seconds.", kSleepDelay);
		sleep(kSleepDelay);
		syslog(LOG_DEFAULT, "Waking up.");
	}
		
	if(InitDynamicStore() == FALSE) {
		syslog(LOG_ERR, "Couldn't initialize Dynamic Store!");
		exitval = EXIT_FAILURE;
		goto EXIT;
	}
	
	if(InitIOKit() == FALSE) {
		syslog(LOG_ERR, "Couldn't initialize IOKit!");
		exitval = EXIT_FAILURE;
		goto EXIT;
	}
	
	if(RegisterConsoleUserChangeCallback() == FALSE) {
		syslog(LOG_ERR, "Couldn't register console user change callback!");
		exitval = EXIT_FAILURE;
		goto EXIT;
	}
	
	if(RegisterServiceMatchedCallback() == FALSE) {
		syslog(LOG_ERR, "Couldn't register service matched callback!");
		exitval = EXIT_FAILURE;
		goto EXIT;
	}
	
//	if(RegisterServiceTerminatedCallback() == FALSE) {
//		syslog(LOG_ERR, "Couldn't register service terminated callback!");
//		exitval = EXIT_FAILURE;
//		goto EXIT;
//	}
	
    signal(SIGTERM, SignalHandlerFunction);
	if(!daemonize)
		signal(SIGINT, SignalHandlerFunction);
	
	CFRunLoopRun();
	
EXIT:
	syslog(LOG_DEFAULT, "Cleaning up.");
	CleanupDynamicStore();
	CleanupIOKit();
	if(consoleUser != NULL)
		CFRelease(consoleUser);
	unlink(kPIDFile);
	
	syslog(LOG_DEFAULT, "Exiting with return code %d.", exitval);
    return exitval;
}
