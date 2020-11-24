GCC := gcc
LIBS := 
RESULTDIR := ./compile
build:
	rm -f $(RESULTDIR)/kbdmouse
	$(GCC) -o $(RESULTDIR)/kbdmouse kbdmouse.c

debug:
	rm -f $(RESULTDIR)/kbdmouse_debug
	$(GCC) -g3 -o $(RESULTDIR)/kbdmouse_debug kbdmouse.c
