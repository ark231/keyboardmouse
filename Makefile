GCC := gcc
INCLUDEDIRS := -I"/usr/include/libevdev-1.0"
LIBS := -levdev
RESULTDIR := ./compile
build:
	rm -f $(RESULTDIR)/kbdmouse
	$(GCC) -o $(RESULTDIR)/kbdmouse kbdmouse.c $(INCLUDEDIRS) $(LIBS)

debug:
	rm -f $(RESULTDIR)/kbdmouse_debug
	$(GCC) -g3 -o $(RESULTDIR)/kbdmouse_debug kbdmouse.c -DDEBUG $(INCLUDEDIRS) $(LIBS)
