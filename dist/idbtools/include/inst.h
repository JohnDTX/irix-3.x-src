/* inst.h -- installation tool and support declarations
 */

/* limits */

#define		Maxargs		32		/* command mode arguments */
#define		Strsize		1024		/* general buffers */
#define		Namesize	12		/* image and subsystem names */
#define		Idsize		64		/* image and subsystem id */
#define		Expsize		128		/* limited expressions */
#define		Maxcand		64		/* maximum candidates */
#define		Nameblk		32		/* data name blocking factor */
#define		Maxprereq	8		/* prerequisites per subsys */
#define		Maxprod		64		/* products */

/* various flag bits */

#define		Main		0x0001
#define		Default		0x0002
#define		Select		0x0004
#define		Reject		0x0008
#define		Present		0x0010
#define		Deleted		0x0020
#define		Add		0x0040
#define		Installed	0x0080

#define		Chpr		0x0001
#define		Chim		0x0002
#define		Chss		0x0004

/* Product format: the layout of the product descriptor, i.e. the
 * Prod, Image, Subsys, and Prereq structures.
 *
 * Format_prod == 1		initial version
 * Format_prod == 2		add "instdate" to Subsys, introduced Mark
 *				structure, changed Prereq, put rep array
 *				in Subsys.
 * Format_prod == 3		Added order to Image.
 */

#define		Format_prod	3	/* current product format */

/* Image format: the layout of the image itself.
 *
 * Format_image == 1		initial version; i.e. a null terminated idb
 *				followed by name/data for each file (but not
 *				nodes), where the name is in string format
 *				(i.e. preceeded by length).
 */

#define		Format_image	1	/* current image format */

/* other constants */

#define		Prompt		1	/* prompt user interface */
#define		Command		2	/* command user interface */
#define		Special		3	/* special command-line functions */

#define		Magic_prod	1987	/* magic number for products */
#define		Magic_image	1909	/* magic number for images */

#define		Root		1	/* interested in /root device */
#define		Usr		2	/* interested in /root/usr device */

#define		Is_file		1	/* source is a file */
#define		Is_dir		2	/* source is a directory */
#define		Is_tape		3	/* source is tape */

#define		Padsize		4096	/* default buffer size */

#define		ProdFileno	2	/* product position on tape */
#define		ImageFileno	3	/* image[0] position on tape */

#define		Silent		1	/* run process silently */
#define		Noisy		2	/* run process noisily */

/* distinguished names */

#define		instbase	"usr/lib/inst"
#define		histfile	"usr/lib/inst/versions"
#define		lockfile	"usr/lib/inst/lock"
#define		helpfile	"/usr/lib/inst/help"
#define		DefSource	"/dev/nrtape"

/* macros */

#define padded(n, p)	((((n) + p - 1) / p) * p)
#define maxint(type)	((type) ~(1 << (sizeof (type) * 8 - 1)))

/* types */

typedef unsigned int	Checksum;	/* actually uses only 16 bits */

typedef struct Mark {			/* subsystem markers: */
	char		*pname;		/* product name */
	char		*iname;		/* image name */
	char		*sname;		/* subsystem name */
	long		lowvers;	/* lowest matching version */
	long		highvers	/* highest matching version */
} Mark;

typedef struct Prereq {
	Mark		*pq;		/* table of brother prerequisites */
	Mark		*pqend;		/* end of table */
} Prereq;

typedef struct Subsys {			/* subsystem of image: */
	short		flags;		/* various flag bits */
	char		*name;		/* name of subsystem */
	char		*id;		/* subsystem identification */
	char		*exp;		/* selection expression */
	long		instdate;	/* installation date */
	Mark		*rep;		/* replaced subsystems */
	Mark		*repend;	/* end of replaced subsystems */
	Prereq		*prereq;	/* table of prerequisites */
	Prereq		*preend;	/* end of prerequisites */
} Subsys;

typedef struct Image {			/* an image: */
	short		flags;		/* various flag bits */
	char		*name;		/* name of image */
	char		*id;		/* image identification */
	short		format;		/* image format */
	short		order;		/* installation ordering */
	long		version;	/* version */
	long		length;		/* length of image in bytes */
	long		padsize;	/* logical blocking factor */
	Subsys		*subsys;	/* table of subsystems */
	Subsys		*subend;	/* end of subsystems */
} Image;

typedef struct Prod {			/* a product: */
	char		*name;		/* product name */
	char		*id;		/* product id */
	short		flags;		/* various flag bits */
	Image		*image;		/* table of images */
	Image		*imgend;	/* end of images */
} Prod;

typedef struct Spec {			/* a specification: */
	Prod		*prod;		/* table of product */
	Prod		*prodend;	/* end of products */
} Spec;

char		*vreaddir ();
char		*cat ();
long		space ();
char		*machname ();
long		vseek ();
