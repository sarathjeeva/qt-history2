/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPLATFORMDEFS_H
#define QPLATFORMDEFS_H

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif

// Get Qt defines/settings

#include "qglobal.h"

#include <tchar.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <windows.h>
#include <limits.h>

#if !defined(_WIN32_WINNT) || (_WIN32_WINNT-0 < 0x0500)
typedef enum {
    NameUnknown		  = 0, 
    NameFullyQualifiedDN  = 1, 
    NameSamCompatible	  = 2, 
    NameDisplay		  = 3, 
    NameUniqueId	  = 6, 
    NameCanonical	  = 7, 
    NameUserPrincipal	  = 8, 
    NameCanonicalEx	  = 9, 
    NameServicePrincipal  = 10, 
    NameDnsDomain	  = 12
} EXTENDED_NAME_FORMAT, *PEXTENDED_NAME_FORMAT;
#endif

#define Q_FS_FAT
#ifdef QT_LARGEFILE_SUPPORT
#define QT_STATBUF		struct _stati64		// non-ANSI defs
#define QT_STATBUF4TSTAT	struct _stati64		// non-ANSI defs
#define QT_STAT			::_stati64
#define QT_FSTAT		::_fstati64
#else
#define QT_STATBUF		struct _stat		// non-ANSI defs
#define QT_STATBUF4TSTAT	struct _stat		// non-ANSI defs
#define QT_STAT			::_stat
#define QT_FSTAT		::_fstat
#endif
#define QT_STAT_REG		_S_IFREG
#define QT_STAT_DIR		_S_IFDIR
#define QT_STAT_MASK		_S_IFMT
#if defined(_S_IFLNK)
#  define QT_STAT_LNK		_S_IFLNK
#endif
#define QT_FILENO		_fileno
#define QT_OPEN			::_open
#define QT_CLOSE		::_close
#ifdef QT_LARGEFILE_SUPPORT
#define QT_LSEEK		::_lseeki64
#ifndef UNICODE
#define QT_TSTAT		::_stati64
#else
#define QT_TSTAT		::_wstati64
#endif
#else
#define QT_LSEEK		::_lseek
#ifndef UNICODE
#define QT_TSTAT		::_stat
#else
#define QT_TSTAT		::_wstat
#endif
#endif
#define QT_READ			::_read
#define QT_WRITE		::_write
#define QT_ACCESS		::_access
#define QT_GETCWD		::_getcwd
#define QT_CHDIR		::_chdir
#define QT_MKDIR		::_mkdir
#define QT_RMDIR		::_rmdir
#define QT_OPEN_LARGEFILE       O_LARGEFILE
#define QT_OPEN_RDONLY		_O_RDONLY
#define QT_OPEN_WRONLY		_O_WRONLY
#define QT_OPEN_RDWR		_O_RDWR
#define QT_OPEN_CREAT		_O_CREAT
#define QT_OPEN_TRUNC		_O_TRUNC
#define QT_OPEN_APPEND		_O_APPEND
#if defined(O_TEXT)
# define QT_OPEN_TEXT		_O_TEXT
# define QT_OPEN_BINARY		_O_BINARY
#endif

#define QT_FOPEN                ::fopen
#ifdef QT_LARGEFILE_SUPPORT
#define QT_FSEEK                ::fseeko64
#define QT_FTELL                ::ftello64
#else
#define QT_FSEEK                ::fseek
#define QT_FTELL                ::ftell
#endif
#define QT_FGETPOS              ::fgetpos
#define QT_FSETPOS              ::fsetpos
#define QT_FPOS_T               fpos_t
#ifdef QT_LARGEFILE_SUPPORT
#define QT_OFF_T                off64_t
#else
#define QT_OFF_T                long
#endif

#define QT_SIGNAL_ARGS		int

#define QT_VSNPRINTF		::_vsnprintf
#define QT_SNPRINTF		::_snprintf

# define F_OK	0
# define X_OK	1
# define W_OK	2
# define R_OK	4


#endif // QPLATFORMDEFS_H
