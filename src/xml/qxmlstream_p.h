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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QXMLSTREAM_P_H
#define QXMLSTREAM_P_H

class QXmlStreamReader_Table
{
public:
  enum {
    EOF_SYMBOL = 0,
    AMPERSAND = 5,
    ANY = 41,
    ATTLIST = 31,
    BANG = 25,
    CDATA = 46,
    CDATA_START = 28,
    COLON = 17,
    COMMA = 19,
    DASH = 20,
    DBLQUOTE = 8,
    DIGIT = 27,
    DOCTYPE = 29,
    DOT = 23,
    ELEMENT = 30,
    EMPTY = 40,
    ENTITIES = 50,
    ENTITY = 32,
    ENTITY_DONE = 45,
    EQ = 14,
    ERROR = 43,
    FIXED = 39,
    HASH = 6,
    ID = 47,
    IDREF = 48,
    IDREFS = 49,
    IMPLIED = 38,
    LANGLE = 3,
    LBRACK = 9,
    LETTER = 26,
    LPAREN = 11,
    NDATA = 36,
    NMTOKEN = 51,
    NMTOKENS = 52,
    NOTATION = 33,
    NOTOKEN = 1,
    PARSE_ENTITY = 44,
    PCDATA = 42,
    PERCENT = 15,
    PIPE = 13,
    PLUS = 21,
    PUBLIC = 35,
    QUESTIONMARK = 24,
    QUOTE = 7,
    RANGLE = 4,
    RBRACK = 10,
    REQUIRED = 37,
    RPAREN = 12,
    SEMICOLON = 18,
    SLASH = 16,
    SPACE = 2,
    STAR = 22,
    SYSTEM = 34,
    VERSION = 54,
    XML = 53,

    ACCEPT_STATE = 409,
    RULE_COUNT = 261,
    STATE_COUNT = 420,
    TERMINAL_COUNT = 55,
    NON_TERMINAL_COUNT = 81,

    GOTO_INDEX_OFFSET = 420,
    GOTO_INFO_OFFSET = 980,
    GOTO_CHECK_OFFSET = 980,
  };

  static const char  *const spell [];
  static const int            lhs [];
  static const int            rhs [];
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


const char *const QXmlStreamReader_Table::spell [] = {
  "end of file", 0, " ", "<", ">", "&", "#", "\'", "\"", "[", 
  "]", "(", ")", "|", "=", "%", "/", ":", ";", ",", 
  "-", "+", "*", ".", "?", "!", "[a-zA-Z]", "[0-9]", "[CDATA[", "DOCTYPE", 
  "ELEMENT", "ATTLIST", "ENTITY", "NOTATION", "SYSTEM", "PUBLIC", "NDATA", "REQUIRED", "IMPLIED", "FIXED", 
  "EMPTY", "ANY", "PCDATA", 0, 0, 0, "CDATA", "ID", "IDREF", "IDREFS", 
  "ENTITIES", "NMTOKEN", "NMTOKENS", "<?xml", "version"};

const int QXmlStreamReader_Table::lhs [] = {
  55, 55, 57, 57, 57, 57, 57, 57, 57, 57, 
  65, 66, 62, 70, 70, 70, 72, 64, 64, 64, 
  64, 76, 75, 77, 77, 77, 77, 77, 77, 78, 
  78, 78, 78, 78, 78, 78, 84, 80, 85, 85, 
  85, 85, 88, 89, 90, 90, 90, 90, 91, 91, 
  93, 93, 93, 94, 94, 95, 95, 96, 96, 97, 
  97, 86, 86, 92, 87, 98, 98, 100, 100, 100, 
  100, 100, 100, 100, 100, 100, 100, 101, 102, 102, 
  102, 102, 104, 106, 107, 107, 81, 81, 108, 108, 
  109, 109, 82, 82, 82, 63, 63, 73, 111, 61, 
  112, 113, 83, 83, 83, 114, 114, 114, 114, 114, 
  114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 
  114, 114, 114, 114, 114, 114, 115, 115, 115, 115, 
  68, 68, 68, 68, 116, 117, 116, 117, 116, 117, 
  116, 117, 119, 119, 119, 119, 119, 119, 119, 119, 
  119, 119, 119, 119, 119, 119, 119, 119, 119, 119, 
  119, 119, 119, 119, 118, 110, 110, 110, 110, 120, 
  121, 120, 121, 120, 121, 120, 121, 122, 122, 122, 
  122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 
  122, 122, 122, 122, 122, 122, 122, 122, 122, 122, 
  122, 122, 103, 103, 103, 103, 125, 126, 125, 126, 
  125, 125, 126, 126, 127, 127, 127, 127, 130, 69, 
  69, 69, 131, 131, 132, 60, 58, 59, 133, 79, 
  124, 129, 128, 123, 134, 134, 134, 134, 56, 56, 
  56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 
  71, 67, 67, 105, 74, 99, 99, 99, 99, 99, 
  135};

const int QXmlStreamReader_Table:: rhs[] = {
  2, 1, 4, 2, 2, 2, 2, 2, 2, 0, 
  1, 1, 9, 2, 4, 0, 4, 6, 4, 4, 
  6, 1, 3, 1, 1, 1, 2, 2, 2, 1, 
  1, 1, 1, 1, 1, 1, 4, 4, 1, 1, 
  1, 1, 1, 2, 1, 1, 1, 0, 2, 2, 
  2, 6, 6, 1, 5, 1, 5, 3, 5, 0, 
  1, 6, 8, 4, 2, 1, 5, 1, 1, 1, 
  1, 1, 1, 1, 1, 6, 7, 1, 2, 2, 
  1, 4, 3, 3, 1, 2, 5, 6, 4, 6, 
  3, 5, 5, 3, 4, 4, 5, 2, 3, 2, 
  2, 4, 5, 5, 7, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  2, 2, 3, 3, 2, 2, 2, 2, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 2, 2, 3, 3, 2, 
  2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
  1, 1, 2, 2, 3, 3, 2, 2, 2, 2, 
  1, 1, 1, 1, 1, 1, 1, 1, 5, 0, 
  1, 3, 1, 3, 2, 4, 3, 5, 3, 3, 
  3, 3, 4, 4, 1, 1, 2, 2, 2, 4, 
  2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 
  1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 
  2};

const int QXmlStreamReader_Table::action_default [] = {
  10, 250, 0, 2, 1, 0, 124, 116, 118, 119, 
  126, 129, 122, 11, 113, 107, 0, 108, 128, 110, 
  114, 112, 120, 123, 125, 106, 109, 111, 117, 115, 
  127, 121, 12, 243, 247, 242, 0, 130, 239, 246, 
  16, 241, 249, 248, 0, 245, 250, 220, 244, 0, 
  0, 255, 0, 236, 235, 0, 238, 237, 233, 229, 
  98, 254, 0, 225, 0, 0, 251, 96, 97, 100, 
  0, 0, 252, 0, 0, 0, 163, 155, 157, 158, 
  132, 144, 161, 152, 146, 147, 143, 149, 153, 151, 
  159, 162, 142, 145, 148, 150, 156, 154, 164, 160, 
  140, 165, 0, 134, 138, 136, 141, 131, 139, 0, 
  137, 133, 135, 0, 15, 14, 253, 0, 22, 19, 
  252, 0, 0, 18, 0, 0, 31, 36, 30, 0, 
  32, 252, 0, 33, 0, 24, 0, 34, 0, 26, 
  35, 25, 0, 230, 40, 39, 252, 42, 48, 252, 
  41, 0, 43, 252, 48, 252, 0, 48, 252, 0, 
  0, 47, 45, 46, 50, 51, 252, 252, 0, 56, 
  252, 53, 252, 0, 57, 0, 54, 252, 52, 252, 
  0, 55, 64, 49, 0, 252, 60, 252, 0, 58, 
  61, 62, 0, 252, 0, 0, 59, 63, 44, 65, 
  0, 38, 0, 0, 252, 0, 93, 94, 0, 0, 
  0, 0, 252, 0, 199, 190, 192, 194, 167, 179, 
  197, 188, 182, 180, 183, 178, 185, 187, 195, 198, 
  177, 181, 184, 186, 191, 189, 193, 196, 200, 202, 
  201, 175, 0, 0, 0, 0, 234, 231, 169, 173, 
  171, 0, 0, 92, 176, 166, 174, 0, 172, 168, 
  170, 91, 0, 95, 0, 0, 0, 0, 0, 252, 
  85, 252, 0, 253, 0, 86, 0, 88, 68, 73, 
  72, 69, 70, 71, 252, 74, 75, 0, 0, 0, 
  260, 259, 257, 258, 256, 66, 252, 0, 252, 0, 
  0, 67, 76, 252, 0, 252, 0, 0, 77, 0, 
  78, 0, 81, 84, 0, 0, 204, 214, 213, 0, 
  216, 218, 217, 215, 0, 232, 206, 210, 208, 212, 
  203, 211, 0, 209, 205, 207, 0, 80, 79, 0, 
  82, 0, 83, 87, 99, 0, 37, 0, 0, 0, 
  0, 90, 89, 0, 102, 23, 27, 29, 28, 0, 
  0, 252, 253, 0, 252, 0, 105, 104, 252, 0, 
  103, 101, 0, 0, 20, 252, 17, 0, 21, 0, 
  0, 240, 0, 252, 0, 228, 0, 221, 227, 0, 
  226, 223, 252, 252, 253, 222, 224, 0, 252, 0, 
  219, 252, 0, 252, 0, 220, 0, 0, 13, 261, 
  9, 5, 8, 4, 0, 7, 250, 6, 0, 3};

const int QXmlStreamReader_Table::goto_default [] = {
  2, 4, 3, 46, 381, 41, 35, 48, 45, 39, 
  239, 49, 117, 75, 386, 72, 116, 40, 44, 157, 
  120, 121, 136, 135, 139, 128, 126, 130, 137, 129, 
  149, 150, 147, 159, 158, 199, 155, 154, 156, 177, 
  170, 187, 191, 296, 295, 288, 314, 313, 312, 272, 
  63, 270, 271, 132, 131, 212, 36, 33, 138, 37, 
  38, 109, 102, 323, 101, 257, 242, 241, 238, 240, 
  332, 319, 318, 320, 322, 391, 392, 47, 43, 55, 
  0};

const int QXmlStreamReader_Table::action_index [] = {
  -9, -55, 54, 198, 925, 103, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, 129, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, 66, -55, -55, -55, 
  79, -55, -55, -55, 132, -55, -55, 102, -55, -18, 
  116, -55, 25, -55, -55, 138, -55, -55, -55, -55, 
  -55, -55, 34, -55, 71, 42, -55, -55, -55, -55, 
  86, 82, 102, 289, 342, 102, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, 395, -55, -55, -55, -55, -55, -55, 315, 
  -55, -55, -55, 84, -55, -55, -55, 92, -55, -55, 
  102, 194, 57, -55, 63, 55, -55, -55, -55, 152, 
  -55, 62, 195, -55, 191, -55, 202, -55, 65, -55, 
  -55, -55, 60, -55, -55, -55, 102, -55, 148, 40, 
  -55, 165, -55, 36, 157, 102, 43, 161, 47, 24, 
  99, -55, -55, -55, -55, 114, 102, 102, 115, -55, 
  102, 33, 102, 113, -55, 111, -55, 102, 38, 102, 
  112, -55, -55, -55, 134, 102, 37, 102, 32, -55, 
  -55, -55, 136, 35, 23, 13, -55, -55, -55, -55, 
  30, -55, 14, 44, 45, 46, -55, -55, 607, 137, 
  819, 98, 39, 109, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, -55, -55, -55, -55, -55, -55, -55, -55, 
  -55, -55, 501, 85, 61, 168, -55, -55, -55, -55, 
  -55, 102, 100, -55, -55, -55, -55, 766, -55, -55, 
  -55, -55, 59, -55, 56, 48, 41, 105, 51, 102, 
  -55, 102, 222, 50, 52, -55, 53, -55, -55, -55, 
  -55, -55, -55, -55, 102, -55, -55, 58, 160, 236, 
  -55, -55, -55, -55, -55, -55, -2, 145, -2, -2, 
  193, -55, -55, -2, 236, -2, 106, -2, -55, 448, 
  -55, 713, -55, -55, 150, 91, -55, -55, -55, 660, 
  -55, -55, -55, -55, -17, -55, -55, -55, -55, -55, 
  -55, -55, 554, -55, -55, -55, 29, -55, -55, 95, 
  -55, 28, -55, -55, -55, 27, -55, 26, 22, -6, 
  4, -55, -55, 2, -55, -55, -55, -55, -55, 76, 
  64, 5, 67, 1, 0, 49, -55, -55, 6, 11, 
  -55, -55, -13, 171, -55, 7, -55, 21, -55, 872, 
  182, -55, -16, 9, 10, -55, 124, -10, -55, 8, 
  -55, -55, 15, 31, -3, -55, -55, 18, 16, 78, 
  -55, 17, 12, 20, 143, 19, 3, -1, -55, -55, 
  -55, -55, -55, -55, 101, -55, -55, -55, 872, -55, 

  -81, -81, -81, 118, 75, -14, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, 64, -16, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, 57, -81, 62, -81, -81, -81, -81, -81, 
  -81, 66, -81, -7, -12, 48, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, 53, -81, -81, -81, -81, -81, -81, 52, 
  -81, -81, -81, 16, -81, -81, -81, -81, -81, -81, 
  27, 101, -81, -81, -81, 23, -81, -81, -81, 8, 
  -81, 28, -81, -81, -81, -81, 204, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, 18, -81, -81, 19, 
  -81, -81, -81, 20, 25, 34, -81, 39, 42, -81, 
  -81, -81, -81, -81, -81, -81, 40, 32, 30, -81, 
  56, -81, 36, 33, -81, 31, -81, 35, -81, 29, 
  37, -81, -81, -81, -81, 43, -81, 46, 44, -81, 
  -81, -81, -81, 41, -81, 38, -81, -81, -81, -81, 
  -81, -81, 17, -81, 21, -81, -81, -81, -81, 15, 
  -50, 22, 26, 24, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, -30, -45, -81, -81, -81, -81, -81, -81, 
  -81, 73, 70, -81, -81, -81, -81, -18, -81, -81, 
  -81, -81, -81, -81, 55, -81, 68, 69, 67, 85, 
  -81, 98, -81, 58, -81, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, 50, -81, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, 14, -81, 59, 54, 
  51, -81, -81, 49, 47, 60, -81, 129, -81, 65, 
  -81, 74, -81, -81, -81, 63, -81, -81, -81, 82, 
  -81, -81, -81, -81, -81, -81, -81, -81, -81, -81, 
  -81, -81, 81, -81, -81, -81, 61, -81, -81, 45, 
  -81, -15, -81, -81, -81, 11, -81, 6, 4, -9, 
  -3, -81, -81, 0, -81, -81, -81, -81, -81, 10, 
  -10, 84, -4, -81, -1, -81, -81, -81, 13, -81, 
  -81, -81, -13, 78, -81, -5, -81, -81, -81, 71, 
  -81, -81, -38, 2, -81, -81, -81, -46, -81, -81, 
  -81, -81, 87, 9, 72, -81, -81, -81, 3, -40, 
  -81, 12, -81, 7, 5, 88, -81, -81, -81, -81, 
  -81, -81, -81, -81, -81, -81, 1, -81, 102, -81};

const int QXmlStreamReader_Table::action_info [] = {
  66, 325, 66, 408, 66, 367, 66, 66, 66, 66, 
  61, 66, 390, 51, 385, 370, 61, 66, 66, 66, 
  51, 66, 66, 61, 66, 378, 403, 407, 66, 66, 
  66, 66, 398, 66, 201, 1, 401, 66, 66, 51, 
  51, 66, 66, 59, 0, 197, 68, 66, 207, 66, 
  206, 179, 172, 366, 409, 182, 343, 277, 51, 190, 
  51, 123, 0, 263, 66, 0, 198, 51, 344, 303, 
  69, 74, 73, 66, 74, 73, 61, 51, 143, 247, 
  0, 51, 61, 74, 73, 311, 309, 62, 60, 74, 
  73, 74, 73, 74, 73, 65, 119, 50, 202, 360, 
  359, 118, 311, 309, 66, 74, 73, 74, 73, 50, 
  153, 54, 53, 71, 70, 243, 0, 51, 307, 298, 
  347, 265, 153, 153, 153, 51, 153, 167, 388, 51, 
  372, 51, 0, 166, 0, 51, 0, 51, 51, 51, 
  389, 51, 54, 53, 74, 73, 186, 185, 194, 193, 
  74, 73, 265, 62, 60, 61, 58, 299, 298, 0, 
  371, 372, 0, 146, 57, 56, 310, 311, 309, 161, 
  163, 152, 162, 66, 0, 374, 153, 0, 161, 163, 
  118, 162, 161, 163, 0, 162, 246, 338, 337, 336, 
  0, 51, 145, 144, 57, 56, 66, 124, 382, 0, 
  410, 16, 210, 208, 66, 124, 62, 60, 61, 125, 
  290, 265, 355, 291, 0, 0, 293, 125, 0, 294, 
  292, 266, 264, 267, 268, 0, 0, 0, 0, 211, 
  209, 0, 0, 284, 0, 0, 0, 0, 0, 13, 
  0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 
  0, 32, 0, 290, 280, 287, 291, 0, 0, 293, 
  0, 0, 294, 292, 0, 0, 0, 0, 278, 281, 
  282, 283, 279, 285, 286, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 98, 0, 93, 0, 84, 92, 80, 85, 94, 
  87, 95, 89, 83, 88, 97, 77, 96, 78, 79, 
  90, 99, 82, 91, 76, 86, 81, 98, 0, 93, 
  0, 84, 111, 110, 85, 94, 87, 95, 89, 83, 
  88, 97, 77, 96, 78, 79, 90, 99, 82, 91, 
  76, 86, 81, 0, 98, 0, 93, 0, 84, 107, 
  106, 85, 94, 87, 95, 89, 83, 88, 97, 77, 
  96, 78, 79, 90, 99, 82, 91, 76, 86, 81, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 98, 0, 93, 
  0, 84, 104, 103, 85, 94, 87, 95, 89, 83, 
  88, 97, 77, 96, 78, 79, 90, 99, 82, 91, 
  76, 86, 81, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  98, 0, 93, 315, 84, 317, 316, 85, 94, 87, 
  95, 89, 83, 88, 97, 77, 96, 78, 79, 90, 
  99, 82, 91, 76, 86, 81, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 236, 223, 231, 213, 222, 249, 248, 
  224, 232, 226, 233, 227, 221, 0, 235, 215, 234, 
  216, 217, 228, 237, 220, 229, 214, 225, 219, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 98, 0, 93, 315, 
  84, 334, 333, 85, 94, 87, 95, 89, 83, 88, 
  97, 77, 96, 78, 79, 90, 99, 82, 91, 76, 
  86, 81, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 236, 
  223, 231, 213, 222, 230, 218, 224, 232, 226, 233, 
  227, 221, 0, 235, 215, 234, 216, 217, 228, 237, 
  220, 229, 214, 225, 219, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 98, 0, 93, 315, 84, 327, 326, 85, 
  94, 87, 95, 89, 83, 88, 97, 77, 96, 78, 
  79, 90, 99, 82, 91, 76, 86, 81, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 98, 0, 93, 315, 84, 
  330, 329, 85, 94, 87, 95, 89, 83, 88, 97, 
  77, 96, 78, 79, 90, 99, 82, 91, 76, 86, 
  81, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 236, 223, 
  231, 213, 222, 259, 258, 224, 232, 226, 233, 227, 
  221, 0, 235, 215, 234, 216, 217, 228, 237, 220, 
  229, 214, 225, 219, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 236, 223, 231, 213, 222, 255, 254, 224, 232, 
  226, 233, 227, 221, 0, 235, 215, 234, 216, 217, 
  228, 237, 220, 229, 214, 225, 219, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 30, 380, 25, 5, 15, 24, 
  10, 17, 26, 19, 27, 21, 14, 20, 29, 7, 
  28, 8, 9, 22, 31, 12, 23, 6, 18, 11, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 
  0, 0, 0, 0, 0, 32, 0, 30, 16, 25, 
  5, 15, 24, 10, 17, 26, 19, 27, 21, 14, 
  20, 29, 7, 28, 8, 9, 22, 31, 12, 23, 
  6, 18, 11, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  13, 0, 0, 0, 0, 0, 0, 0, 32, 0, 

  387, 342, 418, 368, 393, 52, 373, 377, 400, 364, 
  350, 365, 383, 351, 384, 399, 354, 256, 405, 404, 
  352, 397, 349, 361, 402, 369, 297, 346, 251, 114, 
  151, 200, 160, 205, 245, 261, 204, 250, 262, 122, 
  203, 180, 142, 244, 175, 148, 165, 178, 173, 260, 
  0, 108, 168, 195, 184, 188, 100, 196, 192, 0, 
  164, 304, 289, 189, 113, 379, 169, 176, 171, 174, 
  302, 300, 306, 181, 183, 321, 64, 339, 67, 115, 
  0, 42, 324, 253, 321, 42, 353, 345, 348, 252, 
  305, 321, 321, 340, 376, 301, 363, 274, 375, 395, 
  362, 273, 406, 394, 387, 269, 419, 127, 341, 140, 
  276, 133, 42, 0, 273, 112, 105, 141, 0, 134, 
  0, 416, 393, 413, 411, 417, 415, 412, 0, 0, 
  0, 0, 0, 0, 0, 0, 414, 0, 0, 0, 
  0, 0, 0, 0, 34, 308, 331, 396, 34, 275, 
  0, 0, 0, 335, 328, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 34, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  127, 0, 140, 0, 133, 0, 0, 0, 0, 0, 
  358, 0, 134, 0, 0, 0, 0, 356, 357, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0};

const int QXmlStreamReader_Table::action_check [] = {
  2, 18, 2, 4, 2, 4, 2, 2, 2, 2, 
  26, 2, 4, 26, 4, 4, 26, 2, 2, 2, 
  26, 2, 2, 26, 2, 4, 14, 24, 2, 2, 
  2, 2, 14, 2, 4, 44, 54, 2, 2, 26, 
  26, 2, 2, 18, -1, 22, 4, 2, 4, 2, 
  4, 13, 19, 4, 0, 12, 4, 4, 26, 22, 
  26, 4, -1, 4, 2, -1, 42, 26, 20, 11, 
  4, 7, 8, 2, 7, 8, 26, 26, 18, 18, 
  -1, 26, 26, 7, 8, 7, 8, 24, 25, 7, 
  8, 7, 8, 7, 8, 24, 4, 6, 36, 34, 
  35, 9, 7, 8, 2, 7, 8, 7, 8, 6, 
  11, 26, 27, 34, 35, 6, -1, 26, 12, 13, 
  15, 20, 11, 11, 11, 26, 11, 13, 4, 26, 
  29, 26, -1, 19, -1, 26, -1, 26, 26, 26, 
  16, 26, 26, 27, 7, 8, 12, 13, 12, 13, 
  7, 8, 20, 24, 25, 26, 18, 12, 13, -1, 
  28, 29, -1, 11, 26, 27, 6, 7, 8, 21, 
  22, 6, 24, 2, -1, 4, 11, -1, 21, 22, 
  9, 24, 21, 22, -1, 24, 18, 37, 38, 39, 
  -1, 26, 40, 41, 26, 27, 2, 3, 16, -1, 
  2, 3, 7, 8, 2, 3, 24, 25, 26, 15, 
  17, 20, 10, 20, -1, -1, 23, 15, -1, 26, 
  27, 30, 31, 32, 33, -1, -1, -1, -1, 34, 
  35, -1, -1, 11, -1, -1, -1, -1, -1, 45, 
  -1, -1, -1, -1, -1, -1, -1, 45, -1, -1, 
  -1, 53, -1, 17, 32, 33, 20, -1, -1, 23, 
  -1, -1, 26, 27, -1, -1, -1, -1, 46, 47, 
  48, 49, 50, 51, 52, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 2, -1, 4, -1, 6, 7, 8, 9, 10, 
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 
  21, 22, 23, 24, 25, 26, 27, 2, -1, 4, 
  -1, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
  25, 26, 27, -1, 2, -1, 4, -1, 6, 7, 
  8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, 2, -1, 4, 
  -1, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
  25, 26, 27, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  2, -1, 4, 5, 6, 7, 8, 9, 10, 11, 
  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
  22, 23, 24, 25, 26, 27, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, 45, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, 2, 3, 4, 5, 6, 7, 8, 
  9, 10, 11, 12, 13, 14, -1, 16, 17, 18, 
  19, 20, 21, 22, 23, 24, 25, 26, 27, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, 45, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, 2, -1, 4, 5, 
  6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 
  26, 27, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, 45, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 
  3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 
  13, 14, -1, 16, 17, 18, 19, 20, 21, 22, 
  23, 24, 25, 26, 27, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, 45, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, 2, -1, 4, 5, 6, 7, 8, 9, 
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 
  20, 21, 22, 23, 24, 25, 26, 27, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, 45, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, 2, -1, 4, 5, 6, 
  7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 
  27, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, 45, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, 2, 3, 
  4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 
  14, -1, 16, 17, 18, 19, 20, 21, 22, 23, 
  24, 25, 26, 27, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 45, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
  11, 12, 13, 14, -1, 16, 17, 18, 19, 20, 
  21, 22, 23, 24, 25, 26, 27, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, 45, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, 2, 3, 4, 5, 6, 7, 
  8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 
  18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, 45, -1, -1, 
  -1, -1, -1, -1, -1, 53, -1, 2, 3, 4, 
  5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
  25, 26, 27, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  45, -1, -1, -1, -1, -1, -1, -1, 53, -1, 

  16, 16, 1, 13, 50, 19, 19, 12, 48, 13, 
  19, 12, 50, 16, 12, 12, 16, 67, 13, 12, 
  16, 12, 16, 13, 12, 12, 12, 16, 13, 13, 
  12, 12, 12, 12, 79, 13, 19, 67, 12, 12, 
  12, 12, 19, 19, 12, 37, 12, 12, 12, 67, 
  -1, 63, 12, 12, 12, 12, 63, 19, 12, -1, 
  35, 12, 12, 19, 16, 1, 36, 36, 12, 36, 
  16, 12, 12, 36, 35, 10, 19, 16, 16, 13, 
  -1, 10, 19, 13, 10, 10, 19, 19, 19, 16, 
  43, 10, 10, 48, 16, 44, 12, 12, 20, 12, 
  16, 16, 14, 16, 16, 50, 4, 6, 50, 8, 
  12, 10, 10, -1, 16, 63, 63, 16, -1, 18, 
  -1, 3, 50, 5, 6, 7, 8, 9, -1, -1, 
  -1, -1, -1, -1, -1, -1, 18, -1, -1, -1, 
  -1, -1, -1, -1, 73, 16, 72, 75, 73, 51, 
  -1, -1, -1, 72, 72, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, 73, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  6, -1, 8, -1, 10, -1, -1, -1, -1, -1, 
  16, -1, 18, -1, -1, -1, -1, 23, 24, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
  -1, -1, -1, -1, -1};


#include <QCoreApplication>
template <typename T> class QXmlStreamSimpleStack {
    T *data;
    int tos, cap;
public:
    inline QXmlStreamSimpleStack():data(0), tos(-1), cap(0){}
    inline ~QXmlStreamSimpleStack(){ if (data) qFree(data); }

    inline void reserve(int extraCapacity) {
        if (tos + extraCapacity + 1 > cap) {
            cap = qMax(tos + extraCapacity + 1, cap << 1 );
            data = reinterpret_cast<T *>(qRealloc(data, cap * sizeof(T)));
        }
    }

    inline T &push() { reserve(1); return data[++tos]; }
    inline T &rawPush() { return data[++tos]; }
    inline const T &top() const { return data[tos]; }
    inline T &top() { return data[tos]; }
    inline T &pop() { return data[tos--]; }
    inline T &operator[](int index) { return data[index]; }
    inline const T &at(int index) const { return data[index]; }
    inline int size() const { return tos + 1; }
    inline void resize(int s) { tos = s - 1; }
    inline bool isEmpty() const { return tos < 0; }
    inline void clear() { tos = -1; }
};


class QXmlStream
{
    Q_DECLARE_TR_FUNCTIONS(QXmlStream)
};

class QXmlStreamPrivateTagStack {
public:
    struct NamespaceDeclaration
    {
        QStringRef prefix;
        QStringRef namespaceUri;
    };

    struct Tag
    {
        QStringRef name;
        QStringRef qualifiedName;
        NamespaceDeclaration namespaceDeclaration;
        int tagStackStringStorageSize;
        int namespaceDeclarationsSize;
    };


    QXmlStreamPrivateTagStack();
    QXmlStreamSimpleStack<NamespaceDeclaration> namespaceDeclarations;
    QString tagStackStringStorage;
    int tagStackStringStorageSize;
    int tagStackDefaultStringStorageSize;
    bool tagsDone;

    inline QStringRef addToStringStorage(const QStringRef &s) {
        int pos = tagStackStringStorageSize;
        tagStackStringStorage.insert(pos, s.unicode(), s.size());
        tagStackStringStorageSize = tagStackStringStorage.size();
        return QStringRef(&tagStackStringStorage, pos, s.size());
    }
    inline QStringRef addToStringStorage(const QString &s) {
        int pos = tagStackStringStorageSize;
        tagStackStringStorage.insert(pos, s.unicode(), s.size());
        tagStackStringStorageSize = tagStackStringStorage.size();
        return QStringRef(&tagStackStringStorage, pos, s.size());
    }

    QXmlStreamSimpleStack<Tag> tagStack;

    inline void initTagStack() {
        tagStackStringStorageSize = tagStackDefaultStringStorageSize;
        namespaceDeclarations.resize(1);
    }

    inline Tag &tagStack_pop() {
        Tag& tag = tagStack.pop();
        tagStackStringStorageSize = tag.tagStackStringStorageSize;
        namespaceDeclarations.resize(tag.namespaceDeclarationsSize);
        tagsDone = tagStack.isEmpty();
        return tag;
    }
    inline Tag &tagStack_push() {
        Tag &tag = tagStack.push();
        tag.tagStackStringStorageSize = tagStackStringStorageSize;
        tag.namespaceDeclarationsSize = namespaceDeclarations.size();
        return tag;
    }
};

class QXmlStreamReaderPrivate : public QXmlStreamReader_Table, public QXmlStreamPrivateTagStack{
    QXmlStreamReader *q_ptr;
    Q_DECLARE_PUBLIC(QXmlStreamReader)
public:
    QXmlStreamReaderPrivate(QXmlStreamReader *q);
    ~QXmlStreamReaderPrivate();
    void init();

    QByteArray rawReadBuffer;
    QByteArray dataBuffer;
    uchar firstByte;
    qint64 nbytesread;
    QString readBuffer;
    int readBufferPos;
    QXmlStreamSimpleStack<uint> putStack;
    struct Entity {
        Entity(const QString& str = QString())
            :value(str), external(false), unparsed(false), literal(false),
             hasBeenParsed(false), isCurrentlyReferenced(false){}
        static inline Entity createLiteral(const QString &entity)
            { Entity result(entity); result.literal = result.hasBeenParsed = true; return result; }
        QString value;
        uint external : 1;
        uint unparsed : 1;
        uint literal : 1;
        uint hasBeenParsed : 1;
        uint isCurrentlyReferenced : 1;
    };
    QHash<QString, Entity> entityHash;
    QHash<QString, Entity> parameterEntityHash;
    QXmlStreamSimpleStack<Entity *>entityReferenceStack;
    inline bool referenceEntity(Entity &entity) {
        if (entity.isCurrentlyReferenced) {
            raiseWellFormedError(QXmlStream::tr("Recursive entity detected."));
            return false;
        }
        entity.isCurrentlyReferenced = true;
        entityReferenceStack.push() = &entity;
        injectToken(ENTITY_DONE);
        return true;
    }


    QIODevice *device;
    bool deleteDevice;
    QTextCodec *codec;
    QTextDecoder *decoder;
    bool atEnd;

    /*!
      \sa setType()
     */
    QXmlStreamReader::TokenType type;
    QXmlStreamReader::Error error;
    QString errorString;

    qint64 lineNumber, lastLineStart, characterOffset;


    void write(const QString &);
    void write(const char *);


    QXmlStreamAttributes attributes;
    QStringRef namespaceForPrefix(const QStringRef &prefix);
    void resolveTag();
    void resolvePublicNamespaces();
    void resolveDtd();
    uint resolveCharRef(int symbolIndex);
    bool checkStartDocument();
    void startDocument(const QStringRef &version);
    void parseError();
    void checkPublicLiteral(const QStringRef &publicId);

    bool scanDtd;
    QStringRef lastAttributeValue;
    bool lastAttributeIsCData;
    struct DtdAttribute {
        QStringRef tagName;
        QStringRef attributeQualifiedName;
        QStringRef attributePrefix;
        QStringRef attributeName;
        QStringRef defaultValue;
        bool isCDATA;
    };
    QXmlStreamSimpleStack<DtdAttribute> dtdAttributes;
    struct NotationDeclaration {
        QStringRef name;
        QStringRef publicId;
        QStringRef systemId;
    };
    QXmlStreamSimpleStack<NotationDeclaration> notationDeclarations;
    QXmlStreamNotationDeclarations publicNotationDeclarations;
    QXmlStreamNamespaceDeclarations publicNamespaceDeclarations;

    struct EntityDeclaration {
        QStringRef name;
        QStringRef notationName;
        QStringRef publicId;
        QStringRef systemId;
        QStringRef value;
        bool parameter;
        bool external;
        inline void clear() {
            name.clear();
            notationName.clear();
            publicId.clear();
            systemId.clear();
            value.clear();
            parameter = external = false;
        }
    };
    QXmlStreamSimpleStack<EntityDeclaration> entityDeclarations;
    QXmlStreamEntityDeclarations publicEntityDeclarations;

    QStringRef text;

    QStringRef prefix, namespaceUri, qualifiedName, name;
    QStringRef processingInstructionTarget, processingInstructionData;
    uint isEmptyElement : 1;
    uint isWhitespace : 1;
    uint isCDATA : 1;
    uint standalone : 1;
    uint hasCheckedStartDocument : 1;
    uint hasSeenTag : 1;
    uint inParseEntity : 1;
    uint referenceToUnparsedEntityDetected : 1;
    uint referenceToParameterEntityDetected : 1;
    uint hasExternalDtdSubset : 1;
    uint lockEncoding : 1;
    uint namespaceProcessing : 1;

    int resumeReduction;
    void resume(int rule);

    inline bool entitiesMustBeDeclared() const {
        return (!inParseEntity
                && (standalone
                    || (!referenceToUnparsedEntityDetected
                        && !referenceToParameterEntityDetected // Errata 13 as of 2006-04-25
                        && !hasExternalDtdSubset)));
    }

    // qlalr parser
    int tos;
    int stack_size;
    struct Value {
        int pos;
        int len;
        int prefix;
        ushort c;
    };

    Value *sym_stack;
    int *state_stack;
    inline void reallocateStack();
    inline Value &sym(int index) const
    { return sym_stack[tos + index - 1]; }
    QString textBuffer, dtdBuffer;
    inline void clearTextBuffer() {
        if (!scanDtd) {
            textBuffer.resize(0);
            textBuffer.reserve(256);
        }
    }
    struct Attribute {
        Value key;
        Value value;
    };
    QXmlStreamSimpleStack<Attribute> attributeStack;

    inline QStringRef symString(int index) {
        const Value &symbol = sym(index);
        return QStringRef(&textBuffer, symbol.pos + symbol.prefix, symbol.len - symbol.prefix);
    }
    inline QStringRef symName(int index) {
        const Value &symbol = sym(index);
        return QStringRef(&textBuffer, symbol.pos, symbol.len);
    }
    inline QStringRef symString(int index, int offset) {
        const Value &symbol = sym(index);
        return QStringRef(&textBuffer, symbol.pos + symbol.prefix + offset, symbol.len - symbol.prefix -  offset);
    }
    inline QStringRef symPrefix(int index) {
        const Value &symbol = sym(index);
        if (symbol.prefix)
            return QStringRef(&textBuffer, symbol.pos, symbol.prefix - 1);
        return QStringRef();
    }
    inline QStringRef symString(const Value &symbol) {
        return QStringRef(&textBuffer, symbol.pos + symbol.prefix, symbol.len - symbol.prefix);
    }
    inline QStringRef symName(const Value &symbol) {
        return QStringRef(&textBuffer, symbol.pos, symbol.len);
    }
    inline QStringRef symPrefix(const Value &symbol) {
        if (symbol.prefix)
            return QStringRef(&textBuffer, symbol.pos, symbol.prefix - 1);
        return QStringRef();
    }

    inline void clearSym() { Value &val = sym(1); val.pos = textBuffer.size(); val.len = 0; }


    short token;
    ushort token_char;

    uint filterCarriageReturn();
    inline uint getChar();
    inline ushort peekChar();
    inline void putChar(uint c) { putStack.push() = c; }
    inline void putChar(QChar c) { putStack.push() =  c.unicode(); }
    void putString(const QString &s, int from = 0);
    void putStringLiteral(const QString &s);
    void putStringWithLiteralQuotes(const QString &s);
    ushort getChar_helper();

    bool scanUntil(const char *str, short tokenToInject = -1);
    bool scanString(const char *str, short tokenToInject, bool requireSpace = true);
    inline void injectToken(ushort tokenToInject) {
        putChar(int(tokenToInject) << 16);
    }

    static bool validateName(const QStringRef &name);

    void parseEntity(const QString &value);
    QXmlStreamReaderPrivate *entityParser;

    bool scanAfterLangleBang();
    bool scanPublicOrSystem();
    bool scanNData();
    bool scanAfterDefaultDecl();
    bool scanAttType();


    // scan optimization functions. Not strictly necessary but LALR is
    // not very well suited for scanning fast
    int fastScanLiteralContent();
    int fastScanSpace();
    int fastScanContentCharList();
    int fastScanName(int *prefix = 0);
    inline int fastScanNMTOKEN();


    bool parse();
    inline void consumeRule(int);

    void raiseError(QXmlStreamReader::Error error, const QString& message = QString());
    void raiseWellFormedError(const QString &message);

private:
    /*! \internal
       Never assign to variable type directly. Instead use this function.

       This prevents errors from being ignored.
     */
    inline void setType(const QXmlStreamReader::TokenType t)
    {
        if(type != QXmlStreamReader::Invalid)
            type = t;
    }
};

bool QXmlStreamReaderPrivate::parse()
{
    // cleanup currently reported token

    switch (type) {
    case QXmlStreamReader::StartElement:
        name.clear();
        prefix.clear();
	qualifiedName.clear();
        namespaceUri.clear();
        if (publicNamespaceDeclarations.size())
            publicNamespaceDeclarations.clear();
        if (attributes.size())
            attributes.resize(0);
        if (isEmptyElement) {
            setType(QXmlStreamReader::EndElement);
            Tag &tag = tagStack_pop();
            namespaceUri = tag.namespaceDeclaration.namespaceUri;
            name = tag.name;
	    qualifiedName = tag.qualifiedName;
            isEmptyElement = false;
            return true;
        }
        clearTextBuffer();
        break;
    case QXmlStreamReader::EndElement:
        name.clear();
        prefix.clear();
	qualifiedName.clear();
        namespaceUri.clear();
        clearTextBuffer();
        break;
    case QXmlStreamReader::DTD:
        publicNotationDeclarations.clear();
        publicEntityDeclarations.clear();
        // fall through
    case QXmlStreamReader::Comment:
    case QXmlStreamReader::Characters:
        isCDATA = isWhitespace = false;
        text.clear();
        clearTextBuffer();
        break;
    case QXmlStreamReader::EntityReference:
        text.clear();
        name.clear();
        clearTextBuffer();
        break;
    case QXmlStreamReader::ProcessingInstruction:
        processingInstructionTarget.clear();
        processingInstructionData.clear();
	clearTextBuffer();
        break;
    case QXmlStreamReader::NoToken:
    case QXmlStreamReader::Invalid:
        break;
    case QXmlStreamReader::StartDocument:
	lockEncoding = true;
	if(decoder->hasFailure()) {
	    raiseWellFormedError(QXmlStream::tr("Encountered incorrectly encoded content."));
	    readBuffer.clear();
	    return false;
	}
        // fall through
    default:
        clearTextBuffer();
        ;
    }

    setType(QXmlStreamReader::NoToken);


    // the main parse loop
    int act, r;

    if (resumeReduction) {
        act = state_stack[tos-1];
        r = resumeReduction;
        resumeReduction = 0;
        goto ResumeReduction;
    }

    act = state_stack[tos];

    forever {
        if (token == -1 && - TERMINAL_COUNT != action_index[act]) {
            uint cu = getChar();
            token = NOTOKEN;
            token_char = cu;
            if (cu & 0xff0000) {
                token = cu >> 16;
            } else switch (token_char) {
            case 0xfffe:
            case 0xffff:
                token = ERROR;
                break;
            case '\r':
                token = SPACE;
                if (cu == '\r') {
                    if ((token_char = filterCarriageReturn())) {
                        ++lineNumber;
                        lastLineStart = characterOffset + readBufferPos;
                        break;
                    }
                } else {
                    break;
                }
                // fall through
            case '\0': {
                token = EOF_SYMBOL;
                if (!tagsDone && !inParseEntity) {
                    int a = t_action(act, token);
                    if (a < 0) {
                        raiseError(QXmlStreamReader::PrematureEndOfDocumentError);
                        return false;
                    }
                }

            } break;
            case '\n':
                ++lineNumber;
                lastLineStart = characterOffset + readBufferPos;
            case ' ':
            case '\t':
                token = SPACE;
                break;
            case '&':
                token = AMPERSAND;
                break;
            case '#':
                token = HASH;
                break;
            case '\'':
                token = QUOTE;
                break;
            case '\"':
                token = DBLQUOTE;
                break;
            case '<':
                token = LANGLE;
                break;
            case '>':
                token = RANGLE;
                break;
            case '[':
                token = LBRACK;
                break;
            case ']':
                token = RBRACK;
                break;
            case '(':
                token = LPAREN;
                break;
            case ')':
                token = RPAREN;
                break;
            case '|':
                token = PIPE;
                break;
            case '=':
                token = EQ;
                break;
            case '%':
                token = PERCENT;
                break;
            case '/':
                token = SLASH;
                break;
            case ':':
                token = COLON;
                break;
            case ';':
                token = SEMICOLON;
                break;
            case ',':
                token = COMMA;
                break;
            case '-':
                token = DASH;
                break;
            case '+':
                token = PLUS;
                break;
            case '*':
                token = STAR;
                break;
            case '.':
                token = DOT;
                break;
            case '?':
                token = QUESTIONMARK;
                break;
            case '!':
                token = BANG;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                token = DIGIT;
                break;
            default:
                if (cu < 0x20)
                    token = NOTOKEN;
                else
                    token = LETTER;
                break;
            }
        }

        act = t_action (act, token);
        if (act == ACCEPT_STATE) {
            // reset the parser in case someone resumes (process instructions can follow a valid document)
            tos = 0;
            state_stack[tos++] = 0;
            state_stack[tos] = 0;
            return true;
        } else if (act > 0) {
            if (++tos == stack_size)
                reallocateStack();

            Value &val = sym_stack[tos];
            val.c = token_char;
            val.pos = textBuffer.size();
            val.prefix = 0;
            val.len = 1;
            if (token_char)
                textBuffer.inline_append(token_char);

            state_stack[tos] = act;
            token = -1;


        } else if (act < 0) {
            r = - act - 1;

#if defined (QLALR_DEBUG)
            int ridx = rule_index[r];
            printf ("%3d) %s ::=", r + 1, spell[rule_info[ridx]]);
            ++ridx;
            for (int i = ridx; i < ridx + rhs[r]; ++i) {
                int symbol = rule_info[i];
                if (const char *name = spell[symbol])
                    printf (" %s", name);
                else
                    printf (" #%d", symbol);
            }
            printf ("\n");
#endif

            tos -= rhs[r];
            act = state_stack[tos++];
        ResumeReduction:
            switch (r) {

        case 0:
            setType(QXmlStreamReader::EndDocument);
        break;

        case 1:
            if (type != QXmlStreamReader::Invalid) {
                if (hasSeenTag || inParseEntity) {
                    setType(QXmlStreamReader::EndDocument);
                } else {
                    raiseError(QXmlStreamReader::PrematureEndOfDocumentError, QXmlStream::tr("Start tag expected."));
                    // reset the parser
                    tos = 0;
                    state_stack[tos++] = 0;
                    state_stack[tos] = 0;
                    return false;
                }
            }
        break;

        case 10:
            entityReferenceStack.pop()->isCurrentlyReferenced = false;
            clearSym();
        break;

        case 11:
            if (!scanString(spell[VERSION], VERSION, false) && atEnd) {
                resume(11);
                return false;
            }
        break;

        case 12:
            setType(QXmlStreamReader::StartDocument);
            startDocument(symString(6));
        break;

        case 13:
            hasExternalDtdSubset = true;
        break;

        case 14:
            checkPublicLiteral(symString(2));
            hasExternalDtdSubset = true;
        break;

        case 16:
            if (!scanPublicOrSystem() && atEnd) {
                resume(16);
                return false;
            }
        break;

        case 17:
        case 18:
        case 19:
        case 20:
            setType(QXmlStreamReader::DTD);
            text = &textBuffer;
        break;

        case 21:
            scanDtd = true;
        break;

        case 22:
            scanDtd = false;
        break;

        case 36:
            if (!scanString(spell[EMPTY], EMPTY, false)
                && !scanString(spell[ANY], ANY, false)
                && atEnd) {
                resume(36);
                return false;
            }
        break;

        case 42:
            if (!scanString(spell[PCDATA], PCDATA, false) && atEnd) {
                resume(42);
                return false;
            }
        break;

        case 67: {
            lastAttributeIsCData = true;
        } break;

        case 77:
            if (!scanAfterDefaultDecl() && atEnd) {
                resume(77);
                return false;
            }
        break;

        case 82:
                sym(1) = sym(2);
                lastAttributeValue.clear();
                lastAttributeIsCData = false;
                if (!scanAttType() && atEnd) {
                    resume(82);
                    return false;
                }
        break;

        case 83: {
            DtdAttribute &dtdAttribute = dtdAttributes.push();
            dtdAttribute.tagName.clear();
            dtdAttribute.isCDATA = lastAttributeIsCData;
            dtdAttribute.attributePrefix = addToStringStorage(symPrefix(1));
            dtdAttribute.attributeName = addToStringStorage(symString(1));
            dtdAttribute.attributeQualifiedName = addToStringStorage(symName(1));
            if (lastAttributeValue.isNull()) {
                dtdAttribute.defaultValue.clear();
            } else {
                if (dtdAttribute.isCDATA)
                    dtdAttribute.defaultValue = addToStringStorage(lastAttributeValue);
                else
                    dtdAttribute.defaultValue = addToStringStorage(lastAttributeValue.toString().simplified());

            }
        } break;

        case 87: {
            if (referenceToUnparsedEntityDetected && !standalone)
                break;
            int n = dtdAttributes.size();
            QStringRef tagName = addToStringStorage(symString(3));
            while (n--) {
                DtdAttribute &dtdAttribute = dtdAttributes[n];
                if (!dtdAttribute.tagName.isNull())
                    break;
                dtdAttribute.tagName = tagName;
                for (int i = 0; i < n; ++i) {
                    if ((dtdAttributes[i].tagName.isNull() || dtdAttributes[i].tagName == tagName)
                        && dtdAttributes[i].attributeQualifiedName == dtdAttribute.attributeQualifiedName) {
                        dtdAttribute.attributeQualifiedName.clear(); // redefined, delete it
                        break;
                    }
                }
            }
        } break;

        case 88: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(88);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.push();
            entityDeclaration.clear();
            entityDeclaration.name = symString(3);
        } break;

        case 89: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(89);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.push();
            entityDeclaration.clear();
            entityDeclaration.name = symString(5);
            entityDeclaration.parameter = true;
        } break;

        case 90: {
            if (!scanNData() && atEnd) {
                resume(90);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            entityDeclaration.systemId = symString(3);
            entityDeclaration.external = true;
        } break;

        case 91: {
            if (!scanNData() && atEnd) {
                resume(91);
                return false;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            checkPublicLiteral((entityDeclaration.publicId = symString(3)));
            entityDeclaration.systemId = symString(5);
            entityDeclaration.external = true;
        } break;

        case 92: {
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            entityDeclaration.notationName = symString(3);
            if (entityDeclaration.parameter)
                raiseWellFormedError(QXmlStream::tr("NDATA in parameter entity declaration."));
        }
        //fall through

        case 93:
        case 94: {
            if (referenceToUnparsedEntityDetected && !standalone) {
                entityDeclarations.pop();
                break;
            }
            EntityDeclaration &entityDeclaration = entityDeclarations.top();
            if (!entityDeclaration.external)
                entityDeclaration.value = symString(2);
            QString entityName = entityDeclaration.name.toString();
            QHash<QString, Entity> &hash = entityDeclaration.parameter ? parameterEntityHash : entityHash;
            if (!hash.contains(entityName)) {
                Entity entity(entityDeclaration.value.toString());
                entity.unparsed = (!entityDeclaration.notationName.isNull());
                entity.external = entityDeclaration.external;
                hash.insert(entityName, entity);
            }
        } break;

        case 95: {
            setType(QXmlStreamReader::ProcessingInstruction);
            int pos = sym(4).pos + sym(4).len;
            processingInstructionTarget = symString(3);
            if (scanUntil("?>")) {
                processingInstructionData = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 2);
                const QString piTarget(processingInstructionTarget.toString());
                if (!piTarget.compare(QLatin1String("xml"), Qt::CaseInsensitive)) {
                    raiseWellFormedError(QXmlStream::tr("XML declaration not at start of document."));
                }
                else if(!QXmlUtils::isNCName(piTarget))
                    raiseWellFormedError(QXmlStream::tr("%1 is an invalid processing instruction name.").arg(piTarget));
            } else if (type != QXmlStreamReader::Invalid){
                resume(95);
                return false;
            }
        } break;

        case 96:
            setType(QXmlStreamReader::ProcessingInstruction);
            processingInstructionTarget = symString(3);
            if (!processingInstructionTarget.toString().compare(QLatin1String("xml"), Qt::CaseInsensitive))
                raiseWellFormedError(QXmlStream::tr("Invalid processing instruction name."));
        break;

        case 97:
            if (!scanAfterLangleBang() && atEnd) {
                resume(97);
                return false;
            }
        break;

        case 98:
            if (!scanUntil("--")) {
                resume(98);
                return false;
            }
        break;

        case 99: {
            setType(QXmlStreamReader::Comment);
            int pos = sym(1).pos + 4;
            text = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 3);
        } break;

        case 100: {
            setType(QXmlStreamReader::Characters);
            isCDATA = true;
            int pos = sym(2).pos;
            if (scanUntil("]]>", -1)) {
                text = QStringRef(&textBuffer, pos, textBuffer.size() - pos - 3);
            } else {
                resume(100);
                return false;
            }
        } break;

        case 101: {
            if (!scanPublicOrSystem() && atEnd) {
                resume(101);
                return false;
            }
            NotationDeclaration &notationDeclaration = notationDeclarations.push();
            notationDeclaration.name = symString(3);
        } break;

        case 102: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            notationDeclaration.systemId = symString(3);
            notationDeclaration.publicId.clear();
        } break;

        case 103: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            notationDeclaration.systemId.clear();
            checkPublicLiteral((notationDeclaration.publicId = symString(3)));
        } break;

        case 104: {
            NotationDeclaration &notationDeclaration = notationDeclarations.top();
            checkPublicLiteral((notationDeclaration.publicId = symString(3)));
            notationDeclaration.systemId = symString(5);
        } break;

        case 126:
            isWhitespace = true;
            // fall through

        case 127:
        case 128:
        case 129:
            setType(QXmlStreamReader::Characters);
            sym(1).len += fastScanContentCharList();
            if (atEnd && !inParseEntity) {
                resume(129);
                return false;
            }
            text = &textBuffer;
        break;

        case 130:
        case 131:
            clearSym();
        break;

        case 132:
        case 133:
            sym(1) = sym(2);
        break;

        case 134:
        case 135:
        case 136:
        case 137:
            sym(1).len += sym(2).len;
        break;

        case 163:
            textBuffer.data()[textBuffer.size()-1] = QLatin1Char(' ');
        break;

        case 164:
            sym(1).len += fastScanLiteralContent();
            if (atEnd) {
                resume(164);
                return false;
            }
        break;

        case 165:
        case 166:
            clearSym();
        break;

        case 167:
        case 168:
	    sym(1) = sym(2);
        break;

        case 169:
        case 170:
        case 171:
        case 172:
            sym(1).len += sym(2).len;
        break;

        case 202:
        case 203:
            clearSym();
        break;

        case 204:
        case 205:
            sym(1) = sym(2);
            lastAttributeValue = symString(1);
        break;

        case 206:
        case 207:
        case 208:
        case 209:
            sym(1).len += sym(2).len;
        break;

        case 218: {
            QStringRef prefix = symPrefix(1);
            if (prefix.isEmpty() && symString(1) == QLatin1String("xmlns") && namespaceProcessing) {
                NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.push();
                namespaceDeclaration.prefix.clear();

                const QStringRef ns(symString(5));
                if(ns == QLatin1String("http://www.w3.org/2000/xmlns/") ||
                   ns == QLatin1String("http://www.w3.org/XML/1998/namespace"))
                    raiseWellFormedError(QXmlStream::tr("Illegal namespace declaration."));
                else
                    namespaceDeclaration.namespaceUri = addToStringStorage(ns);
            } else {
                Attribute &attribute = attributeStack.push();
                attribute.key = sym(1);
                attribute.value = sym(5);

                QStringRef attributeQualifiedName = symName(1);
                bool normalize = false;
                for (int a = 0; a < dtdAttributes.size(); ++a) {
                    DtdAttribute &dtdAttribute = dtdAttributes[a];
                    if (!dtdAttribute.isCDATA
                        && dtdAttribute.tagName == qualifiedName
                        && dtdAttribute.attributeQualifiedName == attributeQualifiedName
                        ) {
                        normalize = true;
                        break;
                    }
                }
                if (normalize) {
                    // normalize attribute value (simplify and trim)
                    int pos = textBuffer.size();
                    int n = 0;
                    bool wasSpace = true;
                    for (int i = 0; i < attribute.value.len; ++i) {
                        QChar c = textBuffer.at(attribute.value.pos + i);
                        if (c.unicode() == ' ') {
                            if (wasSpace)
                                continue;
                            wasSpace = true;
                        } else {
                            wasSpace = false;
                        }
                        textBuffer.inline_append(textBuffer.at(attribute.value.pos + i));
                        ++n;
                    }
                    if (wasSpace)
                        while (n && textBuffer.at(pos + n - 1).unicode() == ' ')
                            --n;
                    attribute.value.pos = pos;
                    attribute.value.len = n;
                }
                if (prefix == QLatin1String("xmlns") && namespaceProcessing) {
                    NamespaceDeclaration &namespaceDeclaration = namespaceDeclarations.push();
                    QStringRef namespacePrefix = symString(attribute.key);
                    QStringRef namespaceUri = symString(attribute.value);
                    attributeStack.pop();
                    if ((namespacePrefix == QLatin1String("xml")
                         ^ namespaceUri == QLatin1String("http://www.w3.org/XML/1998/namespace"))
                        || namespaceUri == QLatin1String("http://www.w3.org/2000/xmlns/")
                        || namespaceUri.isEmpty()
                        || namespacePrefix == QLatin1String("xmlns"))
                        raiseWellFormedError(QXmlStream::tr("Illegal namespace declaration."));

                    namespaceDeclaration.prefix = addToStringStorage(namespacePrefix);
                    namespaceDeclaration.namespaceUri = addToStringStorage(namespaceUri);
                }
            }
        } break;

        case 224: {
            Tag &tag = tagStack_push();
            prefix = tag.namespaceDeclaration.prefix  = addToStringStorage(symPrefix(2));
            name = tag.name = addToStringStorage(symString(2));
            qualifiedName = tag.qualifiedName = addToStringStorage(symName(2));
            if (!validateName(qualifiedName))
                raiseWellFormedError(QXmlStream::tr("Invalid XML name."));
        } break;

        case 225:
            isEmptyElement = true;
        // fall through

        case 226:
            setType(QXmlStreamReader::StartElement);
            resolveTag();
            if (tagStack.size() == 1 && hasSeenTag && !inParseEntity)
                raiseWellFormedError(QXmlStream::tr("Extra content at end of document."));
            hasSeenTag = true;
        break;

        case 227: {
            setType(QXmlStreamReader::EndElement);
            Tag &tag = tagStack_pop();

            namespaceUri = tag.namespaceDeclaration.namespaceUri;
            name = tag.name;
            qualifiedName = tag.qualifiedName;
            if (qualifiedName != symName(3))
                raiseWellFormedError(QXmlStream::tr("Opening and ending tag mismatch."));
        } break;

        case 228: {
            sym(1).len += sym(2).len + 1;
            QString reference = symString(2).toString();
            if (entityHash.contains(reference)) {
                Entity &entity = entityHash[reference];
                if (entity.unparsed) {
                    raiseWellFormedError(QXmlStream::tr("Reference to unparsed entity '%1'.").arg(reference));
                } else {
                    if (!entity.hasBeenParsed) {
                        parseEntity(entity.value);
                        entity.hasBeenParsed = true;
                    }
                    if (entity.literal)
                        putStringLiteral(entity.value);
                    else if (referenceEntity(entity))
                        putString(entity.value);
                    textBuffer.chop(2 + sym(2).len);
                    clearSym();
                }
                break;
            } else if (entitiesMustBeDeclared()) {
                raiseWellFormedError(QXmlStream::tr("Entity '%1' not declared.").arg(reference));
                break;
            }
            setType(QXmlStreamReader::EntityReference);
            name = symString(2);

        } break;

        case 229: {
            sym(1).len += sym(2).len + 1;
            QString reference = symString(2).toString();
            if (parameterEntityHash.contains(reference)) {
                referenceToParameterEntityDetected = true;
                Entity &entity = parameterEntityHash[reference];
                if (entity.unparsed || entity.external) {
                    referenceToUnparsedEntityDetected = true;
                } else {
                    if (referenceEntity(entity))
                        putString(entity.value);
                    textBuffer.chop(2 + sym(2).len);
                    clearSym();
                }
            } else if (entitiesMustBeDeclared()) {
                raiseWellFormedError(QXmlStream::tr("Entity '%1' not declared.").arg(symString(2).toString()));
            }
        } break;

        case 230:
            sym(1).len += sym(2).len + 1;
        break;

        case 231: {
            sym(1).len += sym(2).len + 1;
            QString reference = symString(2).toString();
            if (entityHash.contains(reference)) {
                Entity &entity = entityHash[reference];
                if (entity.unparsed || entity.value.isNull()) {
                    raiseWellFormedError(QXmlStream::tr("Reference to external entity '%1' in attribute value.").arg(reference));
                    break;
                }
                if (!entity.hasBeenParsed) {
                    parseEntity(entity.value);
                    entity.hasBeenParsed = true;
                }
                if (entity.literal)
                    putStringLiteral(entity.value);
                else if (referenceEntity(entity))
                    putStringWithLiteralQuotes(entity.value);
                textBuffer.chop(2 + sym(2).len);
                clearSym();
            } else if (entitiesMustBeDeclared()) {
                raiseWellFormedError(QXmlStream::tr("Entity '%1' not declared.").arg(reference));
            }
        } break;

        case 232: {
            if (uint s = resolveCharRef(3)) {
                if (s >= 0xffff)
                    putStringLiteral(QString::fromUcs4(&s, 1));
                else
                    putChar((LETTER << 16) | s);

                textBuffer.chop(3 + sym(3).len);
                clearSym();
            } else {
                raiseWellFormedError(QXmlStream::tr("Invalid character reference."));
            }
        } break;

        case 233: {
            if (uint s = resolveCharRef(3)) {
                if ( s == '\n' || s == '\r')
                    sym(1).len += 2 + sym(3).len;
                else {
                    if (s >= 0xffff)
                        putStringLiteral(QString::fromUcs4(&s, 1));
                    else
                        putChar((LETTER << 16) | s);

                    textBuffer.chop(3 + sym(3).len);
                    clearSym();
                }
            } else {
                raiseWellFormedError(QXmlStream::tr("Invalid character reference."));
            }
        } break;

        case 236:
        case 237:
            sym(1).len += sym(2).len;
        break;

        case 250:
            sym(1).len += fastScanSpace();
            if (atEnd) {
                resume(250);
                return false;
            }
        break;

        case 253: {
            sym(1).len += fastScanName(&sym(1).prefix);
            if (atEnd) {
                resume(253);
                return false;
            }
        } break;

        case 254:
            sym(1).len += fastScanName();
            if (atEnd) {
                resume(254);
                return false;
            }
        break;

        case 255:
        case 256:
        case 257:
        case 258:
        case 259:
            sym(1).len += fastScanNMTOKEN();
            if (atEnd) {
                resume(259);
                return false;
            }

        break;

    default:
        ;
    } // switch
            act = state_stack[tos] = nt_action (act, lhs[r] - TERMINAL_COUNT);
            if (type != QXmlStreamReader::NoToken)
                return true;
        } else {
            parseError();
            break;
        }
    }
    return false;
}

#endif // QXMLSTREAM_P_H

