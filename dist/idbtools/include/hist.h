
/* Hformat -- style of history file
 *
 * Hformat == 1		Initial Version, none of which exist.  Promise.
 * Hformat == 2		Changed H_node->attr from a pointer to an array.
 * Hformat == 3		Removed H_node.version.
 *			Removed Hist.chandle.
 *			Defined Handle type.
 *			Changed H_node.ident to H_node.handle.
 *			Changed Hist.nident to Hist.nhandle.
 *			Changed Hist.ident to Hist.handle.
 *			Changed Hist.handle from an array of (char *) to
 *				an array of Handle.
 * Hformat == 4		No changes to format, except that formats 1-3 are
 *			obsoleted because of possible corruption.
 */

#define Hformat		4

/* A Handle maps an external name "prod.image.subsys" onto the structures
 * that describe it.  Only the names are actually stored in the versions file;
 * the structure pointers are filled in by scanning through Hist.spec and
 * matching up the names.
 */

typedef struct Handle {
	char		*name;		/* pname.iname.sname */
	Prod		*pr;		/* the Prod */
	Image		*im;		/* the Image */
	Subsys		*ss;		/* the Subsys */
} Handle;

/* An H_node describes one installed file, directory, device node, etc.
 * including installation information that is required later.  The hx
 * is an index into the Handle table, which gives us the product.image.subsys
 * to which this node belongs.  Only the file name is stored; the full path
 * must by built by catenating down through the parent nodes.  We currently
 * allow for 8 attributes, which are generally copied from the defining idb
 * line for the file.  An attribute here is a one byte index into the attribute
 * table, where the actual name is stored.  If this is a directory, an array
 * of children will be defined.
 */

typedef struct H_node {
	unsigned short	type;		/* file type bits, munged */
	short		hx;		/* index into handle table */
	unsigned short	chksum;		/* checksum ("sum -r" algorithm) */
	char		*name;		/* file name, cwd component only */
	short		nattr;		/* number of attributes */
	char		attr [8];	/* attribute indices */
	struct H_node	*child;		/* array of children */
	struct H_node	*chend;		/* end of array of children */
	struct H_node	*parent;	/* parent of this node */
} H_node;

/* A Hist is the top-level structure in the versions file.  The first short
 * is the format, which is subject to change from release to release.  Old
 * formats can be converted to new as the versions file is read in.  The
 * chx is the current handle index, which is a transient value used as files
 * are installed.  The table of handles resides here; these point into the
 * spec.  There is also a table of attributes, which are the character
 * representations of the attribute strings referenced by H_nodes.
 */

typedef struct Hist {
	short		format;		/* history (versions) file format */
	H_node		*root;		/* root node */
	short		chx;		/* current handle index */
	short		nhandle;	/* number of defined handles */
	Handle		*handle;	/* handle value table */
	short		nattr;		/* number of attributes */
	char		**attr;		/* attribute value table */
	Spec		*spec;		/* known product specifications */
} Hist;

/* special type values: H_node->type normally contains the S_IFMT bits of
 * the stat.st_mode field for the file; when these are written out, they
 * are right shifted to become a single-byte value, to be left-shifted as
 * the are read in.  The lower order 4 bits are available for special values.
 */

#define H_pop		1			/* ascend to parent (..) */

Hist		*histread ();
Hist		*histnew ();
H_node		*histnode ();
H_node		*histadd ();
