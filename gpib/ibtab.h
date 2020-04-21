/* structure used by ibtab package */

struct ibtab
{
    char *ibt_file;		/*name of ib device file*/
    char *ibt_cfile;		/*name of controlling device file*/
    int ibt_node;		/*logical node num (tail of name)*/
    char *ibt_flags;		/*flags*/
    char ibt_tag;		/*IEEE 488 bus address*/
    char ibt_ppr;		/*parallel poll configuration message*/
# ifdef notdef
    char ibt_T;			/*poll code for talk*/
    char ibt_L;			/*poll code for listen*/
    char ibt_C;			/*poll code for takectl*/
    char ibt_E;			/*poll code for error*/
    char ibt_I;			/*poll code for idle*/
    char ibt_Tslot;		/*talk slot when listening*/
    int ibt_Lmap;		/*listeners when talking*/
# endif notdef
    char *ibt_comment;		/*etc*/
};

# define IBTAB "/etc/ibtab"
