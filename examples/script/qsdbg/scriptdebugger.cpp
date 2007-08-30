#include "scriptdebugger.h"
#include "scriptdebugger_p.h"
#include "scriptbreakpointmanager.h"

#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContextInfo>
#include <QtScript/QScriptValueIterator>
#include <QtCore/QTextStream>

static QString safeValueToString(const QScriptValue &value)
{
    if (value.isObject())
        return QLatin1String("[object Object]");
    else
        return value.toString();
}

class ScriptInfo
{
public:
    ScriptInfo(const QString &code, const QString &fileName, int lineNumber)
        : m_code(code), m_fileName(fileName), m_lineNumber(lineNumber)
        { }

    inline QString code() const
        { return m_code; }
    inline QString fileName() const
        { return m_fileName; }
    inline int lineNumber() const
        { return m_lineNumber; }

    QString lineText(int lineNumber);
    QMap<int, int> m_lineOffsets;

private:
    int lineOffset(int lineNumber);

    QString m_code;
    QString m_fileName;
    int m_lineNumber;
};

int ScriptInfo::lineOffset(int lineNumber)
{
    QMap<int, int>::const_iterator it = m_lineOffsets.constFind(lineNumber);
    if (it != m_lineOffsets.constEnd())
        return it.value();

    int offset;
    it = m_lineOffsets.constFind(lineNumber - 1);
    if (it != m_lineOffsets.constEnd()) {
        offset = it.value();
        offset = m_code.indexOf(QLatin1Char('\n'), offset);
        if (offset != -1)
            ++offset;
        m_lineOffsets.insert(lineNumber, offset);
    } else {
        int index;
        it = m_lineOffsets.lowerBound(lineNumber);
        --it;
        if (it != m_lineOffsets.constBegin()) {
            index = it.key();
            offset = it.value();
        } else {
            index = m_lineNumber;
            offset = 0;
        }
        int j = index;
        for ( ; j < lineNumber; ++j) {
            m_lineOffsets.insert(j, offset);
            offset = m_code.indexOf(QLatin1Char('\n'), offset);
            if (offset == -1)
                break;
            ++offset;
        }
        m_lineOffsets.insert(j, offset);
    }
    return offset;
}

QString ScriptInfo::lineText(int lineNumber)
{
    int startOffset = lineOffset(lineNumber);
    if (startOffset == -1)
        return QString();
    int endOffset = lineOffset(lineNumber + 1);
    if (endOffset == -1)
        return m_code.mid(startOffset);
    else
        return m_code.mid(startOffset, endOffset - startOffset - 1);
}



ScriptDebuggerPrivate::ScriptDebuggerPrivate(QScriptEngine *engine)
    : QScriptEngineAgent(engine), m_mode(Run)
{
    m_bpManager = new ScriptBreakpointManager;
    m_defaultInputStream = new QTextStream(stdin);
    m_defaultOutputStream = new QTextStream(stdout);
    m_defaultErrorStream = new QTextStream(stderr);
    m_inputStream = m_defaultInputStream;
    m_outputStream = m_defaultOutputStream;
    m_errorStream = m_defaultErrorStream;
}

ScriptDebuggerPrivate::~ScriptDebuggerPrivate()
{
    delete m_defaultInputStream;
    delete m_defaultOutputStream;
    delete m_defaultErrorStream;
    delete m_bpManager;
    qDeleteAll(m_scripts);
}

QString ScriptDebuggerPrivate::readLine()
{
    return m_inputStream->readLine();
}

void ScriptDebuggerPrivate::output(const QString &text)
{
    *m_outputStream << text;
}

void ScriptDebuggerPrivate::message(const QString &text)
{
    *m_outputStream << text << endl;
    m_outputStream->flush();
}

void ScriptDebuggerPrivate::errorMessage(const QString &text)
{
    *m_errorStream << text << endl;
    m_errorStream->flush();
}

void ScriptDebuggerPrivate::setMode(Mode mode)
{
    m_mode = mode;
}

ScriptDebuggerPrivate::Mode ScriptDebuggerPrivate::mode() const
{
    return m_mode;
}

QScriptContext *ScriptDebuggerPrivate::frameContext(int index) const
{
    QScriptContext *ctx = engine()->currentContext();
    for (int i = 0; i < index; ++i) {
        ctx = ctx->parentContext();
        if (!ctx)
            break;
    }
    return ctx;
}

int ScriptDebuggerPrivate::currentFrameIndex() const
{
    return m_currentFrameIndex;
}

void ScriptDebuggerPrivate::setCurrentFrameIndex(int index)
{
    m_currentFrameIndex = index;
    m_listLineNumber = -1;
}

int ScriptDebuggerPrivate::listLineNumber() const
{
    return m_listLineNumber;
}

void ScriptDebuggerPrivate::setListLineNumber(int lineNumber)
{
    m_listLineNumber = lineNumber;
}

QScriptContext *ScriptDebuggerPrivate::currentFrameContext() const
{
    return frameContext(currentFrameIndex());
}

int ScriptDebuggerPrivate::frameCount() const
{
    int count = 0;
    QScriptContext *ctx = engine()->currentContext();
    while (ctx) {
        ++count;
        ctx = ctx->parentContext();
    }
    return count;
}

ScriptInfo *ScriptDebuggerPrivate::scriptInfo(QScriptContext *context) const
{
    QStack<qint64> pids = m_contextProgramIds.value(context);
    if (pids.isEmpty())
        return 0;
    return m_scripts.value(pids.top());
}

QString ScriptDebuggerPrivate::contextString(QScriptContext *context) const
{
    QScriptContextInfo info(context);
    QString result;

    QString functionName = info.functionName();
    if (functionName.isEmpty()) {
        if (context->parentContext()) {
            if (info.functionType() == QScriptContextInfo::ScriptFunction)
                result.append(QLatin1String("<anonymous>"));
            else
                result.append(QLatin1String("<native>"));
        } else {
            result.append(QLatin1String("<global>"));
        }
    } else {
        result.append(functionName);
    }

    QStringList parameterNames = info.functionParameterNames();
    result.append(QLatin1String(" ("));
    for (int i = 0; i < context->argumentCount(); ++i) {
        if (i > 0)
            result.append(QLatin1String(", "));
        if (i < parameterNames.count()) {
            result.append(parameterNames.at(i));
            result.append(QLatin1Char('='));
        }
        QScriptValue arg = context->argument(i);
        result.append(safeValueToString(arg));
    }
    result.append(QLatin1String(")"));

    QString fileName = info.fileName();
    int lineNumber = info.lineNumber();
    result.append(QLatin1String(" at "));
    if (!fileName.isEmpty()) {
        result.append(fileName);
        result.append(QLatin1Char(':'));
    }
    result.append(QString::number(lineNumber));
    return result;
}

void ScriptDebuggerPrivate::interactive()
{
    setCurrentFrameIndex(0);

    QString qsdbgPrompt = QString::fromLatin1("(qsdbg) ");
    QString dotPrompt = QString::fromLatin1(".... ");
    QString prompt = qsdbgPrompt;

    QString code;

    forever {

         *m_outputStream << prompt;
        m_outputStream->flush();

        QString line = readLine();

        if (code.isEmpty() && (line.isEmpty() || line.startsWith(QLatin1Char('/')))) {
            if (line.isEmpty())
                line = m_lastInteractiveCommand;
            else
                m_lastInteractiveCommand = line;

            QStringList parts = line.split(QLatin1Char(' '), QString::SkipEmptyParts);
            if (!parts.isEmpty()) {
                QString command = parts.takeFirst().mid(1);
                if (executeCommand(command, parts))
                    break;
            }

        } else {
            if (line.isEmpty())
                continue;

            code += line;
            code += QLatin1Char('\n');

            if (line.trimmed().isEmpty()) {
                continue;

            } else if (! engine()->canEvaluate(code)) {
                prompt = dotPrompt;

            } else {
                setMode(Run);
                QScriptValue result = engine()->evaluate(code, QLatin1String("typein"));

                code.clear();
                prompt = qsdbgPrompt;

                if (! result.isUndefined()) {
                    errorMessage(result.toString());
                    engine()->clearExceptions();
                }
            }
        }
    }
}

bool ScriptDebuggerPrivate::executeCommand(const QString &command, const QStringList &args)
{
    if (command == QLatin1String("c")
        || command == QLatin1String("continue")) {
        setMode(Run);
        return true;
    } else if (command == QLatin1String("s")
               || command == QLatin1String("step")) {
        setMode(StepInto);
        return true;
    } else if (command == QLatin1String("n")
               || command == QLatin1String("next")) {
        setMode(StepOver);
        m_stepDepth = 0;
        return true;
    } else if (command == QLatin1String("f")
               || command == QLatin1String("frame")) {
        bool ok = false;
        int index = args.value(0).toInt(&ok);
        if (ok) {
            if (index < 0 || index >= frameCount()) {
                errorMessage("No such frame.");
            } else {
                setCurrentFrameIndex(index);
                QScriptContext *ctx = currentFrameContext();
                message(QString::fromLatin1("#%0  %1").arg(index).arg(contextString(ctx)));
            }
        }
    } else if (command == QLatin1String("bt")
               || command == QLatin1String("backtrace")) {
        QScriptContext *ctx = engine()->currentContext();
        int index = -1;
        while (ctx) {
            ++index;
            QString line = contextString(ctx);
            message(QString::fromLatin1("#%0  %1").arg(index).arg(line));
            ctx = ctx->parentContext();
        }
    } else if (command == QLatin1String("up")) {
        int index = currentFrameIndex() + 1;
        if (index == frameCount()) {
            errorMessage(QString::fromLatin1("Initial frame selected; you cannot go up."));
        } else {
            setCurrentFrameIndex(index);
            QScriptContext *ctx = currentFrameContext();
            message(QString::fromLatin1("#%0  %1").arg(index).arg(contextString(ctx)));
        }
    } else if (command == QLatin1String("down")) {
        int index = currentFrameIndex() - 1;
        if (index < 0) {
            errorMessage(QString::fromLatin1("Bottom (innermost) frame selected; you cannot go down."));
        } else {
            setCurrentFrameIndex(index);
            QScriptContext *ctx = currentFrameContext();
            message(QString::fromLatin1("#%0  %1").arg(index).arg(contextString(ctx)));
        }
    } else if (command == QLatin1String("b")
               || command == QLatin1String("break")) {
        QString str = args.value(0);
        int colonIndex = str.indexOf(QLatin1Char(':'));
        if (colonIndex != -1) {
            // filename:line form
            QString fileName = str.left(colonIndex);
            int lineNumber = str.mid(colonIndex+1).toInt();
            int id = m_bpManager->setBreakpoint(fileName, lineNumber);
            message(QString::fromLatin1("Breakpoint %0 at %1, line %2.").arg(id+1).arg(fileName).arg(lineNumber));
        } else {
            // function
            QScriptValue fun = engine()->globalObject().property(str);
            if (fun.isFunction()) {
                int id = m_bpManager->setBreakpoint(fun);
                message(QString::fromLatin1("Breakpoint %0 at %1().").arg(id+1).arg(str));
            }
        }
    } else if (command == QLatin1String("d")
               || command == QLatin1String("delete")) {
        int id = args.value(0).toInt() - 1;
        m_bpManager->removeBreakpoint(id);
    } else if (command == QLatin1String("disable")) {
        int id = args.value(0).toInt() - 1;
        m_bpManager->setBreakpointEnabled(id, false);
    } else if (command == QLatin1String("enable")) {
        int id = args.value(0).toInt() - 1;
        m_bpManager->setBreakpointEnabled(id, true);
    } else if (command == QLatin1String("list")) {
        QScriptContext *ctx = currentFrameContext();
        ScriptInfo *progInfo = scriptInfo(ctx);
        if (!progInfo) {
            errorMessage("No source text available for this frame.");
        } else {
            QScriptContextInfo ctxInfo(ctx);
            bool ok;
            int line = args.value(0).toInt(&ok);
            if (ok) {
                line = qMax(1, line - 5);
            } else {
                line = listLineNumber();
                if (line == -1)
                    line = qMax(progInfo->lineNumber(), ctxInfo.lineNumber() - 5);
            }
            for (int i = line; i < line + 10; ++i) {
                message(QString::fromLatin1("%0\t%1").arg(i).arg(progInfo->lineText(i)));
            }
            setListLineNumber(line + 10);
        }
    } else if (command == QLatin1String("info")) {
        if (args.size() < 1) {
        } else {
            QString what = args.value(0);
            if (what == QLatin1String("locals")) {
                QScriptValueIterator it(currentFrameContext()->activationObject());
                while (it.hasNext()) {
                    it.next();
                    QString line;
                    line.append(it.name());
                    line.append(QLatin1String(" = "));
                    line.append(safeValueToString(it.value()));
                    message(line);
                }
            }
        }
    } else if (command == QLatin1String("help")) {
        message("/continue - continue execution\n"
                "/step     - step into statement\n"
                "/next     - step over statement\n"
                "/list     - show where you are\n"
                "\n"
                "/break    - set breakpoint\n"
                "/delete   - remove breakpoint\n"
                "/disable  - disable breakpoint\n"
                "/enable   - enable breakpoint\n"
                "\n"
                "/backtrace - show backtrace\n"
                "/up       - one frame up\n"
                "/down     - one frame down\n"
                "/frame    - set frame\n"
                "\n"
                "/info locals - show local variables");
    } else {
        errorMessage(QString::fromLatin1("Undefined command \"%0\". Try \"/help\".")
                     .arg(command));
    }

    return false;
}


// QScriptEngineAgent interface

void ScriptDebuggerPrivate::scriptLoad(qint64 id, const QString &program,
                                       const QString &fileName, int lineNumber)
{
    ScriptInfo *info = new ScriptInfo(program, fileName, lineNumber);
    m_scripts.insert(id, info);
}

void ScriptDebuggerPrivate::scriptUnload(qint64 id)
{
    ScriptInfo *info = m_scripts.take(id);
    delete info;
}

void ScriptDebuggerPrivate::functionEntry(qint64 scriptId)
{
    if (scriptId != -1) {
        QScriptContext *ctx = engine()->currentContext();
        QStack<qint64> ids = m_contextProgramIds.value(ctx);
        ids.push(scriptId);
        m_contextProgramIds.insert(ctx, ids);
    }

    if (mode() == StepOver)
        ++m_stepDepth;
}

void ScriptDebuggerPrivate::functionExit(qint64 scriptId,
                                         const QScriptValue &/*returnValue*/)
{
    if (scriptId != -1) {
        QScriptContext *ctx = engine()->currentContext();
        QStack<qint64> ids = m_contextProgramIds.value(ctx);
        Q_ASSERT(!ids.isEmpty());
        Q_ASSERT(ids.top() == scriptId);
        ids.pop();
        m_contextProgramIds.insert(ctx, ids);
    }

    if (mode() == StepOver)
        --m_stepDepth;
}

void ScriptDebuggerPrivate::positionChange(qint64 scriptId,
                                           int lineNumber, int /*columnNumber*/)
{
    ScriptInfo *info = 0;
    bool enterInteractiveMode = false;

    if (m_bpManager->hasBreakpoints()) {
        // check if we hit a breakpoint
        info = m_scripts.value(scriptId);
        QScriptContext *ctx = engine()->currentContext();
        QScriptContextInfo ctxInfo(ctx);
        QScriptValue callee = ctx->callee();

        // try fileName:lineNumber
        int bpid = m_bpManager->findBreakpoint(info->fileName(), lineNumber);
        if ((bpid != -1) && m_bpManager->isBreakpointEnabled(bpid)) {
            message(QString::fromLatin1("Breakpoint %0 at %1:%2")
                    .arg(bpid + 1).arg(info->fileName()).arg(lineNumber));
            if (m_bpManager->isBreakpointSingleShot(bpid))
                m_bpManager->removeBreakpoint(bpid);
        }
        if (bpid == -1) {
            // try function
            bpid = m_bpManager->findBreakpoint(callee);
            if ((bpid != -1) && m_bpManager->isBreakpointEnabled(bpid)) {
                message(QString::fromLatin1("Breakpoint %0, %1()")
                        .arg(bpid + 1).arg(ctxInfo.functionName()));
                if (m_bpManager->isBreakpointSingleShot(bpid))
                    m_bpManager->removeBreakpoint(bpid);
            }
        }
        if ((bpid == -1) && !ctxInfo.functionName().isEmpty()) {
            // try functionName:fileName
            bpid = m_bpManager->findBreakpoint(ctxInfo.functionName(), ctxInfo.fileName());
            if ((bpid != -1) && m_bpManager->isBreakpointEnabled(bpid)) {
                message(QString::fromLatin1("Breakpoint %0, %1():%2").arg(bpid + 1)
                        .arg(ctxInfo.functionName()).arg(ctxInfo.fileName()));
                if (m_bpManager->isBreakpointSingleShot(bpid))
                    m_bpManager->removeBreakpoint(bpid);
            }
        }

        enterInteractiveMode = (bpid != -1);
    }

    switch (mode()) {
    case Run:
        break;

    case StepInto:
        enterInteractiveMode = true;
        break;

    case StepOver:
        enterInteractiveMode = enterInteractiveMode || (m_stepDepth <= 0);
        break;
    }

    if (enterInteractiveMode) {
        if (!info)
            info = m_scripts.value(scriptId);
        message(QString::fromLatin1("%0\t%1").arg(lineNumber).arg(info->lineText(lineNumber)));
        interactive();
    }
}

void ScriptDebuggerPrivate::exceptionThrow(qint64 /*scriptId*/,
                                           const QScriptValue &exception,
                                           bool hasHandler)
{
    if (!hasHandler) {
        errorMessage(QString::fromLatin1("uncaught exception: %0").arg(exception.toString()));
        interactive();
    }
}



ScriptDebugger::ScriptDebugger(QScriptEngine *engine)
    : d_ptr(new ScriptDebuggerPrivate(engine))
{
    d_ptr->q_ptr = this;
    engine->setAgent(d_ptr);
}

ScriptDebugger::ScriptDebugger(QScriptEngine *engine, ScriptDebuggerPrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    engine->setAgent(d_ptr);
}

ScriptDebugger::~ScriptDebugger()
{
    delete d_ptr;
    d_ptr = 0;
}

void ScriptDebugger::breakAtNextStatement()
{
    Q_D(ScriptDebugger);
    d->setMode(ScriptDebuggerPrivate::StepInto);
}

void ScriptDebugger::setBreakpoint(const QString &fileName, int lineNumber)
{
    Q_D(ScriptDebugger);
    d->m_bpManager->setBreakpoint(fileName, lineNumber);
}

void ScriptDebugger::setBreakpoint(const QString &functionName, const QString &fileName)
{
    Q_D(ScriptDebugger);
    d->m_bpManager->setBreakpoint(functionName, fileName);
}

void ScriptDebugger::setBreakpoint(const QScriptValue &function)
{
    Q_D(ScriptDebugger);
    d->m_bpManager->setBreakpoint(function);
}

QTextStream *ScriptDebugger::inputStream() const
{
    Q_D(const ScriptDebugger);
    return d->m_inputStream;
}

void ScriptDebugger::setInputStream(QTextStream *inputStream)
{
    Q_D(ScriptDebugger);
    d->m_inputStream = inputStream;
}

QTextStream *ScriptDebugger::outputStream() const
{
    Q_D(const ScriptDebugger);
    return d->m_outputStream;
}

void ScriptDebugger::setOutputStream(QTextStream *outputStream)
{
    Q_D(ScriptDebugger);
    d->m_outputStream = outputStream;
}

QTextStream *ScriptDebugger::errorStream() const
{
    Q_D(const ScriptDebugger);
    return d->m_errorStream;
}

void ScriptDebugger::setErrorStream(QTextStream *errorStream)
{
    Q_D(ScriptDebugger);
    d->m_errorStream = errorStream;
}
