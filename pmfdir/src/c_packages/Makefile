# Makefile for the c_packages library
# Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
# This file latest updated: Sept 21, 1991

PACKAGES = strings_galore generic_list safe_malloc copy_bytes

all:
	for i in $(PACKAGES) ; \
	do \
		echo Making $$i...; \
		( cd $$i; make ) \
	done
	ln -s */*.o .
	ar rv libc_packs.a *.o
	ranlib libc_packs.a
	rm *.o

clean:
	rm -f *.o */*.o libc_packs.a
