# include "toyfs.h"
# include "bell_toyfs.h"

# include "stdio.h"
# include "sys/stat.h"


/*
 2074 -rw-rw-r--  1  165.0        1756 16:03, 10jun85
 */
int
bell_idump(ip, level, F)
    I *ip;
    int level;
    FILE *F;
{
    extern char *cdate();


    bell_cidump(ip, F);

    if( level <= 1 )
    {
	fprintf(F, " %13s", cdate(&ip->i_imtime));
	return;
    }
    bell_didump(&ip->i_iatime, F);
}

bell_didump(dates, F)
    register time_t *dates;
    FILE *F;
{
    extern char *cdate();

    fprintf(F, " <%-13s", cdate(dates++));
    fprintf(F, " >%-13s", cdate(dates++));
    fprintf(F, " *%-13s", cdate(dates++));
}

bell_cidump(ip, F)
    I *ip;
    FILE *F;
{
    extern char *bell_mode();
    register struct dinode *dip;
    int ftype;
    dev_t rdev;

    if( ip == 0 )
	return -1;

    dip = &((struct bell_toyinode *)ip->i_dinode)->inode;
    ftype = ip->i_imode&S_IFMT;

    fprintf(F, "%5u", ip->i_number);
    fprintf(F, " %s", bell_mode(ip->i_imode));
    fprintf(F, "%3u", ip->i_inlink);

    fprintf(F, "%5u.%-3u", ip->i_iuid, ip->i_igid);
 
    if( ftype == S_IFCHR || ftype == S_IFBLK )
	fprintf(F, "%3u, %-2u", major(ip->i_irdev), minor(ip->i_irdev));
    else
	fprintf(F, "%6lu", ip->i_isize);

    return 0;
}

struct charval
{
    unsigned short mask, val;
    short index;
    char symbol;
};

# define PROTSHIFT	3
# define OWNERSHIFT	(0*PROTSHIFT)
# define GROUPSHIFT	(1*PROTSHIFT)
# define PUBLICSHIFT	(2*PROTSHIFT)
# define PROTBIT(g, b)	((b)>>g/**/SHIFT)

static
struct charval modechars[] =
{
	{0, 0, 0, ' '},
	{S_ISVTX, S_ISVTX, 0, '%'},

	{0, 0, 1, '?'},
	{S_IFMT, S_IFDIR, 1, 'd'},
	{S_IFMT, S_IFCHR, 1, 'c'},
	{S_IFMT, S_IFBLK, 1, 'b'},
	{S_IFMT, S_IFREG, 1, 'f'},
	{S_IFMT, S_IFIFO, 1, 'p'},
	{S_IFMT, S_IFLNK, 1, 'l'},

	{0, 0, 2, '-'},
	{PROTBIT(OWNER, S_IREAD), PROTBIT(OWNER, S_IREAD), 2, 'r'},
	{0, 0, 3, '-'},
	{PROTBIT(OWNER, S_IWRITE), PROTBIT(OWNER, S_IWRITE), 3, 'w'},
	{0, 0, 4, '-'},
	{PROTBIT(OWNER, S_IEXEC), PROTBIT(OWNER, S_IEXEC), 4, 'x'},
	{S_ISUID, S_ISUID, 4, 's'},

	{0, 0, 5, '-'},
	{PROTBIT(GROUP, S_IREAD), PROTBIT(GROUP, S_IREAD), 5, 'r'},
	{0, 0, 6, '-'},
	{PROTBIT(GROUP, S_IWRITE), PROTBIT(GROUP, S_IWRITE), 6, 'w'},
	{0, 0, 7, '-'},
	{PROTBIT(GROUP, S_IEXEC), PROTBIT(GROUP, S_IEXEC), 7, 'x'},
	{S_ISUID, S_ISUID, 7, 's'},

	{0, 0, 8, '-'},
	{PROTBIT(PUBLIC, S_IREAD), PROTBIT(PUBLIC, S_IREAD), 8, 'r'},
	{0, 0, 9, '-'},
	{PROTBIT(PUBLIC, S_IWRITE), PROTBIT(PUBLIC, S_IWRITE), 9, 'w'},
	{0, 0, 10, '-'},
	{PROTBIT(PUBLIC, S_IEXEC), PROTBIT(PUBLIC, S_IEXEC), 10, 'x'},

	{0, 0, 11, 000},
};

char *
bell_mode(mode)
    unsigned short mode;
{
    static char modebuf[32];

    register struct charval *cp;

    for( cp = modechars; cp->symbol != 000; cp++ )
	if( (mode&cp->mask) == cp->val )
	    modebuf[cp->index] = cp->symbol;

    modebuf[cp->index] = 000;
    return modebuf;
}
