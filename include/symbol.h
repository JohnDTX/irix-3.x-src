/*
 * $Source: /d2/3.7/src/include/RCS/symbol.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:12:02 $
 */

/*
 * Structure of a symbol table entry
 */

struct	symbol {
	char	sy_name[8];
	char	sy_type;
	int	sy_value;
};
