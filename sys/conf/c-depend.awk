# compute header file dependencies
#   This is intended to work with the C preprocessor.
#   Modified to work on concatenated cpp -M output; also no associative
#   arrays.  -be

#   1 April 1986


# start things
BEGIN {
    INDENT = "\t"
}

# for each line of pruned cpp -M output
NF > 0 {
# generate a (non-redundant) dependency entry for every header file
    if ($1 != lhs) {
	lhs = $1
	print p_line
	p_line = $1 " "
	lim = 72
    }
    if (length(p_line) + length($2) > lim) {
	print p_line "\\"
	p_line = INDENT
	lim = 63
    }
    p_line = p_line $2 " "
}


END {
    print p_line
}
