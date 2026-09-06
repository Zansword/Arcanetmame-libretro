###########################################################################
#
#   retro.mak
#
#   Libretro OSD layer for MAME 0.135.
#
###########################################################################

RETROSRC = $(SRC)/osd/retro
RETROOBJ = $(OBJ)/osd/retro

OBJDIRS += $(RETROOBJ)

OSDCOREOBJS = \
	$(RETROOBJ)/retrodir.o \
	$(RETROOBJ)/retrofile.o \
	$(RETROOBJ)/retromisc.o \
	$(RETROOBJ)/retroos.o \
	$(RETROOBJ)/retrosync.o \
	$(RETROOBJ)/retrowork.o

OSDOBJS = \
	$(RETROOBJ)/retromain.o

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OSDOBJS)

RESFILE =
