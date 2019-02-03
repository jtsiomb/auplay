obj = main.obj audio.obj aufile.obj auwav.obj dma.obj au_sb.obj dpmi.obj
bin = auplay.exe

opt = -5
dbg = -d3
def = -dLITTLEENDIAN

incpath = -Isrc

!ifdef __UNIX__
RM = rm -f
!else
RM = del
!endif

AS = nasm
CC = wcc386
ASFLAGS = -fobj
CFLAGS = $(dbg) $(opt) $(def) -zq -bt=dos $(incpath)
LDFLAGS = option map
LD = wlink

$(bin): $(obj)
	%write objlist.lnk $(obj)
	$(LD) debug all name $@ system dos4g file { @objlist } $(LDFLAGS)

.c: src
.asm: src

.c.obj: .autodepend
	$(CC) -fo=$@ $(CFLAGS) $[*

.asm.obj:
	$(AS) $(ASFLAGS) -o $@ $[*.asm

clean: .symbolic
	$(RM) *.obj
	$(RM) *.lnk
	$(RM) *.map
	$(RM) $(bin)
