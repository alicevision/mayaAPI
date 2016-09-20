"""
# ----------------------------------------------------------------------
# clex.py
#
# A lexer for ANSI C.
# ----------------------------------------------------------------------
"""

import sys
import pymel.util.external.ply.lex as lex

def t_COMMENT_BLOCK(t):
    """
    /\*(.|\n)*?\*/|/\*(.|\n)*?$
    """

    pass


def t_SEMI(t):
    """
    ;
    """

    pass


def t_ID(t):
    """
    ([|]?([:]?([.]?[A-Za-z_][\w]*)+)+)+?
    """

    pass


def t_ELLIPSIS(t):
    """
    \.\.
    """

    pass


def t_COMPONENT(t):
    """
    \.[xyz]
    """

    pass


def t_CAPTURE(t):
    """
    `
    """

    pass


def t_LPAREN(t):
    """
    \(
    """

    pass


def t_NEWLINE(t):
    """
    \n+|\r+
    """

    pass


def t_LBRACKET(t):
    """
    \[
    """

    pass


def t_VAR(t):
    """
    \$[A-Za-z_][\w_]*
    """

    pass


def t_RBRACKET(t):
    """
    \]
    """

    pass


def t_RPAREN(t):
    """
    \)
    """

    pass


def t_COMMENT(t):
    """
    //.*
    """

    pass



id_state = None

reserved = ()

t_PLUS = r'\+'

t_RBRACE = r'\}'

t_GE = '>='

t_EQ = '=='

t_MOD = '%'

t_MINUSMINUS = '--'

t_LT = '<'

suspend_depth = 0

t_PLUSEQUAL = r'\+='

t_CONDOP = r'\?'

t_GT = '>'

t_LAND = '&&'

t_DIVEQUAL = '/='

t_SCONST = r'"([^\\\n]|(\\.)|\\\n)*?"'

t_MINUSEQUAL = '-='

t_FCONST = r'(((\d+\.)(\d+)?|(\d+)?(\.\d+))(e(\+|-)?(\d+))?|(\d+)e(\+|-)?(\d+))([lL]|[fF])?'

t_ICONST = r'(0x[a-fA-F0-9]*)|\d+'

t_PLUSPLUS = r'\+\+'

t_LBRACE = r'\{'

t_MINUS = '-'

t_COLON = ':'

t_NE = '!='

t_RVEC = '>>'

reserved_map = {}

t_LVEC = '<<'

t_CROSSEQUAL = '^='

r = 'YES'

t_MODEQUAL = '%='

t_CROSS = r'\^'

t_ignore = ' \t\x0c'

t_TIMES = r'\*'

t_LE = '<='

t_EQUALS = '='

t_NOT = '!'

t_COMMA = ','

t_DIVIDE = '/'

t_LOR = r'\|\|'

t_TIMESEQUAL = r'\*='

tokens = ()


