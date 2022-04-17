.PHONY: clean

include config.mk
include MakefileDebService.mk

clean:
	make -C src $@

