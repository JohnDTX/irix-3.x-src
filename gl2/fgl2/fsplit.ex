"
" This file will take a c file and split it into lots of little files
" Of course the file has to be in a particular format
" 
" line 1 has a include on it
" each function that is to go into its own file is preceded by
" a comment that  looks like:
" /*
" ** prim <filename>
" */
" 
" enjoy: Peter Broadwell, Thu Apr  5 20:43:35 PST 1984
"
" mark end of file
$a
----ORIGINAL_END_OF_FILE----
.
" transcribe a copy of each interesting line to the end
1,$g/\*\* prim \(.*\)/t $
" throw away all but desired name
/----ORIGINAL_END_OF_FILE----/+1,$s/\*\* prim ...:\(.*\)(.*/\1/
" double last entry so we can "safely" run off end
" $t$
" pad so we can be assured of matching eight characters
/----ORIGINAL_END_OF_FILE----/+1,$s/$/++++++++/
" truncate names to eight characters
/----ORIGINAL_END_OF_FILE----/+1,$s/\(........\).*/\1/
" take off extra padding
/----ORIGINAL_END_OF_FILE----/+1,$s/+*$//
" convert each line into magic that will write out top part of file
" then delete top part
/----ORIGINAL_END_OF_FILE----/+1,$s/.*/1\
	\/\\\*\\\* prim ...:&\/\
	\/\\\*\\\* prim \/--\
	mark a\
	1,'a write! lib\/&.fc\
	2,'a delete
" change last write to deal with end of file properly
$
?write?
s/'a/$/
$d
" write out commands to a file
/----ORIGINAL_END_OF_FILE----/+1,$w! splitter.junk
" get rid of the evidence
/----ORIGINAL_END_OF_FILE----/,$d
" source file just made to do the dirty work
source splitter.junk
