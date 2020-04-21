/*
 * Code supporting the assertion feature.  Contains assert testing and
 * printout routines, done here to save kernel space.
 *
 * $Source: /d2/3.7/src/sys/debug/RCS/dbg_assert.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:41 $
 */
#include "../h/types.h"
#include "../h/printf.h"
#include "../debug/debug.h"

#ifdef	OS_ASSERT

static void
_assertfault(type, line, file)
	char *type;
	int line;
	char *file;
{
	forceconsole();
	printf("assertion fault: type=``%s'', line=%d file=``%s''\n",
			  type, line, file);
	debug("assert");
}

void
_checkrangeofp(p, size_of_p, base_of_p, num_of_p, type_of_p, line, file)
	caddr_t p;
	int size_of_p;
	caddr_t base_of_p;
	int num_of_p;
	char *type_of_p, *file;
	int line;
{
	if ((p < base_of_p) || (p >= base_of_p + num_of_p*size_of_p) ||
	    ((p - base_of_p) % size_of_p)) {
		forceconsole();
		printf("rangeofp: p=%x type=``%s''\n", p, type_of_p);
		_assertfault("rangeofp", line, file);
	}
}

void
_rangeofi(line, file, i, step)
	int line;
	char *file;
	long i, step;
{
	forceconsole();
	printf("rangeofi: i=%d 0x%x 0%o, step=%d 0x%x 0%o\n",
			  i, i, i, step, step, step);
	_assertfault("rangeofi", line, file);
}

void
_mustbetrue(line, file)
	int line;
	char *file;
{
	_assertfault("mustbetrue", line, file);
}

void
_mustbefalse(line, file)
	int line;
	char *file;
{
	_assertfault("mustbefalse", line, file);
}

void
_asserte1e2(line, file)
	int line;
	char *file;
{
	_assertfault("ifthen", line, file);
}

/*
 * Assertion failure notifier.
 */
void
_assertbotch(expr, file, line)
	char *expr, *file;
	int line;
{
	forceconsole();
	printf("assertion botch: %s, file \"%s\", line %d\n",
			  expr, file, line);
	debug("assertbotch");
	panic("botch for vjs");
}

forceconsole()
{
	if (kswitch)
		setConsole(CONSOLE_ON_SERIAL);
}

#endif	/* OS_ASSERT */
