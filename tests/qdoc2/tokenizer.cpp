/*
  tokenizer.cpp
*/

#include <qregexp.h>
#include <qstring.h>

#include <ctype.h>
#include <string.h>

#include "configuration.h"
#include "messages.h"
#include "tokenizer.h"

/* tmake ignore Q_OBJECT */

/*
  If you change this, make sure to change tokenizer.h as well.
*/
static const char kwords[][12] = {
    "char", "class", "const", "double", "enum", "inline", "int", "long",
    "operator", "private", "protected", "public", "short", "signals", "signed",
    "slots", "static", "struct", "template", "typedef", "union", "unsigned",
    "virtual", "void", "volatile", "Q_ENUMS", "Q_EXPORT", "Q_OBJECT",
    "Q_OVERRIDE", "Q_PROPERTY"
};

static const int KwordHashTableSize = 512;
static int kwordHashTable[KwordHashTableSize];
static bool kwordInitialized = FALSE;

/*
  This function is a perfect hash function for the 37 keywords of C99 (with a
  hash table size of 512).  It should perform well on our part of Qt-enhanced
  C++.
*/
static int hashKword( const char *s, int len )
{
    return ( ((uchar) s[0]) + (((uchar) s[2]) << 5) +
	     (((uchar) s[len - 1]) << 3) ) % KwordHashTableSize;
}

Tokenizer::~Tokenizer()
{
}

int Tokenizer::getToken()
{
    char *t = yyPrevLex;
    yyPrevLex = yyLex;
    yyLex = t;

    while ( yyCh != EOF ) {
	yyTokLoc = yyCurLoc;
	yyLexLen = 0;

	if ( isspace(yyCh) ) {
	    do {
		yyCh = getChar();
	    } while ( isspace(yyCh) );
	} else if ( isalpha(yyCh) || yyCh == '_' ) {
	    do {
		yyCh = getChar();
	    } while ( isalnum(yyCh) || yyCh == '_' );

	    int k = hashKword( yyLex, yyLexLen );
	    while ( TRUE ) {
		int i = kwordHashTable[k];
		if ( i == 0 )
		    return Tok_Ident;
		else if ( strcmp(yyLex, kwords[i - 1]) == 0 )
		    return (int) Tok_FirstKeyword + i - 1;
		if ( ++k == KwordHashTableSize )
		    k = 0;
	    }
	} else if ( isdigit(yyCh) ) {
	    do {
		yyCh = getChar();
	    } while ( isalnum(yyCh) || yyCh == '.' || yyCh == '+' ||
		      yyCh == '-' );
	    return Tok_Number;
	} else {
	    switch ( yyCh ) {
	    case '!':
	    case '%':
	    case '^':
		yyCh = getChar();
		if ( yyCh == '=' )
		    yyCh = getChar();
		return Tok_SomeOperator;
	    case '"':
		yyCh = getChar();

		while ( yyCh != EOF && yyCh != '"' ) {
		    if ( yyCh == '\\' )
			yyCh = getChar();
		    yyCh = getChar();
		}
		yyCh = getChar();

		if ( yyCh == EOF )
		    warning( 0, yyTokLoc, "Unterminated C++ string literal" );
		else
		    return Tok_String;
		break;
	    case '#':
		return getTokenAfterPreprocessor();
	    case '&':
		yyCh = getChar();
		if ( yyCh == '&' || yyCh == '=' ) {
		    yyCh = getChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_Ampersand;
		}
	    case '\'':
		yyCh = getChar();
		if ( yyCh == '\\' )
		    yyCh = getChar();
		do {
		    yyCh = getChar();
		} while ( yyCh != EOF && yyCh != '\'' );

		if ( yyCh == EOF ) {
		    warning( 0, yyTokLoc,
			     "Unterminated C++ character literal" );
		} else {
		    yyCh = getChar();
		    return Tok_Number;
		}
		break;
	    case '(':
		yyCh = getChar();
		yyParenDepth++;
		if ( yyCh == '*' ) {
		    yyCh = getChar();
		    return Tok_LeftParenAster;
		}
		return Tok_LeftParen;
	    case ')':
		yyCh = getChar();
		yyParenDepth--;
		return Tok_RightParen;
	    case '*':
		yyCh = getChar();
		if ( yyCh == '=' ) {
		    yyCh = getChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_Aster;
		}
	    case '+':
		yyCh = getChar();
		if ( yyCh == '+' || yyCh == '=' )
		    yyCh = getChar();
		return Tok_SomeOperator;
	    case ',':
		yyCh = getChar();
		return Tok_Comma;
	    case '-':
		yyCh = getChar();
		if ( yyCh == '-' || yyCh == '=' ) {
		    yyCh = getChar();
		} else if ( yyCh == '>' ) {
		    yyCh = getChar();
		    if ( yyCh == '*' )
			yyCh = getChar();
		}
		return Tok_SomeOperator;
	    case '.':
		yyCh = getChar();
		if ( yyCh == '*' ) {
		    yyCh = getChar();
		} else if ( yyCh == '.' ) {
		    do {
			yyCh = getChar();
		    } while ( yyCh == '.' );
		    return Tok_Ellipsis;
		} else if ( isdigit(yyCh) ) {
		    do {
			yyCh = getChar();
		    } while ( isalnum(yyCh) || yyCh == '.' || yyCh == '+' ||
			      yyCh == '-' );
		    return Tok_Number;
		}
		return Tok_SomeOperator;
	    case '/':
		yyCh = getChar();
		if ( yyCh == '/' ) {
		    do {
			yyCh = getChar();
		    } while ( yyCh != EOF && yyCh != '\n' );
		} else if ( yyCh == '*' ) {
		    bool metSlashAsterBang = FALSE;
		    bool metAster = FALSE;
		    bool metAsterSlash = FALSE;

		    yyCh = getChar();
		    if ( yyCh == '!' )
			metSlashAsterBang = TRUE;

		    while ( !metAsterSlash ) {
			if ( yyCh == EOF ) {
			    warning( 0, yyTokLoc, "Unterminated C++ comment" );
			    break;
			} else {
			    if ( yyCh == '*' )
				metAster = TRUE;
			    else if ( metAster && yyCh == '/' )
				metAsterSlash = TRUE;
			    else
				metAster = FALSE;
			}
			yyCh = getChar();
		    }
		    if ( metSlashAsterBang )
			return Tok_Doc;
		    else if ( yyParenDepth > 0 )
			return Tok_Comment;
		} else {
		    if ( yyCh == '=' )
			yyCh = getChar();
		    return Tok_SomeOperator;
		}
		break;
	    case ':':
		yyCh = getChar();
		if ( yyCh == ':' ) {
		    yyCh = getChar();
		    return Tok_Gulbrandsen;
		} else {
		    return Tok_Colon;
		}
	    case ';':
		yyCh = getChar();
		return Tok_Semicolon;
	    case '<':
		yyCh = getChar();
		if ( yyCh == '<' ) {
		    yyCh = getChar();
		    if ( yyCh == '=' )
			yyCh = getChar();
		    return Tok_SomeOperator;
		} else if ( yyCh == '=' ) {
		    yyCh = getChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_LeftAngle;
		}
	    case '=':
		yyCh = getChar();
		if ( yyCh == '=' ) {
		    yyCh = getChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_Equal;
		}
	    case '>':
		yyCh = getChar();
		if ( yyCh == '>' ) {
		    yyCh = getChar();
		    if ( yyCh == '=' )
			yyCh = getChar();
		    return Tok_SomeOperator;
		} else if ( yyCh == '=' ) {
		    yyCh = getChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_RightAngle;
		}
	    case '?':
		yyCh = getChar();
		return Tok_SomeOperator;
	    case '[':
		yyCh = getChar();
		yyBracketDepth++;
		return Tok_LeftBracket;
	    case ']':
		yyCh = getChar();
		yyBracketDepth--;
		return Tok_RightBracket;
	    case '{':
		yyCh = getChar();
		yyBraceDepth++;
		return Tok_LeftBrace;
	    case '}':
		yyCh = getChar();
		yyBraceDepth--;
		return Tok_RightBrace;
	    case '|':
		yyCh = getChar();
		if ( yyCh == '|' || yyCh == '=' )
		    yyCh = getChar();
		return Tok_SomeOperator;
	    case '~':
		yyCh = getChar();
		return Tok_Tilde;
	    default:		
		warning( 1, yyTokLoc, "Hostile character 0%.2o in C++ source",
			 yyCh );
		yyCh = getChar();
	    }
	}
    }

    if ( yyPreprocessorSkipping.count() > 1 )
	warning( 0, yyTokLoc, "Expected #endif before end of file" );

    strcpy( yyLex, "end-of-input" );
    yyLexLen = strlen( yyLex );
    return Tok_Eoi;
}

Tokenizer::Tokenizer()
{
    yyPrevLex = yyLexBuf1;
    yyPrevLex[0] = '\0';
    yyLex = yyLexBuf2;
    yyLex[0] = '\0';
    yyLexLen = 0;
    yyPreprocessorSkipping.push( FALSE );
    yyNumPreprocessorSkipping = 0;
    yyBraceDepth = 0;
    yyParenDepth = 0;
    yyBracketDepth = 0;
    yyCh = '\0';

    if ( !kwordInitialized ) {
	for ( int i = 0; i < Tok_LastKeyword - Tok_FirstKeyword + 1; i++ ) {
	    int k = hashKword( kwords[i], strlen(kwords[i]) );
	    while ( kwordHashTable[k] != 0 ) {
		if ( ++k == KwordHashTableSize )
		    k = 0;
	    }
	    kwordHashTable[k] = i + 1;
	}
	kwordInitialized = TRUE;
    }
}

int Tokenizer::getch()
{
    return EOF;
}

void Tokenizer::start( const Location& loc )
{
    yyTokLoc = loc;
    yyCurLoc = loc;
    strcpy( yyPrevLex, "beginning-of-input" );
    strcpy( yyLex, "beginning-of-input" );
    yyLexLen = strlen( yyLex );
    yyBraceDepth = 0;
    yyParenDepth = 0;
    yyBracketDepth = 0;
    yyCh = '\0';
    yyCh = getChar();
}

/*
  Returns the next token, if # was met.  This function interprets the
  preprocessor directive, skips over any #ifdef'd out tokens, and returns the
  token after all of that.
*/
int Tokenizer::getTokenAfterPreprocessor()
{
    static QRegExp *comment = 0;
    static QRegExp *versionX = 0;

    /*
      Build our regular expressions for matching C or C++ style comment, and for
      matching the symbol that gives the text for \version.
    */
    if ( comment == 0 ) {
	comment = new QRegExp( QString("/(?:\\*.*\\*/|/.*\n)") );
	comment->setMinimal( TRUE );
	versionX = new QRegExp;
	if ( !config->versionSymbol().isEmpty() )
	    versionX->setPattern( QString(
		    "[ \t]*(?:%1)[ \t]+\"([^\"]*)\"[ \t]*")
		    .arg(config->versionSymbol()) );
    }

    yyCh = getChar();
    while ( isspace(yyCh) && yyCh != '\n' )
	yyCh = getChar();

    /*
      #directive condition
    */
    QString directive;
    QString condition;

    while ( isalpha(yyCh) ) {
	directive += QChar( yyCh );
	yyCh = getChar();
    }
    if ( directive.isEmpty() )
	return getToken();

    while ( yyCh != EOF && yyCh != '\n' ) {
	if ( yyCh == '\\' )
	    yyCh = getChar();
	condition += yyCh;
	yyCh = getChar();
    }
    condition.replace( *comment, QString::null );
    condition = condition.simplifyWhiteSpace();

    /*
      The #if, #ifdef, #ifndef, #elif, #else, and #endif directives have an
      effect on the skipping stack.  For instance, if the code processed so far
      is

	  #if 1
	  #if 0
	  #if 1
	  // ...
	  #else

      the skipping stack contains, from bottom to top, FALSE TRUE TRUE (assuming
      0 is false and 1 is true).  If at least one entry of the stack is TRUE, the
      tokens are skipped.

      This mechanism is simple and unreadable.
    */
    if ( directive[0] == QChar('i') ) {
	if ( directive == QString("if") )
	    pushSkipping( !isTrue(condition) );
	else if ( directive == QString("ifdef") )
	    pushSkipping( !config->isDef(condition) );
	else if ( directive == QString("ifndef") )
	    pushSkipping( config->isDef(condition) );
    } else if ( directive[0] == QChar('e') ) {
	if ( directive == QString("elif") ) {
	    bool old = popSkipping();
	    if ( old )
		pushSkipping( !isTrue(condition) );
	    else
		pushSkipping( TRUE );
	} else if ( directive == QString("else") ) {
	    pushSkipping( !popSkipping() );
	} else if ( directive == QString("endif") ) {
	    popSkipping();
	}
    } else if ( directive == QString("define") ) {
	if ( versionX->match(condition) )
	    config->setVersion( versionX->cap(1) );
    }

    int tok;
    do {
	/*
	  This is subtle.  If getToken() meets another #, it will call
	  getTokenAfterPreprocessor() once again, which could in turn call
	  getToken() again, etc.
	*/
	tok = getToken();
    } while ( yyNumPreprocessorSkipping > 0 );
    return tok;
}

/*
  Pushes a new skipping value onto the stack.  This corresponds to entering a
  new #if block.
*/
void Tokenizer::pushSkipping( bool skip )
{
    yyPreprocessorSkipping.push( skip );
    if ( skip )
	yyNumPreprocessorSkipping++;
}

/*
  Pops a skipping value from the stack.  This corresponds to reaching a #endif.
*/
bool Tokenizer::popSkipping()
{
    if ( yyPreprocessorSkipping.isEmpty() ) {
	warning( 0, yyTokLoc, "Unexpected #elif, #else or #endif" );
	return TRUE;
    }

    bool skip = yyPreprocessorSkipping.pop();
    if ( skip )
	yyNumPreprocessorSkipping--;
    return skip;
}

/*
  Returns TRUE if the condition evaluates as true, otherwise FALSE.  The
  condition is represented by a string.  Unsophisticated parsing techniques are
  used.
*/
bool Tokenizer::isTrue( const QString& condition ) const
{
    static QRegExp *definedX = 0;

    if ( definedX == 0 )
	definedX = new QRegExp( QString("defined ?\\( ?([A-Z_0-9a-z]+) ?\\)") );

    int firstOr = -1;
    int firstAnd = -1;
    int parenDepth = 0;

    /*
      Find the first logical operator at top level, but be careful about
      precedence.  Examples:

	  X || Y          // the or
	  X || Y || Z     // the leftmost or
	  X || Y && Z     // the or
	  X && Y || Z     // the or
	  (X || Y) && Z   // the and
    */
    for ( int i = 0; i < (int) condition.length() - 1; i++ ) {
	QChar ch = condition[i];
	if ( ch == QChar('(') ) {
	    parenDepth++;
	} else if ( ch == QChar(')') ) {
	    parenDepth--;
	} else if ( parenDepth == 0 ) {
	    if ( condition[i + 1] == ch ) {
		if ( ch == QChar('|') ) {
		    firstOr = i;
		    break;
		} else if ( ch == QChar('&') ) {
		    if ( firstAnd == -1 )
			firstAnd = i;
		}
	    }
	}
    }
    if ( firstOr != -1 )
	return isTrue( condition.left(firstOr) ) ||
	       isTrue( condition.mid(firstOr + 2) );
    if ( firstAnd != -1 )
	return isTrue( condition.left(firstAnd) ) &&
	       isTrue( condition.mid(firstAnd + 2) );

    QString t = condition.simplifyWhiteSpace();
    if ( t.isEmpty() )
	return TRUE;

    if ( t[0] == QChar('!') )
	return !isTrue( t.mid(1) );
    if ( t[0] == QChar('(') && t.right(1)[0] == QChar(')') )
	return isTrue( t.mid(1, t.length() - 2) );

    if ( definedX->match(t) )
	return config->isDef( definedX->cap(1) );
    else
	return config->isTrue( t );
}

FileTokenizer::FileTokenizer()
    : yyIn( stdin )
{
}

int FileTokenizer::getch()
{
    return getc( yyIn );
}

void FileTokenizer::start( const Location& loc, FILE *in )
{
    yyIn = in;
    Tokenizer::start( loc );
}

StringTokenizer::StringTokenizer()
    : yyIn( 0 ), yyPos( 0 ), yyLen( 0 )
{
}

int StringTokenizer::getch()
{
    return yyPos == yyLen ? EOF : yyIn[yyPos++];
}

void StringTokenizer::start( const Location& loc, const char *in, int len )
{
    yyIn = in;
    yyPos = 0;
    yyLen = len;
    Tokenizer::start( loc );
}

void StringTokenizer::stop()
{
    yyIn = 0;
    yyPos = 0;
    yyLen = 0;
}
