#include "inst.h"
#include "idb.h"
#include <ctype.h>

split (string, part1, divider, part2)
	char		*string;
	char		*part1;
	char		divider;
	char		*part2;
{
	while (*string) {
		if ((*part1 = *string++) == divider) break;
		++part1;
	}
	*part1 = '\0';
	while (*string) {
		*part2++ = *string++;
	}
	*part2 = '\0';
}
