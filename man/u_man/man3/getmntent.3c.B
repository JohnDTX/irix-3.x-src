'\"macro stdmacro
.TH GETMNTENT 3C
.SH NAME
setmntent, getmntent, addmntent, endmntent, hasmntopt \- get file system descriptor file entry
.SH SYNOPSIS
.nf
.B #include <stdio.h>
.B #include <mntent.h>
.PP
.B FILE *setmntent(filep, type)
.B char *filep;
.B char *type;
.PP
.B struct mntent *getmntent(filep)
.B FILE *filep;
.PP
.B int addmntent(filep, mnt)
.B FILE *filep;
.B struct mntent *mnt;
.PP
.B char *hasmntopt(mnt, opt)
.B struct mntent *mnt;
.B char *opt;
.PP
.B int endmntent(filep)
.B FILE *filep;
.fi
.SH DESCRIPTION
These routines replace the
.I getfsent
routines for accessing the file system description file
.IR /etc/fstab .
They are also used to access the mounted file system description file
.IR /etc/mtab .
.PP
.I Setmntent
opens a file system description file and returns
a file pointer which can then be used with
.IR getmntent ,
.IR addmntent ,
or
.IR endmntent .
The 
.I type
argument is the same as in
.IR fopen (3S).
.I Getmntent
reads the next line from
.I filep
and returns a pointer to an object with the following structure
containing the broken-out fields of a line in the filesystem description file,
.IR <mntent.h> .
The fields have meanings described in
.IR fstab (4).
.RS
.PP
.ta \w'#define'u +\w'char\0\0'u +\w'*mnt_fsname;\0\0'u
.nf
struct mntent {
	char    *mnt_fsname;    /* file system name */
	char    *mnt_dir;       /* file system path prefix */
	char    *mnt_type;      /* efs, nfs, swap, or xx */
	char    *mnt_opts;      /* ro, quota, etc. */
	int	mnt_freq;	/* dump frequency, in days */
	int	mnt_passno;	/* pass number on parallel fsck */
};
.fi
.RE
.PP
.I Addmntent
adds the
.I mntent
structure
.I mnt
to the end of the open file
.IR filep .
Note that
.I filep
has to be opened for writing if this is to work.
.I Hasmntopt
scans the
.I mnt_opts
field of the
.I mntent
structure
.I mnt
for a substring that matches
.IR opt .
It returns the address of the substring if a match is found,
0 otherwise.
.I Endmntent
closes the file.
.SH FILES
/etc/fstab
.br
/etc/mtab
.SH "SEE ALSO"
fstab(4) 
.SH DIAGNOSTICS
Null pointer (0) returned on EOF or error.
.SH BUGS
The returned
.I mntent
structure points to static information that is overwritten in each call.
