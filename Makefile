src = $(wildcard src/*.c)

ifeq ($(findstring COMMAND.COM, $(SHELL)), COMMAND.COM)
	hostsys = dos
	obj = $(subst /,\,$(src:.c=.obj))
	RM = del
else
	hostsys = unix
	obj = $(src:.c=.obj)
endif
bin = auplay.exe

opt = -5
dbg = -d1
def = -dLITTLEENDIAN

incpath = -Isrc

AS = nasm
CC = wcc386
ASFLAGS = -fobj
CFLAGS = $(dbg) $(opt) $(def) -zq -bt=dos $(incpath)
LDFLAGS = option map
LD = wlink

$(bin): $(obj)
	$(file >objlist.lnk,$(obj))
	$(LD) debug all name $@ system dos4g file { @objlist } $(LDFLAGS)

%.obj: %.c
	$(CC) -fo=$@ $(CFLAGS) $<

%.obj: %.asm
	$(AS) $(ASFLAGS) -o $@ $<

clean:
	$(RM) $(obj)
	$(RM) *.obj
	$(RM) src\*.obj
	$(RM) $(bin)
	$(RM) objlist.lnk
	$(RM) auplay.map
