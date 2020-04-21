/*
 * Object stuff.
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/object.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:29 $
 */

#include "pane.h"

#ifdef	DEBUG
/*
 * Return a new (unique) type number.  Type numbers are allocated for
 * each object type at run time in order of reference.
 */
long
NewType()
{
	static long types = 1;

	return (types++);
}
#endif

/*
 * allocate a new object
 */
Object_t *
NewObject(size, type, event)
	int size;
	Type type;
	int (*event)();
{
	Object_t *obj;

	obj = (Object_t *) calloc(1, size);
	if (obj) {
		SetType(obj, type);
		obj->o_event = event;
	}
	return (obj);
}

/*
 * Free the given object, and all its contents.
 * Lastly, free the data structure itself.
 */
void
FreeObject(obj)
	register Object_t *obj;
{
	/* free our contents */
	while (obj->o_contents)
		FreeObject(obj->o_contents);

	/*
	 * Now remove ourselves from the sibling list.  If we are the
	 * object that our container object points to, then adjust
	 * the container's o_contents pointer.
	 */
	if (obj->o_prev)
		obj->o_prev->o_next = obj->o_next;
	if (obj->o_next)
		obj->o_next->o_prev = obj->o_prev;
	if (obj->o_container->o_contents == obj) {
		if (obj->o_prev)
			obj->o_container->o_contents = obj->o_prev;
		else
		if (obj->o_next)
			obj->o_container->o_contents = obj->o_next;
		else
			obj->o_container->o_contents = 0;
	}

	/* lastly, free the object itself */
	free((char *) obj);
}

/*
 * Handle an event for the given object
 */
int
HandleObject(obj, e)
	Object_t *obj;
	Event *e;
{
	/*
	 * This generic handler just passes the event down to any sub-objects
	 */
	obj = obj->o_contents;
	while (obj->o_prev)
		obj = obj->o_prev;
	while (obj) {
		if (obj->o_event)
			(*obj->o_event)(obj, e);
		obj = obj->o_next;
	}
}

#ifdef	DEBUG
void
AssertBotch(line, expression, file)
	int line;
	char *expression, *file;
{
	printf("Assertion botch: \"%s\", line %d, file \"%s\"\n",
			  expression, line, file);
	abort();
	/*NOTREACHED*/
}
#endif
