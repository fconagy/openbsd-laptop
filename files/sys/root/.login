# C shell login file for root.

# We know what terminal we got.
#if ( -x /usr/bin/tset ) then
#	set noglob histchars=""
#	onintr finish
#	if ( $?XTERM_VERSION ) then
#		eval `tset -IsQ '-munknown:?vt220' $TERM`
#	else
#		eval `tset -sQ '-munknown:?vt220' $TERM`
#	endif
#	finish:
#	unset noglob histchars
#	onintr
#endif
setenv TERM "vt220"

