#!/bin/sh

# Install packages.

# Const.
SUCCESS=0
FAILURE=1

# Package download directory.
PKG_CACHE=/h/pkg-cache
export PKG_CACHE

# Firmware directory. Download from firmware.openbsd.org.
FIRMWARE=/h/firmware
export FIRMWARE

# Pkg add executable.
PKGX="`which pkg_add`"

# Install firmware command.
FWPKG="`which fw_update`"

# Log file.
LOG="`pwd`/pkg-add.log"

# Check.
PKG_FULLPATH="`realpath $PKG_CACHE`"
if [ ! -d $PKG_FULLPATH ]
then
	echo "Cannot find package cache directory $PKG_CACHE"
	exit $FAILURE
fi
FIRM_FULLPATH="`realpath $FIRMWARE`"
if [ ! -d $FIRM_FULLPATH ]
then
	echo "Cannot find firmware directory $FIRMWARE"
	exit $FAILURE
fi

# Disable it to print already installed and not found local packages.
QUIET=yes

# Override pkg_add.

pkg_add()
{

	# For all packages.
	for N in $*
	do

		case $N in
		*firmware*)

			# Firmware.
			cd $FIRMWARE
			if [ $? -ne 0 ]
			then
				echo "No firmware directory $FIRMWARE"
				exit $FAILURE
			fi
			if [ `ls ${N}* | wc -l` -ne 1 ]
			then
				echo "Zero or more than 1 firmware files found"
				exit $FAILURE
			else

				# With version number and .tgz extension.
				NN="`echo ${N}*`"

				# With version and without .tgz.
				NNN="`echo ${NN} | sed 's/\.tgz//'`"
			fi
			if ls /var/db/pkg | \
				egrep -e "^${NNN}\$" >/dev/null
			then
				if [ $QUIET != yes ]
				then
					echo "$N installed"
				fi
			else
				echo "Installing local firmware package $N"
				$FWPKG $NN
			fi
			;;
		*.tgz)

			# Local install set.
			if [ ! -r $N ]
			then
				if [ $QUIET != yes ]
				then
					echo "$N notfound"
				fi
			else
				echo "Installing local package $N"
				$PKGX $N
			fi
			;;
		*)

			# Check if the package is in the local db.
			if ls /var/db/pkg | rev | cut -d '-' -f 2- | rev | \
				egrep -e "^${N}\$" >/dev/null
			then

				# Already installed witout version number.
				if [ $QUIET != yes ]
				then
					echo "$N alreadyinstalled"
				fi
			elif [ -d /var/db/pkg/${N} ]
			then

				# Already installed with exect version number.
				if [ $QUIET != yes ]
				then
					echo "$N alreadyinstalled"
				fi
				:
			else

				# Not installed?
				case $N in
				*--*)

					# This is a special case name--ending,
					# Transform it to the right match pattern.
					NN="`echo $N | sed 's/--/-.*-/'`"

					# Download name.
					DNAME="`echo $N | sed 's/--.*$/-/'`"

					# Go for the check.
					NNN="`ls /var/db/pkg | egrep -e \"^${DNAME}\"`"
					if [ "$NNN" = "" ]
					then

						# Not installed so it's OK to go ahead.
						:
					else

						# Actually it is already installed so it was a false
						# positive in this case.
						if [ $QUIET != yes ]
						then
							echo "$N alreadyinstalled"
						fi

						# Quit here.
						break
					fi
					;;
				esac

				# Download package.
				L=${N}.download.log

				# Next three lines are commented out.
				# Actually he downloads the package anyway.
				#echo "Downloading $N"
				#$PKGX -I -n $N 2>&1 | tee $L
				# Actually it always returns 0.
				# With complicated dependencies download only will
				# fail, expecting some dependencies already installed.
				# Have to parse output.
				#if cat $L | grep find >/dev/null
				#then
				#	echo "Downloading $N failed"
				#	cat $L
				#	exit 1
				#fi

				# Install package.
				echo "Installing $N"
				$PKGX -I $N 2>&1 | tee $LOG
				if [ $? -ne 0 ]
				then
					echo "Installing $N failed"
					exit $FAILURE
				else

					# Appearantly always 0.
					if cat $LOG | grep "Can't find" >/dev/null
					then
						echo "Cannot find package $N"
						exit $FAILURE
					fi
					echo "$N installed"
				fi
				if [ -w $L ]
				then
					rm $L
					if [ $? -ne 0 ]
					then
						echo "Cannot delete $L"
						exit $FAILURE
					fi
				fi
			fi
			;;
		esac
	done
}

# Options.
case $1 in
-v|--verbose|-verbose)
	QUIET=no
	shift
	;;
show)
	COUNT="`ls ${PKG_CACHE}/ | wc -l`"
	clear
	while true
	do
		tput home
		ls -ltr ${PKG_CACHE}/ | tail
		printf "%8d\n" $COUNT
		sleep 2
	done
	;;
"")
	# OK.
	;;
*)
	echo "Unknown option $1"
	exit $FAILURE
	;;
esac

# Direct install from package file.
# Firmware packages are under /h/firmware.
# If not found the entry is ignored.
pkg_add inteldrm-firmware
pkg_add uvideo-firmware
pkg_add vmm-firmware
pkg_add bwfm-firmware
pkg_add bwi-firmware
pkg_add athn-firmware
pkg_add iwn-firmware
pkg_add iwm-firmware
# This we don't have in version 7.4.
#pkg_add urtwn-firmware

# Basic system utilities.
pkg_add e2fsprogs
pkg_add rsync--
#pkg_add login_fingerprint
pkg_add wpa_supplicant-2.9p4
pkg_add vim--no_x11
pkg_add ntp
pkg_add base64
pkg_add fdupes
# There is no lsof.
#pkg_add lsof

# Basic graphics environment.
pkg_add xfce
pkg_add firefox-esr
pkg_add thunderbird
pkg_add terminus-font--

# More browsers.
pkg_add netsurf
pkg_add iridium
pkg_add dillo
pkg_add lynx

# Enable numlock when X starts.
pkg_add numlockx

# Office.
pkg_add libreoffice
pkg_add libreoffice-java
pkg_add xpdf
pkg_add gv
pkg_add xpad
pkg_add xfce4-screenshooter
pkg_add sunclock

# Virtual keyboard.
pkg_add xvkbd
pkg_add xfce4-xkb

# The canonical modern ksh.
pkg_add ksh93

# Bash.
pkg_add bash

# Utilities.
pkg_add pciutils
pkg_add gnome-font-viewer
pkg_add kermit
pkg_add xfce4-genmon
pkg_add xfce4-battery
pkg_add slock
pkg_add mupdf-1.24.9
pkg_add evince-46.3.1p0-light
pkg_add nmap

# Terminal emulators.
pkg_add roxterm
pkg_add x3270
pkg_add cool-retro-term

# Archivers.
pkg_add unzip unrar bzip2
pkg_add zip
pkg_add p7zip

# Program development.
pkg_add automake-1.16.5
pkg_add gmake
pkg_add git
pkg_add uemacs
pkg_add qgit
pkg_add gcc-8.4.0p25
pkg_add groff
pkg_add scons
pkg_add png
pkg_add bison
pkg_add libtool
pkg_add meson
pkg_add vala
pkg_add gtk-doc
# Does not seem to exist.
#pkg_add fpc
pkg_add gpc
pkg_add go
pkg_add rust
# Many versions.
#pkg_add erlang
pkg_add erlang-27.0p2v0
pkg_add pandoc
pkg_add cmake
pkg_add py3-pip
# Note that this is just a wrapper,
# it calls synchronous i/o routines.
pkg_add libaio_compat
pkg_add fltk

# Calculators.
pkg_add galculator gnome-calculator

# Emulators.
pkg_add simh
pkg_add qemu

# Let Maxima pull these.
#pkg_add tcl tk
pkg_add maxima

# Multimedia.
pkg_add vlc mplayer tkdvd
pkg_add audacity
pkg_add sdl2-mixer
pkg_add libva-utils
#pkg_add intel-gmmlib-22.3.20
pkg_add intel-media-driver
#pkg_add intel-one-mono-1.3.0
pkg_add intel-vaapi-driver
#pkg_add intel2gas-1.3.3p3
#pkg_add intellij-2024.2.1
#pkg_add py3-intelhex-2.3.0p2
pkg_add cmixer

# Pictures.
pkg_add xv
pkg_add shotwell geeqie
# Not compatible with current gcc.
pkg_add inkscape
# Needs exact version.
pkg_add gimp-2.99.18p3
pkg_add dia
pkg_add fontforge
pkg_add filezilla
pkg_add ntfs_3g
pkg_add exfat-fuse
pkg_add simple-mtpfs

# Yubikey.
pkg_add pcsc-lite
pkg_add pcsc-tools
rcctl enable pcscd
pkg_add yubico-piv-tool
pkg_add yubikey-manager
pkg_add yubikey-personalization-gui
pkg_add opensc

# Chess.
pkg_add xboard gnuchess stockfish

# Amusements.
# Stellarium pulls qt runtime and pulseaudio.
#pkg_add stellarium

exit 0

