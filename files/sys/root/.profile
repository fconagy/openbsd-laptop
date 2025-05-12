# OpenBSD profile for root.

# Set path.
PATH=/sbin:/usr/sbin:/bin:/usr/bin:/usr/X11R6/bin:/usr/local/sbin:/usr/local/bin
export PATH

# Home for root, we know it is /root.
HOME=/root
export HOME

# Our binaries.
PATH=$PATH:$HOME/bin:$HOME/local/bin
export PATH

# Our manual pages.
if [ -r /etc/man.conf ]
then
	MANDEF="`grep '^manpath' /etc/man.conf | awk ' { print $2 } '`"
	MANDEF="`echo $MANDEF | sed 's/ /:/g'`"
else
	MANDEF=/usr/share/man:/usr/X11R6/man:/usr/local/man
fi
MANPATH=$MANDEF:$HOME/local/man
export MANPATH

# Conservative umask.
umask 022

# History.
HISTFILE=$HOME/.sh_history
HISTSIZE=8192
export HISTFILE HISTSIZE
alias history='fc -l 1'

# Set terminal for interactive shell.
case "$-" in
*i*)

	# Interactive shell.
	# We know what the terminal is.
	TERM=vt200
	export TERM
	;;
esac

# Package cache.
PKG_CACHE=$HOME/pkg-cache
export PKG_CACHE

