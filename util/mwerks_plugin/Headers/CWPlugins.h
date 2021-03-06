/*
 *  CWPlugins.h - Common declarations for Metrowerks CodeWarrior plugins
 *
 *  Copyright (c) 1995-1997 Metrowerks, Inc.  All rights reserved.
 *
 */

#ifndef __CWPlugins_H__
#define __CWPlugins_H__

#ifdef __MWERKS__
#	pragma once
#endif

#define CWPLUGIN_HOST_MACOS       1
#define CWPLUGIN_HOST_WIN32       2
#define CWPLUGIN_HOST_SOLARIS     3
#define CWPLUGIN_HOST_LINUX       4

#ifndef CWPLUGIN_HOST
#  ifdef WIN32
#  define CWPLUGIN_HOST CWPLUGIN_HOST_WIN32
#  elif defined(macintosh)
#  define CWPLUGIN_HOST CWPLUGIN_HOST_MACOS
#  elif defined(__sun__)
#  define CWPLUGIN_HOST CWPLUGIN_HOST_SOLARIS
#  elif defined(__linux__)
#  define CWPLUGIN_HOST CWPLUGIN_HOST_LINUX
#  else
#  error
#  endif
#endif

#define CWPLUGIN_API_MACOS 1
#define CWPLUGIN_API_WIN32 2
#define CWPLUGIN_API_UNIX  3

#ifndef CWPLUGIN_API
#  ifdef WIN32
#  define CWPLUGIN_API CWPLUGIN_API_WIN32
#  elif defined(macintosh)
#  define CWPLUGIN_API CWPLUGIN_API_MACOS
#  elif defined(__sun__) || defined(__linux__)
#  define CWPLUGIN_API CWPLUGIN_API_UNIX
#  else
#  error
#  endif
#endif


/*
**  Radix 256 notation where a 32-bit integer is created from four
**  ASCII characters. A four-character constant of this form, say
**  'ABCD', must always be represented with the same pattern, 0x41424344
**  in this case, regardless of big/little endian issues.
*/
typedef long CWFourCharType;
#define CWFOURCHAR(a, b, c, d)                            \
                 (((CWFourCharType) ((a) & 0xff) << 24)   \
                | ((CWFourCharType) ((b) & 0xff) << 16)   \
                | ((CWFourCharType) ((c) & 0xff) << 8)    \
                | ((CWFourCharType) ((d) & 0xff)))


#if CWPLUGIN_API == CWPLUGIN_API_UNIX
#include <sys/param.h>
#endif

#ifndef CW_USE_PRAGMA_EXPORT
#if CWPLUGIN_HOST == CWPLUGIN_HOST_MACOS
#define CW_USE_PRAGMA_EXPORT 1
#else
#define CW_USE_PRAGMA_EXPORT 0
#endif
#endif

#ifndef CW_USE_PRAGMA_IMPORT
#if CWPLUGIN_HOST == CWPLUGIN_HOST_MACOS
#define CW_USE_PRAGMA_IMPORT 1
#else
#define CW_USE_PRAGMA_IMPORT 0
#endif
#endif

#ifdef	__MWERKS__
	#pragma options align=mac68k
#endif

#ifdef	_MSC_VER
	#pragma	pack(push,2)
#endif


#ifdef __cplusplus
	extern "C" {
#endif

#if CW_USE_PRAGMA_IMPORT
#pragma import on
#endif

	/* These constants specify the action the IDE is asking the plugin to execute			*/
enum {
	reqInitialize  = -2,		/* called when the plugin is loaded							*/
	reqTerminate   = -1,		/* called when the plugin is unloaded						*/
	reqIdle		   = -100,		/* called periodically to allow for plugin tasks EP 6/24/98	*/
	reqAbout	   = -101,		/* called to ask plugin to display about dialog EP 6/24/98	*/
	reqPrefsChange = -102		/* called when an associated pref panel changes EP 6/24/98  */
};

	/* Used in CWFileInfo.dependencyType to indicate what type of make						*/
	/* dependency to establish between files												*/
typedef enum CWDependencyType {
	cwNoDependency,				/* do not establish a dependency							*/
	cwNormalDependency,			/* recompile dependent whenever prereq changes				*/
	cwInterfaceDependency		/* recompile only if interface to file changes				*/
} CWDependencyType;

	/* Used in CWFileInfo.filedatatype to indicate the type of data in a loaded file		*/
enum {
	cwFileTypeUnknown,			/* unknown binary data										*/
	cwFileTypeText,				/* normal text file											*/
	cwFileTypePrecompiledHeader	/* cached precompiled header								*/
};

	/* constant for CWFileInfo.isdependentoffile 											*/
#define kCurrentCompiledFile		-1L

	/* constant for CWStorePluginData/CWGetPluginData										*/
#define kTargetGlobalPluginData		-1L

	/* constant for CWNewProjectEntryInfo link order, segment, and overlay values				*/
#define kDefaultLinkPosition		-1L

	/* Selectors for CWFindLogicalDirectory */
enum {
	kIDEDirectorySelector = 1,				/* parent directory of IDE application; "bin" folder on Win32	*/
    kCodeWarriorDirectorySelector,			/* root CodeWarrior directory									*/
    kSystemDirectorySelector,				/* system directory												*/
    kProjectDirectorySelector,				/* parent directory of current project							*/
    kProjectDataDirectorySelector,			/* project data directory										*/
    kTargetDataDirectorySelector,			/* target data directory (within project data directory)		*/
    kTargetObjectCodeDirectorySelector,		/* object code directory (within target data directory)			*/
    kDebuggerCacheDirectorySelector,		/* "CW Debugging Cache" directory								*/
    kHelperAppsDirectorySelector,			/* "(Helper Apps)" directory									*/
    kPluginsDirectorySelector,				/* "CodeWarrior Plugins" (Mac) or "plugins" (Win32) directory	*/
    kPluginParentDirectorySelector,			/* parent directory of current plugin							*/
    kStationeryDirectorySelector,			/* "(Project Stationery)" directory								*/
    kRADStationeryDirectorySelector,		/* "RAD Stationery" directory									*/
    kLocalizedResourcesDirectorySelector	/* "resources" directory										*/
};


	/* CWPluginContext is a magic cookie passed to all plugins. It must						*/
	/* be passed back to all IDE callbacks													*/
	
typedef struct CWPluginPrivateContext* CWPluginContext;

	/* CWResult is the error/status result returned by all IDE API routine.					*/
	/* The most common errors are returned directly. For OS-specific errors, the 			*/
	/* CWResult is cwErrOSError, and the OS-specific error can be obtained by				*/
	/* calling CWGetOSError()																*/ 
typedef long			CWResult;

	/* CWMemHandle is an abstraction for memory used in some parts						*/
	/* of the plugin API. API routines must be used to allocate							*/
	/* and free CWMemHandles, or to convert them to pointers.							*/
	 
typedef struct CWMemHandlePrivateStruct* CWMemHandle;

	/* Used to identify custom data associated by a plugin with							*/
	/* a project file or a target as a whole. Must be a four character					*/
	/* constant. All lower case constants are reserved by the IDE						*/
typedef unsigned long CWDataType;

	/* Some information used in the compiler/linker API is platform-dependent				*/
	/* We use some typedefs to isolate the differences										*/
	/* CWFileSpec contains the native platform file specifier.								*/
	/* CWFileName contains the string type for a native file name							*/
	/* CWFileTime contains the native platform file timestamp								*/
	/* CWOSResult contains the native platform error return value							*/
	/* CWResult contains an API routine error/status result									*/
	/* CW_CALLBACK is a macro defining the calling convention and return type for			*/
	/*  IDE callback routines.																*/
	/* CW_PLUGINENTRY is a macro defining the calling convention and return type for		*/
	/*  plugin entry points.																*/
	/* CWSUCCESS is a macro that evaluates to true when given a CWResult indicating an 		*/
	/*   routine succeeded																	*/

#if CWPLUGIN_API == CWPLUGIN_API_MACOS

typedef FSSpec			CWFileSpec;
typedef char			CWFileName[32];
typedef unsigned long 	CWFileTime;
typedef OSErr			CWOSResult;

#elif CWPLUGIN_API == CWPLUGIN_API_WIN32

typedef unsigned char 	Boolean;
typedef struct CWFileSpec { char path[MAX_PATH]; } CWFileSpec;
typedef char			CWFileName[65];
typedef FILETIME		CWFileTime;
typedef DWORD			CWOSResult;

#elif CWPLUGIN_API == CWPLUGIN_API_UNIX

#define MAX_PATH	MAXPATHLEN

#ifndef __MACTYPES__
typedef unsigned char 	Boolean;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef struct CWFileSpec { char path[MAX_PATH]; } CWFileSpec;
typedef char			CWFileName[65];
typedef time_t			CWFileTime;
typedef int				CWOSResult;

#else

#error Unknown plugin API!

#endif

#define CWSUCCESS(result)	((result) == 0)

#if CWPLUGIN_HOST == CWPLUGIN_HOST_MACOS

#define CW_CALLBACK							pascal CWResult
#define	CW_CALLBACK_PTR(function_name)		pascal CWResult (function_name)
#define CWPLUGIN_ENTRY(function_name)		pascal short (function_name)

#elif CWPLUGIN_HOST == CWPLUGIN_HOST_WIN32

#define CW_CALLBACK							CWResult __stdcall
#define	CW_CALLBACK_PTR(function_name)		CWResult (__stdcall function_name)
#define CWPLUGIN_ENTRY(function_name)		short (__stdcall function_name)

#elif CWPLUGIN_HOST==CWPLUGIN_HOST_SOLARIS || CWPLUGIN_HOST==CWPLUGIN_HOST_LINUX

#define CW_CALLBACK							CWResult
#define	CW_CALLBACK_PTR(function_name)		CWResult (function_name)
#define CWPLUGIN_ENTRY(function_name)		short (function_name)

#else

#error Unknown plugin host!

#endif

/* information returned by CWFindAndLoadFile */
typedef struct CWFileInfo {
	Boolean 		fullsearch;			/* [<-] do we do a full search of paths for file	*/
	char	 		dependencyType;		/* [<-] type CWDependencyType						*/
	long			isdependentoffile;	/* [<-] this file is a dependent of file id			*/
										/*		(zero based) or -1L if of current file		*/
	Boolean 		suppressload;		/* [<-] find but don't load the file				*/
	Boolean 		padding;			/* [**] structure padding							*/
	const char*		filedata;			/* [->] point to the file text/data, or NULL		*/
	long			filedatalength;		/* [->] length of filedata							*/
	short			filedatatype;		/* [->] type of data pointed to by filedata			*/
	
	/* the remaining members are valid only when called by compilers						*/
	short			fileID;				/* [->] use in browse records and dependencies		*/
	CWFileSpec		filespec;			/* [->] specifies the file on disk					*/
	Boolean			alreadyincluded;	/* [->] TRUE if already included in current compile	*/
	Boolean 		recordbrowseinfo;	/* [->] record browse info for this file?			*/
} CWFileInfo;

/* information maintained by the IDE for each segment in the project */
typedef struct CWProjectSegmentInfo {
	char			name[32];			/* segment name			*/
	short			attributes;			/* segment attributes	*/
} CWProjectSegmentInfo;	

/* 64 bit address	*/
typedef struct CWAddr64 {
	long	lo;							/* low order longword of address						*/
	long	hi;							/* high order longword of address						*/
} CWAddr64;

/* describes an overlay group, Use CWGetOverlayGroup1Info to iterate over overlay groups 		*/
typedef struct CWOverlay1GroupInfo {
	char			name[256];			/* overlay group name									*/
	CWAddr64		address;			/* load address											*/
	long			numoverlays;		/* number of overlays in this group						*/

} CWOverlay1GroupInfo;

/* describes an overlay, use CWGetOverlay1Info to iterate over overlays							*/ 
typedef struct CWOverlay1Info {
	char			name[256];			/* name of this overlay									*/
	long			numfiles;			/* number of files in the overlay						*/
} CWOverlay1Info;

/* describes a file in an overlay, use CWGetOverlayFile1Info to iterate over files. Use		*/
/* whichfile in calls taking a file number, e.g. CWLoadObjectData or CWGetFileInfo			*/ 
typedef struct CWOverlay1FileInfo {
	long			whichfile;			/* flat file number										*/
} CWOverlay1FileInfo;

/*
 *	All compiler errors or warnings which occur at some specific location in some
 *	source file are identified by a CWMessageRef structure. This structure
 *	provides sufficient information for the development environment to locate
 *	and display the exact position associated with a message. For each message,
 *  the compiler provides:
 *
 *		errorstring: contains a description of the error, e.g. "syntax error"
 * 		errorline:   contains a subset of the text containing the error
 *		errorlevel:  indicates if the "error" is an error, warning, or informational message
 *      CWMessageRef: further info so the IDE can perform two tasks:
 
 *			- Display a summary of each message, with the "error token"
 *			  underlined within the compiler-provided errorline. This 
 *			  information is provided by the tokenoffset and tokenlength
 *			  fields. If tokenLength is zero then no underlining is performed.
 *
 *			- Open the file containing the error and select the full
 *    		  text of the error. This information is provided by the
 *			  selectionoffset and selectionlength fields. 
 */

typedef struct CWMessageRef
{
	CWFileSpec	sourcefile;			/* file containing error							*/
	long		linenumber;			/* error linenumber in file							*/
	short 		tokenoffset;        /* offset into errorline of token underline 		*/
	short 		tokenlength;        /* length of error token to be underlined			*/
	long		selectionoffset;	/* start of error for text selection				*/
	long		selectionlength;	/* length of error for text selection				*/
} CWMessageRef;

/* message types, used for errorlevel parameter to message routines */
enum {
	messagetypeInfo,					/* informational only								*/
	messagetypeWarning,					/* warning message									*/
	messagetypeError					/* error message									*/
};


/* information maintained by the IDE for each file in the project */
typedef struct CWProjectFileInfo 
{
	CWFileSpec		filespec;			/* CW_FileSpec of file in project			*/
	CWFileTime		moddate;			/* date source file was last modified		*/
	short			segment;			/* segment number of file					*/
	Boolean			hasobjectcode;		/* file has object code to be linked		*/
	Boolean			hasresources;		/* file has resources to be linked			*/
	Boolean			isresourcefile;		/* file -is- a resource file to be linked	*/
	Boolean			weakimport;			/* file has "Import Weak" flag set			*/
	Boolean			initbefore;			/* file has "Init Before" flag set			*/
	Boolean			gendebug;			/* file has generate debug info on			*/
	CWFileTime		objmoddate;			/* date object code was last modified		*/
	CWFileName		dropinname;			/* name of dropin used to process this file	*/
	short			fileID;				/* fileID to use in browse records			*/
	Boolean		 	recordbrowseinfo;	/* record browse info for this file?		*/
	Boolean			reserved;			/* reserved and used internally				*/
#if CWPLUGIN_HOST == CWPLUGIN_HOST_MACOS
	/* some Macintosh-specific information											*/
	OSType			filetype;			/* MacOS file type							*/
	OSType			filecreator;		/* MacOS file creator						*/
#endif
	Boolean			hasunitdata;		/* file has associated unit data (Pascal)	*/
	Boolean			mergeintooutput;	/* file has "Merge Into Output" flag set	*/
	unsigned long	unitdatadependencytag;	/* dependency tag (i.e. checksum) of unit data (Pascal)	*/
} CWProjectFileInfo;

typedef struct CWNewTextDocumentInfo
{
	const char*		documentname;		/* name for new document, can be NULL		*/
	CWMemHandle		text;				/* initial text for document				*/
	Boolean			markDirty;			/* mark doc as needing to be saved?			*/
} CWNewTextDocumentInfo;

typedef struct CWNewProjectEntryInfo
{	/* use kDefaultLinkPosition to get default link order, overlay, or segment		*/
	long			position;			/* optional link order position				*/
	long			segment;			/* optional segment number					*/
	long			overlayGroup;		/* optional overlay group number			*/
	long			overlay;			/* optional overlay number					*/
	const char*		groupPath;			/* optional fully qualified path to file group */
	Boolean			mergeintooutput;	/* set merge into output flag?				*/
	Boolean			weakimport;			/* set weak import flag?					*/
	Boolean			initbefore;			/* set initbefore flag?						*/
} CWNewProjectEntryInfo;

typedef struct CWIDEInfo
{
	unsigned short		majorVersion;
	unsigned short		minorVersion;
	unsigned short		bugFixVersion;
	unsigned short		buildVersion;
	unsigned short		dropinAPIVersion;
} CWIDEInfo;

/* Structures for getting the list of access paths.								*/
/* The callback does the filtering on the host flags, so the list returned		*/
/* will only contain the paths that are enabled for the host platform.			*/
/* There are separate APIs to get the Mac OS X framework style access paths.	*/

typedef enum CWAccessPathType {
	cwSystemPath,
	cwUserPath
} CWAccessPathType;

typedef struct CWAccessPathInfo
{
	CWFileSpec			pathSpec;
	Boolean				recursive;
	long				subdirectoryCount;
} CWAccessPathInfo;

typedef struct CWAccessPathListInfo
{
	long		systemPathCount;
	long		userPathCount;
	Boolean		alwaysSearchUserPaths;
	Boolean		convertPaths;
} CWAccessPathListInfo;

/* Structures for getting the list of Mac OS X framework access paths.			*/
/* The callback does the filtering on the host flags, so the list returned		*/
/* will only contain the Mac OS X framework style paths that are enabled for	*/
/* the host platform.															*/
/* There are separate APIs to get the traditional style access paths.			*/

typedef struct CWFrameworkAccessPathInfo
{
	CWFileSpec			pathSpec;
	Boolean				framework;
} CWFrameworkAccessPathInfo;

typedef struct CWFrameworkAccessPathListInfo
{
	long		systemPathCount;
	long		userPathCount;
	Boolean		alwaysSearchUserPaths;
	Boolean		convertPaths;
	Boolean		requireFrameworkIncludes;
} CWFrameworkAccessPathListInfo;

/* constants for different types of plugins											*/
/* Used in the dropintype in the DropInFlags, as well as for the MacOS file type	*/
enum
{
	CWDROPINLINKERTYPE	= CWFOURCHAR('L','i','n','k'),		/*	type for DropIn linkers			*/
	CWDROPINCOMPILERTYPE= CWFOURCHAR('C','o','m','p'),		/*	type for DropIn compilers		*/
	CWDROPINPREFSTYPE	= CWFOURCHAR('P','a','n','L'),		/*	type for DropIn panels			*/
	CWDROPINPREFSTYPE_1	= CWFOURCHAR('P','a','n','l'),		/*  type for 1.x IDE DropIn panels	*/
	CWDROPINVCSTYPE		= CWFOURCHAR('V','C','S',' '),		/*	type for DropIn version control	*/
	CWDROPINCOMTYPE		= CWFOURCHAR('C','O','M',' ')		/*  type for COM-only plugins 		*/
};
typedef long CWPluginType; // one of the above types

/* Format of 'Flag' resources, or data returned by dropin's GetDropinFlags entry    */
/* point.																			*/
/*																					*/
/* For the version 2 of these resource, we renamed the 'apiversion' field to		*/
/* 'earliestCompatibleAPIVersion' and added the 'newestAPIVersion' field.			*/
/* This allows plugins to support more than one API version and therefore run		*/
/* under more than one version of the IDE. The CWGetAPIVersion call should be used  */
/* to determine what API version the IDE is using to talk to a plugin.				*/

#define kCurrentDropInFlagsVersion 2

typedef struct DropInFlags {
	short			rsrcversion;		/*	version number of resource				*/
	CWDataType		dropintype;			/*	dropin type (compiler, panel, etc)		*/
										/*  earliest API support by this plugin		*/
	unsigned short	earliestCompatibleAPIVersion;
	unsigned long	dropinflags;		/*	capability flags						*/
	CWDataType		edit_language;		/*	language								*/
	unsigned short	newestAPIVersion;	/*	newest API version supported			*/
	
} DropInFlags, **DropInFlagsHandle;

#define kCurrentCWPanelListVersion 1

typedef struct CWPanelList {
	short			version;
	short			count;
	const char**	names;
} CWPanelList;

#define kCurrentCWFamilyListVersion	1
#define kCurrentCWFamilyResourceVersion 1

typedef struct CWFamily {
	CWDataType		type;
	const char*		name;
} CWFamily;

typedef struct CWFamilyList {
	short			version;
	short			count;
	CWFamily*		families;
} CWFamilyList;

typedef struct CWFamilyResource {
	short			version;
	CWDataType		type;
	unsigned char	name[64];
} CWFamilyResource;

#define kCurrentCWHelpInfoVersion 1

typedef struct CWHelpInfo {
	short			version;
	const char*		helpFileName;
} CWHelpInfo;


#define	kCurrentCWRelativePathVersion		1

typedef enum CWRelativePathFormat
{
	format_Generic	= 0,		// Simple name, not platform-specific
	format_Mac,					// Uses : as separator :: for parent directory
	format_Win,					// Uses \ as separator .. for parent directory
	format_Unix					// Uses / as separator .. for parent directory

} CWRelativePathFormat;

typedef enum CWRelativePathTypes
{
	type_Absolute	= 0,
	type_Project,
	type_Compiler,
	type_System,
	type_UserDefined
} CWRelativePathTypes;

typedef struct CWRelativePath
{
	short			version;				// version number
	unsigned char	pathType;				// use CWRelativePathTypes
	unsigned char	pathFormat;				// use CWRelativePathFormat
	char			userDefinedTree[256];	// user-defined tree name
	char			pathString[512];		// actual path string
} CWRelativePath;

/*
 *
 */
 
#define kCurrentCWPluginInfoVersion 1

typedef struct CWPluginInfo
{
	short			version;				// struct version number
	const char*		companyName;			// i.e. Metrowerks	
	const char*		pluginName;				// Defaults to Dropin->GetName()
	const char*		pluginDisplayName;
	const char* 	familyName;				// i.e. Java
	unsigned short	majorIDEVersion;		// Version of IDE Required
	unsigned short	minorIDEVersion;
		  
} CWPluginInfo;


/* Declaration of plugin entry points that must be implemented by non-MacOS plugins		*/
/* It can also be implemented for MacOS plugins instead of having a 'Flag' resource		*/

CWPLUGIN_ENTRY (CWPlugin_GetDropInFlags)(const DropInFlags**, long* flagsSize);

/* Declaration of plugin entry points that may optionally be implemented by plugins		*/
/* These entry points override the corresponding resources on MacOS						*/

CWPLUGIN_ENTRY (CWPlugin_GetDropInName)(const char** dropInName);
CWPLUGIN_ENTRY (CWPlugin_GetDisplayName)(const char** displayName);
CWPLUGIN_ENTRY (CWPlugin_GetPanelList)(const CWPanelList** panelList);
CWPLUGIN_ENTRY (CWPlugin_GetFamilyList)(const CWFamilyList** familyList);
CWPLUGIN_ENTRY (CWPlugin_GetHelpInfo)(const CWHelpInfo** helpInfo);

/* Declaration of info plugin entry point that must be implemented by all COM plugins   */

CWPLUGIN_ENTRY (CWPlugin_GetPluginInfo)(const CWPluginInfo** pluginInfo);


/* Callback declarations: these callbacks are supported for all CodeWarrior plugins		*/
	
	/* Get the action the IDE is requesting of the plugin								*/
CW_CALLBACK CWGetPluginRequest(CWPluginContext context, long* request);

	/* Call when finished handling a request, just before returning to the shell		*/
CW_CALLBACK CWDonePluginRequest(CWPluginContext, CWResult resultCode);

	/* Get the version number of API used by the IDE to talk to the plugin				*/
CW_CALLBACK	CWGetAPIVersion(CWPluginContext context, long* version);

	/* Get information about the IDE being used											*/
CW_CALLBACK CWGetIDEInfo(CWPluginContext context, CWIDEInfo* info);

	/* Get the OS error associated with the last callback								*/
CW_CALLBACK CWGetCallbackOSError(CWPluginContext context, CWOSResult* error);

	/* Set the OS error associated with a failed plugin request							*/
CW_CALLBACK CWSetPluginOSError(CWPluginContext context, CWOSResult);
	
	/* Get the file specifier for the current project									*/
CW_CALLBACK	CWGetProjectFile(CWPluginContext context, CWFileSpec* projectSpec);

	/* Get the directory where the IDE stores target-specific generated data			*/
CW_CALLBACK CWGetTargetDataDirectory(CWPluginContext context, CWFileSpec* targetDataDirectorySpec);

	/* Get the name of the current target in the current project						*/
CW_CALLBACK CWGetTargetName(CWPluginContext context, char* name, short maxLength);

	/* Get the directory where output files should be stored						*/
CW_CALLBACK CWGetOutputFileDirectory(CWPluginContext context, CWFileSpec* outputFileDirectory);

	/* Get the number of files in the current project									*/
CW_CALLBACK CWGetProjectFileCount(CWPluginContext context, long* count);

	/* Get information about a particular file in the project							*/
CW_CALLBACK	CWGetFileInfo(CWPluginContext context, long whichfile, Boolean checkFileLocation, CWProjectFileInfo* fileinfo);

	/* Search for a file by name on the current file's access paths. 					*/
CW_CALLBACK	CWFindAndLoadFile(CWPluginContext context, const char* filename, CWFileInfo *fileinfo);

	/* Get the access paths for the current target			 					*/
CW_CALLBACK	CWGetAccessPathListInfo(CWPluginContext context, CWAccessPathListInfo* pathListInfo);
CW_CALLBACK	CWGetAccessPathInfo(CWPluginContext context, CWAccessPathType pathType, long whichPath, CWAccessPathInfo* pathInfo);
CW_CALLBACK	CWGetAccessPathSubdirectory(CWPluginContext context, CWAccessPathType pathType, long whichPath, long whichSubdirectory, CWFileSpec* subdirectory);
CW_CALLBACK	CWGetFrameworkAccessPathListInfo(CWPluginContext context, CWFrameworkAccessPathListInfo* pathListInfo);
CW_CALLBACK	CWGetFrameworkAccessPathInfo(CWPluginContext context, CWAccessPathType pathType, long whichPath, CWFrameworkAccessPathInfo* pathInfo);

	/* Get file text, from the editor, include file cache, or by reading the file		*/
CW_CALLBACK CWGetFileText(CWPluginContext context, const CWFileSpec* filespec, const char** text, long* textLength, short* filedatatype);

	/* Release file text returned by CWFindAndLoadFile and CWGetFileText				*/
CW_CALLBACK	CWReleaseFileText(CWPluginContext context, const char* text);

	/* Get information about a project segment											*/
CW_CALLBACK	CWGetSegmentInfo(CWPluginContext context, long whichsegment, CWProjectSegmentInfo* segmentinfo);

	/* Get the number of overlay groups in the target									*/
CW_CALLBACK	CWGetOverlay1GroupsCount(CWPluginContext context, long* count);

	/* Get information about a project overlay group									*/
CW_CALLBACK CWGetOverlay1GroupInfo(CWPluginContext context, long whichgroup, CWOverlay1GroupInfo* groupinfo);

	/* Get information about an overlay	within a group									*/
CW_CALLBACK CWGetOverlay1Info(CWPluginContext context, long whichgroup, long whichoverlay, CWOverlay1Info* overlayinfo);

	/* Get information about a file in an overlay										*/
CW_CALLBACK CWGetOverlay1FileInfo(CWPluginContext context, long whichgroup, long whichoverlay, long whichoverlayfile, CWOverlay1FileInfo* fileinfo);

	/* Report a error, warning, or informational message								*/
CW_CALLBACK	CWReportMessage(CWPluginContext context, const CWMessageRef* msgRef, const char *line1, const char *line2, short errorlevel, long errorNumber);

	/* Display an alert. May actually be put in a message, depending on the plugin request */
CW_CALLBACK	CWAlert(CWPluginContext context, const char* msg1, const char* msg2, const char* msg3, const char* msg4);

	/* Display one or two status messages to the user									*/
CW_CALLBACK	CWShowStatus(CWPluginContext context, const char *line1, const char *line2);

	/* Give to the IDE to handle events and check if user has canceled this operation	*/
CW_CALLBACK	CWUserBreak(CWPluginContext context);

	/* Return stored preference data, referenced by name. Typically used for preference	*/
	/* panel settings.																	*/
CW_CALLBACK	CWGetNamedPreferences(CWPluginContext context, const char* prefsname, CWMemHandle* prefsdata);

	/* Store data referenced by a data type and file number								*/
CW_CALLBACK	CWStorePluginData(CWPluginContext context, long whichfile, CWDataType type, CWMemHandle prefsdata);

	/* Return stored data referenced by a data type and file number						*/
CW_CALLBACK	CWGetPluginData(CWPluginContext context, long whichfile, CWDataType type, CWMemHandle* prefsdata);

	/* Inform the IDE that a file modification date has changed. isGenerated is for use	*/
	/* by compiler and linker plugins only												*/
CW_CALLBACK	CWSetModDate(CWPluginContext context, const CWFileSpec* filespec, CWFileTime* moddate, Boolean isGenerated);

	/* Ask the IDE to add a file to the current target in the current project. isGenerated */
	/* is for use by compiler plugins only.												*/
CW_CALLBACK CWAddProjectEntry(CWPluginContext context, const CWFileSpec* fileSpec, Boolean isGenerated, const CWNewProjectEntryInfo* projectEntryInfo, long* whichfile);

	/* Ask the IDE to remove a file from the current target (link-order/segment/overlay) in the current project. */
	/* If it's the last target that contains the file, it would be removed from the file list.								    	  									   */
CW_CALLBACK CWRemoveProjectEntry(CWPluginContext context, const CWFileSpec* fileSpec);

	/* Create a new editor window, supplying initial text and an optional document name */
CW_CALLBACK CWCreateNewTextDocument(CWPluginContext, const CWNewTextDocumentInfo* docinfo);

	/* Allocate memory. Permanent memory is not freed until the plugin is unloaded.		*/
	/* Temporary memory is freed after each plugin request completes.					*/
CW_CALLBACK	CWAllocateMemory(CWPluginContext context, long size, Boolean isPermanent, void** ptr);

	/* Free memory allocated via CWAllocateMemory										*/
CW_CALLBACK CWFreeMemory(CWPluginContext context, void* ptr, Boolean isPermanent);

	/* Allocate a memory handle of the requested size. All handles are automatically 	*/
	/* freed at the end of each compiler/linker request. useTempMemory is MacOS-specific*/
CW_CALLBACK	CWAllocMemHandle(CWPluginContext context, long size, Boolean useTempMemory, CWMemHandle* handle);

	/* Free a memory handle																*/
CW_CALLBACK	CWFreeMemHandle(CWPluginContext context, CWMemHandle handle);

	/* Return the current size of a memory handle										*/
CW_CALLBACK	CWGetMemHandleSize(CWPluginContext context, CWMemHandle handle, long* size);

	/* Resize an existing memory handle													*/
CW_CALLBACK	CWResizeMemHandle(CWPluginContext context, CWMemHandle handle, long newSize);

	/* To obtain a pointer to the block, you must lock the handle						*/
	/* moveHi is MacOS-specific															*/
CW_CALLBACK	CWLockMemHandle(CWPluginContext context, CWMemHandle handle, Boolean moveHi, void** ptr);

	/* Unlock a memory handle, the pointer returned by locking the handle may no		*/
	/* longer be valid																	*/
CW_CALLBACK	CWUnlockMemHandle(CWPluginContext context, CWMemHandle handle);

#if CWPLUGIN_HOST == CWPLUGIN_HOST_MACOS
	/* Utility function to map MacOS error codes to a CWResult code. Plugins			*/
	/* may internally generate OSErrs, but need to return CWResult to the				*/
	/* CodeWarrior IDE																	*/
CW_CALLBACK CWMacOSErrToCWResult(CWPluginContext context, OSErr err);
#endif

	/* Turn off the built in PP window manager while the plugin displays a dialog		*/
CW_CALLBACK CWPreDialog(CWPluginContext context);

	/* Turn on the built in PP window manager after the plugin is through with its 		*/
	/* dialog																			*/
CW_CALLBACK CWPostDialog(CWPluginContext context);

	/* Notify the IDE that the plugin will be performing actions on a file 				*/
CW_CALLBACK CWPreFileAction(CWPluginContext context, const CWFileSpec *theFile);

	/* Notify the IDE that the plugin is finished performing actions on a file 			*/
CW_CALLBACK CWPostFileAction(CWPluginContext context, const CWFileSpec *theFile);

CW_CALLBACK CWResolveRelativePath(CWPluginContext context, const CWRelativePath* relativePath, CWFileSpec* fileSpec, Boolean create);

CW_CALLBACK CWFindLogicalDirectory(CWPluginContext context, long selector, CWFileSpec *dirSpec);

CW_CALLBACK CWOpenFileInEditor(CWPluginContext context, const CWFileSpec *fileSpec);


#if CWPLUGIN_HOST==CWPLUGIN_HOST_SOLARIS || CWPLUGIN_HOST==CWPLUGIN_HOST_LINUX
/* Forward declarations */
struct ICodeWarriorApp;
struct ICodeWarriorProject;
struct ICodeWarriorDesign;
struct ICodeWarriorTarget;
#endif /* CWPLUGIN_HOST==CWPLUGIN_HOST_SOLARIS || CWPLUGIN_HOST==CWPLUGIN_HOST_LINUX */


	/* Get the IDE application COM interface. Fails if called from any thread 			*/
	/* other than the main IDE thread, e.g. a build thread.								*/
CW_CALLBACK CWGetCOMApplicationInterface(CWPluginContext context, struct ICodeWarriorApp **app);

	/* Get the current project COM interface. Adds a reference upon return. 			*/
	/* Only succeeds if the plugin is being called in the context of a					*/
	/* particular project. Fails if called from any thread								*/
	/* other than the main IDE thread, e.g. a build thread.								*/
CW_CALLBACK CWGetCOMProjectInterface(CWPluginContext context, struct ICodeWarriorProject **project);

	/* Get the current design COM interface. Adds a reference upon return. 				*/
	/* Only succeeds if the plugin is being called in the context of a					*/
	/* particular target that is associated with a design.								*/
	/* Fails if called from any thread other than the main IDE thread, 					*/
	/* e.g. a build thread.																*/
CW_CALLBACK CWGetCOMDesignInterface(CWPluginContext context, struct ICodeWarriorDesign **design);

	/* Get the current target COM interface. Adds a reference upon return. 				*/
	/* Only succeeds if the plugin is being called in the context of a					*/
	/* particular target. Fails if called from any thread								*/
	/* other than the main IDE thread, e.g. a build thread.								*/
CW_CALLBACK CWGetCOMTargetInterface(CWPluginContext context, struct ICodeWarriorTarget **target);

#if CW_USE_PRAGMA_IMPORT
#pragma import reset
#endif

#ifdef __cplusplus
	}
#endif

#ifdef	_MSC_VER
#pragma	pack(pop)
#endif

#ifdef	__MWERKS__
#pragma options align=reset
#endif

#endif	/* __CWPlugins_H__ */
