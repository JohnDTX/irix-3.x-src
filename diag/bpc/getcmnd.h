/*
 *	Kurt Akeley
 *	4 August 1984
 *
 *	Header for getcmnd.c
 */

#define MAXARG		10

typedef struct {
    char *name;
    int value;
    } Argdef;

typedef struct {
    int number;
    char *name;
    char *prefix;
    int minargs;
    int maxargs;
    int maxprint;
    Argdef arg[MAXARG];
    } Cmnddef;

typedef struct {
    int number;
    int args;
    int arg[MAXARG]
    } Command;

typedef struct {
    int number;
    char *string;
    } Help;

typedef int boolean;

