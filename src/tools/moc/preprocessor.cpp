/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "preprocessor.h"
#include "utils.h"
#include "keywords.cpp"
#include "ppkeywords.cpp"
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QFileInfo>

bool Preprocessor::preprocessOnly = false;
void Preprocessor::skipUntilEndif()
{
    while(index < symbols.size() - 1 && symbols.at(index).token != PP_ENDIF){
        switch (symbols.at(index).token) {
        case PP_IF:
        case PP_IFDEF:
        case PP_IFNDEF:
            ++index;
            skipUntilEndif();
            break;
        default:
            ;
        }
        ++index;
    }
}

bool Preprocessor::skipBranch()
{
    while (index < symbols.size() - 1
          && (symbols.at(index).token != PP_ENDIF
               && symbols.at(index).token != PP_ELIF
               && symbols.at(index).token != PP_ELSE)
       ){
        switch (symbols.at(index).token) {
        case PP_IF:
        case PP_IFDEF:
        case PP_IFNDEF:
            ++index;
            skipUntilEndif();
            break;
        default:
            ;
        }
        ++index;
    }
    return (index < symbols.size() - 1);
}


enum TokenizeMode { TokenizeCpp, TokenizePreprocessor, PreparePreprocessorStatement, TokenizePreprocessorStatement, TokenizeInclude };
static Symbols tokenize(const QByteArray &input, int lineNum = 1, TokenizeMode mode = TokenizeCpp)
{
    Symbols symbols;
    const char *begin = input;
    const char *data = begin;
    while (*data) {
        if (mode == TokenizeCpp) {
            int column = 0;

            const char *lexem = data;
            int state = 0;
            Token token = NOTOKEN;
            for (;;) {
                if (static_cast<signed char>(*data) < 0) {
                    ++data;
                    continue;
                }
                int nextindex = keywords[state].next;
                int next = 0;
                if (*data == keywords[state].defchar)
                    next = keywords[state].defnext;
                else if (!state || nextindex)
                    next = keyword_trans[nextindex][(int)*data];
                if (!next)
                    break;
                state = next;
                token = keywords[state].token;
                ++data;
            }

            // suboptimal, is_ident_char  should use a table
            if (keywords[state].ident && is_ident_char(*data))
                token = keywords[state].ident;

            if (token == NOTOKEN) {
                // an error really
                ++data;
                continue;
            }

            ++column;

            if (token > SPECIAL_TREATMENT_MARK) {
                switch (token) {
                case QUOTE:
                    data = skipQuote(data);
                    token = STRING_LITERAL;
                    break;
                case SINGLEQUOTE:
                    while (*data && (*data != '\''
                                     || (*(data-1)=='\\'
                                         && *(data-2)!='\\')))
                        ++data;
                    if (*data)
                        ++data;
                    token = CHARACTER_LITERAL;
                    break;
                case DIGIT:
                    while (is_digit_char(*data))
                        ++data;
                    if (!*data || *data != '.') {
                        token = INTEGER_LITERAL;
                        if (data - lexem == 1 &&
                            (*data == 'x' || *data == 'X')
                            && *lexem == '0') {
                            ++data;
                            while (is_hex_char(*data))
                                ++data;
                        }
                        break;
                    }
                    token = FLOATING_LITERAL;
                    ++data;
                    // fall through
                case FLOATING_LITERAL:
                    while (is_digit_char(*data))
                        ++data;
                    if (*data == '+' || *data == '-')
                        ++data;
                    if (*data == 'e' || *data == 'E') {
                        ++data;
                        while (is_digit_char(*data))
                            ++data;
                    }
                    if (*data == 'f' || *data == 'F'
                        || *data == 'l' || *data == 'L')
                        ++data;
                    break;
                case HASH:
                    if (column == 1) {
                        mode = PreparePreprocessorStatement;
                        while (*data && (*data == ' ' || *data == '\t'))
                            ++data;
                        if (is_ident_char(*data))
                            mode = TokenizePreprocessorStatement;
                        continue;
                    }
                    break;
                case NEWLINE:
                    ++lineNum;
                    continue;
                case BACKSLASH:
                {
                    const char *rewind = data;
                    while (*data && (*data == ' ' || *data == '\t'))
                        ++data;
                    if (*data && *data == '\n') {
                        ++data;
                        continue;
                    }
                    data = rewind;
                } break;
                case CHARACTER:
                    while (is_ident_char(*data))
                        ++data;
                    token = IDENTIFIER;
                    break;
                case C_COMMENT:
                    while (*data && (*(data-1) != '/' || *(data-2) != '*')) {
                        if (*data == '\n')
                            ++lineNum;
                        ++data;
                    }
                    token = WHITESPACE; // one comment, one whitespace
                    // fall through;
                case WHITESPACE:
                    if (column == 1)
                        column = 0;
                    while (*data && (*data == ' ' || *data == '\t'))
                        ++data;
                    if (Preprocessor::preprocessOnly) // tokenize whitespace
                        break;
                    continue;
                case CPP_COMMENT:
                    while (*data && *data != '\n')
                        ++data;
                    continue; // ignore safly, the newline is a seperator
                default:
                    continue; //ignore
                }
            }
#ifdef USE_LEXEM_STORE
            if (!Preprocessor::preprocessOnly
                && token != IDENTIFIER
                && token != STRING_LITERAL
                && token != FLOATING_LITERAL
                && token != INTEGER_LITERAL)
                symbols += Symbol(lineNum, token);
            else
#endif
                symbols += Symbol(lineNum, token, input, lexem-begin, data-lexem);

        } else { //   Preprocessor

            const char *lexem = data;
            int state = 0;
            Token token = NOTOKEN;
            if (mode == TokenizePreprocessorStatement) {
                state = pp_keyword_trans[0][(int)'#'];
                mode = TokenizePreprocessor;
            }
            for (;;) {
                if (static_cast<signed char>(*data) < 0) {
                    ++data;
                    continue;
                }

                int nextindex = pp_keywords[state].next;
                int next = 0;
                if (*data == pp_keywords[state].defchar)
                    next = pp_keywords[state].defnext;
                else if (!state || nextindex)
                    next = pp_keyword_trans[nextindex][(int)*data];
                if (!next)
                    break;
                state = next;
                token = pp_keywords[state].token;
                ++data;
            }
            // suboptimal, is_ident_char  should use a table
            if (pp_keywords[state].ident && is_ident_char(*data))
                token = pp_keywords[state].ident;

            switch (token) {
            case NOTOKEN:
                ++data;
                break;
            case PP_IFDEF:
                symbols += Symbol(lineNum, PP_IF);
                symbols += Symbol(lineNum, PP_DEFINED);
                continue;
            case PP_IFNDEF:
                symbols += Symbol(lineNum, PP_IF);
                symbols += Symbol(lineNum, PP_NOT);
                symbols += Symbol(lineNum, PP_DEFINED);
                continue;
            case PP_INCLUDE:
                mode = TokenizeInclude;
                break;
            case PP_QUOTE:
                data = skipQuote(data);
                token = PP_STRING_LITERAL;
                break;
            case PP_SINGLEQUOTE:
                while (*data && (*data != '\''
                                 || (*(data-1)=='\\'
                                     && *(data-2)!='\\')))
                    ++data;
                if (*data)
                    ++data;
                token = PP_CHARACTER_LITERAL;
                break;
            case PP_DIGIT:
                while (is_digit_char(*data))
                    ++data;
                if (!*data || *data != '.') {
                    token = PP_INTEGER_LITERAL;
                    if (data - lexem == 1 &&
                        (*data == 'x' || *data == 'X')
                        && *lexem == '0') {
                        ++data;
                        while (is_hex_char(*data))
                            ++data;
                    }
                    break;
                }
                token = PP_FLOATING_LITERAL;
                ++data;
                // fall through
            case PP_FLOATING_LITERAL:
                while (is_digit_char(*data))
                    ++data;
                if (*data == '+' || *data == '-')
                    ++data;
                if (*data == 'e' || *data == 'E') {
                    ++data;
                    while (is_digit_char(*data))
                        ++data;
                }
                if (*data == 'f' || *data == 'F'
                    || *data == 'l' || *data == 'L')
                    ++data;
                break;
            case PP_CHARACTER:
                if (mode == PreparePreprocessorStatement) {
                    // rewind entire token to begin
                    data = lexem;
                    mode = TokenizePreprocessorStatement;
                    continue;
                }
                while (is_ident_char(*data))
                    ++data;
                token = PP_IDENTIFIER;
                break;
            case PP_C_COMMENT:
                while (*data && (*(data-1) != '/' || *(data-2) != '*')) {
                    if (*data == '\n')
                        ++lineNum;
                    ++data;
                }
                token = PP_WHITESPACE; // one comment, one whitespace
                // fall through;
            case PP_WHITESPACE:
                while (*data && (*data == ' ' || *data == '\t'))
                    ++data;
                continue; // the preprocessor needs no whitespace
            case PP_CPP_COMMENT:
                while (*data && *data != '\n')
                    ++data;
                continue; // ignore safly, the newline is a seperator
            case PP_NEWLINE:
                ++lineNum;
                mode = TokenizeCpp;
                break;
            case PP_BACKSLASH:
            {
                const char *rewind = data;
                while (*data && (*data == ' ' || *data == '\t'))
                    ++data;
                if (*data && *data == '\n') {
                    ++data;
                    continue;
                }
                data = rewind;
            } break;
            case PP_LANGLE:
                if (mode != TokenizeInclude)
                    break;
                token = PP_STRING_LITERAL;
                while (*data && *data != '\n' && *(data-1) != '>')
                    ++data;
                break;
            default:
                break;
            }
            if (mode == PreparePreprocessorStatement)
                continue;
#ifdef USE_LEXEM_STORE
            if (token != PP_IDENTIFIER
                && token != PP_STRING_LITERAL
                && token != PP_FLOATING_LITERAL
                && token != PP_INTEGER_LITERAL)
                symbols += Symbol(lineNum, token);
            else
#endif
                symbols += Symbol(lineNum, token, input, lexem-begin, data-lexem);
        }
    }
    symbols += Symbol(); // eof symbol
    return symbols;
}

void Preprocessor::substituteMacro(const MacroName &macro, Symbols &substituted, MacroSafeSet safeset)
{
    Symbols saveSymbols = symbols;
    int saveIndex = index;

    symbols = macros.value(macro).symbols;
    index = 0;

    safeset += macro;
    substituteUntilNewline(substituted, safeset);

    symbols = saveSymbols;
    index = saveIndex;
}



void Preprocessor::substituteUntilNewline(Symbols &substituted, MacroSafeSet safeset)
{
    while (hasNext()) {
        Token token = next();
        if (token == PP_IDENTIFIER) {
            MacroName macro = symbol();
            if (macros.contains(macro) && !safeset.contains(macro)) {
                substituteMacro(macro, substituted, safeset);
                continue;
            }
        } else if (token == PP_DEFINED) {
            test(PP_LPAREN);
            next(PP_IDENTIFIER);
            Symbol definedOrNotDefined = symbol();
            definedOrNotDefined.token = macros.contains(definedOrNotDefined)? PP_MOC_TRUE : PP_MOC_FALSE;
            substituted += definedOrNotDefined;
            test(PP_RPAREN);
            continue;
        } else if (token == PP_NEWLINE) {
            substituted += symbol();
            break;
        }
        substituted += symbol();
    }
}


class PP_Expression : public Parser
{
public:
    int value() { index = 0; return unary_expression_lookup() ?  conditional_expression() : 0; }

    int conditional_expression();
    int logical_OR_expression();
    int logical_AND_expression();
    int inclusive_OR_expression();
    int exclusive_OR_expression();
    int AND_expression();
    int equality_expression();
    int relational_expression();
    int shift_expression();
    int additive_expression();
    int multiplicative_expression();
    int unary_expression();
    bool unary_expression_lookup();
    int primary_expression();
    bool primary_expression_lookup();
};

int PP_Expression::conditional_expression()
{
    int value = logical_OR_expression();
    if (test(PP_QUESTION)) {
        int alt1 = conditional_expression();
        int alt2 = test(PP_COLON) ? conditional_expression() : 0;
        return value ? alt1 : alt2;
    }
    return value;
}

int PP_Expression::logical_OR_expression()
{
    int value = logical_AND_expression();
    if (test(PP_OROR))
        return logical_OR_expression() || value;
    return value;
}

int PP_Expression::logical_AND_expression()
{
    int value = inclusive_OR_expression();
    if (test(PP_ANDAND))
        return logical_AND_expression() && value;
    return value;
}

int PP_Expression::inclusive_OR_expression()
{
    int value = exclusive_OR_expression();
    if (test(PP_OR))
        return value | inclusive_OR_expression();
    return value;
}

int PP_Expression::exclusive_OR_expression()
{
    int value = AND_expression();
    if (test(PP_HAT))
        return value ^ exclusive_OR_expression();
    return value;
}

int PP_Expression::AND_expression()
{
    int value = equality_expression();
    if (test(PP_AND))
        return value & AND_expression();
    return value;
}

int PP_Expression::equality_expression()
{
    int value = relational_expression();
    switch (next()) {
    case PP_EQEQ:
        return value == equality_expression();
    case PP_NE:
        return value != equality_expression();
    default:
        prev();
        return value;
    }
}

int PP_Expression::relational_expression()
{
    int value = shift_expression();
    switch (next()) {
    case PP_LANGLE:
        return value < relational_expression();
    case PP_RANGLE:
        return value > relational_expression();
    case PP_LE:
        return value <= relational_expression();
    case PP_GE:
        return value >= relational_expression();
    default:
        prev();
        return value;
    }
}

int PP_Expression::shift_expression()
{
    int value = additive_expression();
    switch (next()) {
    case PP_LTLT:
        return value << shift_expression();
    case PP_GTGT:
        return value >> shift_expression();
    default:
        prev();
        return value;
    }
}

int PP_Expression::additive_expression()
{
    int value = multiplicative_expression();
    switch (next()) {
    case PP_PLUS:
        return value + additive_expression();
    case PP_MINUS:
        return value - additive_expression();
    default:
        prev();
        return value;
    }
}

int PP_Expression::multiplicative_expression()
{
    int value = unary_expression();
    switch (next()) {
    case PP_STAR:
        return value * multiplicative_expression();
    case PP_PERCENT:
    {
        int remainder = multiplicative_expression();
        return remainder ? value % remainder : 0;
    }
    case PP_SLASH:
    {
        int div = multiplicative_expression();
        return div ? value / div : 0;
    }
    default:
        prev();
        return value;
    };
}

int PP_Expression::unary_expression()
{
    switch (next()) {
    case PP_PLUS:
        return unary_expression();
    case PP_MINUS:
        return -unary_expression();
    case PP_NOT:
        return !unary_expression();
    case PP_TILDE:
        return ~unary_expression();
    case PP_MOC_TRUE:
        return 1;
    case PP_MOC_FALSE:
        return 0;
    default:
        prev();
        return primary_expression();
    }
}

bool PP_Expression::unary_expression_lookup()
{
    Token t = lookup();
    return (primary_expression_lookup()
            || t == PP_PLUS
            || t == PP_MINUS
            || t == PP_NOT
            || t == PP_TILDE
            || t == PP_DEFINED);
}

int PP_Expression::primary_expression()
{
    int value;
    if (test(PP_LPAREN)) {
        value = conditional_expression();
        test(PP_RPAREN);
    } else {
        next();
        value = QString(lexem()).toInt(0, 0);
    }
    return value;
}

bool PP_Expression::primary_expression_lookup()
{
    Token t = lookup();
    return (t == PP_IDENTIFIER
            || t == PP_INTEGER_LITERAL
            || t == PP_FLOATING_LITERAL
            || t == PP_MOC_TRUE
            || t == PP_MOC_FALSE
            || t == PP_LPAREN);
}

int Preprocessor::evaluateCondition()
{
    PP_Expression expression;
    expression.currentFilenames = currentFilenames;

    substituteUntilNewline(expression.symbols);

    return expression.value();
}

void Preprocessor::preprocess(const QByteArray &filename, Symbols &preprocessed)
{
    currentFilenames.push(filename);
    preprocessed.reserve(preprocessed.size() + symbols.size());
    while (hasNext()) {
        Token token = next();

        switch (token) {
        case PP_INCLUDE:
        {
            int lineNum = symbol().lineNum;
            QByteArray include;
            bool local = false;
            if (test(PP_STRING_LITERAL)) {
                local = lexem().startsWith('\"');
                include = unquotedLexem();
            } else
                continue;
            until(PP_NEWLINE);

            // #### stringery
            QFileInfo fi;
            if (local)
                fi.setFile(QFileInfo(QString::fromLocal8Bit(filename)).dir(), QString::fromLocal8Bit(include));
            for (int j = 0; j < Preprocessor::includes.size() && !fi.exists(); ++j)
                fi.setFile(QString::fromLocal8Bit(Preprocessor::includes.at(j)), QString::fromLocal8Bit(include));
            if (!fi.exists())
                continue;
            include = fi.filePath().toLocal8Bit();

            if (Preprocessor::preprocessedIncludes.contains(include))
                continue;
            Preprocessor::preprocessedIncludes.insert(include);

            QFile file(QString::fromLocal8Bit(include));
            if (!file.open(QFile::ReadOnly|QFile::Text))
                continue;

            QByteArray input = file.readAll();
            file.close();
            if (input.isEmpty())
                continue;

            Symbols saveSymbols = symbols;
            int saveIndex = index;

            // phase 1: tokenize for the preprocessor
            symbols = tokenize(input);
            input.clear();

            index = 0;

            // phase 2: preprocess conditions and substitute macros
            preprocessed += Symbol(0, MOC_INCLUDE_BEGIN, include);
            preprocess(include, preprocessed);
            preprocessed += Symbol(lineNum, MOC_INCLUDE_END, include);

            symbols = saveSymbols;
            index = saveIndex;
            continue;
        }
        case PP_DEFINE:
        {
            next(IDENTIFIER);
            QByteArray name = lexem();
            int start = index;
            until(PP_NEWLINE);
            Macro macro;
            macro.symbols.reserve(index - start - 1);
            for (int i = start; i < index - 1; ++i)
                macro.symbols += symbols.at(i);
            macros.insert(name, macro);
            continue;
        }
        case PP_UNDEF: {
            next(IDENTIFIER);
            QByteArray name = lexem();
            until(PP_NEWLINE);
            macros.remove(name);
            continue;
        }
        case PP_IDENTIFIER:
        {
//             if (macros.contains(symbol()))
//                 ;
        }
            // we _could_ easily substitute macros by the following
            // four lines, but we choose not to.
            /*
            if (macros.contains(sym.lexem())) {
                preprocessed += substitute(macros, symbols, i);
                continue;
            }
            */
            break;
        case PP_HASH:
            until(PP_NEWLINE);
            continue; // skip unknown preprocessor statement
        case PP_IFDEF:
        case PP_IFNDEF:
        case PP_IF:
            while (!evaluateCondition()) {
                if (!skipBranch())
                    break;
                if (test(PP_ELIF)) {
                } else {
                    until(PP_NEWLINE);
                    break;
                }
            }
            continue;
        case PP_ELIF:
        case PP_ELSE:
            skipUntilEndif();
            // fall through
        case PP_ENDIF:
            until(PP_NEWLINE);
            continue;
        case SIGNALS:
        case SLOTS: {
            Symbol sym = symbol();
            if (macros.contains("QT_NO_KEYWORDS"))
                sym.token = IDENTIFIER;
            else
                sym.token = (token == SIGNALS ? Q_SIGNALS_TOKEN : Q_SLOTS_TOKEN);
            preprocessed += sym;
        } continue;
        default:
            break;
        }
        preprocessed += symbol();
    }

    currentFilenames.pop();
}

Symbols Preprocessor::preprocessed(const QByteArray &filename, FILE *file)
{
    QFile qfile;
    qfile.open(file, QFile::ReadOnly|QFile::Text);
    QByteArray input = qfile.readAll();
    if (input.isEmpty())
        return symbols;

    // phase 1: tokenize for the preprocessor
    symbols = tokenize(input);

#if 0
    for (int j = 0; j < symbols.size(); ++j)
        fprintf(stderr, "line %d: %s(%s)\n",
               symbols[j].lineNum,
               symbols[j].lexem().constData(),
               tokenTypeName(symbols[j].token));
#endif

    // phase 2: preprocess conditions and substitute macros
    Symbols result;
    preprocess(filename, result);

#if 0
    for (int j = 0; j < result.size(); ++j)
        fprintf(stderr, "line %d: %s(%s)\n",
               result[j].lineNum,
               result[j].lexem().constData(),
               tokenTypeName(result[j].token));
#endif

    return result;
}

void Preprocessor::until(Token t)
{
    while(hasNext() && next() != t)
        ;
}

