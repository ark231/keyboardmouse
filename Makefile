GCC := gcc
INCLUDEDIRS := -I"/usr/include/libevdev-1.0"
LIBS := -levdev
RESULTDIR := ./compile
ESCAPE_TYPE := -DDEFAULT #or OLD_ESCAPE or MIX_ESCAPE
build:
	rm -f $(RESULTDIR)/kbdmouse
	$(GCC) -o $(RESULTDIR)/kbdmouse kbdmouse.c $(ESCAPE_TYPE) $(INCLUDEDIRS) $(LIBS)

debug:
	rm -f $(RESULTDIR)/kbdmouse_debug
	$(GCC) -g3 -o $(RESULTDIR)/kbdmouse_debug kbdmouse.c -DDEBUG $(ESCAPE_TYPE) $(INCLUDEDIRS) $(LIBS)
