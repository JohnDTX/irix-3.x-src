/* find all user names in /etc/passwd
 *	This is not as simple as running sed(1) over /etc/passwd in the
 *	presence of Yellow Pages
 */

#include <pwd.h>
#include <stdio.h>

extern struct passwd *getpwent();


main()
{
	register struct passwd *ent;

	setpwent();
	while (ent = getpwent())
		printf("%s %s\n", ent->pw_name, ent->pw_dir);
	exit(0);
}
