#include "stream.h"
#include "rect.h"

ostream&
operator<<(ostream& s, point a)
{
	return s << "(" << a.x() << "," << a.y() << ")";
}

istream&
operator>>(istream& s, point& a)
/*
	( f , f )
*/
{
	int ix = 0, iy = 0;
	char 	c = 0;

	s>>c;
	if (c == '(') {
		s>>ix>>c;
		if (c == ',') s>>iy>>c;
		if (c != ')') s.clear(_bad);
	}
	else {
		s.clear(_bad);
	}

	if (s) a = point(ix,iy);
	return s;
}

ostream&
operator<<(ostream& s, rectangle a)
{
	return s << "(" << a.origin() << a.extent() << ")";
}

istream&
operator>>(istream& s, rectangle& a)
/*
	((f,f)(f,f))
*/
{
	point org, ext;
	char 	c = 0;

	s>>c;
	if (c == '(') {
		s>>org>>c;
		if (c == '(') {
			s.putback(c);
			s>>ext>>c;
			if (c != ')')
				s.clear(_bad);
		} else {
			s.clear(_bad);
		}
	}
	else {
		s.clear(_bad);
	}

	if (s) a = rectangle(org, ext);
	return s;
}
