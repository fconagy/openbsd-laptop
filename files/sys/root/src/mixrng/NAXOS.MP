# Custom kernel on NAXOS.

# !!!! We textually include the kernel config file GENERIC
# so we see what we got. The next line is commented out.
# Below is the full GENERIC with changes and then the .MP
# stuff at the end of the file.
#include "arch/amd64/conf/GENERIC"

machine		amd64
include		"../../../conf/GENERIC"
maxusers	80			# estimated number of users

option		USER_PCICONF	# user-space PCI configuration

option		APERTURE	# in-kernel aperture driver for XFree86
option		MTRR		# CPU memory range attributes control

option		NTFS		# NTFS support
option		SUSPEND
option		HIBERNATE

# !!!! Green is the color.
option		WS_KERNEL_FG=WSCOL_GREEN
option		WS_KERNEL_BG=WSCOL_BLACK

# !!!! Quiet boot.
option		BOOT_QUIET

config		bsd	swap generic

mainbus0 at root

bios0	at mainbus?
cpu0	at mainbus?
ioapic*	at mainbus?
isa0	at mainbus0
isa0	at pcib?
isa0	at amdpcib?
isa0	at tcpcib?
pci*	at mainbus0
vmm0	at mainbus0
pvbus0	at mainbus0

acpi0		at bios0
acpitimer*	at acpi?
acpihpet*	at acpi?
acpiac*		at acpi?
acpibat*	at acpi?
acpibtn*	at acpi?
acpicpu*	at acpi?
acpicmos*	at acpi?
acpidock*	at acpi?
acpidmar0	at acpi? disable
acpiec*		at acpi?
acpipci*	at acpi?
acpiprt*	at acpi?
acpisbs*	at acpi?
acpitz*		at acpi?
acpimadt0	at acpi?
acpimcfg*	at acpi?
acpiasus*	at acpi?
acpisony*	at acpi?
acpithinkpad*	at acpi?
acpitoshiba*	at acpi?
acpivideo*	at acpi?
acpivout*	at acpivideo?
acpipwrres*	at acpi?
aibs*		at acpi?
amdgpio*	at acpi?
aplgpio*	at acpi?
bytgpio*	at acpi?
chvgpio*	at acpi?
com0		at acpi? addr 0x3f8
com1		at acpi? addr 0x2f8
com2		at acpi? addr 0x3e8
com3		at acpi? addr 0x2e8
com*		at acpi?
glkgpio*	at acpi?
pchgpio*	at acpi?
sdhc*		at acpi?
acpicbkbd*	at acpi?
acpials*	at acpi?
abl*		at acpi?	# Apple Backlight
asmc*		at acpi?	# Apple SMC
tpm*		at acpi?
acpihve*	at acpi?
acpisurface*	at acpi?
acpihid*	at acpi?
ipmi0		at acpi? disable
ccpmic*		at iic?
tipmic*		at iic?

efi0		at bios0
mpbios0		at bios0

ipmi0	at mainbus? disable	# IPMI

vmt0	at pvbus?		# VMware Tools
pvclock0 at pvbus?		# KVM pvclock

xen0	at pvbus?		# Xen HVM domU
xnf*	at xen?			# Xen Netfront
xbf*	at xen?			# Xen Blkfront

hyperv0	at pvbus?		# Hyper-V guest
hvn*	at hyperv?		# Hyper-V NetVSC
hvs*	at hyperv?		# Hyper-V StorVSC

option		PCIVERBOSE
option		USBVERBOSE

pchb*	at pci?			# PCI-Host bridges
aapic*	at pci?			# AMD 8131 IO apic
ppb*	at pci?			# PCI-PCI bridges
pci*	at ppb?
pci*	at pchb?
pcib*	at pci?			# PCI-ISA bridge
amdpcib* at pci?		# AMD 8111 LPC bridge
tcpcib*	at pci?			# Intel Atom E600 LPC bridge
kate*	at pci?			# AMD K8 temperature sensor
km*	at pci?			# AMD K10 temperature sensor
ksmn*	at pci?			# AMD K17 temperature sensor
amas*	at pci? disable		# AMD memory configuration
pchtemp* at pci?		# Intel C610 temperature sensor
ccp*	at pci?			# AMD Cryptographic Co-processor

# National Semiconductor LM7[89] and compatible hardware monitors
lm0	at isa? port 0x290
#lm1	at isa? port 0x280
#lm2	at isa? port 0x310

it*	at isa? port 0x2e	# ITE IT8705F, IT8712F, IT8716F, IT8718F,
it*	at isa? port 0x4e	# IT8726F and SiS SiS950 monitors and
				# watchdog timer

schsio*	at isa? port 0x2e	# SMSC SCH311x Super I/O
schsio*	at isa? port 0x4e
schsio*	at isa? port 0x162e
schsio*	at isa? port 0x164e

#viasio* at isa? port 0x2e flags 0x0000	# VIA VT1211 LPC Super I/O
#viasio* at isa? port 0x4e flags 0x0000

wbsio*	at isa? port 0x2e	# Winbond LPC Super I/O
wbsio*	at isa? port 0x4e
lm*	at wbsio?
uguru0	at isa? disable port 0xe0	# ABIT uGuru

aps0	at isa? port 0x1600		# ThinkPad Active Protection System

piixpm*	at pci?			# Intel PIIX PM
iic*	at piixpm?
ichiic*	at pci?			# Intel ICH SMBus controller
iic*	at ichiic?
viapm*	at pci?			# VIA SMBus controller
iic*	at viapm?
amdiic* at pci?			# AMD-8111 SMBus controller
iic*	at amdiic?
nviic*	at pci?			# NVIDIA nForce2/3/4 SMBus controller
iic*	at nviic?
amdpm*	at pci?			# AMD-7xx/8111 and NForce SMBus controller
iic*	at amdpm?
dwiic*	at acpi?		# DesignWare Synopsys i2c controller
dwiic*	at pci?
iic*	at dwiic?
iosf*	at acpi?		# Intel OnChip System Fabric
iosf*	at pci?

itherm*	at pci?			# Intel 3400 Thermal Sensor
adc*	at iic?			# Analog Devices AD7416/AD7417/7418
adl*	at iic?			# Andigilog aSC7621
admtemp* at iic?		# Analog Devices ADM1021
admlc*	at iic?			# Analog Devices ADM1024
admtm*	at iic?			# Analog Devices ADM1025
admcts*	at iic?			# Analog Devices ADM1026
admtmp*	at iic?			# Analog Devices ADM1030
admtt*	at iic?			# Analog Devices ADM1031
adt*	at iic?			# Analog Devices ADT7460
andl*	at iic?			# Andigilog aSC7611
bgw*	at iic?			# Bosch BMC150 6-axis eCompass
lisa*	at iic?			# STMicroelectronics LIS331DL motion sensor
lm*	at iic?			# National Semiconductor LM78/79
lmenv*	at iic?			# National Semiconductor LM87
lmtemp*	at iic?			# National Semiconductor LM75/LM77
lmn*	at iic?			# National Semiconductor LM93
maxds*	at iic?			# Maxim DS1631
maxtmp*	at iic?			# Maxim MAX6642/MAX6690
spdmem*	at iic?			# SPD memory eeproms
sdtemp*	at iic?			# SO-DIMM (JC-42.4) temperature
wbng*	at iic?			# Winbond W83793G
nvt*	at iic?			# Novoton W83795G
ihidev*	at iic?			# HID-over-i2c
ims*	at ihidev?		# HID-over-i2c mouse/trackpad
wsmouse* at ims? mux 0
ikbd*	at ihidev?		# i2c keyboard
wskbd*	at ikbd? mux 1
imt*	at ihidev?		# HID-over-i2c multitouch trackpad
wsmouse* at imt? mux 0
iatp* at iic?			# Atmel maXTouch i2c touchpad/touchscreen
wsmouse* at iatp? mux 0
ietp* at iic?			# Elantech touchpad
wsmouse* at ietp? mux 0
icc*	at ihidev?		# Consumer Control keyboards
wskbd*	at icc? mux 1

skgpio0 at isa? port 0x680	# Soekris net6501 GPIO and LEDs
gpio* at skgpio?

#option PCMCIAVERBOSE

# PCI PCMCIA controllers
#pcic*	at pci?

# PCMCIA bus support
#pcmcia*	at pcic?

# CardBus bus support
cbb*		at pci?
cardslot*	at cbb?
cardbus*	at cardslot?
pcmcia*		at cardslot?

# USB Controllers
xhci*	at pci?			# eXtensible Host Controller
ehci*	at pci?			# Enhanced Host Controller
ehci*	at cardbus?		# Enhanced Host Controller
uhci*	at pci?			# Universal Host Controller (Intel)
uhci*	at cardbus?		# Universal Host Controller (Intel)
ohci*	at pci?			# Open Host Controller
ohci*	at cardbus?		# Open Host Controller

# USB bus support
usb*	at xhci?
usb*	at ehci?
usb*	at uhci?
usb*	at ohci?

# USB devices
uhub*	at usb?			# USB Hubs
uhub*	at uhub?		# USB Hubs
urng*	at uhub?		# USB Random Number Generator

# !!!! Additional. Will be disabled for testing.
qrng*	at uhub?		# Quantis hw rng

uonerng* at uhub?		# Moonbase Otago OneRNG
umodem*	at uhub?		# USB Modems/Serial
ucom*	at umodem?
uvisor*	at uhub?		# Handspring Visor
ucom*	at uvisor?
uvscom*	at uhub?		# SUNTAC Slipper U VS-10U serial
ucom*	at uvscom?
ubsa*	at uhub?		# Belkin serial adapter
ucom*	at ubsa?
ukspan* at uhub?		# Keyspan USA19HS serial adapter
ucom*	at ukspan?
uftdi*	at uhub?		# FTDI FT8U100AX serial adapter
ucom*	at uftdi?
uplcom* at uhub?		# I/O DATA USB-RSAQ2 serial adapter
ucom*	at uplcom?
umct*	at uhub?		# MCT USB-RS232 serial adapter
ucom*	at umct?
uslcom*	at uhub?		# Silicon Laboratories CP210x serial
ucom*	at uslcom?
uscom*	at uhub?		# Simple USB serial adapters
ucom*	at uscom?
ucrcom*	at uhub?		# Chromebook serial
ucom*	at ucrcom?
uark*	at uhub?		# Arkmicro ARK3116 serial
ucom*	at uark?
moscom*	at uhub?		# MosChip MCS7703 serial
ucom*	at moscom?
umcs*	at uhub?		# MosChip MCS78x0 serial
ucom*	at umcs?
uipaq*	at uhub?		# iPAQ serial adapter
ucom*	at uipaq?
umsm*	at uhub?		# Qualcomm MSM EVDO
ucom*	at umsm?
uchcom*	at uhub?		# WinChipHead CH341/340 serial
ucom*	at uchcom?
uticom*	at uhub?		# TI serial
ucom*	at uticom?
uxrcom*	at uhub?		# Exar XR21V1410 serial
ucom*	at uxrcom?
uaudio* at uhub?		# USB Audio
audio*	at uaudio?
umidi*	at uhub?		# USB MIDI
midi*	at umidi?
ulpt*	at uhub?		# USB Printers
umass*	at uhub?		# USB Mass Storage devices
ubcmtp*	at uhub?		# Broadcom USB trackpad
wsmouse* at ubcmtp? mux 0
uhidev*	at uhub?		# Human Interface Devices
ums*	at uhidev?		# USB mouse
wsmouse* at ums? mux 0
umt*	at uhidev?		# USB multitouch touchpad
wsmouse* at umt? mux 0
uts*	at uhub?		# USB touchscreen
wsmouse* at uts? mux 0
uwacom*	at uhidev?		# USB Wacom tablet
wsmouse* at uwacom? mux 0
ukbd*	at uhidev?		# USB keyboard
wskbd*	at ukbd? mux 1
ucycom*	at uhidev?			# Cypress serial
ucom*	at ucycom?
uslhcom* at uhidev?		# Silicon Labs CP2110 USB HID UART
ucom*	at uslhcom?
uhid*	at uhidev?		# USB generic HID support
fido*	at uhidev?		# FIDO/U2F security key support
ucc*	at uhidev?		# Consumer Control keyboards
wskbd*	at ucc? mux 1
ujoy*	at uhidev?		# USB joystick/gamecontroller support
uhidpp*	at uhidev?		# Logitech HID++ Devices
upd*	at uhidev?		# USB Power Devices sensors
umstc*	at uhidev?		# Microsoft Surface Type Cover
aue*	at uhub?		# ADMtek AN986 Pegasus Ethernet
atu*	at uhub?		# Atmel AT76c50x based 802.11b
axe*	at uhub?		# ASIX Electronics AX88172 USB Ethernet
axen*	at uhub?		# ASIX Electronics AX88179 USB Ethernet
cue*	at uhub?		# CATC USB-EL1201A based Ethernet
kue*	at uhub?		# Kawasaki KL5KUSB101B based Ethernet
smsc*	at uhub?		# SMSC LAN95xx Ethernet
cdce*	at uhub?		# CDC Ethernet
urndis*	at uhub?		# Remote NDIS Ethernet
upl*	at uhub?		# Prolific PL2301/PL2302 host-to-host `network'
ugl*	at uhub?		# Genesys Logic GL620USB-A host-to-host `network'
udav*	at uhub?		# Davicom DM9601 based Ethernet
mos*	at uhub?		# MOSCHIP MCS7730/7830 10/100 Ethernet
mue*	at uhub?		# Microchip LAN75xx/LAN78xx Ethernet
url*	at uhub?		# Realtek RTL8150L based adapters
ure*	at uhub?		# Realtek RTL8152 based adapters
uaq*	at uhub?		# Aquantia AQC111U/AQC112U based adapters
wi*	at uhub?		# WaveLAN IEEE 802.11DS
udsbr*	at uhub?		# D-Link DSB-R100 radio
radio*	at udsbr?		# USB radio
uberry*	at uhub?		# Research In Motion BlackBerry
ugen*	at uhub?		# USB Generic driver
uath*	at uhub?		# Atheros AR5005UG/AR5005UX
ural*	at uhub?		# Ralink RT2500USB
rum*	at uhub?		# Ralink RT2501USB/RT2601USB
run*	at uhub?		# Ralink RT2700U/RT2800U/RT3000U
mtw*	at uhub?		# MediaTek MT7601U
otus*	at uhub?		# Atheros AR9001U
athn*	at uhub?		# Atheros AR9002U
zyd*	at uhub?		# Zydas ZD1211
upgt*	at uhub?		# Conexant/Intersil PrismGT SoftMAC USB
urtw*	at uhub?		# Realtek 8187
rsu*	at uhub?		# Realtek RTL8188SU/RTL8191SU/RTL8192SU
urtwn*	at uhub?		# Realtek RTL8188CU/RTL8192CU
udcf*	at uhub?		# Gude Expert mouseCLOCK
umbg*	at uhub?		# Meinberg Funkuhren USB5131/DCF600USB
umb*	at uhub?		# Mobile Broadband Interface Model
uthum*	at uhidev?		# TEMPerHUM sensor
ugold*	at uhidev?		# gold TEMPer sensor
utrh*	at uhidev?		# USBRH sensor
utwitch* at uhidev?		# YUREX BBU sensor
uow*	at uhub?		# Maxim/Dallas DS2490 1-Wire adapter
uoakrh*	at uhidev?		# Toradex OAK temp and rel humidity
uoaklux* at uhidev?		# Toradex OAK LUX
uoakv*	at uhidev?		# Toradex OAK 10V sensor
onewire* at uow?
uvideo*	at uhub?		# USB Video
video*	at uvideo?
utvfu*	at uhub?		# Fushicai Audio-Video Grabber
video*	at utvfu?
audio*	at utvfu?
udl*	at uhub?		# DisplayLink USB displays
wsdisplay* at udl?
bwfm*	at uhub?		# Broadcom FullMAC

puc*	at pci?			# PCI "universal" communication device
com*	at cardbus?
com*	at pci?

sdhc*	at pci?			# SD Host Controller
sdmmc*	at sdhc?		# SD/MMC bus
rtsx*	at pci?			# Realtek SD Card Reader
sdmmc*	at rtsx?		# SD/MMC bus
bwfm*	at sdmmc?		# Broadcom FullMAC

isadma0	at isa?

option	WSDISPLAY_COMPAT_USL		# VT handling
option	WSDISPLAY_COMPAT_RAWKBD		# provide raw scancodes; needed for X11
option	WSDISPLAY_DEFAULTSCREENS=6	# initial number of text consoles

pckbc0		at isa? flags 0x00	# PC keyboard controller
pckbd*		at pckbc?		# PC keyboard
pms*		at pckbc?		# PS/2 mouse for wsmouse
vga0		at isa?
option		X86EMU			# to POST video cards
vga*		at pci?
wsdisplay0	at vga? console 1
wskbd*		at pckbd? mux 1
wsmouse*	at pms? mux 0

#mmuagp*	at pchb?	# amd64 mmu agp.
#agp*		at mmuagp?

inteldrm*	at pci? 	# Intel i915, i945 DRM driver
intagp*		at inteldrm?
agp*		at intagp?
drm0		at inteldrm? primary 1
drm*		at inteldrm?
wsdisplay0	at inteldrm? primary 1
wsdisplay*	at inteldrm? mux -1
radeondrm*	at pci? 	# ATI Radeon DRM driver
drm0		at radeondrm? primary 1
drm*		at radeondrm?
wsdisplay0	at radeondrm? primary 1
wsdisplay*	at radeondrm? mux -1
amdgpu*		at pci?
drm0		at amdgpu? primary 1
drm*		at amdgpu?
wsdisplay0	at amdgpu? primary 1
wsdisplay*	at amdgpu? mux -1

pcppi0		at isa?

com0	at isa? port 0x3f8 irq 4	# standard PC serial ports
com1	at isa? port 0x2f8 irq 3
com2	at isa? port 0x3e8 irq 5
com3	at isa? disable port 0x2e8 irq 9 # (conflicts with some video cards)

com*	at pcmcia?			# PCMCIA modems/serial ports
com*	at puc?

# options CY_HW_RTS
cy*	at pci?				# PCI cyclom serial card
#cz*	at pci?				# Cyclades-Z multi-port serial boards

lpt0	at isa? port 0x378 irq 7	# standard PC parallel ports
#lpt1	at isa? port 0x278
#lpt2	at isa? port 0x3bc
lpt*	at puc?

efifb0		at mainbus?		# EFI Framebuffer
wsdisplay0	at efifb?

ahc*	at pci?				# Adaptec 2940 SCSI controllers
jmb*	at pci?				# JMicron JMB36x controllers
ahci*	at jmb? flags 0x0000		# flags 0x0001 to force SATA 1 (1.5Gb/s)
pciide*	at jmb?
ahci*	at pci? flags 0x0000		# AHCI SATA controllers
					# flags 0x0001 to force SATA 1 (1.5Gb/s)
sili*	at pci?				# Silicon Image 3124/3132/3531 SATA
ahd*	at pci?				# Adaptec 79?? SCSI controllers
arc*	at pci?				# Areca RAID Controller
mpi*	at pci?				# LSI Logic Message Passing Interface
mpii*	at pci?				# LSI Message Passing Interface II
gdt*	at pci?				# ICP Vortex GDT RAID controllers
twe*	at pci?				# 3ware Escalade RAID controllers
#aac*	at pci?				# Adaptec FSA RAID controllers
ami*	at pci?				# AMI MegaRAID controllers
mfi*	at pci?				# LSI MegaRAID SAS controllers
mfii*	at pci?				# LSI MegaRAID SAS Fusion controllers
#cac*	at pci?				# Compaq Smart ARRAY [234]* RAID controllers
ciss*	at pci?				# Compaq Smart ARRAY [56]* RAID controllers
iha*	at pci?				# Initio Ultra/UltraWide SCSI controllers
ips*	at pci?				# IBM ServeRAID controllers
qlw*	at pci?				# QLogic ISP SCSI
qla*	at pci?				# QLogic ISP 2[123]xx FibreChannel
qle*	at pci?				# QLogic ISP 2[45]xx FibreChannel
aic*	at pcmcia?			# PCMCIA Adaptec 152[02] SCSI
siop*	at pci?				# NCR 538XX SCSI controllers
adw*	at pci?				# AdvanSys ULTRA WIDE SCSI
pcscp*	at pci?				# AMD 53c974 PCscsi-PCI SCSI
#trm*	at pci?				# Tekram DC-3x5U SCSI Controllers
vmwpvs*	at pci?				# VMware ParaVirtual SCSI
nvme*	at pci?				# NVMe controllers

scsibus* at scsi?
sd*	at scsibus?			# SCSI disk drives
st*	at scsibus?			# SCSI tape drives
cd*	at scsibus?			# SCSI CD-ROM drives
ch*	at scsibus?			# SCSI autochangers
safte*	at scsibus?			# SCSI accessed fault-tolerant encl
ses*	at scsibus?			# SCSI enclosure services
uk*	at scsibus?			# unknown SCSI

mpath0	at root
emc*	at scsibus?
hds*	at scsibus?
rdac*	at scsibus?
sym*	at scsibus?

fdc0	at isa? port 0x3f0 irq 6 drq 2	# standard PC floppy controllers
#fdc1	at isa? port 0x370
fd*	at fdc? flags 0x00

# IDE controllers
pciide* at pci? flags 0x0000

wdc0	at isa? disable port 0x1f0 irq 14 flags 0x00
wdc1	at isa? disable port 0x170 irq 15 flags 0x00
wdc*	at pcmcia?

# IDE hard drives
wd*	at wdc? flags 0x0000
wd*	at pciide? flags 0x0000

# ATAPI<->SCSI
atapiscsi* at wdc?
atapiscsi* at pciide?

# Networking devices
de*	at pci?				# DC21X4X-based ethernet
fxp*	at pci?				# EtherExpress 10/100B ethernet
fxp*	at cardbus?			# Intel PRO/100 ethernet
ne*	at pci?				# NE2000-compat ethernet
ep*	at pci?				# 3C59x ethernet
ne*	at pcmcia?			# PCMCIA based NE2000 ethernet
ep*	at pcmcia?			# PCMCIA based 3C5xx ethernet
sm*	at pcmcia?			# PCMCIA based sm ethernet
xe*	at pcmcia?			# Xircom ethernet
xl*	at pci?				# 3C9xx ethernet
xl*	at cardbus?			# 3C575/3C656 ethernet
rl*	at pci?				# Realtek 81[23]9 ethernet
rl*	at cardbus?			# Realtek 81[23]9 ethernet
#mtd*	at pci?				# Myson MTD800/803/891
epic*	at pci?				# SMC EPIC/100 ethernet
vr*	at pci?				# VIA Rhine ethernet
sf*	at pci?				# Adaptec AIC-6915 ethernet
sis*	at pci?				# SiS 900/7016 ethernet
se*	at pci?				# SiS 190/191 ethernet
pcn*	at pci?				# AMD PCnet-PCI Ethernet
dc*	at pci?				# 21143, "tulip" clone ethernet
dc*	at cardbus?			# 21143, "tulip" clone ethernet
ti*	at pci?				# Alteon Tigon 1Gb ethernet
skc*	at pci?				# SysKonnect GEnesis 984x
sk*	at skc?				#  each port of above
mskc*	at pci?				# Marvell Yukon-2
msk*	at mskc?			#  each port of above
em*	at pci?				# Intel Pro/1000 ethernet
ixgb*	at pci?				# Intel Pro/10Gb ethernet
ix*	at pci?				# Intel 82598EB 10Gb ethernet
myx*	at pci?				# Myricom Myri-10G 10Gb ethernet
oce*	at pci?				# Emulex OneConnect 10Gb ethernet
txp*	at pci?				# 3com 3CR990
#nge*	at pci?				# NS DP83820/DP83821 GigE
bge*	at pci?				# Broadcom BCM57xx (aka Tigon3)
bnx*	at pci?				# Broadcom BCM5706/5708 GigE
re*	at pci?				# Realtek 8169/8169S/8110S
re*	at cardbus?			# Realtek 8169/8169S/8110S
rge*	at pci?				# Realtek 8125
stge*	at pci?				# Sundance TC9021 GigE
#lge*	at pci?				# Level1 LXT1001 GigE
hme*	at pci?				# Sun Happy Meal
vge*	at pci?				# VIA VT612x
nfe*	at pci?				# NVIDIA nForce Ethernet
xge*	at pci?				# Neterion Xframe-I/II 10Gb ethernet
thtc*	at pci?				# Tehuti Networks 10Gb ethernet
tht*	at thtc?
gem*	at pci?				# Sun GEM 10/100/Gigabit
cas*	at pci?				# Sun Cassini 100/Gigabit
bce*	at pci?				# Broadcom BCM4401
vic*	at pci?				# VMware VMXnet virtual interface
vmx*	at pci?				# VMware VMXNET3 virtual interface
et*	at pci?				# Agere/LSI ET1310
age*	at pci?				# Attansic L1 Ethernet
alc*	at pci?				# Attansic L1C/L1D/L2C Ethernet
ale*	at pci?				# Attansic L1E Ethernet
lii*	at pci?				# Attansic L2 Ethernet
jme*	at pci?				# JMicron JMC250/JMC260 Ethernet
bnxt*	at pci?				# Broadcom BCM573xx, BCM574xx
ixl*	at pci?				# Intel Ethernet 700 Series
mcx*	at pci?				# Mellanox ConnectX-4
iavf*	at pci?				# Intel Ethernet Adaptive VF
aq*	at pci?				# Aquantia aQtion Ethernet
igc*	at pci?				# Intel I225 Ethernet
ngbe*	at pci?				# WangXun WX1860 Ethernet

# Wireless network cards
acx*	at pci?				# TI ACX100/ACX111 (802.11b/g)
acx*	at cardbus?			# TI ACX100/ACX111 (802.11b/g)
ath*	at pci?				# Atheros AR5k (802.11a/b/g)
ath*	at cardbus?			# Atheros AR5k (802.11a/b/g)
athn*	at pci?				# Atheros AR9k (802.11a/g/n)
athn*	at cardbus?			# Atheros AR9k (802.11a/g/n)
atw*	at pci?				# ADMtek ADM8211 (802.11)
atw*	at cardbus?			# ADMtek ADM8211 (802.11)
bwi*	at pci?				# Broadcom AirForce (802.11b/g)
bwi*	at cardbus?			# Broadcom AirForce (802.11b/g)
wi*	at pci?				# WaveLAN IEEE 802.11DS
wi*	at pcmcia?			# WaveLAN IEEE 802.11DS
an*	at pci?				# Aironet IEEE 802.11DS
an*	at pcmcia?			# Aironet IEEE 802.11DS
iwi*	at pci?				# Intel PRO/Wireless 2200BG/2915ABG
wpi*	at pci?				# Intel PRO/Wireless 3945ABG
iwn*	at pci?				# Intel WiFi Link 4965/5000/1000/6000
iwm*	at pci?				# Intel WiFi Link 7xxx
iwx*	at pci?				# Intel WiFi Link 22xxx
ral*	at pci?				# Ralink RT2500/RT2501/RT2600
ral*	at cardbus?			# Ralink RT2500/RT2501/RT2600
rtw*	at pci?				# Realtek 8180
rtw*	at cardbus?			# Realtek 8180
rtwn*	at pci?				# Realtek 8188CE/8192CE
pgt*	at pci?				# Prism54 Full-MAC
pgt*	at cardbus?			# Prism54 Full-MAC
malo*	at pci?				# Marvell Libertas
malo*	at cardbus?			# Marvell Libertas
malo*	at pcmcia?			# Marvell 88W8385
mbg*	at pci?				# Meinberg Funkuhren radio clocks
bwfm*	at pci?				# Broadcom FullMAC

# Media Independent Interface (mii) drivers
exphy*	at mii?				# 3Com internal PHYs
inphy*	at mii?				# Intel 82555 PHYs
iophy*	at mii?				# Intel 82553 PHYs
icsphy*	at mii?				# ICS 1890 PHYs
lxtphy*	at mii?				# Level1 LXT970 PHYs
nsphy*	at mii?				# NS and compatible PHYs
nsphyter*	at mii?			# NS and compatible PHYs
qsphy*	at mii?				# Quality Semi QS6612 PHYs
luphy*	at mii?				# Lucent LU6612 PHY
sqphy*	at mii?				# Seeq 8x220 PHYs
rlphy* at mii?				# Realtek 8139 internal PHYs
mtdphy* at mii?				# Myson MTD972 PHYs
dcphy*	at mii?				# Digital Clone PHYs
acphy*	at mii?				# Altima AC101 PHYs
amphy*	at mii?				# AMD 79C873 PHYs
tqphy*	at mii?				# TDK 78Q212x PHYs
bmtphy*	at mii?				# Broadcom 10/100 PHYs
brgphy*	at mii?				# Broadcom Gigabit PHYs
eephy* at mii?				# Marvell 88E1000 series PHY
xmphy*	at mii?				# XaQti XMAC-II PHYs
nsgphy*	at mii?				# NS gigabit PHYs
rgephy*	at mii?				# Realtek 8169S/8110S PHY
urlphy*	at mii?				# Realtek RTL8150L internal PHY
ciphy*	at mii?				# Cicada CS8201 10/100/1000 copper PHY
gentbi*	at mii?				# Generic 1000BASE-X ten-bit PHY
etphy*	at mii?				# Agere/LSI ET1011 TruePHY
jmphy*	at mii?				# JMicron JMP202/JMP211 PHYs
atphy*	at mii?				# Attansic F1 PHYs
ipgphy*	at mii?				# IC Plus IP1000A PHYs
ukphy*	at mii?				# "unknown" PHYs

eap*	at pci?				# Ensoniq AudioPCI S5016
envy*	at pci?				# VIA Envy24 (aka ICE1712)
#eso*	at pci?				# ESS Solo-1 PCI AudioDrive
#sv*	at pci?				# S3 SonicVibes (S3 617)
#neo*	at pci?				# NeoMagic 256AV/ZX
cmpci*	at pci?				# C-Media CMI8338/8738
auacer*	at pci?				# Acer Labs M5455
auich*	at pci? flags 0x0000		# i82801 ICH AC'97 audio
auixp*	at pci?				# ATI IXP AC'97 Audio
#autri*	at pci? flags 0x0000		# Trident 4D WAVE
auvia*	at pci?				# VIA VT82C686A
azalia*	at pci?				# High Definition Audio
clcs*	at pci?				# CS4280 CrystalClear audio
#clct*	at pci?				# CS4281 CrystalClear audio
#fms*	at pci?				# Forte Media FM801
#maestro* at pci?			# ESS Maestro PCI
#esa*	at pci?				# ESS Maestro3 PCI
yds*	at pci? flags 0x0000		# Yamaha YMF Audio
emu*	at pci?				# SB Live!
mpu*	at isa? port 0x330

# MIDI support
#midi*	at autri?
midi*	at eap?
midi*	at envy?
midi*	at mpu?

spkr0	at pcppi?		# PC speaker

# Audio Support
audio*	at eap?
audio*	at envy?
#audio*	at eso?
#audio*	at sv?
#audio*	at neo?
audio*	at cmpci?
audio*	at clcs?
#audio*	at clct?
audio*	at auacer?
audio*	at auich?
audio*	at auixp?
#audio*	at autri?
audio*	at auvia?
audio*	at azalia?
#audio*	at fms?
audio*	at uaudio?
#audio*	at maestro?
#audio*	at esa?
audio*	at yds?
audio*	at emu?

bktr0	at pci?

# FM-Radio support
radio*	at bktr?

#wdt0	at pci?			# Ind Computer Source PCI-WDT50x driver

xspd0	at pci?			# XenSource Platform Device

# 1-Wire devices
option ONEWIREVERBOSE
owid*	at onewire?		# ID
owsbm*	at onewire?		# Smart Battery Monitor
owtemp* at onewire?		# Temperature
owctr*	at onewire?		# Counter device

pseudo-device	pctr		1
pseudo-device	nvram		1
pseudo-device	hotplug		1	# devices hot plugging
pseudo-device	dt

# !!!! Will be added.
pseudo-device	mixrng

# mouse & keyboard multiplexor pseudo-devices
pseudo-device	wsmux		2

# Virtio devices
virtio*		at pci?		# Virtio PCI device
vioblk*		at virtio?	# Virtio block device
vio*		at virtio?	# Virtio network device
viomb*		at virtio?	# Virtio memory ballooning device
viornd*		at virtio?	# Virtio entropy device
vioscsi*	at virtio?	# Virtio SCSI device
#viocon*	at virtio?	# Virtio console device
vmmci*		at virtio?	# VMM control interface
#viogpu*	at virtio?	# VirtIO GPU device
#wsdisplay0	at viogpu? console 1
#wsdisplay*	at viogpu? mux -1

option	MULTIPROCESSOR
#option	MP_LOCKDEBUG
#option	WITNESS

cpu*		at mainbus?
