#include "idb.h"

filematch (name, pat)
	char		*name;
	char		*pat;
{
	int		c, m;
	char		*p;
	Memset		*tmpset;

	if (*name == '\0') {
		return (*pat == '\0');
	}
	if (*pat == 0) return (1);
	while ((c = *name) || (*pat)) {
		switch (*pat) {
		case '*':
			for (p = name + strlen (name); p >= name; --p) {
				if (filematch (p, pat + 1)) return (1);
			}
			return (0);
		case '[':
			++pat;
			m = 0;
			while (*pat && *pat != ']') {
				if (pat [1] == '-' && pat [2] != '\0') {
					if (c >= pat [0] && c <= pat [2])
						++m;
					pat += 3;
				} else {
					if (c == *pat) ++m;
					pat += 1;
				}
			}
			if (!m) return (0);
			if (*pat) ++pat;
			break;
		case '?':
			++pat;
			break;
		default:
			if (c != *pat++) return (0);
			break;
		}
		++name;
	}
	return (1);
}
