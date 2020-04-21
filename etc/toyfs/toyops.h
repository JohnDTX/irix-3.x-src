# ifndef RONLY

# define DECL(prefix) struct toyops prefix/**/_wops = OPDEFS(prefix)

# define FUNCDEFS(prefix) \
	extern FS *prefix/**/_wmount(); \
	extern int prefix/**/_umount(); \
	extern int prefix/**/_sync(); \
	extern I *prefix/**/_imake(); \
	extern I *prefix/**/_iread(); \
	extern int prefix/**/_iwrite(); \
	extern int prefix/**/_itrunc(); \
	extern off_t prefix/**/_readi(); \
	extern off_t prefix/**/_writei(); \
	extern TOYDIR *prefix/**/_readdir(); \
	extern int prefix/**/_writedir(); \
	extern int prefix/**/_idump()

# define OPDEFS(prefix) \
	{ prefix/**/_wmount, prefix/**/_umount, prefix/**/_sync, \
	  prefix/**/_imake, prefix/**/_iread, prefix/**/_iwrite, \
	  prefix/**/_itrunc, \
	  prefix/**/_readi, prefix/**/_writei, \
	  prefix/**/_readdir, prefix/**/_writedir, \
	  prefix/**/_idump }

# else   RONLY

# define DECL(prefix) struct toyops prefix/**/_rops = OPDEFS(prefix)

# define FUNCDEFS(prefix) \
	extern FS *prefix/**/_mount(); \
	extern int prefix/**/_umount(); \
	extern int toy_nullsync(); \
	extern I *toy_nullimake(); \
	extern I *prefix/**/_iread(); \
	extern int toy_nulliwrite(); \
	extern int toy_nullitrunc(); \
	extern off_t prefix/**/_readi(); \
	extern off_t toy_nullwritei(); \
	extern TOYDIR *prefix/**/_readdir(); \
	extern int toy_nullwritedir(); \
	extern int prefix/**/_idump()

# define OPDEFS(prefix) \
	{ prefix/**/_mount, prefix/**/_umount, toy_nullsync, \
	  toy_nullimake, prefix/**/_iread, toy_nulliwrite, \
	  toy_nullitrunc, \
	  prefix/**/_readi, toy_nullwritei, \
	  prefix/**/_readdir, toy_nullwritedir, \
	  prefix/**/_idump }

# endif  RONLY
