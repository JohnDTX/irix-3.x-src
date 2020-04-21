#! /bin/sh
# NAME
#	idbcanon - canonicalize an installation database
# SYNOPSIS
#	idbcanon rawidb
# DESCRIPTION
#	Idbcanon takes a ``raw'' (non-canonical) installation database (idb)
#	and transforms it into one usable by the idb tools.  In particular,
#
#	(1) The string 5bin is replaced by bin to fix a cross-compilation
#	    environment problem.
#	(2) Degenerate pathname components ("/./", "/. ", "/../[^\./]*/..",
#	    "//*") are collapsed.
#	(3) Leading slashes from srcpath and dstpath are removed.
#	(4) Implicitly created directories are collated and compressed.
#	    These records begin with D and may have bogus attributes.
#	(5) Sort with uniqueness on the fields from dstpath through the
#	    attributes.  Duplicate directory records are tolerated within
#	    reason (identical modes, etc.).
#
me="`basename $0 .sh`"
case $# in
  1)
	rawidb=$1;;
  *)
	1>&2 echo "usage: $me rawidb"; exit 1
esac
idbtmp=/usr/tmp/idb.$$

#
# Clean up automatically generated slop.
#
sed -e 's@5bin@bin@g' \
    -e 's@/\.\([/ ]\)@\1@g' \
    -e 's@\(/\.\.\)/[^\./][^\./]*/\.\.@\1@g' \
    -e 's@//*@/@g' \
    -e 's@ / @ . @g' \
    -e 's@ /@ @g' \
    -e 's@/ @ @g' \
    $rawidb |
    sort > $idbtmp

#
# Detect arrant redundancies other than implicit directories.  First sed out
# the implicit directories, which are distinguished either by a type of "D" or
# a "__SYSDIRLIST__" attribute.  The latter is turned into "std.sw.unix".
#
idbDirs=/usr/tmp/idbDirs.$$
idbnodirs=/usr/tmp/idbnodirs.$$
idbuniq=/usr/tmp/idbuniq.$$

sed -n \
    -e '/^D/p' \
    -e 's/^d\(.*\) __SYSDIRLIST__\(.*\)/D\1 std.sw.unix\2/p' \
    $idbtmp |
    sort -u > $idbDirs
grep -v '^[dD]' $idbtmp > $idbnodirs
uniq -u $idbnodirs > $idbuniq

cmp -s $idbnodirs $idbuniq
if test $? -ne 0; then
	idbdups=/usr/tmp/idbdups.$$
	1>&2 echo "$me: the following records in $rawidb occur more than once:"
	uniq -d $idbnodirs | 1>&2 tee $idbdups
	1>&2 echo "$me: duplicate idb records are in $idbdups"
fi

#
# Grab explicit directories, cat them to the implicit ones, and massage with
# awk as follows:
#
#	for (each set of records having the same dstpath)
#		if (the set contains explicit records)
#			union their attributes to make the final record
#		else if (the set contains more than one implicit) 
#			standardize the first one's attributes
#		else
#			remove any bogus attributes
# Then cat non-directory idb records to this mess and sort on fields from
# dstpath through the last attribute.
#
grep '^d' $idbtmp | grep -v '__SYSDIRLIST__' | cat $idbDirs - |
    awk '
	/^[dD]/ {
	    if ($1 == "D") {
		#
		# $0 is an implicit directory, either created by install or
		# from sysdirlist by the make wrapper, Mk (cmd/swtools/Mk.sh).
		# Save this record if we have not seen another, otherwise flag
		# the implicit dstpath duplication.
		#
		if (idb[$5] == "") {
		    idb[$5] = $0
		} else if (substr(idb[$5], 1, 1) == "D") {
		    dup[$5] = 1
		}
	    } else {
		#
		# An explicit record, i.e. one created by install -dir.
		# Check the saved record (if it exists) for compatible fields.
		# The first six must match exactly.  Attributes accumulate, so
		# two or more install -dirs with different idb tags are ok.
		#
		rec = idb[$5]	# old record
		nrec = $0	# new record
		if (substr(rec, 1, 1) == "d") {
		    nflds = split(rec, srec, " ")
		    for (i = 1; i <= nflds; i++) {
			if (i <= 6) {
			    if ($i != srec[i]) {
				print "'$me': incompatible records [" rec \
				    "] and [" $0 "]"
				break
			    }
			} else {
			    found = 0
			    for (j = 7; j <= NF; j++) {
				if ($j == srec[i]) found = 1
			    }
			    if (!found) nrec = nrec " " srec[i]
			}
		    }
		}
		idb[$5] = nrec
	    }
	}
	
	END {
	    for (dstpath in idb) {
		rec = idb[dstpath]
		if (substr(rec, 1, 1) == "D") {
		    nflds = split(rec, srec, " ")
		    if (nflds < 6) print "'$me': malformed record [" rec "]"
		    rec = "d"
		    #
		    # If this dstpath has duplicates, throw its attributes
		    # away and use "std.sw.unix" only.  Otherwise ensure that
		    # its attributes are kosher.
		    #
		    hasdups = dup[dstpath]
		    if (hasdups) nflds = 6
		    for (i = 2; i <= nflds; i++) {
			if (i > 6 \
			  && (index(srec[i], "symval") == 1 \
			   || index(srec[i], "maj") == 1 \
			   || index(srec[i], "min") == 1)) {
			    # strip bogus attributes
			    continue
			}
			rec = rec " " srec[i]
		    }
		    if (hasdups) rec = rec " std.sw.unix"
		}
		print rec
	    }
	}
    ' |
    cat $idbnodirs - |
    sort -u +4

if test $? -eq 0; then
	rm -f $idbtmp $idbDirs $idbnodirs $idbuniq
fi
