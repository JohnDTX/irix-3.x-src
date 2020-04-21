/* ioctls for the ikc driver */
# ifndef ikiocode
# define ikiocode(n)	('k'<<8|(n))

# define IKIOPEEK	ikiocode('a')
# define IKIOPOKE	ikiocode('b')
# define IKIOPIOMODE	ikiocode('c')
# define IKIORESET	ikiocode('d')
# define IKIOSETVSTATE	ikiocode('e')
# define IKIOGETVSTATE	ikiocode('f')

struct poke
{
	int f, v;
};

struct vstate
{
	int f;
	int dummy[2];
};
# endif ikiocode
