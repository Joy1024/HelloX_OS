//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 04, Apri
//    Module Name               : apiset.c
//    Module Funciton           : 
//                                Contains the map between API set name
//                                and the container DLL's name.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "hellox.h"
#include "stdio.h"
#include "string.h"
#include "pteb.h"
#include "loader.h"

/* 
 * String map between api set, versions, and it's 
 * container dll. 
 */
typedef struct tag__WIN_API_NAME {
	const char* api_set_name;
	int master_ver;
	int major_ver;
	int minor_ver;
	const char* dll_name;
}__WIN_API_NAME;

const __WIN_API_NAME win_api_name[] =
{
{"api-ms-win-appmodel-runtime", 1, 1, 2, "kernelbase.dll"},
{"api-ms-win-base-bootconfig", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-base-util", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-core-apiquery", 1, 1, 0, "ntdll.dll"},
{"api-ms-win-core-apiquery", 2, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-appcompat", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-appinit", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-atoms", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-backgroundtask", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-calendar", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-com", 1, 1, 1, "combase.dll"},
{"api-ms-win-core-com", 2, 1, 1, "coml2.dll"},
{"api-ms-win-core-com-midlproxystub", 1, 1, 0, "combase.dll"},
{"api-ms-win-core-com-private", 1, 1, 0, "combase.dll"},
{"api-ms-win-core-com-private", 1, 2, 0, "combase.dll"},
{"api-ms-win-core-com-private", 1, 3, 0, "combase.dll"},
{"api-ms-win-core-comm", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-console-ansi", 2, 1, 0, "kernel32.dll"},
{"api-ms-win-core-console-internal", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-console", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-console", 1, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-console", 2, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-console", 2, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-console", 3, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-console", 3, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-crt", 1, 1, 0, "msvcrt.dll"},
{"api-ms-win-core-crt", 2, 1, 0, "msvcrt.dll"},
{"api-ms-win-core-datetime", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-datetime", 1, 1, 2, "kernelbase.dll"}, /* ADDED MANULLY. */
{"api-ms-win-core-debug", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-delayload", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-enclave", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-errorhandling", 1, 1, 3, "kernelbase.dll"},
{"api-ms-win-core-featurestaging", 1, 1, 0, "shcore.dll"},
{"api-ms-win-core-fibers", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-fibers", 2, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-file-ansi", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-file-ansi", 2, 1, 0, "kernel32.dll"},
{"api-ms-win-core-file", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-file", 1, 2, 2, "kernelbase.dll"},
{"api-ms-win-core-file", 2, 1, 2, "kernelbase.dll"},
{"api-ms-win-core-firmware", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-guard", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-handle", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-heap", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-heap", 1, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-heap", 2, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-heap-obsolete", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-interlocked", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-interlocked", 1, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-io", 1, 1, 1, "kernel32.dll"},
{"api-ms-win-core-ioring", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-job", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-job", 2, 1, 0, "kernel32.dll"},
{"api-ms-win-core-kernel32-legacy-ansi", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-kernel32-legacy", 1, 1, 5, "kernel32.dll"},
{"api-ms-win-core-kernel32-private", 1, 1, 2, "kernel32.dll"},
{"api-ms-win-core-kernel32-private", 1, 2, 0, "kernel32.dll"},
{"api-ms-win-core-largeinteger", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-libraryloader", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-libraryloader", 1, 2, 2, "kernelbase.dll"},
{"api-ms-win-core-libraryloader", 2, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-libraryloader-private", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-localization-ansi", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-localization", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-localization", 1, 2, 2, "kernelbase.dll"},
{"api-ms-win-core-localization", 2, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-localization-obsolete", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-localization-obsolete", 1, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-localization-obsolete", 1, 3, 0, "kernelbase.dll"},
{"api-ms-win-core-localization-private", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-localregistry", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-marshal", 1, 1, 0, "combase.dll"},
{"api-ms-win-core-memory", 1, 1, 4, "kernelbase.dll"},
{"api-ms-win-core-misc", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-multipleproviderrouter", 1, 1, 0, "mpr.dll"},
{"api-ms-win-core-namedpipe-ansi", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-namedpipe", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-namedpipe", 1, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-namedpipe", 1, 2, 2, "kernelbase.dll"}, /* ADDED MANULLY. */
{"api-ms-win-core-namespace-ansi", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-namespace", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-normalization", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-path", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-pcw", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-perfcounters", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-perfcounters", 1, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-perfstm", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-privateprofile", 1, 1, 1, "kernel32.dll"},
{"api-ms-win-core-processenvironment-ansi", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-processenvironment", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-processenvironment", 1, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-processsecurity", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-processsnapshot", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-processthreads", 1, 1, 3, "kernel32.dll"},
{"api-ms-win-core-processtopology", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-processtopology", 1, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-processtopology-obsolete", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-processtopology-private", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-profile", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-psapi-ansi", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-psapi", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-psapi-obsolete", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-psapiansi", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-psm-appnotify", 1, 1, 0, "twinapi.appcore.dll"},
{"api-ms-win-core-psm-key", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-quirks", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-realtime", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-registry", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-registry", 2, 1, 0, "advapi32.dll"},
{"api-ms-win-core-registry", 2, 2, 0, "advapi32.dll"},
{"api-ms-win-core-registry", 2, 3, 0, "advapi32.dll"},
{"api-ms-win-core-registry-private", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-core-registryuserspecific", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-rtlsupport", 1, 1, 0, "ntdll.dll"},
{"api-ms-win-core-rtlsupport", 1, 2, 0, "ntdll.dll"},
{"api-ms-win-core-shlwapi-legacy", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-shlwapi-obsolete", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-shlwapi-obsolete", 1, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-shutdown-ansi", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-core-shutdown", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-core-sidebyside-ansi", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-sidebyside", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-slapi", 1, 1, 0, "slc.dll"},
{"api-ms-win-core-state-helpers", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-string", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-string", 2, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-string-obsolete", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-stringansi", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-stringloader", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-synch-ansi", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-synch", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-synch", 1, 2, 1, "kernelbase.dll"},
{"api-ms-win-core-sysinfo", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-sysinfo", 1, 2, 1, "kernelbase.dll"},
{"api-ms-win-core-sysinfo", 1, 2, 3, "kernelbase.dll" }, /* ADDED MANULLY. */
{"api-ms-win-core-sysinfo", 2, 1, 0, "advapi32.dll"},
{"api-ms-win-core-systemtopology", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-systemtopology", 1, 1, 1, "kernelbase.dll" }, /* ADDED MANULLY. */
{"api-ms-win-core-threadpool", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-threadpool", 1, 2, 0, "kernelbase.dll"},
{"api-ms-win-core-threadpool-legacy", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-threadpool-private", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-timezone", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-timezone-private", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-toolhelp", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-ums", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-url", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-util", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-core-version", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-version-private", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-versionansi", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-windowsceip", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-core-windowserrorreporting", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-winrt-error", 1, 1, 1, "combase.dll"},
{"api-ms-win-core-winrt-errorprivate", 1, 1, 1, "combase.dll"},
{"api-ms-win-core-winrt", 1, 1, 0, "combase.dll"},
{"api-ms-win-core-winrt-propertysetprivate", 1, 1, 1, "wintypes.dll"},
{"api-ms-win-core-winrt-registration", 1, 1, 0, "combase.dll"},
{"api-ms-win-core-winrt-robuffer", 1, 1, 0, "wintypes.dll"},
{"api-ms-win-core-winrt-roparameterizediid", 1, 1, 0, "combase.dll"},
{"api-ms-win-core-winrt-string", 1, 1, 1, "combase.dll"},
{"api-ms-win-core-wow64", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-core-wow64", 1, 1, 3, "kernelbase.dll" }, /* ADDED MANULLY. */
{"api-ms-win-core-xstate", 1, 1, 0, "ntdll.dll"},
{"api-ms-win-core-xstate", 2, 1, 0, "kernelbase.dll"},
{"api-ms-win-crt-conio", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-convert", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-environment", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-filesystem", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-heap", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-locale", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-math", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-multibyte", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-private", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-process", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-runtime", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-stdio", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-string", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-time", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-crt-utility", 1, 1, 0, "ucrtbase.dll"},
{"api-ms-win-deprecated-apis-obsolete", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-devices-config", 1, 1, 1, "cfgmgr32.dll"},
{"api-ms-win-devices-query", 1, 1, 1, "cfgmgr32.dll"},
{"api-ms-win-devices-swdevice", 1, 1, 1, "cfgmgr32.dll"},
{"api-ms-win-downlevel-advapi32", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-downlevel-advapi32", 2, 1, 0, "sechost.dll"},
{"api-ms-win-downlevel-advapi32", 4, 1, 0, "advapi32.dll"},
{"api-ms-win-downlevel-kernel32", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-downlevel-kernel32", 2, 1, 0, "kernel32.dll"},
{"api-ms-win-downlevel-normaliz", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-downlevel-ole32", 1, 1, 0, "combase.dll"},
{"api-ms-win-downlevel-shell32", 1, 1, 0, "shcore.dll"},
{"api-ms-win-downlevel-shlwapi", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-downlevel-shlwapi", 2, 1, 0, "shcore.dll"},
{"api-ms-win-downlevel-user32", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-downlevel-version", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-dwmapi", 1, 1, 0, "dwmapi.dll"},
{"api-ms-win-dx-d3dkmt", 1, 1, 0, "gdi32.dll"},
{"api-ms-win-eventing-classicprovider", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-eventing-consumer", 1, 1, 0, "sechost.dll"},
{"api-ms-win-eventing-controller", 1, 1, 0, "sechost.dll"},
{"api-ms-win-eventing-legacy", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-eventing-obsolete", 1, 1, 0, "sechost.dll"},
{"api-ms-win-eventing-provider", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-eventing-tdh", 1, 1, 2, "tdh.dll"},
{"api-ms-win-eventlog-legacy", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-eventlog-private", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-gaming-deviceinformation", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-gaming-tcui", 1, 1, 0, "gamingtcui.dll"},
{"api-ms-win-gdi-dpiinfo", 1, 1, 0, "gdi32.dll"},
{"api-ms-win-gdi-internal-uap", 1, 1, 0, "gdi32.dll" }, /* ADDED MANULLY. */
{"api-ms-win-http-time", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-legacy-shlwapi", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-mm-joystick", 1, 1, 0, "winmm.dll"},
{"api-ms-win-mm-mci", 1, 1, 0, "winmm.dll"},
{"api-ms-win-mm-misc", 1, 1, 1, "winmm.dll"},
{"api-ms-win-mm-misc", 2, 1, 0, "winmm.dll"},
{"api-ms-win-mm-mme", 1, 1, 0, "winmm.dll"},
{"api-ms-win-mm-playsound", 1, 1, 0, "winmm.dll"},
{"api-ms-win-mm-time", 1, 1, 0, "winmm.dll"},
{"api-ms-win-ntuser-dc-access", 1, 1, 0, "user32.dll"},
{"api-ms-win-ntuser-ie-message", 1, 1, 0, "user32.dll"},
{"api-ms-win-ntuser-ie-window", 1, 1, 0, "user32.dll"},
{"api-ms-win-ntuser-ie-wmpointer", 1, 1, 0, "user32.dll"},
{"api-ms-win-ntuser-rectangle", 1, 1, 0, "user32.dll"},
{"api-ms-win-ntuser-sysparams", 1, 1, 0, "user32.dll"},
{"api-ms-win-obsolete-localization", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-obsolete-psapi", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-obsolete-shlwapi", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-ole32-ie", 1, 1, 0, "ole32.dll"},
{"api-ms-win-oobe-notification", 1, 1, 0, "kernel32.dll"},
{"api-ms-win-perf-legacy", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-power-base", 1, 1, 0, "powrprof.dll"},
{"api-ms-win-power-limitsmanagement", 1, 1, 0, "powrprof.dll"},
{"api-ms-win-power-setting", 1, 1, 0, "powrprof.dll"},
{"api-ms-win-ro-typeresolution", 1, 1, 1, "wintypes.dll"},
{"api-ms-win-rtcore-ntuser-clipboard", 1, 1, 0, "user32.dll"},
{"api-ms-win-rtcore-ntuser-draw", 1, 1, 0, "user32.dll"},
{"api-ms-win-rtcore-ntuser-powermanagement", 1, 1, 0, "user32.dll"},
{"api-ms-win-rtcore-ntuser-private", 1, 1, 0, "user32.dll"},
{"api-ms-win-rtcore-ntuser-shell", 1, 1, 0, "user32.dll"},
{"api-ms-win-rtcore-ntuser-synch", 1, 1, 0, "user32.dll"},
{"api-ms-win-rtcore-ntuser-window", 1, 1, 0, "user32.dll"},
{"api-ms-win-rtcore-ntuser-winevent", 1, 1, 0, "user32.dll"},
{"api-ms-win-rtcore-ntuser-wmpointer", 1, 1, 0, "user32.dll"},
{"api-ms-win-rtcore-ntuser-wmpointer", 1, 2, 0, "user32.dll"},
{"api-ms-win-rtcore-ole32-clipboard", 1, 1, 0, "ole32.dll"},
{"api-ms-win-security-accesshlpr", 1, 1, 0, "sechost.dll"},
{"api-ms-win-security-activedirectoryclient", 1, 1, 0, "ntdsapi.dll"},
{"api-ms-win-security-appcontainer", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-security-audit", 1, 1, 1, "sechost.dll"},
{"api-ms-win-security-base-ansi", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-security-base", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-security-base", 1, 2, 0, "kernelbase.dll"},
{"api-ms-win-security-base-private", 1, 1, 1, "kernelbase.dll"},
{"api-ms-win-security-capability", 1, 1, 0, "sechost.dll"},
{"api-ms-win-security-cpwl", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-security-credentials", 1, 1, 0, "sechost.dll"},
{"api-ms-win-security-credentials", 2, 1, 0, "sechost.dll"},
{"api-ms-win-security-cryptoapi", 1, 1, 0, "cryptsp.dll"},
{"api-ms-win-security-grouppolicy", 1, 1, 0, "kernelbase.dll"},
{"api-ms-win-security-isolatedcontainer", 1, 1, 1, "shcore.dll"},
{"api-ms-win-security-isolationapi", 1, 1, 0, "sechost.dll"},
{"api-ms-win-security-isolationapi", 1, 2, 0, "sechost.dll"},
{"api-ms-win-security-isolationpolicy", 1, 1, 0, "sechost.dll"},
{"api-ms-win-security-isolationpolicy", 1, 2, 0, "sechost.dll"},
{"api-ms-win-security-logon", 1, 1, 1, "advapi32.dll"},
{"api-ms-win-security-lsalookup-ansi", 2, 1, 0, "advapi32.dll"},
{"api-ms-win-security-lsalookup", 1, 1, 1, "sechost.dll"},
{"api-ms-win-security-lsalookup", 2, 1, 1, "advapi32.dll"},
{"api-ms-win-security-lsapolicy", 1, 1, 0, "sechost.dll"},
{"api-ms-win-security-provider-ansi", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-security-provider", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-security-sddl-ansi", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-security-sddl", 1, 1, 0, "sechost.dll"},
{"api-ms-win-security-sddl-private", 1, 1, 0, "sechost.dll"},
{"api-ms-win-security-sddlparsecond", 1, 1, 0, "sechost.dll"},
{"api-ms-win-security-systemfunctions", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-security-trustee", 1, 1, 1, "advapi32.dll"},
{"api-ms-win-service-core-ansi", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-service-core", 1, 1, 1, "sechost.dll"},
{"api-ms-win-service-legacy", 1, 1, 0, "advapi32.dll"},
{"api-ms-win-service-management", 1, 1, 0, "sechost.dll"},
{"api-ms-win-service-management", 2, 1, 0, "sechost.dll"},
{"api-ms-win-service-private", 1, 1, 1, "sechost.dll"},
{"api-ms-win-service-private", 1, 2, 0, "sechost.dll"},
{"api-ms-win-service-winsvc", 1, 1, 0, "sechost.dll"},
{"api-ms-win-service-winsvc", 1, 2, 0, "sechost.dll"},
{"api-ms-win-shcore-comhelpers", 1, 1, 0, "shcore.dll"},
{"api-ms-win-shcore-obsolete", 1, 1, 0, "shcore.dll"},
{"api-ms-win-shcore-path", 1, 1, 0, "shcore.dll"},
{"api-ms-win-shcore-registry", 1, 1, 1, "shcore.dll"},
{"api-ms-win-shcore-scaling", 1, 1, 1, "shcore.dll"},
{"api-ms-win-shcore-stream", 1, 1, 0, "shcore.dll"},
{"api-ms-win-shcore-stream-winrt", 1, 1, 0, "shcore.dll"},
{"api-ms-win-shcore-sysinfo", 1, 1, 0, "shcore.dll"},
{"api-ms-win-shcore-taskpool", 1, 1, 0, "shcore.dll"},
{"api-ms-win-shcore-thread", 1, 1, 0, "shcore.dll"},
{"api-ms-win-shcore-unicodeansi", 1, 1, 0, "shcore.dll"},
{"api-ms-win-shell-shdirectory", 1, 1, 0, "shcore.dll"},
{"api-ms-win-shell-shellcom", 1, 1, 0, "shell32.dll"},
{"api-ms-win-shell-shellfolders", 1, 1, 0, "shell32.dll"},
{"api-ms-win-shlwapi-ie", 1, 1, 0, "shlwapi.dll"},
{"api-ms-win-shlwapi-winrt-storage", 1, 1, 1, "shlwapi.dll"},
{"api-ms-win-stateseparation-helpers", 1, 1, 0, "kernelbase.dll"},
{"ext-ms-onecore-dcomp", 1, 1, 0, "dcomp.dll"},
{"ext-ms-onecore-hlink", 1, 1, 0, "hlink.dll"},
{"ext-ms-onecore-shlwapi", 1, 1, 0, "shlwapi.dll"},
{"ext-ms-win-adsi-activeds", 1, 1, 0, "activeds.dll"},
{"ext-ms-win-advapi32-auth", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-encryptedfile", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-eventingcontroller", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-eventlog-ansi", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-eventlog", 1, 1, 1, "advapi32.dll"},
{"ext-ms-win-advapi32-hwprof", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-idletask", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-lsa", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-msi", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-npusername", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-ntmarta", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-registry", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-safer", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-advapi32-shutdown", 1, 1, 0, "advapi32.dll"},
{"ext-ms-win-appcompat-apphelp", 1, 1, 2, "apphelp.dll"},
{"ext-ms-win-appmodel-opc", 1, 1, 0, "opcservices.dll"},
{"ext-ms-win-authz-claimpolicies", 1, 1, 0, "authz.dll"},
{"ext-ms-win-authz-context", 1, 1, 0, "authz.dll"},
{"ext-ms-win-base-psapi", 1, 1, 0, "psapi.dll"},
{"ext-ms-win-base-rstrtmgr", 1, 1, 0, "rstrtmgr.dll"},
{"ext-ms-win-bluetooth-apis-internal", 1, 1, 0, "bluetoothapis.dll"},
{"ext-ms-win-bluetooth-apis", 1, 1, 0, "bluetoothapis.dll"},
{"ext-ms-win-bluetooth-apis-private", 1, 1, 0, "bluetoothapis.dll"},
{"ext-ms-win-cluster-clusapi", 1, 1, 5, "clusapi.dll"},
{"ext-ms-win-cluster-resutils", 1, 1, 3, "resutils.dll"},
{"ext-ms-win-com-ole32", 1, 1, 3, "ole32.dll"},
{"ext-ms-win-com-ole32", 1, 2, 0, "ole32.dll"},
{"ext-ms-win-com-ole32", 1, 3, 0, "ole32.dll"},
{"ext-ms-win-com-ole32", 1, 4, 0, "ole32.dll"},
{"ext-ms-win-com-sta", 1, 1, 0, "ole32.dll"},
{"ext-ms-win-compositor-hosting", 1, 1, 0, "user32.dll"},
{"ext-ms-win-core-iuri", 1, 1, 0, "urlmon.dll"},
{"ext-ms-win-core-marshal", 2, 1, 0, "ole32.dll"},
{"ext-ms-win-drvinst-desktop", 1, 1, 0, "newdev.dll"},
{"ext-ms-win-dwmapi-ext", 1, 1, 0, "dwmapi.dll"},
{"ext-ms-win-dwmapidxgi-ext", 1, 1, 1, "dwmapi.dll"},
{"ext-ms-win-dx-d3d9", 1, 1, 0, "d3d9.dll"},
{"ext-ms-win-dx-d3dkmt-dxcore", 1, 1, 3, "dxcore.dll"},
{"ext-ms-win-dx-d3dkmt-gdi", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-dx-ddraw", 1, 1, 0, "ddraw.dll"},
{"ext-ms-win-dx-dinput8", 1, 1, 0, "dinput8.dll"},
{"ext-ms-win-dxcore-internal", 1, 1, 0, "dxcore.dll"},
{"ext-ms-win-dxcore", 1, 1, 0, "dxcore.dll"},
{"ext-ms-win-eventing-pdh", 1, 1, 2, "pdh.dll"},
{"ext-ms-win-eventing-tdh-ext", 1, 1, 0, "tdh.dll"},
{"ext-ms-win-eventing-tdh-priv", 1, 1, 0, "tdh.dll"},
{"ext-ms-win-gaming-xinput", 1, 1, 0, "xinputuap.dll"},
{"ext-ms-win-gdi-clipping", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-gdi-dc-create", 1, 1, 1, "gdi32.dll"},
{"ext-ms-win-gdi-dc", 1, 2, 0, "gdi32.dll"},
{"ext-ms-win-gdi-devcaps", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-gdi-draw", 1, 1, 1, "gdi32.dll"},
{"ext-ms-win-gdi-font", 1, 1, 1, "gdi32.dll"},
{"ext-ms-win-gdi-gdiplus", 1, 1, 0, "gdiplus.dll"},
{"ext-ms-win-gdi-metafile", 1, 1, 2, "gdi32.dll"},
{"ext-ms-win-gdi-path", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-gdi-print", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-gdi-private", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-gdi-render", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-gdi-rgn", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-gdi-wcs", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-kernel32-appcompat", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-kernel32-datetime", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-kernel32-elevation", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-kernel32-errorhandling", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-kernel32-file", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-kernel32-localization", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-kernel32-package-current", 1, 1, 0, "kernelbase.dll"},
{"ext-ms-win-kernel32-package", 1, 1, 1, "kernelbase.dll"},
{"ext-ms-win-kernel32-process", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-kernel32-quirks", 1, 1, 1, "kernel32.dll"},
{"ext-ms-win-kernel32-registry", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-kernel32-sidebyside", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-kernel32-transacted", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-kernel32-updateresource", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-kernel32-windowserrorreporting", 1, 1, 1, "kernel32.dll"},
{"ext-ms-win-kernelbase-processthread", 1, 1, 0, "kernel32.dll"},
{"ext-ms-win-mapi-mapi32", 1, 1, 0, "mapistub.dll"},
{"ext-ms-win-media-avi", 1, 1, 0, "avifil32.dll"},
{"ext-ms-win-mm-msacm", 1, 1, 0, "msacm32.dll"},
{"ext-ms-win-mm-pehelper", 1, 1, 0, "mf.dll"},
{"ext-ms-win-mm-wmvcore", 1, 1, 0, "wmvcore.dll"},
{"ext-ms-win-msi-misc", 1, 1, 0, "msi.dll"},
{"ext-ms-win-msimg-draw", 1, 1, 0, "msimg32.dll"},
{"ext-ms-win-networking-wlanapi", 1, 1, 0, "wlanapi.dll"},
{"ext-ms-win-ntdsapi-activedirectoryclient", 1, 1, 1, "ntdsapi.dll"},
{"ext-ms-win-ntuser-caret", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-chartranslation", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-dc-access-ext", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-dialogbox", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-draw", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-gui", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-gui", 1, 2, 0, "user32.dll"},
{"ext-ms-win-ntuser-gui", 1, 3, 0, "user32.dll"},
{"ext-ms-win-ntuser-keyboard-ansi", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-keyboard", 1, 1, 1, "user32.dll"},
{"ext-ms-win-ntuser-keyboard", 1, 2, 0, "user32.dll"},
{"ext-ms-win-ntuser-keyboard", 1, 3, 0, "user32.dll"},
{"ext-ms-win-ntuser-menu", 1, 1, 2, "user32.dll"},
{"ext-ms-win-ntuser-message", 1, 1, 1, "user32.dll"},
{"ext-ms-win-ntuser-misc", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-misc", 1, 2, 0, "user32.dll"},
{"ext-ms-win-ntuser-misc", 1, 3, 0, "user32.dll"},
{"ext-ms-win-ntuser-misc", 1, 5, 1, "user32.dll"},
{"ext-ms-win-ntuser-misc", 1, 6, 0, "user32.dll"},
{"ext-ms-win-ntuser-misc", 1, 7, 0, "user32.dll"},
{"ext-ms-win-ntuser-mit", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-mouse", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-powermanagement", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-private", 1, 1, 1, "user32.dll"},
{"ext-ms-win-ntuser-private", 1, 2, 0, "user32.dll"},
{"ext-ms-win-ntuser-private", 1, 3, 3, "user32.dll"},
{"ext-ms-win-ntuser-private", 1, 4, 0, "user32.dll"},
{"ext-ms-win-ntuser-private", 1, 5, 0, "user32.dll"},
{"ext-ms-win-ntuser-private", 1, 6, 0, "user32.dll"},
{"ext-ms-win-ntuser-rawinput", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-rawinput", 1, 2, 0, "user32.dll"},
{"ext-ms-win-ntuser-rectangle-ext", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-rim", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-rim", 1, 2, 0, "user32.dll"},
{"ext-ms-win-ntuser-rotationmanager", 1, 1, 1, "user32.dll"},
{"ext-ms-win-ntuser-server", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-string", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-synch", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-sysparams-ext", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-touch-hittest", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-uicontext-ext", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-window", 1, 1, 4, "user32.dll"},
{"ext-ms-win-ntuser-windowclass", 1, 1, 1, "user32.dll"},
{"ext-ms-win-ntuser-windowstation-ansi", 1, 1, 0, "user32.dll"},
{"ext-ms-win-ntuser-windowstation", 1, 1, 1, "user32.dll"},
{"ext-ms-win-odbc-odbc32", 1, 1, 0, "odbc32.dll"},
{"ext-ms-win-ole32-bindctx", 1, 1, 0, "ole32.dll"},
{"ext-ms-win-ole32-ie-ext", 1, 1, 0, "ole32.dll"},
{"ext-ms-win-ole32-oleautomation", 1, 1, 0, "ole32.dll"},
{"ext-ms-win-oleacc", 1, 1, 0, "oleacc.dll"},
{"ext-ms-win-printer-prntvpt", 1, 1, 2, "prntvpt.dll"},
{"ext-ms-win-printer-winspool-core", 1, 1, 0, "winspool.drv"},
{"ext-ms-win-printer-winspool", 1, 1, 4, "winspool.drv"},
{"ext-ms-win-printer-winspool", 1, 2, 0, "winspool.drv"},
{"ext-ms-win-profile-extender", 1, 1, 0, "userenv.dll"},
{"ext-ms-win-ras-rasapi32", 1, 1, 0, "rasapi32.dll"},
{"ext-ms-win-ras-rasdlg", 1, 1, 0, "rasdlg.dll"},
{"ext-ms-win-ras-tapi32", 1, 1, 1, "tapi32.dll"},
{"ext-ms-win-ro-typeresolution", 1, 1, 1, "wintypes.dll"},
{"ext-ms-win-rtcore-gdi-devcaps", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-rtcore-gdi-object", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-rtcore-gdi-rgn", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-rtcore-ntuser-cursor", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-dc-access", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-dpi", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-dpi", 1, 2, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-iam", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-inputintercept", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-integration", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-keyboard", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-message-ansi", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-message", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-rawinput", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-synch-ext", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-syscolors", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-sysparams", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-usersecurity", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-window-ansi", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-window-ext", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-window", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-winevent-ext", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ntuser-wmpointer", 1, 1, 0, "user32.dll"},
{"ext-ms-win-rtcore-ole32-dragdrop", 1, 1, 0, "ole32.dll"},
{"ext-ms-win-rtcore-ole32-misc", 1, 1, 0, "ole32.dll"},
{"ext-ms-win-secur32-translatename", 1, 1, 0, "secur32.dll"},
{"ext-ms-win-security-credui", 1, 1, 0, "credui.dll"},
{"ext-ms-win-security-cryptui", 1, 1, 0, "cryptui.dll"},
{"ext-ms-win-security-kerberos", 1, 1, 0, "kerberos.dll"},
{"ext-ms-win-security-slc", 1, 1, 0, "slc.dll"},
{"ext-ms-win-session-usertoken", 1, 1, 0, "wtsapi32.dll"},
{"ext-ms-win-session-winsta", 1, 1, 4, "winsta.dll"},
{"ext-ms-win-session-wtsapi32", 1, 1, 0, "wtsapi32.dll"},
{"ext-ms-win-setupapi-classinstallers", 1, 1, 2, "setupapi.dll"},
{"ext-ms-win-setupapi-inf", 1, 1, 1, "setupapi.dll"},
{"ext-ms-win-setupapi-logging", 1, 1, 0, "setupapi.dll"},
{"ext-ms-win-shell-aclui", 1, 1, 0, "aclui.dll"},
{"ext-ms-win-shell-comctl32-da", 1, 1, 0, "comctl32.dll"},
{"ext-ms-win-shell-comctl32-init", 1, 1, 0, "comctl32.dll"},
{"ext-ms-win-shell-comctl32", 1, 1, 0, "comctl32.dll"},
{"ext-ms-win-shell-comctl32-window", 1, 1, 0, "comctl32.dll"},
{"ext-ms-win-shell-comdlg32", 1, 1, 0, "comdlg32.dll"},
{"ext-ms-win-shell-exports-internal", 1, 1, 0, "shell32.dll"},
{"ext-ms-win-shell-propsys", 1, 1, 1, "propsys.dll"},
{"ext-ms-win-shell-shdocvw", 1, 1, 0, "shdocvw.dll"},
{"ext-ms-win-shell-shell32", 1, 2, 0, "shell32.dll"},
{"ext-ms-win-shell-shell32", 1, 3, 0, "shell32.dll"},
{"ext-ms-win-shell-shell32", 1, 4, 0, "shell32.dll"},
{"ext-ms-win-shell-shell32", 1, 5, 0, "shell32.dll"},
{"ext-ms-win-shell-shlwapi", 1, 1, 1, "shlwapi.dll"},
{"ext-ms-win-shell-shlwapi", 1, 2, 0, "shlwapi.dll"},
{"ext-ms-win-sxs-oleautomation", 1, 1, 0, "sxs.dll"},
{"ext-ms-win-tsf-msctf", 1, 1, 4, "msctf.dll"},
{"ext-ms-win-uiacore", 1, 1, 3, "uiautomationcore.dll"},
{"ext-ms-win-usp10", 1, 1, 0, "gdi32.dll"},
{"ext-ms-win-uxtheme-themes", 1, 1, 0, "uxtheme.dll"},
{"ext-ms-win-wevtapi-eventlog", 1, 1, 3, "wevtapi.dll"},
{"ext-ms-win-wlan-scard", 1, 1, 0, "winscard.dll"},
{NULL, 0, 0, 0, NULL},
};

/* Convert a api set name string to struct. */
static int ConvertAPISet(char* api_set_name,
	__WIN_API_NAME* pResult)
{
	int name_len = strlen(api_set_name);
	int index = 0;

	if (name_len < 16)
	{
		/* api set name's length must longer 16. */
		return -1;
	}
	name_len -= 4;
	if (strncmp(&api_set_name[name_len], ".dll", 4))
	{
		/* Must has sufix '.dll'. */
		return -1;
	}

	name_len -= 1;
	pResult->minor_ver = api_set_name[name_len];
	pResult->minor_ver -= '0';
	name_len -= 2;
	pResult->major_ver = api_set_name[name_len];
	pResult->major_ver -= '0';
	name_len -= 2;
	pResult->master_ver = api_set_name[name_len];
	pResult->master_ver -= '0';
	name_len -= 2;
	api_set_name[name_len] = 0;
	pResult->api_set_name = api_set_name;
	pResult->dll_name = NULL;

	return 0;
}

/* Get the container DLL's name given the API set name. */
static const char* __get_container_dll_name(const char* api_set_name,
	const __WIN_API_NAME* name_array,
	BOOL prefix_match)
{
	__WIN_API_NAME api_set;
	char api_set_string[MAX_MODULE_NAME_LEN];
	const __WIN_API_NAME* current = NULL;
	const __WIN_API_NAME* prefix_matched = NULL;
	const char* result = NULL;

	/*
	 * Use local array to hold api set name,
	 * since it's will be changed in convert
	 * routine.
	 */
	strncpy(api_set_string, api_set_name, MAX_MODULE_NAME_LEN);
	if (ConvertAPISet(api_set_string, &api_set) < 0)
	{
		_hx_printf("[%s]Convert API set [%s] fail.\r\n",
			__func__, api_set_name);
		goto __TERMINAL;
	}

	/* Search array to find the api set. */
	current = name_array;
	while (current->api_set_name)
	{
		if (0 == strncmp(current->api_set_name, api_set_string, MAX_MODULE_NAME_LEN))
		{
			break;
		}
		current++;
	}
	if (NULL == current->api_set_name)
	{
		/* Can not match. */
		goto __TERMINAL;
	}

	/*
	 * Find the api set element, compare
	 * one by one since there maybe many.
	 */
	while (current->api_set_name)
	{
		if (strncmp(current->api_set_name, api_set_string, MAX_MODULE_NAME_LEN))
		{
			break;
		}
		if (current->master_ver > api_set.master_ver)
		{
			result = current->dll_name;
			goto __TERMINAL;
		}
		if (current->master_ver < api_set.master_ver)
		{
			prefix_matched = current;
			current++;
			continue;
		}
		/* Master version equal. */
		if (current->major_ver > api_set.major_ver)
		{
			result = current->dll_name;
			goto __TERMINAL;
		}
		if (current->major_ver < api_set.major_ver)
		{
			prefix_matched = current;
			current++;
			continue;
		}
		/* Major version equal. */
		if (current->minor_ver < api_set.minor_ver)
		{
			prefix_matched = current;
			current++;
			continue;
		}
		result = current->dll_name;
		goto __TERMINAL;
	}

	/* 
	 * No match found, if prefix match specified, 
	 * return the one that nearest to the required.
	 */
	if (prefix_match)
	{
		result = prefix_matched->dll_name;
	}

__TERMINAL:
	return result;
}

/* Return the container DLL's nave given API set name. */
const char* GetContainerDllName(const char* api_name)
{
	const char* dll_name = NULL;
	char tmp_name[MAX_MODULE_NAME_LEN];
	int prefix_len = 0;

	/* Validate the API name's format. */
	strncpy(tmp_name, api_name, MAX_MODULE_NAME_LEN);
	prefix_len = strlen(WIN_API_SET_PREFIX_API);
	tmp_name[prefix_len] = 0;
	if ((strncmp(tmp_name, WIN_API_SET_PREFIX_API, prefix_len) != 0) &&
		(strncmp(tmp_name, WIN_API_SET_PREFIX_EXT, prefix_len) != 0))
	{
		/* Specified name is not valid. */
		goto __TERMINAL;
	}

	dll_name = __get_container_dll_name(api_name, win_api_name, TRUE);

__TERMINAL:
	return dll_name;
}
