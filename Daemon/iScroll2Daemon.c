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
#define kProgramVersion		"0.32"
#define kPIDFile			"/var/run/" kProgramName ".pid"
#define kDriverClassName	"iScroll2"

SCDynamicStoreRef		dsSession = NULL;
SCDynamicStoreContext	dsContext = {0, NULL, NULL, NULL, NULL};
CFRunLoopSourceRef		dsRunLoopSource = NULL;
IONotificationPortRef	ioKitNotificationPort = NULL;
CFRunLoopSourceRef		ioRunLoopSource = NULL;
CFStringRef				consoleUser = NULL;


void write_log(int prio, char * fmt, ...) {
	va_list	arg;
	
	va_start(arg, fmt);
	vsyslog(prio, fmt, arg);
	if(prio <= LOG_ERR) {
		vfprintf(stderr, fmt, arg);
		fprintf(stderr, "\n");
	} else {
		vfprintf(stdout, fmt, arg);
		fprintf(stdout, "\n");
	}
	va_end(arg);
}


static Boolean LoadSettingsForUser(CFStringRef username) {
	io_iterator_t		iter = 0;
	io_registry_entry_t	regEntry = 0;
	CFDictionaryRef		newSettings = NULL;
	const char			* name;
	Boolean				success = TRUE;

	if(username != NULL) {
		name = CFStringGetCStringPtr(username, kCFStringEncodingMacRoman);
		write_log(LOG_NOTICE, "Loading settings for user '%s'.", name);
	
		CFPreferencesSynchronize(CFSTR(kDriverAppID), username, 
									kCFPreferencesAnyHost);
		newSettings = CFPreferencesCopyValue(CFSTR(kCurrentParametersKey), 
											CFSTR(kDriverAppID), username, 
											kCFPreferencesAnyHost);
		if(newSettings == NULL)
			write_log(LOG_NOTICE, "No custom settings found.");
	}
	
	if(newSettings == NULL) {
		write_log(LOG_NOTICE, "Loading default settings.");
	
		CFPreferencesSynchronize(CFSTR(kDriverAppID), 
								kCFPreferencesAnyUser, 
								kCFPreferencesCurrentHost);
		newSettings = CFPreferencesCopyValue(CFSTR(kDefaultParametersKey), 
											 CFSTR(kDriverAppID), 
											 kCFPreferencesAnyUser, 
											 kCFPreferencesCurrentHost);
		if(newSettings == NULL) {
			write_log(LOG_ERR, "No default settings found.");
			success = FALSE;
			goto EXIT;
		}
	}
	
	if(IOServiceGetMatchingServices(kIOMasterPortDefault, 
									IOServiceMatching(kIOHIDSystemClass),
									&iter) != KERN_SUCCESS) {
		iter = 0;
		write_log(LOG_ERR, "Error locating HIDSystem service!");
		success = FALSE;
		goto EXIT;
	}
	
	regEntry = IOIteratorNext(iter);
	if(regEntry == 0) {
		write_log(LOG_ERR, "No HIDSystem service found!");
		success = FALSE;
		goto EXIT;
	}
	
	if(IORegistryEntrySetCFProperty(regEntry, CFSTR(kiScroll2SettingsKey), 
									newSettings) != KERN_SUCCESS) {
		write_log(LOG_ERR, "Error setting HIDSystem properties!");
		success = FALSE;
		goto EXIT;
	}
	
EXIT:
	if(newSettings != NULL) {
		CFRelease(newSettings);
		newSettings = NULL;
	}
	if(regEntry != 0) {
		IOObjectRelease(regEntry);
		regEntry = 0;
	}
	if(iter != 0) {
		IOObjectRelease(iter);
		iter = 0;
	}
	return success;
}


static void ConsoleUserChangedCallbackFunction(SCDynamicStoreRef store, 
											   CFArrayRef changedKeys, 
											   void * context) {
	const char			* oldName;
	const char			* newName;
    uid_t				uid;
    gid_t				gid;
	CFStringRef			newConsoleUser;
	
	newConsoleUser = SCDynamicStoreCopyConsoleUser(store, &uid, &gid);

	if(consoleUser != NULL)
		oldName = CFStringGetCStringPtr(consoleUser, kCFStringEncodingMacRoman);
	
	if(newConsoleUser != NULL) {
		newName = CFStringGetCStringPtr(newConsoleUser, 
										kCFStringEncodingMacRoman);
										
		if(consoleUser != NULL) {
			write_log(LOG_NOTICE, "Console user changed from '%s' to '%s'.", 
					oldName, newName);
		} else
			write_log(LOG_NOTICE, "Console user '%s' logged in.", newName); 
			
	} else if(consoleUser != NULL)
		write_log(LOG_NOTICE, "Console user '%s' logged out.", oldName);

	if(consoleUser != NULL) {
		CFRelease(consoleUser);
		consoleUser = NULL;
	}
	consoleUser = newConsoleUser;
	if(uid == getuid())
		LoadSettingsForUser(newConsoleUser);
	else if(newConsoleUser != NULL)
		write_log(LOG_NOTICE, "Not updating settings as this instance of %s "
								"does not handle user '%s'.", 
								kProgramName, newName);
}


static Boolean InitDynamicStore() {
	Boolean	success = TRUE;
	
	write_log(LOG_NOTICE, "Creating DynamicStore session.");
	
	dsSession = SCDynamicStoreCreate(NULL, CFSTR(kProgramName), 
									 ConsoleUserChangedCallbackFunction, 
									 &dsContext);  
    if(dsSession == NULL) {
		write_log(LOG_ERR, "Couldn't create DynamicStore session!");
		success = FALSE;
		goto EXIT;
	}
	
	write_log(LOG_NOTICE, "Creating DynamicStore RunLoop source.");
	
    dsRunLoopSource = SCDynamicStoreCreateRunLoopSource(NULL, dsSession, 
														(CFIndex)0);
	if(dsRunLoopSource == NULL) {
		write_log(LOG_ERR, "Couldn't create DynamicStore RunLoop source!");
		success = FALSE;
		goto EXIT;
	}
	
    CFRunLoopAddSource(CFRunLoopGetCurrent(), dsRunLoopSource, 
					   kCFRunLoopDefaultMode);
	
EXIT:
	return success;
}


static Boolean RegisterConsoleUserChangeCallback() {
	CFStringRef	consoleUserNameChangeKey = NULL;
	CFArrayRef	notificationKeys = NULL;
	Boolean		success = TRUE;
	
	write_log(LOG_NOTICE, "Creating ConsoleUser key.");
	
	consoleUserNameChangeKey = SCDynamicStoreKeyCreateConsoleUser(NULL);
    if(consoleUserNameChangeKey == NULL) {
		write_log(LOG_ERR, "Couldn't create ConsoleUser key!");
		success = FALSE;
        goto EXIT;
    }
	
	write_log(LOG_NOTICE, "Creating notification key array.");
	
    notificationKeys = CFArrayCreate(NULL, (void*)&consoleUserNameChangeKey, 
									 (CFIndex)1, &kCFTypeArrayCallBacks);
    if(notificationKeys == NULL) {
		write_log(LOG_ERR, "Couldn't create notification key array!"); 
		success = FALSE;
        goto EXIT;
    }
	
	write_log(LOG_NOTICE, "Setting up DynamicStore notification.");
	
	if(SCDynamicStoreSetNotificationKeys(dsSession, notificationKeys, NULL) == 
	   FALSE) {
		write_log(LOG_ERR, "Couldn't set up DynamicStore notification!"); 
		success = FALSE;
        goto EXIT;
	}
	
EXIT:
	if(notificationKeys != NULL) {
		CFRelease(notificationKeys);
		notificationKeys = NULL;
	}
	if(consoleUserNameChangeKey != NULL) {
		CFRelease(consoleUserNameChangeKey);
		consoleUserNameChangeKey = NULL;
	}
	return success;
}


static void ServiceMatchedCallbackFunction(void * context, 
											io_iterator_t iter) {
	io_object_t		ioObject;
	Boolean			anyMatched = FALSE;
    uid_t			uid;
    gid_t			gid;

	if(consoleUser != NULL) {
		CFRelease(consoleUser);
		consoleUser = NULL;
	}
	
	while((ioObject = IOIteratorNext(iter)) != 0) {
		anyMatched = TRUE;
		IOObjectRelease(ioObject);
	}
	
	if(anyMatched == TRUE) {
		write_log(LOG_NOTICE, "Service '" kDriverClassName "' matched.");

		if(dsSession == NULL && 
			(InitDynamicStore() == FALSE ||
				RegisterConsoleUserChangeCallback() == FALSE)) {
			write_log(LOG_ERR, "Unable to initiate session!");
			kill(getpid(), SIGTERM);
		} else {
			const char * name;
			consoleUser = SCDynamicStoreCopyConsoleUser(dsSession, &uid, &gid);
			if(consoleUser != NULL) {
				name = CFStringGetCStringPtr(consoleUser, 
												kCFStringEncodingMacRoman);
				write_log(LOG_NOTICE, "User '%s' is logged in.", name);
			} else
				write_log(LOG_NOTICE, "No user is logged in.");

			if(uid == getuid())
				LoadSettingsForUser(consoleUser);
			else if(consoleUser != NULL)
				write_log(LOG_NOTICE,
							"Not updating settings as this instance of %s does "
							"not handle user '%s'.", 
							kProgramName, name);
		}
	} else
		write_log(LOG_WARNING, "Service '%s' not matched!", kDriverClassName);
}


static void SignalHandlerFunction(int signo) {
    uid_t		uid;
    gid_t		gid;
	const char	*name;
	
	write_log(LOG_NOTICE, "Received signal %d.", signo);
	switch(signo) {
		case SIGINT:
		case SIGTERM:
			CFRunLoopStop(CFRunLoopGetCurrent());
			break;
		case SIGHUP:
			consoleUser = SCDynamicStoreCopyConsoleUser(dsSession, &uid, &gid);
			if (consoleUser != NULL)
				name = CFStringGetCStringPtr(consoleUser, 
												kCFStringEncodingMacRoman);
			if(uid == getuid())
				LoadSettingsForUser(consoleUser);
			else if(consoleUser != NULL)
				write_log(LOG_NOTICE, "Not updating settings as this instance "
							"of %s does not handle user '%s'.",
							kProgramName, name);
			break;
		case SIGINFO:
			write_log(LOG_NOTICE, "%s %s", kProgramName, kProgramVersion);
			if(consoleUser != NULL)
				name = CFStringGetCStringPtr(consoleUser,
												kCFStringEncodingMacRoman);
			write_log(LOG_NOTICE, "dsSession = '%d'", dsSession);
			write_log(LOG_NOTICE, "dsRunLoopSource = '%d'", dsRunLoopSource);
			write_log(LOG_NOTICE, "ioKitNotificationPort = '%d'", 
					ioKitNotificationPort);
			write_log(LOG_NOTICE, "ioRunLoopSource = '%d'", ioRunLoopSource);
			if(consoleUser != NULL)
				write_log(LOG_NOTICE, "consoleUser = '%s'", name);
			else
				write_log(LOG_NOTICE, "consoleUser = NULL");
	}
}


static Boolean InitIOKit() {
	Boolean	success = TRUE;
	
	write_log(LOG_NOTICE, "Creating IOKit notification port.");
	
	ioKitNotificationPort = IONotificationPortCreate(kIOMasterPortDefault);
	
    if(ioKitNotificationPort == NULL) {
		write_log(LOG_ERR, "Couldn't create IOKit notification port!");
		success = FALSE;
		goto EXIT;
	}
	
	write_log(LOG_NOTICE, "Creating IOKit RunLoop source.");
	
	ioRunLoopSource = IONotificationPortGetRunLoopSource(ioKitNotificationPort);
	if(ioRunLoopSource == NULL) {
		write_log(LOG_ERR, "Couldn't create IOKit RunLoop source!");
		success = FALSE;
		goto EXIT;
	}
	
    CFRunLoopAddSource(CFRunLoopGetCurrent(), ioRunLoopSource, 
					   kCFRunLoopDefaultMode);
	
EXIT:
	return success;
}


static Boolean RegisterServiceMatchedCallback() {
	io_iterator_t	iter = 0;
	CFDictionaryRef	matchDictionary = NULL;
	Boolean			success = TRUE;
	
	write_log(LOG_NOTICE, "Creating matching dictionary for class '"
			   kDriverClassName "'.");
			   
	matchDictionary = IOServiceMatching(kDriverClassName);
	if(matchDictionary == NULL) {
		write_log(LOG_ERR, "Couldn't create matching dictionary for class '"
			   kDriverClassName "'!");
		success = FALSE;
		goto EXIT;
	}
	
	write_log(LOG_NOTICE, "Registering service matched notification.");
	
	if(IOServiceAddMatchingNotification(ioKitNotificationPort,
										kIOMatchedNotification,	matchDictionary,
										ServiceMatchedCallbackFunction, NULL, 
										&iter) != kIOReturnSuccess) {
		write_log(LOG_ERR, "Couldn't register service matched notification!");
		success = FALSE;
		goto EXIT;
	}
	
	ServiceMatchedCallbackFunction(NULL, iter);
	
EXIT:
	return success;
}


static void CleanupDynamicStore() {
//	if(dsRunLoopSource != NULL)
//		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), dsRunLoopSource, 
//							  kCFRunLoopDefaultMode);
	if(dsSession != NULL) {
		CFRelease(dsSession);
		dsSession = NULL;
	}
}


static void CleanupIOKit() {
//	if(ioRunLoopSource != NULL)
//		CFRunLoopRemoveSource(CFRunLoopGetCurrent(), ioRunLoopSource, 
//							  kCFRunLoopDefaultMode);
	if(ioKitNotificationPort != NULL)
		IONotificationPortDestroy(ioKitNotificationPort);
}


int main(int argc, const char * argv[]) {
	int		exitval = EXIT_SUCCESS;
	Boolean	daemonize;
    FILE	* pid_file;

	openlog(argv[0], LOG_CONS | LOG_PID, LOG_DAEMON);
	
	write_log(LOG_NOTICE, "Starting %s %s.", kProgramName, kProgramVersion);
	daemonize = (argc < 2) || (strncmp(argv[1], "-foreground", 2) != 0);
	if(daemonize) {
		write_log(LOG_NOTICE, "Spawning daemon.");
		if(daemon(0, 0) != 0) {
			write_log(LOG_ERR, "Unable to daemonize!");
			exitval = EXIT_FAILURE;
			goto EXIT;
		}
		pid_file = fopen(kPIDFile, "w");
		if(pid_file == NULL) {
			write_log(LOG_ERR, "Couldn't write PID to " kPIDFile "!\n");
			exitval = EXIT_FAILURE;
			goto EXIT;
		}
		fprintf(pid_file, "%d\n", getpid());
		fclose(pid_file);	
	} else
		write_log(LOG_NOTICE, "Running in foreground mode.");
	
	if(InitIOKit() == FALSE) {
		exitval = EXIT_FAILURE;
		goto EXIT;
	}
	
	if(RegisterServiceMatchedCallback() == FALSE) {
		exitval = EXIT_FAILURE;
		goto EXIT;
	}
	
    signal(SIGINFO, SignalHandlerFunction);
    signal(SIGHUP, SignalHandlerFunction);
    signal(SIGTERM, SignalHandlerFunction);
	if(!daemonize)
		signal(SIGINT, SignalHandlerFunction);
	
	CFRunLoopRun();
	
EXIT:
	write_log(LOG_NOTICE, "Cleaning up.");
	CleanupDynamicStore();
	CleanupIOKit();
	if(consoleUser != NULL)
		CFRelease(consoleUser);
	if(daemonize)
		unlink(kPIDFile);
	
	write_log(LOG_NOTICE, "Exiting with return code %d.", exitval);
	closelog();
    return exitval;
}
