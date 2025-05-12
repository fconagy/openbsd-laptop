# C shell rc script for root on OpenBSD.

# Conservative umask.
umask 022

# Strange name.
alias mail = Mail

# Long history. Less advanced that FreeBSD.
set history = 8192

# Home. We know where we are.
set home = /root

# Path.
set path = (/sbin /usr/sbin /bin /usr/bin /usr/X11R6/bin /usr/local/sbin /usr/local/bin)

# Add to path.
set path = ($path $home/bin)

# Manuals.
if ( -r /etc/man.conf ) then
	set mandef = "`grep '^manpath' /etc/man.conf | awk ' { print $2 } '`"
	set mandef = "`echo $mandef | sed 's/ /:/g'`"
else
	set mandef = (/usr/share/man /usr/X11R6/man /usr/local/man)
endif
set manpath = ($mandef $home/local/man)

# Long history. Added date for history for
# more advanced C shells like on FreeBSD.
#set history = (8192 '%Y-%W-%DT%P %R\n')
#set savehist = (8192 merge)
set histfile = ".history"

# Enable file name completion. I do not use it.
#set filec

# Standard block size.
set blocksize = 1k

# Strange name.
alias mail Mail

# Aliases. I don't use these.
#alias	cd	'set old="$cwd"; chdir \!*'
#alias	h	history
#alias	j	jobs -l
#alias	ll	ls -l
#alias	l	ls -alF
#alias	back	'set back="$old"; set old="$cwd"; cd "$back"; unset back; dirs'
#alias	z	suspend
#alias	x	exit
#alias	pd	pushd
#alias	pd2	pushd +2
#alias	pd3	pushd +3
#alias	pd4	pushd +4

# Set prompt. We know what we want.
#if ($?prompt) then
#	set prompt="`hostname -s`# "
#endif
set prompt = "`hostname -s`# "

# Terminal. We know what we got.
set term = vt220

# Downloaded packages.
set pkg_cache = "$home/pkg-cache"

