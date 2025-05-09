



## OpenBSD 7.6 on the old Dell laptop


#### Hardware

It is a Dell Latitude E4200 laptop.

    BIOS Version: A24
    Processor: Intel Core2 Duo U9400 1.40 GHz
    Memory: 3 GB
    Graphics: Intel GM45 1280x800
    Disk: 128 GB Samsung SSD THIN VAM0 as sd0
    Audio: IDT 92HD71
    WiFi: Intel wireless
    Broadband: Dell wireless 5530 HSPA mobile broadband minicard.


#### BIOS setup

Press F2 to get to ROM BIOS setup.  
Set boot list to include USB drive first.  
Enable legacy USB support.  
SATA mode is AHCI.  
Set administrator and user passwords.  
Disable secure boot.  
Disable TPM.  
The rest is left as default.  


#### Boot

Press F2 to get to ROM BIOS.  
Select 'Boot menu'.  
Choose 'UEFI Kingston' for the memory stick.  


#### Base install

Boot OpenBSD 7.6 memory stick.  
Choose to get to shell.  
Check disks:

    dmesg | grep sd

we got:

    sd0 128 GB Samsung SSD THIN VAM0
    sd5 512 GB Kingston DataTraveler memory stick

Enable existing disks.

    cd /dev
    sh MAKEDEV sd0
    sh MAKEDEV sd5
    cd /

Sanitize disk(s).

    disklabel sd0
    dd if=/dev/zero of=/dev/rsd0c bs=1M count=64
    disklabel sd1
    dd if=/dev/zero of=/dev/rsd1c bs=1M count=64

The destination disk is sd0.

Exit from the shell, back to the install program.

Select install.  
United Kingdom keymap: uk  
Select host name: chios  
Configure network interfaces: done.  
DNS domain name: lan  
DNS name servers: none  
Set root password.  
Start SSHD by default: no.  
Do you want X window system to be started: no  
Change the default console to com0: no  
Setup a user, as you wish.  
Which disk is the root disk: sd0  
Encrypt the root disk: no  
Use whole disk for MBR: whole  
Custom layout: C  
Enters label editor.  
Add partitions:

    Part Offset Size  Type   Mountpoint
    a    64     4096M 4.2BSD /
    b           3072M swap
    d           4096M 4.2BSD /tmp
    e           4096M 4.2BSD /var
    f           rest  4.2BSD /usr

Write, quit.  
Leave alone the rest (done).  

Install the sets.  
Location of sets: disk  
Is the partition already mounted: no  
Which disk contains the install media: sd5  
Which partition has the install sets: a  
Pathname to the sets: Accept default.  
Select sets. All sets selected by default, accept default  
Continue without verification: yes  

Installation starts.  
After it did finish asks again for location of sets.  
Accept the default which is 'done'.  
What time zone you are in: UTC  

Firmware update `fw_update` will fail, as there is no network
connection.

Instead of running `fw_update` manually afterwards we
will install just the selected firmware sets later on.

Accept the default to reboot.

Boots with BIOS.

Remove USB stick when asked BIOS password.


#### Save disk information

Save the top blocks containing the GPT or MBR partition table.

    dd if=/dev/rsd0c of=disk-64-blocks.dd bs=512 count=64

Save label and top of the disk on the OpenBSD partition.

    dd if=/dev/rsd0c of=disk-2048-blocks.dd bs=512 count=2048


#### Copy old root

Copy the old root home directory over from the old device.
Possibly using the memory stick. Put the new root home in
place. Exit and reboot.


#### Hardening

Run hardening script.

    cd scripts
    ./hardening.sh

Mostly unwanted daemons.

Check:

    netstat -an | egrep -e '^tcp|^udp'

There should be no open ports.

    ps -axu

There should be no unnecessary daemons running.


#### Prepare destination directories

With the current laptop layout.

    mkdir -p /usr/save/chios
    mkdir /usr/save/chios/firmware /usr/save/chios/pkg-cache /usr/save/chios/save
    mkdir /h
    ln -s /usr/save/chios/firmware /h/firmware
    ln -s /usr/save/chios/pkg-cache /h/pkg-cache
    ln -s /usr/save/chios/save /h/save

We also need it from under root.

    ln -s /usr/save/chios/pkg-cache /root/pkg-cache


#### Get and install firmware

Create directories if it was not done.

    mkdir /usr/save/firmware
    ln -s /usr/save/firmware /h/firmware

Download firmware:

    cd /h/firmware
    wget -nH -nd -np -r -l 1 http://firmware.openbsd.org/firmware/7.4/
    rm index.html*

Install only the necessary firmware.
The firmware will be installed by the `packages-install.sh` script.


#### Install packages

Use the script to install packages.

    cd scripts
    ./packages-install.sh

For some packages there might be different versions offered with
the new OS version.

In which case the installer offers to choose which version
to install. But this prompt is not visible so the installation
process seems to be stucked. In this case the exact version
should be specified in the script.


#### Prepare for kernel build

Get the release source tarballs.

    cd syssrc
    ./get-syssrc.sh

Will also check the signatures.
Unpack tarballs into `/usr/src`, `/usr/ports`, `/usr/xenocara`.

    ./source-unpack.sh

Create `syssrc` account as uid 300 gid 300.
There should be already a `wsrc` group, make it a member.
Set protections so future CVS updates can go to this place.

    ./set-protections.sh


#### Dealing with modified files

There is a script called mod to deal with config files.
Before changing a file do:

    mod fullpath

which will save the file, then edit the config file.

    mod dir

will show a list of 'modded' files.
A snapshot of current modified config files in the system
can be created like:

    mod checkpoint

which will put the files under a directory `~/mod` identified
by host name and timestamp.

It is possible to install modded files from a saved checkpoint.
Go to the latest checkpoint directory where you will see the files
with timestamp and mangled filenames. There is a script
mod-patch which can be used to check the changes for the
system file.

    mod-patch diff fullpath

will show the differences.

    mod-patch patch fullpath

will save the original file like `mod file` then carry
out the changes to the system file.


#### Kernel build

Create the kernel config file.

    cd /usr/src/sys/arch/amd64/conf
    cp GENERIC.MP CHIOS.MP
    cd ~/scripts

Change the kernel name in the script.

    ./kernel-rebuild.sh 2>&1 | tee kernel-rebuild.log

Reboot.

Test build seems ok.

Kernel config file changes.

    cd /usr/src/sys/arch/amd64/conf
    vi NAXOS.MP

We leave this alone, this includes `GENERIC`.

We use `/usr/src/sys/conf/GENERIC` so machine independent changes will be
included in all kernels.

Disable options `INET6` and `IPSEC`.
Also disable pseudo-device sec.

    mod /usr/src/sys/conf/GENERIC
    vi /usr/src/sys/conf/GENERIC

Now for machine dependent generic config.

    mod /usr/src/sys/arch/amd64/conf/GENERIC
    vi /usr/src/sys/arch/amd64/conf/GENERIC

Additions for `GENERIC`

    mod /usr/src/sys/conf/GENERIC
    vi /usr/src/sys/conf/GENERIC

Add under `HIBERNATE`:

    # !!!! Green is the color.
    option      WS_KERNEL_FG=WSCOL_GREEN
    option      WS_KERNEL_BG=WSCOL_BLACK

    # !!!! Quiet boot.
    option      BOOT_QUIET

Green is the colour.

    mod /usr/src/sys/dev/wscons/wsdisplayvar.h
    vi /usr/src/sys/dev/wscons/wsdisplayvar.h

On the top change `WS_DEFAULT_FG` to green.

In the file below there is an out: label defined
for IPV6 error handling, take that out when there is
no IPV6.

    mod /usr/src/sys/net/pf.c
    vi /usr/src/sys/net/pf.c

Rebuild kernel and reboot.


#### Standalone things.

Check how to build the standalone things.

    cd /usr/src/sys/arch/amd64/stand
    make clean
    make
    make install

to build the standalone stuff for booting.
This is the `.EFI` files in the FAT formatted
partition and the boot block for the OS.
Issuing `make install` should find the EFI boot
partition and update the EFI modules there.
If there was any, that is. It is created
at build time under

    cd /usr/src/sys/arch/amd64/stand/efiboot/bootx64

    ls *.EFI

In the case of legacy BIOS boot use:

    disklabel sd0
    installboot -n -v sd0 # Check
    installboot -v sd0

Will update primary and secondary bootstrap from
biosboot and boot under /usr/mdec.
To update the legacy BIOS boot record:

    fdisk -y -u -f /usr/mdec/mbr sd0

to install the boot code. The output would look like:

    chios# installboot -v sd0
    Using / as root
    installing bootstrap on /dev/rsd0c
    using first-stage /usr/mdec/biosboot, second-stage /usr/mdec/boot
    copying /usr/mdec/boot to //boot
    looking for superblock at 65536
    found valid ffs2 superblock
    //boot is 6 blocks x 16384 bytes
    fs block shift 2; part offset 64; inode block 56, offset 2928
    expecting 64-bit fs blocks (incr 4)
    master boot record (MBR) at sector 0
       partition 3: type 0xA6 offset 64 size 250069616
    /usr/mdec/biosboot will be written at sector 64

Reboot.


#### Silent boot

To redirect console to a serial terminal usually
works but with OpenBSD when there is no actual
serial connection and the boot process blocks at some
point and the system freezes. So things needed
to be disabled in files in the standalone source
tree.

There is a preprocessor constant BOOT_QUIET which
does part of the job. Add

    option BOOT_QUIET

to the kernel config file.

There are two directories, in the source tree to
consider, the machine specific

    /usr/src/sys/arch/amd64/stand

and the generic

    /usr/src/sys/stand

The modified files are

    /usr/src/sys/arch/amd64/conf/NAXOS.MP
    /usr/src/sys/conf/GENERIC
    /usr/src/sys/dev/wscons/wsdisplayvar.h
    /usr/src/sys/stand/boot/boot.c
    /usr/src/sys/arch/amd64/stand/efiboot/efiboot.c
    /usr/src/sys/arch/amd64/stand/libsa/machdep.c
    /usr/src/sys/arch/amd64/stand/biosboot/biosboot.S
    /usr/src/sys/ddb/db_elf.c
    /usr/src/sys/arch/amd64/stand/mbr/mbr.S
    /usr/src/sys/arch/amd64/stand/efiboot/machdep.c
    /usr/src/sys/arch/amd64/stand/efiboot/diskprobe.c
    /usr/src/sys/arch/amd64/stand/libsa/bioscons.c
    /usr/src/sys/arch/amd64/include/loadfile_machdep.h
    /usr/src/sys/lib/libsa/printf.c
    /usr/src/sys/arch/amd64/stand/efiboot/exec_i386.c
    /usr/src/sys/arch/amd64/stand/libsa/exec_i386.c
    /usr/src/sys/arch/amd64/stand/libsa/memprobe.c
    /usr/src/sys/arch/amd64/stand/libsa/diskprobe.c

Modifications marked with '!!!!' as usual, they are
mostly printf-s commented out. In `printf.c` there is
a flag 'donottwiddle' which is enabled.

Rebuild kernel and reboot.

The init system is silenced with a wrapper.
The old one is renamed to rc.silent.

    mv /etc/rc /etc/rc.silent

The new /etc/rc is:

    exec /bin/sh /etc/rc.silent $* >/dev/null 2>&1
    #exec /bin/sh /etc/rc.silent $*

Note that file systems are remounted r/w only at a
later stage so we can't have a log file at this point.

Create `/etc/boot.conf` and put a boot command in it.
It is always hd and the BIOS boot list number.

    boot hd0a:/bsd


#### Console

Default word selection with the mouse is not very good
for doubleclick selection of filenames. In

    /usr/src/sys/dev/wscons/wsdisplay.c

there is a charClass table a la `xterm`. Modify that
to include the usual non-alphabetic characters in
filenames.

For the console the size of the font to be used is
determined by the resolution of the screen, so as not
to have too minuscule font on a high resolution screen.
The default font is a bit too large, so to make it smaller
changes are to be made in rasops subsystem.
The logic is in the file

    /usr/src/sys/dev/rasops/rasops.c

just one line change to make the font smaller.
Note that the right size is 12, that gives 43x140 on
the standard screen.

Rebuild kernel and reboot.


#### Host firewall packet filtering.

Update `/etc/pf.conf`. Simple rules, interface agnostic.
Check:

    pfctl -s all


#### Second disk

In case we got a second disk format and mount it.

    cd scripts
    ./newfs-toshiba.sh >newfs-toshiba.log 2>&1

Update `/etc/fstab`.


#### Console terminals

Remove 'secure' for the console so it will ask root
password for single user boot.

    mod /etc/ttys
    vi /etc/ttys

Switch off `getty` on C0 because that screen is flashed
when X restarts. We want that screen black.


#### Configuring X11 xdm

In

    /etc/X11/xenodm/GiveConsole

add killing the console mouse daemon since that is not
compatible with X11.

In

    /etc/X11/xenodm/Xresources

set up login window. There are strange issues with the
fonts, also in `Xsetup_0` I had to specify DPI.
It includes `/etc/xenodm/local.def` which contains the
local host name, file created by `/etc/rc.local`.
Failsafe login disabled.

In 

    /etc/X11/xenodm/Xservers

change to no background and no reset. Also possible to
have an another instance of X running on a different
screen.

In

    /etc/X11/xenodm/Xsetup_0

Switches on NumLock LED so we can see that the machine is on.

We call

    /etc/X11/xenodm/x-displays

from Xsetup. That script deals with various multiple screen setups.
Config file:

    /etc/X11/xenodm/xenodm-config

Restart login screen because of strange behaviour.

Added: don't add. Actually this is outdated and the default
driver is just fine. Also there is a bug in this driver which
makes the second screen freeze.

    /etc/X11/xorg.conf.d/20-intel.conf

which would be:

    # Force the Intel driver instead of the modesetting driver.
    # Modesetting is also good but the Intel seems to be faster.

    # Intel graphics, should detect the proper chipset.
    # Sometimes TearFree required for smooth video.
    # PageFlip might be necessary with Displaylink (in my experience
    # DisplayLink works only on a separate screen like :1 anyway)
    # and uses more CPU.
    Section "Device"
      Identifier   "Intel Graphics"
      Driver       "intel"
      # Added the next two lines.
      Option       "AccelMethod" "sna"
      Option       "DRI" "iris"
      #Option      "TearFree" "true"
      #Option      "PageFlip" "false"
    EndSection

to specify Intel graphics chipset, otherwise it will use
the modesetting driver.

The local startup script `/etc/rc.local` creates a file
`/etc/xenodm/local.def` which contains the local hostname and
it will be included in Xresrouces. Will also start xenodm at the end.

For the Dell laptop there is a docking station. When the laptop
is connected to the docking station the laptop screen stays off
during the boot process. The firmware prompts appear on the external
screen and the boot continues on the external screen. But this
happens only when _both_ external screens are connected. Otherwise
the laptop screen will be on and on the wscons console on the external
screen the bottom half of the screen will be useless, 33 lines
instead of 50x160 which was supposed to be otherwise. It is because
the software tries to accomodate the laptop screen. So for this to
work properly both external screens has to be connected.


#### Local startup files

The local startup script is

    /etc/rc.local

Creates virtual consoles, removes setuid bits declared unnecessary,
disconnects from network, deals with various hardware settings,
starts our simplified print daemon and starts xenodm at the end.
Config file

    /etc/rc.conf.local

specifies most daemons not to start. The sound daemon will enable
the first two sound cards, which would be one in the machine and
one in the port replicator.


#### Printing with lpd

We have got PostScript laser printers connected via a USB/Parallel
port dongle. There is a simplified print spooler daemon which
just copies the file onto the port. The config file

    /etc/printcap

describes the printer as it should be for the standard `lpd` print
daemon but it is not used.

The custom daemon starts from `/etc/rc.local`.


#### Privileged operations

In OpenBSD there is no `sudo`, which is probably a good thing.
The functionality is provided by doas. In

    /etc/doas.conf

we got shutdown/reboot, cdrom etc. access defined.
Extra groups defined:

    sdown:*:300:
    rboot:*:301:
    cdrom:*:302:
    connect:*:303:
    fprint:*:304:
    vndisk:*:305:
    hw:*:306:

Put users into these groups as needed.


#### Cosmetic changes

In

    /etc/gettytab

we customize login prompts.


#### Outside file access

OpenBSD has the unveil mechanism to restrict file access
for application. This is mostly used for the browsers.
Some typical directories like `~/tmp` had been added.
    /etc/firefox-esr/unveil.main
    /etc/iridium/unveil.main


#### Limits and login

There is

    /etc/login.conf

which sets the resource limits and configures login.
Do not forget to run

    cap_mkdb -v /etc/login.conf

after the changes.


#### Logging

Logging is done via syslog. Make sure that it does not bind
a port. Also we don't want console logging, which is how it
is now in `/etc/syslog.conf`.

Log files are rotated and archived via `newsyslog`. This has
a root cron job

    /var/cron/tabs/root

but do not edit this file directly, do as root. Use:

    crontab -e

and add

    # !!!! Our stuff.
    # rotate log files every hour, if necessary
    0   *   *   *   *   /usr/bin/newsyslog -a /h/save/logs
    # send log file notifications, if necessary
    #1-59   *   *   *   *   /usr/bin/newsyslog -m
    #
    # do daily/weekly/monthly maintenance
    30  1   *   *   *   /bin/sh /etc/daily
    30  3   *   *   6   /bin/sh /etc/weekly
    30  5   1   *   *   /bin/sh /etc/monthly

    # Keep time. Every hour.
    0   *   *   *   *   /root/bin/set-time

Also set up

    /etc/newsyslog.conf

config file. Archive to `/h/save/logs`, make sure that it does
exist.

You can use script

    syslog-files

to create empty syslog files with the right protections.
The syslog daemon will not write if his output file is
not allocated already.
The script

    newsyslog

will move the syslog files to under `/h/save/logs`.

    newsyslog -d

will remove the .n. numbers from the saved log files.


#### Timekeeping

We don't use ntpd, check

    /etc/ntpd.conf

We run instead

    /root/bin/set-time

from crontab, which runs `ntpdate` when connected to the network.


#### User changes

There is already a system defined group 'users'. Does not
seem to be in use. Move it from gid 10 to gid 1000.
Remove all users from the wheel group except root.

     mod /etc/group
     vi /etc/group

The home directories would have the usual structure of `/h/users`,
`/h/admins` etc as soft links. However, some applications take
the pain to translate these, beating their purpose and hardwire
the translated links into config files. So for this reason we
use `/usr/users`, `/usr/admins`, etc. All goes under `/usr`, trying to
avoid fragmentation.


#### Device driver for the random number generator

The documentation about how to write a device driver is somewhat
terse.

Follows the pseudo device driver mixrng.
This is used to add randomness to the entropy pool.

In file

    /usr/src/etc/MAKEDEV.common

add lines after the definition of 'kstat'. With 7.6 after 'psp'.

    dnl !!!! Addition.
    target(all, mixrng)dnl

Note the funky syntax for dnl.

Also similar changes

    /usr/src/etc/etc.amd64/MAKEDEV.md:

add after the kstat definition:

    dnl !!!! Added.
    _DEV(mixrng, 102)

Chose 102 as device number, the next in line.
However, this will not create the stanzas to create
the device file. So that is done in the startup process
in `/etc/rc.local`.

In file

    /usr/src/sys/arch/amd64/amd64/conf.c

above 'struct cdevsw'

    /* !!!! Added. */
    #include "mixrng.h"

Also in file

    /usr/src/sys/arch/amd64/amd64/conf.c

above 'int nchrdev' in the structure definition as the last item

/* !!!! Added. */
     cdev_mixrng_init(NMIXRNG,mixrng),   /* 101: Mix RNG */

Note the trailing comma and the device major number 120, which
is the next in line.

Include the device in the generic kernel config file

    /usr/src/sys/arch/amd64/conf/GENERIC

as

    pseudo-device   mixrng

after 'pseudo-device dt'.

After the definition of pseudo-device dt in the file

    /usr/src/sys/conf/files

add after the definition of pseudo-device dt.

    # !!!! Added.
    pseudo-device mixrng
    file    dev/mixrng.c                mixrng  needs-flag

Add the files for the device driver

    /usr/src/sys/dev/mixrngio.h
    /usr/src/sys/dev/mixrng.c

as new files.

_Actually do not use this._  

Use instead the section below marked as 'Use this'.

So skip.

    /usr/src/sys/sys/conf.h

add after the definition of cdev_dt_init

    /* !!!! Added. */
    /* open, close, read, write, ioctl */
    #define cdev_mixrng_init(c,n) { \
     dev_init(c,n,open), \
     dev_init(c,n,close), \
     dev_init(c,n,read), \
     dev_init(c,n,write), \
     dev_init(c,n,ioctl), \
     (dev_type_stop((*))) enodev, 0, selfalse, \
     (dev_type_mmap((*))) enodev, 0, D_CLONE }

_Use this._ In

    /usr/src/sys/sys/conf.h

after '#define cdev_dt_init' add
    /* !!!! Added. */
    /* open, close, read, write, ioctl */
    #define cdev_mixrng_init(c,n) { \
     dev_init(c,n,open), \
     dev_init(c,n,close), \
     dev_init(c,n,read), \
     dev_init(c,n,write), \
     dev_init(c,n,ioctl), \
     (dev_type_stop((*))) enodev, 0, \
     (dev_type_mmap((*))) enodev, 0, D_CLONE }

and after 'cdev_decl(kcov);' add

    /* !!!! Added. */
    cdev_decl(mixrng);

Rebuild kernel and reboot.

    cd scripts
    ./kernel-rebuild.sh 2>&1 | tee kernel-rebuild.log


#### Quantis RNG
 
Now for the Quantis USB quantum random number generator.
Do not enable the kernel driver since it does interfere
with the user mode USB staff, so the qrandom set of utilities
will not work. They run in user mode and feed in via the
`mixrng` pseudo device. Otherwise it would be:

    /usr/src/sys/arch/amd64/conf/GENERIC

add

    qrng*   at uhub?        # Quantis rng
So the above is not used/enabled.

As for to show up as an USB device we might need the
followings. We do not do this for 7.6 for the time being
since the Quantis device shows up in dmesg, so possibly it
will work with libusb. In case it needed:
in:

    /usr/src/sys/dev/usb/files.usb

after device definition urng add

    device qrng
    attach qrng at uhub
    file   dev/usb/qrng.c      qrng

Add file

    /usr/src/sys/dev/usb/qrng.c

Build the kernel like:

    cd scripts
    ./kernel-rebuild.sh 2>&1 | tee kernel-rebuild.log


#### Printing with custom scripts

Printing is made complicated with lpd and printcap.
A very simple print system has been implemented with
two small scripts. We got a daemon

    /root/bin/printd

which is watching the spool directory (`/var/spool/output/print`)
and when a file shows up just copies it to the printer,
assumed to be a PostScript printer. The

    /usr/bin/lpr

executable had been replaced with a script which just copies
the PostScript file to the spool.

So:

    savesys /usr/bin/lpr
    savesys -l
    mv /usr/bin/lpr /usr/bin/lpr.save
    cp -p /root/bin/lpr /usr/bin/lpr


#### Green on black

The green on black theme is a tarball under directory `~/green-on-black`.

It contains the modified files

    gtk-2.0/gtkrc
    gtk-3.0/applications.css
    gtk-3.0/gtk-widgets.css

and the rest of the theme.

The tarball should be untarred in `/usr/local/share/themes`.


#### Fingerprint build original

This is the first build the fingerprint login style,
from the original FreeBSD sources as it came from ports.
Use the next paragraph if you already have the modified
tarball.

    cd /usr/ports/sysutils/login_fingerprint
    make

That will build and install libfprint 8.2 which
in fact does not work. At least not for the UPEK I got.
But it gives the source for those.

    cd /usr/ports/security/libfprint
    make deinstall

Use the files from the FreeBSD 11 ports directory tree
which would be libfprint version 0.7.0.

    cd fingerprint/fprint-freebsd-11
    cd src
    tar -z -xf ../tar/libfprint.tar.gz
    cd libfprint-0.7.0

It is prepared to compile now. Since this was lifted
from an already built system the patches are already done.
In case you would need the patches:

    patch < ../../files/patch-config.h.in
    patch < ../../files/patch-libfprint-drivers-vfs301.c
    patch < ../../files/patch-libfprint-drivers-vfs301_proto.c
    patch < ../../files/patch-libfprint-drivers-vfs301_proto.h
    patch < ../../files/patch-libfprint_Makefile.in

Then the usual:

    make clean
    ./configure --disable-udev-rules
    make

It will fail when it tries to deal with the udev rules. We have
no udev on OpenBSD. So this needs to be disabled in the make file.
Also try compile with `gcc` and `-O0`.
Re-run and then:

    make install

will also put files under `/usr/local/libdata/pkgconfig` which is
a FreeBSD thing I presume it should be OK.
We should have `libfprint` now.

Buld `fprint`_demo

    export FPRINT_CFLAGS="-I/usr/local/include/libfprint"
    export FPRINT_LIBS="-L/usr/local/lib -lfprint"
    ./configure
    make


#### Fingerprint on 7.6

In the directory

    cd /root/finger/src

First build the library.

    cd libfprint-baspi-bsapi
    make clean
    make distclean
    ls -l /usr/local/bin/autoconf*
    # You will see the version.
    export AUTOCONF_VERSION=2.69
    # Actually udev had bee defanged in the source so can do without.
    ./configure --disable-udev-rules
    make
    make install
    make clean
Note that these sources had been already fixed. Gives warnings for
`bozorth` but it should be OK.

Then

    cd ../login_fingerprint
    make
    make install
    make clean

To enable the fingerprint authentication you have to have
the login_fingerprint executable under /usr/libexec/auth
with the right permissions. Then modify /etc/login.conf
like:

    vi /etc/login.conf

add stanza

    fingerprint:\
     :auth=fingerprint,passwd:\
     :x-fingerprint=7:\
     :tc=default:
as a class defintion. Update the database:

    cap_mkdb /etc/login.conf

Then you have to have the class also assigned to the
user in `/etc/master.passwd`. It is the field which comes
after the primary group id.
Specify the auth stanza in `/etc/login.conf` like:

    fconagy:\
	:auth=passwd,yubikey:

Modify

    mod /usr/src/libexec/login_passwd/login_passwd.c
    mod edit login_passwd.c

and add space at the password prompt for a subtle change.

    cd /usr/src/libexec/login_passwd
    make
    make install
    make clean

Modify

    mod /usr/src/usr.bin/login/login.c
    vi /usr/src/usr.bin/login/login.c

add further checks.

    cd /usr/src/usr.bin/login
    make
    make install
    make clean

In

    /usr/src/sys/arch/amd64/amd64/machdep.c

change the initial value of

    lid_action = 0;

from 1.

This will disable going to sleep when the lid closed.
It can be enabled back via `sysctl` later on.

Rebuild kernel afterwards.


#### Disable GVFS

GVS is in the way. Disable it.

    cd ~/scripts
    ./gvfs.sh disable


#### XFCE screensaver

Disable the screensaver, it kicks in even when
there is an another screensaver configured. Also
gets the whole system locked up.

    cd ~/scripts
    ./xfce4-screensaver.sh disable

Use `slock` instead.


#### DHCLIENT does not work and not supported

Use dhcpleased.


#### Fonts issues

There are issues with fontconfig. Fonts which
show up in font path in `xset`:

    xset q

does not recognized in applications. Appearantly
there is a second method to handle fonts, with
fontconfig. Copy the font descriptions for fontconfig:

    cp -i -p ~/fontconfig/* /etc/fonts/conf.avail/
    cd /etc/fonts/conf.d
    for F in `ls ~/fontconfig`
    do
	ln -s ../conf.avail/$F $F
    done

Then run

    cd ~/scripts
    ./font-recache.sh

Still the 3270 fonts do not recognised. Not show in
fc-list, fc-cache reports 0 fonts in that directory.
Use the fonts from the kit, that is in many different
formats, copy all those to `~/.fonts/` and then it should
work.


#### Switch console utility

Compile and install utility wsswitch.

    cd src/wsswitch
    make clean
    make
    make install


#### Ethernet interface

The interface `ure0` in the port replicator is buggy.
If it was to be shut down or disconnected it is not
possible to restart.


#### Youtube downloader

The old youtube-dl is dead. A fork which works is

    https://github.com/yt-dlp/yt-dlp

Get the tarball and build as non-root.

    wget https://github.com/yt-dlp/yt-dlp/releases/latest/download/yt-dlp.tar.gz
    tar -z -xf yt-dlp.tar.gz
    cd yt-dlp
    gmake clean
    gmake yt-dlp
    gmake yt-dlp.1

This should build the python script-executable.

    cp yt-dlp ~/local/bin/
    cp yt-dlp.1 ~/local/man/man1/

The `ydl` script should work just the same as before.

