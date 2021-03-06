/*
 *  CWDropInPanel.h
 *
 *  Copyright (c) 1999 Metrowerks inc.  All rights reserved.
 *
 *   DropIn Preferences Panel Interface for UNIX/WIN32 Metrowerks CodeWarrior
 */

#ifndef __CWDROPINPANEL_H__
#define __CWDROPINPANEL_H__

#ifdef __MWERKS__
#	pragma once
#endif

#ifndef __CWPlugins_H__
#include "CWPlugins.h"
#endif

#ifdef	__MWERKS__
	#pragma options align=mac68k
#endif
#ifdef	_MSC_VER
	#pragma pack(push,2)
#endif

#ifdef __cplusplus
	extern "C" {
#endif


/*
 * Convenience macros for swapping pref-panel values. Use this macros only if your
 * system/libraries do not provide their own public swapping functions/macros.
 *
 */
 
/* Swap 16-bit(short) value	*/
#define CWPREF_BYTESWAP_SHORT(value)	value =			\
		(((((unsigned short)(value))<<8) & 0xFF00)	| 	\
		 ((((unsigned short)(value))>>8) & 0x00FF))

/* Swap 32-bit(long) value	*/
#define CWPREF_BYTESWAP_LONG(value)		value =				\
		(((((unsigned long)(value))<<24) & 0xFF000000)	|	\
		 ((((unsigned long)(value))<< 8) & 0x00FF0000)	|	\
		 ((((unsigned long)(value))>> 8) & 0x0000FF00)	|	\
		 ((((unsigned long)(value))>>24) & 0x000000FF))


#ifndef __DROPINPANEL_H__

/* API version numbers */
#define DROPINPANELAPIVERSION_1	1							/* CW7 API version                       */
#define DROPINPANELAPIVERSION_2	2							/* intermediate version                  */
#define DROPINPANELAPIVERSION_3	3							/* CW8 API version                       */
#define DROPINPANELAPIVERSION_4	4							/* CW9 API version                       */
#define DROPINPANELAPIVERSION_5 5							/* CW Pro 2 API version                  */
#define DROPINPANELAPIVERSION_7 7							/* CW Pro 3 Mac API version              */
#define DROPINPANELAPIVERSION_8 8							/* CW Pro 3 Win32 API version            */
#define DROPINPANELAPIVERSION_9 9							/* CW Pro 5 Win32 API version            */
#define DROPINPANELAPIVERSION_10 10							/* CW Pro 6 Win32 API version            */
#define DROPINPANELAPIVERSION_11 11							/* CW Pro 7 Win32 API version            */
#define DROPINPANELAPIVERSION	DROPINPANELAPIVERSION_11	/* current API version                   */			

/* Error codes you return to the IDE */
#define kBadPrefVersion			1000						/* panel doesn't know about this version */
#define kMissingPrefErr			1001
#define kSettingNotFoundErr		1002
#define kSettingTypeMismatchErr	1003
#define kInvalidCallbackErr		1004
#define kSettingOutOfRangeErr	1005

/*
 * Requests codes - sent by the IDE to your panel
 * Note:	Many of these are Mac-specific but are included for compatability reasons.
 *			Future APIs will reimplement many of them is an OS-neutral way.
 */
enum {
	reqInitPanel = -2,		/* (called when panel is loaded)                                   */
	reqTermPanel = -1,		/* (called when panel is unloaded)                                 */
	reqInitDialog = 0,		/* initialize panel's dialog state                                 */
	reqTermDialog,			/* clean up panel's dialog state                                   */
	reqPutData,				/* copy options data to dialog items                               */
	reqGetData,				/* copy dialog items to options data                               */
	reqFilter,				/* filter a dialog event for the panel					(Mac only) */
	reqItemHit,				/* process an itemHit in the panel                                 */
	reqAEGetPref,			/* get preference setting for AppleEvent request		(Mac only) */
	reqAESetPref,			/* set preference setting from AppleEvent request		(Mac only) */
	reqValidate,			/* tell if current settings force recompile or relink              */
	reqGetFactory,			/* retrieve factory settings                                       */
	reqUpdatePref,			/* verify and/or modify prefs to fit current version               */
	reqUpdateProject,		/* (only sent to built-in panels)                                  */
	reqSetupDebug,			/* change settings to reflect debugging status                     */
	reqRenameProject,		/* change settings that depend on project name                     */
	reqPrefsLoaded,			/* (only sent to built-in panels)                                  */
	reqDrawCustomItem,		/* draw a custom item									(Mac only) */
	reqActivateItem,		/* activate a custom item								(Mac only) */
	reqDeactivateItem,		/* deactivate a custom item								(Mac only) */
	reqHandleClick,			/* handle mouse down in an active custom item			(Mac only) */
	reqHandleKey,			/* handle key down in an active custom item				(Mac only) */
	reqFindStatus,			/* enable/disable menu items for a custom item			(Mac only) */
	reqObeyCommand,			/* execute a menu command for a custom item				(Mac only) */
	reqDragEnter,			/* the user is dragging into the given item				(Mac only) */
	reqDragWithin,			/* the user is dragging within the given item			(Mac only) */
	reqDragExit,			/* the user is dragging out of the given item			(Mac only) */
	reqDragDrop,			/* the user dropped onto the given item					(Mac only) */
	reqByteSwapData,		/* byte swap the fields in the prefs data                          */
	reqFirstLoad,			/* panel has been loaded for the first time                        */
	reqReadSettings,		/* read settings from IDE to construct pref data handle            */
	reqWriteSettings,		/* write the individual settings from pref data handle             */
	reqItemDoubleClick,		/* process an double clicked in the panel, currently implemented for listviews  */
	reqItemIsCellEditable	/* currently implemented for listviews, tells whether or not the listview is editable  */
};

/* Layout and bit flags for 'Flag' resource for panels 								*/
/*																					*/
/* For the version 3 of these resource, we renamed the 'apiversion' field to		*/
/* 'earliestCompatibleAPIVersion' and added the 'newestAPIVersion' field.			*/
/* This allows plugins to support more than one API version and therefore run		*/
/* under more than one version of the IDE.											*/

typedef struct PanelFlags {
	unsigned short	rsrcversion;		/*	version number of resource				*/
	CWDataType		dropintype;			/*	dropin type ('Comp', 'Link', 'Panl')	*/
										/*  earliest API support by this plugin		*/
	unsigned short	earliestCompatibleAPIVersion;
	unsigned long	dropinflags;		/*	capability flags (see enum below)		*/
	CWDataType		panelfamily;		/*	family of panel (linker, codegen, etc)	*/
	unsigned short	newestAPIVersion;	/*	newest API version supported			*/
	unsigned short	dataversion;		/*	version number of prefs data			*/
	unsigned short	panelscope;			/*	scope of panel (see enum below)			*/
} PanelFlags;

/* capability flags, as used in member dropinflags of PanelFlags struct				*/
enum {
	usesStrictAPI			= 1 << 31,			/* this panel is built with the	strict API	*/
	supportsByteSwapping	= 1 << 30,			/* this panel support the byte-swapping request */
	supportsTextSettings	= 1 << 29,			/* this panel supports the read & write settings requests */
	usesCrossPlatformAPI	= 1 << 28			/* uses the cross-platform API rather than Mac API */
	/* remaining flags are reserved for future use and should be zero-initialized	*/
};

/* panel scopes, as used in member panelscope of PanelFlags struct					*/
/*																					*/
/* The scope of a panel tells the IDE which settings window to display the panel.	*/
/* Currently, only panels for debugger plug-ins use panelScopeGlobal and only		*/
/* panels for VCS plug-ins use panelScopeProject. A panel for a compiler or linker	*/
/* must use panelScopeTarget.														*/

enum {
	panelScopeGlobal,	/*	this panel is scoped to the global preferences window	*/
	panelScopeProject,	/*	this panel is scoped to the VCS settings window			*/
	panelScopeTarget	/* 	this panel is scoped to the target settings window		*/
};

/* pre-defined panel families, used in panelfamily field of PanelFlags struct		*/

enum {
	panelFamilyProject		= CWFOURCHAR('p', 'r', 'o', 'j'),
	panelFamilyFrontEnd		= CWFOURCHAR('f', 'e', 'n', 'd'),
	panelFamilyBackEnd		= CWFOURCHAR('b', 'e', 'n', 'd'),
	panelFamilyBrowser		= CWFOURCHAR('b', 'r', 'o', 'w'),
	panelFamilyEditor		= CWFOURCHAR('e', 'd', 'i', 't'),
	panelFamilyDebugger		= CWFOURCHAR('d', 'b', 'u', 'g'),
	panelFamilyLinker		= CWFOURCHAR('l', 'i', 'n', 'k'),
	panelFamilyMisc			= CWFOURCHAR('*', '*', '*', '*')
};

typedef struct MWSetting* CWSettingID;
#define kNoSettingID 0

#endif	/* not defined __DROPINPANEL_H__ */

/*-------------------------------------------------------------------------------------
   Callbacks to the IDE, in addition to those in CWPlugins.h
  -----------------------------------------------------------------------------------*/

/* Dialog related */

CW_CALLBACK	CWPanelAppendItems			(CWPluginContext context, short ditlID);
CW_CALLBACK	CWPanelShowItem				(CWPluginContext context, long whichItem, Boolean showIt);
CW_CALLBACK	CWPanelEnableItem			(CWPluginContext context, long whichItem, Boolean enableIt);
CW_CALLBACK	CWPanelActivateItem			(CWPluginContext context, long whichItem);
CW_CALLBACK	CWPanelGetItemValue			(CWPluginContext context, long whichItem, long* value);
CW_CALLBACK	CWPanelSetItemValue			(CWPluginContext context, long whichItem, long value);
CW_CALLBACK	CWPanelGetItemText			(CWPluginContext context, long whichItem, char* str, short maxLen);
CW_CALLBACK	CWPanelSetItemText			(CWPluginContext context, long whichItem, char* str);
CW_CALLBACK	CWPanelInsertListItem		(CWPluginContext context, long dlgItemID, long index, char* str);
CW_CALLBACK	CWPanelDeleteListItem		(CWPluginContext context, long dlgItemID, long index);
CW_CALLBACK	CWPanelGetListItemSize		(CWPluginContext context, long dlgItemID, long* listSize);
CW_CALLBACK	CWPanelGetListItemText		(CWPluginContext context, long dlgItemID, long index, char* str, short maxLen);
CW_CALLBACK	CWPanelSetListItemText		(CWPluginContext context, long dlgItemID, long index, char* str);
CW_CALLBACK	CWPanelGetPopupItemChecked	(CWPluginContext context, long dlgItemID, long index, Boolean* checked);
CW_CALLBACK	CWPanelSetPopupItemChecked	(CWPluginContext context, long dlgItemID, long index, Boolean checked);
CW_CALLBACK	CWPanelSetPopupItemEnabled	(CWPluginContext context, long dlgItemID, long index, Boolean enabled);
CW_CALLBACK	CWPanelInvalItem			(CWPluginContext context, long whichItem);
CW_CALLBACK	CWPanelValidItem			(CWPluginContext context, long whichItem);
CW_CALLBACK	CWPanelGetNumBaseDialogItems(CWPluginContext context, short* baseItems);
CW_CALLBACK	CWPanelGetDialogItemHit		(CWPluginContext context, short* itemHit);
CW_CALLBACK	CWPanelGetItemTextHandle	(CWPluginContext context, long whichItem, CWMemHandle* text);
CW_CALLBACK	CWPanelSetItemTextHandle	(CWPluginContext context, long whichItem, CWMemHandle text);
CW_CALLBACK	CWPanelGetItemData			(CWPluginContext context, long dlgItemID, void *outData, long *outDataLength);
CW_CALLBACK	CWPanelSetItemData			(CWPluginContext context, long dlgItemID, void *inData, long inDataLength);
CW_CALLBACK	CWPanelGetItemMaxLength		(CWPluginContext context, long dlgItemID, short *outLength);
CW_CALLBACK	CWPanelSetItemMaxLength		(CWPluginContext context, long dlgItemID, short inLength);
CW_CALLBACK	CWPanelChooseRelativePath	(CWPluginContext context, CWRelativePath *ioPath, Boolean isFolder, short filterCount, void *filterList, char *prompt);
CW_CALLBACK	CWPanelGetRelativePathString(CWPluginContext context, CWRelativePath *inPath, char *pathString, long* maxLength);

/* CellView related, with string elements */
// selection
CW_CALLBACK	CWPanelListViewSelectCell			(CWPluginContext context,long dlgItemID,long col,long row,Boolean addToSelection);
CW_CALLBACK	CWPanelListViewUnselectCell			(CWPluginContext context,long dlgItemID,long col,long row,Boolean *result);
CW_CALLBACK	CWPanelListViewSelectNothing		(CWPluginContext context,long dlgItemID);
CW_CALLBACK	CWPanelListViewGetSelectionCount	(CWPluginContext context,long dlgItemID,long *count);
CW_CALLBACK	CWPanelListViewGetSelection			(CWPluginContext context,long dlgItemID,long *col,long *row);
// rows
CW_CALLBACK	CWPanelListViewAddRows				(CWPluginContext context,long dlgItemID,long afterRow,long count);
CW_CALLBACK	CWPanelListViewDeleteRows			(CWPluginContext context,long dlgItemID,long firstRow,long count);
CW_CALLBACK	CWPanelListViewGetRowCount			(CWPluginContext context,long dlgItemID,long *count);
// refresh
CW_CALLBACK	CWPanelListViewRefreshRow			(CWPluginContext context,long dlgItemID,long row);
// columns
CW_CALLBACK	CWPanelListViewAddCols				(CWPluginContext context,long dlgItemID,long afterCol,long count);
CW_CALLBACK	CWPanelListViewDeleteCols			(CWPluginContext context,long dlgItemID,long firstCol,long count);
CW_CALLBACK	CWPanelListViewGetColCount			(CWPluginContext context,long dlgItemID,long *count);
CW_CALLBACK	CWPanelListViewGetColWidth			(CWPluginContext context,long dlgItemID,long col,short *colWidth);
CW_CALLBACK	CWPanelListViewSetColWidth			(CWPluginContext context,long dlgItemID,long col,short colWidth);
CW_CALLBACK	CWPanelListViewNextCell				(CWPluginContext context,long dlgItemID,long *col,long *row,Boolean hNext,Boolean vNext,Boolean selectableOnly,Boolean *result); // col and row is in-> and <-out
CW_CALLBACK	CWPanelListViewRefreshCell			(CWPluginContext context,long dlgItemID,long col,long row);
CW_CALLBACK	CWPanelListViewScrollToCell			(CWPluginContext context,long dlgItemID,long col,long row,Boolean inRefresh);
CW_CALLBACK	CWPanelListViewGetCellTextValue		(CWPluginContext context,long dlgItemID,long col,long row,char* str, short maxLen);
CW_CALLBACK	CWPanelListViewSetCellTextValue		(CWPluginContext context,long dlgItemID,long col,long row,char* str);
// utilities for giving a answer to a request
CW_CALLBACK	CWPanelListViewSetDoubleClickHandled(CWPluginContext context, Boolean value);
CW_CALLBACK	CWPanelListViewSetEditable			(CWPluginContext context, Boolean value);
CW_CALLBACK	CWPanelListViewGetCell				(CWPluginContext context, long *col,long *row);

/* Preference data related */

CW_CALLBACK	CWPanelGetOriginalPrefs		(CWPluginContext context, CWMemHandle* originalPrefs);
CW_CALLBACK	CWPanelGetCurrentPrefs		(CWPluginContext context, CWMemHandle* currentPrefs);
CW_CALLBACK	CWPanelGetFactoryPrefs		(CWPluginContext context, CWMemHandle* factoryPrefs);
CW_CALLBACK	CWPanelGetDebugFlag			(CWPluginContext context, Boolean* debugOn);
CW_CALLBACK	CWPanelSetRevertFlag		(CWPluginContext context, Boolean canRevert);
CW_CALLBACK	CWPanelSetFactoryFlag		(CWPluginContext context, Boolean canFactory);
CW_CALLBACK	CWPanelSetResetPathsFlag	(CWPluginContext context, Boolean resetPaths);
CW_CALLBACK	CWPanelSetRecompileFlag		(CWPluginContext context, Boolean recompile);
CW_CALLBACK	CWPanelSetRelinkFlag		(CWPluginContext context, Boolean relink);
CW_CALLBACK	CWPanelSetReparseFlag		(CWPluginContext context, Boolean reparse);
CW_CALLBACK	CWPanelGetPanelPrefs		(CWPluginContext context, const char* panelName, CWMemHandle *prefs, Boolean* requiresByteSwap);
CW_CALLBACK	CWPanelGetOldProjectFile	(CWPluginContext context, CWFileSpec* projectSpec);
CW_CALLBACK CWPanelGetToEndian			(CWPluginContext context, short* toEndian);

#if CWPLUGIN_HOST == CWPLUGIN_HOST_MACOS
CW_CALLBACK	CWPanelGetAEKeyword			(CWPluginContext context, AEKeyword* keyword);
CW_CALLBACK	CWPanelGetAEDesc			(CWPluginContext context, AEDesc* desc);
CW_CALLBACK	CWPanelSetAEDesc			(CWPluginContext context, const AEDesc* desc);
CW_CALLBACK	CWPanelReadRelativePathAEDesc	(CWPluginContext context, CWRelativePath* path, const AEDesc* desc);
CW_CALLBACK	CWPanelWriteRelativePathAEDesc	(CWPluginContext context, const CWRelativePath* path, AEDesc* desc);
#endif

/* Reading and writing scalar settings */

CW_CALLBACK	CWReadBooleanSetting		(CWPluginContext context, const char* name, Boolean* value);
CW_CALLBACK	CWReadIntegerSetting		(CWPluginContext context, const char* name, long* value);
CW_CALLBACK	CWReadFloatingPointSetting	(CWPluginContext context, const char* name, double* value);
CW_CALLBACK	CWReadStringSetting			(CWPluginContext context, const char* name, const char** value);
CW_CALLBACK	CWReadRelativePathSetting	(CWPluginContext context, const char* name, CWRelativePath* value);

CW_CALLBACK	CWWriteBooleanSetting		(CWPluginContext context, const char* name, Boolean value);
CW_CALLBACK	CWWriteIntegerSetting		(CWPluginContext context, const char* name, long value);
CW_CALLBACK	CWWriteFloatingPointSetting	(CWPluginContext context, const char* name, double value);
CW_CALLBACK	CWWriteStringSetting		(CWPluginContext context, const char* name, const char* value);
CW_CALLBACK	CWWriteRelativePathSetting	(CWPluginContext context, const char* name, const CWRelativePath* value);

/* Reading and writing array and structure settings */

CW_CALLBACK	CWGetNamedSetting			(CWPluginContext context, const char* name, CWSettingID* settingID);
CW_CALLBACK	CWGetStructureSettingField	(CWPluginContext context, CWSettingID settingID, const char* name, CWSettingID* fieldSettingID);
CW_CALLBACK	CWGetArraySettingSize		(CWPluginContext context, CWSettingID settingID, long* size);
CW_CALLBACK	CWGetArraySettingElement	(CWPluginContext context, CWSettingID settingID, long index, CWSettingID* elementSettingID);

CW_CALLBACK	CWGetBooleanValue			(CWPluginContext context, CWSettingID settingID, Boolean* value);
CW_CALLBACK	CWGetIntegerValue			(CWPluginContext context, CWSettingID settingID, long* value);
CW_CALLBACK	CWGetFloatingPointValue		(CWPluginContext context, CWSettingID settingID, double* value);
CW_CALLBACK	CWGetStringValue			(CWPluginContext context, CWSettingID settingID, const char** value);
CW_CALLBACK	CWGetRelativePathValue		(CWPluginContext context, CWSettingID settingID, CWRelativePath* value);

CW_CALLBACK	CWSetBooleanValue			(CWPluginContext context, CWSettingID settingID, Boolean value);
CW_CALLBACK	CWSetIntegerValue			(CWPluginContext context, CWSettingID settingID, long value);
CW_CALLBACK	CWSetFloatingPointValue		(CWPluginContext context, CWSettingID settingID, double value);
CW_CALLBACK	CWSetStringValue			(CWPluginContext context, CWSettingID settingID, const char* value);
CW_CALLBACK	CWSetRelativePathValue		(CWPluginContext context, CWSettingID settingID, const CWRelativePath* value);

#ifdef __cplusplus
	}
#endif

#ifdef __MWERKS__
	#pragma options align=reset
#endif

#ifdef	_MSC_VER
	#pragma pack(pop)
#endif

#endif	/* __CWDROPINPANEL_H__ */
