/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_ODS_FORMULA_ODS_FORMULA_PARSER_HPP_INCLUDED
# define YY_ODS_FORMULA_ODS_FORMULA_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int ods_formuladebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ODST_NUMBER = 258,             /* ODST_NUMBER  */
    ODST_STRING = 259,             /* ODST_STRING  */
    ODST_IDENTIFIER = 260,         /* ODST_IDENTIFIER  */
    ODST_FUNCTION_NO_ARG = 261,    /* ODST_FUNCTION_NO_ARG  */
    ODST_FUNCTION_SINGLE_ARG = 262, /* ODST_FUNCTION_SINGLE_ARG  */
    ODST_FUNCTION_TWO_ARG = 263,   /* ODST_FUNCTION_TWO_ARG  */
    ODST_FUNCTION_THREE_ARG = 264, /* ODST_FUNCTION_THREE_ARG  */
    ODST_FUNCTION_ARG_LIST = 265,  /* ODST_FUNCTION_ARG_LIST  */
    ODST_START = 266,              /* ODST_START  */
    ODST_NOT = 267,                /* ODST_NOT  */
    ODST_OR = 268,                 /* ODST_OR  */
    ODST_AND = 269,                /* ODST_AND  */
    ODST_IF = 270,                 /* ODST_IF  */
    ODST_UMINUS = 271              /* ODST_UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif




int ods_formulaparse (ods_formula_parse_context *context);


#endif /* !YY_ODS_FORMULA_ODS_FORMULA_PARSER_HPP_INCLUDED  */
