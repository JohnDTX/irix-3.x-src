#include "vector.h"

extern	void	error(char *);
extern	void	bcopy(char *, char *, int);

vector::vector(int startingSize)
{
	if (startingSize <= 0)
		error("bad vector size");
	else {
		v_size = startingSize;
		v_v = new int[startingSize];
	}
}

vector::~vector()
{
	delete v_v;
}

void
vector::setSize(int newSize)
{
	int *newv;

	if (v_size == newSize)
		return;

	newv = new int[newSize];
	if (!newv)
		error("eat hot death");
	if (v_size < newSize) {
		bcopy((char *) v_v, (char *) newv, sizeof(int *) * v_size);
	} else {
		bcopy((char *) v_v, (char *) newv, sizeof(int *) * newSize);
	}
	delete v_v;
	v_size = newSize;
	v_v = newv;
}

#ifdef	notdef
int&
vector::operator[](int index)
{
	if ((index < 0) || (index >= v_size))
		error("vector index out of range");
	else
		return v_v[index];
}
#endif
