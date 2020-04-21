#ifndef languages_h
#define languages_h



typedef enum {
    L_PRINTDECL, L_PRINTVAL, L_TYPEMATCH, L_BUILDAREF, L_EVALAREF,
    L_MODINIT, L_HASMODULES, L_PASSADDR,
    L_ENDOP
} LanguageOp;

typedef LanguageOperation();
typedef struct language *Language;
struct language {
    String name;
    String suffix;
    LanguageOperation *op[20];
    Language next;
};

short primlang;

language_init(/*  */);
Language findlanguage(/* suffix */);
String language_name(/* lang */);
Language language_define(/* name, suffix */);
language_setop(/* lang, op, operation */);
LanguageOperation *language_op(/* lang, op */);
#endif

