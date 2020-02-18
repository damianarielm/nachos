# Copyright (c) 1992      The Regents of the University of California.
#               2016-2017 Docentes de la Universidad Nacional de Rosario.
# All rights reserved.  See `copyright.h` for copyright notice and
# limitation of liability and disclaimer of warranty provisions.

MAKE   = make
SH     = bash
ITALIC = \e[3m

.PHONY: all clean

all:
	@echo -e "$(ITALIC)Making thread...\e[0m"
	@$(MAKE) -s -C threads depend
	@$(MAKE) -s -C threads all
	@echo -e "$(ITALIC)Making userprog...\e[0m"
	@$(MAKE) -s -C userprog depend
	@$(MAKE) -s -C userprog all
	@echo -e "$(ITALIC)Making vmem...\e[0m"
	@$(MAKE) -s -C vmem depend
	@$(MAKE) -s -C vmem all
	@echo -e "$(ITALIC)Making filesys...\e[0m"
	@$(MAKE) -s -C filesys depend
	@$(MAKE) -s -C filesys all
#	@echo -e "$(ITALIC)Making .network...\e[0m"
#	@$(MAKE) -s -C .network depend
#	@$(MAKE) -s -C .network all
	@echo -e "$(ITALIC)Making .bin...\e[0m"
	@$(MAKE) -s -C .bin
	@echo -e "$(ITALIC)Making userland...\e[0m"
	@$(MAKE) -s -C userland

# Do not delete executables in `userland` in case there is no cross-compiler.
clean:
	@echo -e "$(ITALIC)Cleaning all...\e[0m"
	@$(MAKE) -s -C threads clean
	@$(MAKE) -s -C userprog clean
	@$(MAKE) -s -C vmem clean
	@$(MAKE) -s -C filesys clean
#	@$(MAKE) -s -C .network clean
	@$(MAKE) -s -C .bin clean
	@$(MAKE) -s -C userland clean
