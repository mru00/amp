# TerraControl, mru 2009

DEV_SER = /dev/ttyS0
DEV_PAR = /dev/parport0


VERSION := $(shell sed -e 's/\#define VERSION "\(.*\)"/\1/p;d' firmware/version.h )

ARCH_FILES = firmware

get_version:
	@echo $(VERSION)

build:
	@$(MAKE) -C firmware build

program: rights
	@$(MAKE) -C firmware program

gui:
	@$(MAKE) -C gui


sync:
	unison -silent -batch -auto -ui text amp-proc

remote_program:
	ssh -t ikarus 'cd ~/dev/08amp/; make sync && make clean program; make sync;'

rights:
	@if [ -e $(DEV_SER) -a ! -w $(DEV_SER) ]; then sudo chmod uog+rw $(DEV_SER); fi
	@if [ -e $(DEV_PAR) -a ! -w $(DEV_PAR) ]; then sudo chmod uog+rw $(DEV_PAR); fi

install_gcc:
	apt-get install gcc-avr uisp avr-libc binutils-avr gtkterm

clean: 
	@$(MAKE) -C firmware clean
#	@$(MAKE) -C pcb clean

arch:
	@tar czf amp-proc-$(VERSION).tar.gz $(ARCH_FILES)

release:
	$(MAKE) -C firmware release

startgeda:
	$(MAKE) -C pcb startgeda

geda_update:
	$(MAKE) -C pcb update

.PHONY: clean rights build program arch geda sync install_gcc remote_program 

