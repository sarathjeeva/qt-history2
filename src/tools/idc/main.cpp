/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QFile>
#include <QSysInfo>
#include <QProcess>
#include <QLibraryInfo>
#include <qt_windows.h>
#include <io.h>

QT_BEGIN_NAMESPACE

static QString quotePath(const QString &s)
{
    if (!s.startsWith(QLatin1String("\"")) && s.contains(QLatin1Char(' ')))
        return QLatin1String("\"") + s + QLatin1String("\"");
    return s;
}


static bool runWithQtInEnvironment(const QString &cmd)
{
    QProcess proc;
    
    // prepend the qt binary directory to the path
    QStringList env = QProcess::systemEnvironment();
    for (int i=0; i<env.count(); ++i) {
        QString var = env.at(i);
        int setidx = var.indexOf(QLatin1Char('='));
        if (setidx != -1) {
            QString varname = var.left(setidx).trimmed().toUpper();
            if (varname == QLatin1String("PATH")) {
                var = var.mid(setidx + 1);
                var = QLatin1String("PATH=") + 
                    QLibraryInfo::location(QLibraryInfo::BinariesPath) +
                    QLatin1Char(';') + var;
                env[i] = var;
                break;
            }
        }
    }

    proc.setEnvironment(env);
    proc.start(cmd);
    proc.waitForFinished(-1);
    
    return (proc.exitCode() == 0);
}

static bool attachTypeLibrary(const QString &applicationName, int resource, const QByteArray &data, QString *errorMessage)
{
    HANDLE hExe = 0;
    QT_WA({
        TCHAR *resourceName = MAKEINTRESOURCEW(resource);
        hExe = BeginUpdateResourceW((TCHAR*)applicationName.utf16(), false);
        if (hExe == 0) {
            if (errorMessage)
                *errorMessage = QString::fromLatin1("Failed to attach type library to binary %1 - could not open file.").arg(applicationName);
            return false;
        }
        if (!UpdateResourceW(hExe,L"TYPELIB",resourceName,0,(void*)data.data(),data.count())) {
            EndUpdateResource(hExe, true);
            if (errorMessage)
                *errorMessage = QString::fromLatin1("Failed to attach type library to binary %1 - could not update file.").arg(applicationName);
            return false;
        }
    }, {
        char *resourceName = MAKEINTRESOURCEA(resource);
        hExe = BeginUpdateResourceA(applicationName.toLocal8Bit(), false);
        if (hExe == 0) {
            if (errorMessage)
                *errorMessage = QString::fromLatin1("Failed to attach type library to binary %1 - could not open file.").arg(applicationName);
            return false;
        }
        if (!UpdateResourceA(hExe,"TYPELIB",resourceName,0,(void*)data.data(),data.count())) {
            EndUpdateResource(hExe, true);
            if (errorMessage)
                *errorMessage = QString::fromLatin1("Failed to attach type library to binary %1 - could not update file.").arg(applicationName);
            return false;
        }
    });
    
    if (!EndUpdateResource(hExe,false)) {
        if (errorMessage)
            *errorMessage = QString::fromLatin1("Failed to attach type library to binary %1 - could not write file.").arg(applicationName);
        return false;
    }
    
    if (errorMessage)
        *errorMessage = QString::fromLatin1("Type library attached to %1.").arg(applicationName);
    return true;
}

static bool registerServer(const QString &input)
{
    bool ok = false;    
    if (input.endsWith(QLatin1String(".exe"))) {
        ok = runWithQtInEnvironment(quotePath(input) + QLatin1String(" -regserver"));
    } else {
        HMODULE hdll = 0;
        QT_WA({
            hdll = LoadLibraryW((TCHAR*)input.utf16());
        }, {
            hdll = LoadLibraryA(input.toLocal8Bit());
        });
        if (!hdll) {
            fprintf(stderr, "Couldn't load library file %s\n", (const char*)input.toLocal8Bit().data());
            return false;
        }
        typedef HRESULT(__stdcall* RegServerProc)();
        RegServerProc DllRegisterServer = (RegServerProc)GetProcAddress(hdll, "DllRegisterServer");
        if (!DllRegisterServer) {
            fprintf(stderr, "Library file %s doesn't appear to be a COM library\n", (const char*)input.toLocal8Bit().data());
            return false;
        }
        ok = DllRegisterServer() == S_OK;
    }
    return ok;
}

static bool unregisterServer(const QString &input)
{
    bool ok = false;
    if (input.endsWith(QLatin1String(".exe"))) {        
        ok = runWithQtInEnvironment(quotePath(input) + QLatin1String(" -unregserver"));
    } else {
        HMODULE hdll = 0;
        QT_WA({
            hdll = LoadLibraryW((TCHAR*)input.utf16());
        }, {
            hdll = LoadLibraryA(input.toLocal8Bit());
        });
        if (!hdll) {
            fprintf(stderr, "Couldn't load library file %s\n", (const char*)input.toLocal8Bit().data());
            return false;
        }
        typedef HRESULT(__stdcall* RegServerProc)();
        RegServerProc DllUnregisterServer = (RegServerProc)GetProcAddress(hdll, "DllUnregisterServer");
        if (!DllUnregisterServer) {
            fprintf(stderr, "Library file %s doesn't appear to be a COM library\n", (const char*)input.toLocal8Bit().data());
            return false;
        }
        ok = DllUnregisterServer() == S_OK;
    }
    return ok;
}

static HRESULT dumpIdl(const QString &input, const QString &idlfile, const QString &version)
{
    HRESULT res = E_FAIL;
    
    if (input.endsWith(QLatin1String(".exe"))) {
        if (runWithQtInEnvironment(quotePath(input) + QLatin1String(" -dumpidl ") + idlfile + QLatin1String(" -version ") + version))
            res = S_OK;
    } else {
        HMODULE hdll = 0;
        QT_WA({
            hdll = LoadLibraryW((TCHAR*)input.utf16());
        }, {
            hdll = LoadLibraryA(input.toLocal8Bit());
        });
        if (!hdll) {
            fprintf(stderr, "Couldn't load library file %s\n", (const char*)input.toLocal8Bit().data());
            return 3;
        }
        typedef HRESULT(__stdcall* DumpIDLProc)(const QString&, const QString&);
        DumpIDLProc DumpIDL = (DumpIDLProc)GetProcAddress(hdll, "DumpIDL");
        if (!DumpIDL) {
            fprintf(stderr, "Couldn't resolve 'DumpIDL' symbol in %s\n", (const char*)input.toLocal8Bit().data());
            return 3;
        }
        res = DumpIDL(idlfile, version);
        FreeLibrary(hdll);
    }
    
    return res;
}

static void slashify(QString &s)
{
    if (!s.contains(QLatin1Char('/')))
        return;
    
    int i = 0;
    while (i < (int)s.length()) {
        if (s[i] == QLatin1Char('/'))
            s[i] = QLatin1Char('\\');
        ++i;
    }
}

int runIdc(int argc, char **argv)
{
    QString error;
    QString tlbfile;
    QString idlfile;
    QString input;
    QString version = QLatin1String("1.0");
    
    int i = 1;
    while (i < argc) {
        QString p = QString::fromLocal8Bit(argv[i]).toLower();
        
        if (p == QLatin1String("/idl") || p == QLatin1String("-idl")) {
            ++i;
            if (i > argc) {
                error = QLatin1String("Missing name for interface definition file!");
                break;
            }
            idlfile = QLatin1String(argv[i]);
            idlfile = idlfile.trimmed().toLower();            
        } else if (p == QLatin1String("/version") || p == QLatin1String("-version")) {
            ++i;
            if (i > argc)
                version = QLatin1String("1.0");
            else
                version = QLatin1String(argv[i]);
        } else if (p == QLatin1String("/tlb") || p == QLatin1String("-tlb")) {
            if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
                fprintf(stderr, "IDC requires Windows NT/2000/XP!\n");
            
            ++i;
            if (i > argc) {
                error = QLatin1String("Missing name for type library file!");
                break;
            }
            tlbfile = QLatin1String(argv[i]);
            tlbfile = tlbfile.trimmed().toLower();            
        } else if (p == QLatin1String("/v") || p == QLatin1String("-v")) {
            fprintf(stdout, "Qt Interface Definition Compiler version 1.0\n");
            return 0;
        } else if (p == QLatin1String("/regserver") || p == QLatin1String("-regserver")) {
            if (!registerServer(input)) {
                fprintf(stderr, "Failed to register server!\n");
                return 1;
            }
            fprintf(stderr, "Server registered successfully!\n");
            return 0;
        } else if (p == QLatin1String("/unregserver") || p == QLatin1String("-unregserver")) {
            if (!unregisterServer(input)) {
                fprintf(stderr, "Failed to unregister server!\n");
                return 1;
            }
            fprintf(stderr, "Server unregistered successfully!\n");
            return 0;
        } else if (p[0] == QLatin1Char('/') || p[0] == QLatin1Char('-')) {
            error = QLatin1String("Unknown option \"") + p + QLatin1String("\"");
            break;
        } else {
            input = QLatin1String(argv[i]);
            input = input.trimmed().toLower();            
        }
        i++;
    }
    if (!error.isEmpty()) {
        fprintf(stderr, "%s", error.toLatin1().data());
        fprintf(stderr, "\n");
        return 5;
    }
    if (input.isEmpty()) {
        fprintf(stderr, "No input file specified!\n");
        return 1;
    }
    if (input.endsWith(QLatin1String(".exe")) && tlbfile.isEmpty() && idlfile.isEmpty()) {
        fprintf(stderr, "No type output file specified!\n");
        return 2;
    }
    if (input.endsWith(QLatin1String(".dll")) && idlfile.isEmpty() && tlbfile.isEmpty()) {
        fprintf(stderr, "No interface definition file and no type library file specified!\n");
        return 3;
    }
    slashify(input);
    if (!tlbfile.isEmpty()) {
        slashify(tlbfile);
        QFile file(tlbfile);
        if (!file.open(QIODevice::ReadOnly)) {
            fprintf(stderr, "Couldn't open %s for read\n", (const char*)tlbfile.toLocal8Bit().data());
            return 4;
        }
        QByteArray data = file.readAll();
        QString error;
        bool ok = attachTypeLibrary(input, 1, data, &error);
        fprintf(stderr, "%s", error.toLatin1().data());
        fprintf(stderr, "\n");
        return ok ? 0 : 4;
    } else if (!idlfile.isEmpty()) {
        slashify(idlfile);
        idlfile = quotePath(idlfile);
        fprintf(stderr, "\n\n%s\n\n", (const char*)idlfile.toLocal8Bit().data());
        quotePath(input);
        HRESULT res = dumpIdl(input, idlfile, version);
        
        switch(res) {
        case S_OK:
            break;
        case -1:
            fprintf(stderr, "Couldn't open %s for writing!\n", (const char*)idlfile.toLocal8Bit().data());
            return res;
        case 1:
            fprintf(stderr, "Malformed appID value in %s!\n", (const char*)input.toLocal8Bit().data());
            return res;
        case 2:
            fprintf(stderr, "Malformed typeLibID value in %s!\n", (const char*)input.toLocal8Bit().data());
            return res;
        case 3:
            fprintf(stderr, "Class has no metaobject information (error in %s)!\n", (const char*)input.toLocal8Bit().data());
            return res;
        case 4:
            fprintf(stderr, "Malformed classID value in %s!\n", (const char*)input.toLocal8Bit().data());
            return res;
        case 5:
            fprintf(stderr, "Malformed interfaceID value in %s!\n", (const char*)input.toLocal8Bit().data());
            return res;
        case 6:
            fprintf(stderr, "Malformed eventsID value in %s!\n", (const char*)input.toLocal8Bit().data());
            return res;
            
        default:
            fprintf(stderr, "Unknown error writing IDL from %s\n", (const char*)input.toLocal8Bit().data());
            return 7;
        }
    }
    return 0;
}

QT_END_NAMESPACE

int main(int argc, char **argv)
{
    return QT_ADD_NAMESPACE(runIdc)(argc, argv);
}
