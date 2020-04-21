/*
 * Kernel debugging support
 *
 * $Source: /d2/3.7/src/sys/debug/RCS/assert.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:40 $
 */

#ifdef	OS_ASSERT
/*
 * Kernel assertion macros:
 *	- if ASSERT is defined (via ``options'' in the config file)
 *	  then these macros will do various checks on their arguments
 */
#undef	ASSERT

/* simple user style assertion */
#define	ASSERT(EXPR) \
	if ((EXPR)) ; else _assertbotch("EXPR", __FILE__, __LINE__)

/*
 * test the range and alignment of a pointer
 */
#define	_RANGEOFP(p, size_of_p, base_of_p, num_of_p, type_of_p) \
	_checkrangeofp((caddr_t)(p), (int)(size_of_p), \
		       (caddr_t)(base_of_p), (int)(num_of_p), \
		       (char *)(type_of_p), __LINE__, __FILE__)

/*
 * test the range of an integer, and stepping values if it has a step
 * function
 */
#define	_RANGEOFI(x, lower_bound, upper_bound, step_size) { \
	if ((x < lower_bound) || (x >= upper_bound) || (x % step_size)) \
		_rangeofi(__LINE__, __FILE__, (long)x, (long)step_size); \
}

/*
 * insure that the given expression is true
 */
#define	_MUSTBETRUE(e) { \
	if (!(e)) \
		_mustbetrue(__LINE__, __FILE__); \
}

/*
 * insure that the given expression is false
 */
#define	_MUSTBEFALSE(e) { \
	if (e) \
		_mustbefalse(__LINE__, __FILE__); \
}

/*
 * if e1 is true, then e2 must be true
 */
#define	_IFTRUETHENTRUE(e1, e2) { \
	if (e1) \
		if (!(e2)) \
			_asserte1e2(__LINE__, __FILE__); \
}

/*
 * if e1 is true, then e2 must be false
 */
#define	_IFTRUETHENFALSE(e1, e2) { \
	if (e1) \
		if (e2) \
			_asserte1e2(__LINE__, __FILE__); \
}

/*
 * if e1 is false, then e2 must be true
 */
#define	_IFFALSETHENTRUE(e1, e2) { \
	if (!(e1)) \
		if (!(e2)) \
			_asserte1e2(__LINE__, __FILE__); \
}

/*
 * if e1 is false, then e2 must be false
 */
#define	_IFFALSETHENFALSE(e1, e2) { \
	if (!(e1)) \
		if (e2) \
			_asserte1e2(__LINE__, __FILE__); \
}

extern	void	_checkrangeop();
extern	void	_rangeofi();
extern	void	_asserte1e2();
extern	void	_mustbetrue();
extern	void	_mustbefalse();
extern	void	_assertbotch();

#else	/* OS_ASSERT */

#define	ASSERT(x)
#define	_RANGEOFP(p, size_of_p, base_of_p, num_of_p, type_of_p)
#define	_RANGEOFI(x, lower_bound, upper_bound, step_size)
#define	_MUSTBETRUE(e)
#define	_IFTRUETHENTRUE(e1, e2)
#define	_IFTRUETHENFALSE(e1, e2)
#define	_IFFALSETHENTRUE(e1, e2)
#define	_IFFALSETHENFALSE(e1, e2)

#endif	/* OS_ASSERT */
