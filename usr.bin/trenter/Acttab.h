struct  _Acttab
{
        String  act_name;
        Routine act_func;
};

typedef struct  _Acttab Acttab;         /* new type: Acttab */

/* external Acttabs: */

extern  Acttab  act_tab[];      /* Verb actions */

extern  Routine act_get();
extern	char	*act_lk();
