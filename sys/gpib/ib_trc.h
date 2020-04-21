# ifndef _IBTRACE_
# define _IBTRACE_

# define NTCELLS 300
# define NTARGS	4
struct trace
{
    int when;
    int args[NTARGS];
};
# endif _IBTRACE_
