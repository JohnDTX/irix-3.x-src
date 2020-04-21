# sed script to fix four common types of cross-reference formatting errors

# NB. according to ed(1), \ within [^ ] has no special meaning, but it must
# be doubled in order for it not to match.

# $Revision: 1.1 $
# $Date: 89/03/27 16:39:27 $

#   \f2name\f1 (section) -> \f2name\f1(section)
#   \f2name\fP (section) -> \f2name\f1(section)
#   \f3name\f1 (section) -> \f2name\f1(section)
#   \f3name\fP (section) -> \f2name\f1(section)
s/\\f[23]\([^\\( ][^\\( ]*\)\\f[1P] *(\([1-8][A-Za-z]*\))\\f[1P]/\\f2\1\\f1(\2)/g

#   \f2name(section)\f1  -> \f2name\f1(section)
#   \f2name(section)\fP  -> \f2name\f1(section)
#   \f3name(section)\f1  -> \f2name\f1(section)
#   \f3name(section)\fP  -> \f2name\f1(section)
s/\\f[23]\([^\\( ][^\\( ]*\) *(\([1-8][A-Za-z]*\))\\f[1P]/\\f2\1\\f1(\2)/g

#   .I name(section)	 -> .IR name (section)
#   .IR name(section)	 -> .IR name (section)
#   .B name(section)	 -> .IR name (section)
#   .BR name(section)	 -> .IR name (section)
s/^\.[IB]R* \([^\\( ][^\\( ]*\)(\([1-8][A-Za-z]*\))/.IR \1 (\2)/

#   name(section)	 -> \f2name\f1(section)   [name originally unitalized]
/^\.SH DESCRIPTION/,/^\.SH.*SEE.*ALSO/ {
    s/[	 ]\([^\\( ][^\\( ]*[^\\( P1]\)(\([1-8][A-Za-z]*\))/ \\f2\1\\f1(\2)/g
    s/^\([^\\( ][^\\( ]*[^\\( P1]\)(\([1-8][A-Za-z]*\))/ \\f2\1\\f1(\2)/g
}
