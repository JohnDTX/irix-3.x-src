# this is the first line of a desktop file describeing how
# programs behave as they startup etc.
# hashmarks(#) at the begining of the line in these
# files make the lines into comments.

# comments and blank lines are ignored, except by the human reader.

foo|bar|name_as_passed_to_getport: \
	:prefsize screenx screeny: \
	:all_other_hints appropriate arguments: \
	:Capitolized_hints appropriate arguments: \
	:getport screenx screeny screenx screeny: \
	:offset xoff yoff: \
	:menu menu_string system string with name replacing all $'s:

#
# Capitolized hints have precidence over hints in the program.
# The x,y pairs after getport are qued as mex dragout box events.
# Offset specifies how much of an offset to give second(third...)
# proccesses withthe same name.
# Menu makes menue string appear in a second("auxilary") mex menu
# when you are over that window but attached to mex.
# Choosing that menu entry would cause mex to system the string.
#	( this is a generalize doc feature, ie.
#	       :menu help ivyview /usr/lib/gl2/doc/$: \
#	would impliment our current doc (in a noncomital way)
#	       :menu refine ivy $.c;mv $ o$;make $;$: \
#	would impliment easy way to run around the edit/compile/test cycle
#	although you end up with many old carcasses left around)

# A new mex "desktop" menu item would replace the current entry
# with the whatever the curent port looked like.

# This might look familiar, I am trying to mimic termcap, although
# more verbosly

# peter
