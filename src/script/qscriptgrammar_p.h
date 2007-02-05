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

#ifndef QSCRIPTGRAMMAR_P_H
#define QSCRIPTGRAMMAR_P_H

class QScriptGrammar
{
public:
  enum {
    EOF_SYMBOL = 0,
    T_AND = 1,
    T_AND_AND = 2,
    T_AND_EQ = 3,
    T_AUTOMATIC_SEMICOLON = 62,
    T_BREAK = 4,
    T_CASE = 5,
    T_CATCH = 6,
    T_COLON = 7,
    T_COMMA = 8,
    T_CONST = 81,
    T_CONTINUE = 9,
    T_DEFAULT = 10,
    T_DELETE = 11,
    T_DIVIDE = 12,
    T_DIVIDE_EQ = 13,
    T_DO = 14,
    T_DOT = 15,
    T_ELSE = 16,
    T_EQ = 17,
    T_EQ_EQ = 18,
    T_EQ_EQ_EQ = 19,
    T_FALSE = 80,
    T_FINALLY = 20,
    T_FOR = 21,
    T_FUNCTION = 22,
    T_GE = 23,
    T_GT = 24,
    T_GT_GT = 25,
    T_GT_GT_EQ = 26,
    T_GT_GT_GT = 27,
    T_GT_GT_GT_EQ = 28,
    T_IDENTIFIER = 29,
    T_IF = 30,
    T_IN = 31,
    T_INSTANCEOF = 32,
    T_LBRACE = 33,
    T_LBRACKET = 34,
    T_LE = 35,
    T_LPAREN = 36,
    T_LT = 37,
    T_LT_LT = 38,
    T_LT_LT_EQ = 39,
    T_MINUS = 40,
    T_MINUS_EQ = 41,
    T_MINUS_MINUS = 42,
    T_NEW = 43,
    T_NOT = 44,
    T_NOT_EQ = 45,
    T_NOT_EQ_EQ = 46,
    T_NULL = 78,
    T_NUMERIC_LITERAL = 47,
    T_OR = 48,
    T_OR_EQ = 49,
    T_OR_OR = 50,
    T_PLUS = 51,
    T_PLUS_EQ = 52,
    T_PLUS_PLUS = 53,
    T_QUESTION = 54,
    T_RBRACE = 55,
    T_RBRACKET = 56,
    T_REMAINDER = 57,
    T_REMAINDER_EQ = 58,
    T_RESERVED = 82,
    T_RETURN = 59,
    T_RPAREN = 60,
    T_SEMICOLON = 61,
    T_STAR = 63,
    T_STAR_EQ = 64,
    T_STRING_LITERAL = 65,
    T_SWITCH = 66,
    T_THIS = 67,
    T_THROW = 68,
    T_TILDE = 69,
    T_TRUE = 79,
    T_TRY = 70,
    T_TYPEOF = 71,
    T_VAR = 72,
    T_VOID = 73,
    T_WHILE = 74,
    T_WITH = 75,
    T_XOR = 76,
    T_XOR_EQ = 77,

    ACCEPT_STATE = 200,
    RULE_COUNT = 231,
    STATE_COUNT = 406,
    TERMINAL_COUNT = 83,
    NON_TERMINAL_COUNT = 86,

    GOTO_INDEX_OFFSET = 406,
    GOTO_INFO_OFFSET = 1106,
    GOTO_CHECK_OFFSET = 1106,
  };

  static const char  *const spell [];
  static const int            lhs [];
  static const int            rhs [];

#ifndef QLALR_NO_QSCRIPTGRAMMAR_DEBUG_INFO
  static const int     rule_index [];
  static const int      rule_info [];
#endif // QLALR_NO_QSCRIPTGRAMMAR_DEBUG_INFO

  static const int   goto_default [];
  static const int action_default [];
  static const int   action_index [];
  static const int    action_info [];
  static const int   action_check [];

  inline int nt_action (int state, int nt) const
  {
    const int *const goto_index = &action_index [GOTO_INDEX_OFFSET];
    const int *const goto_check = &action_check [GOTO_CHECK_OFFSET];

    const int yyn = goto_index [state] + nt;

    if (yyn < 0 || goto_check [yyn] != nt)
      return goto_default [nt];

    const int *const goto_info = &action_info [GOTO_INFO_OFFSET];
    return goto_info [yyn];
  }

  inline int t_action (int state, int token) const
  {
    const int yyn = action_index [state] + token;

    if (yyn < 0 || action_check [yyn] != token)
      return - action_default [state];

    return action_info [yyn];
  }
};


#endif // QSCRIPTGRAMMAR_P_H

