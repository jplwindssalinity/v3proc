#======================================================================
# @(#) $Id$
# Makefile for all SIM source code
#======================================================================

#----------------------------------------------------------------------
# default: make all object and executable files
#----------------------------------------------------------------------

default: eadata/Makefile objs/Makefile programs/Makefile scripts/Makefile \
		HDF/Makefile.svt
	@ (cd HDF; echo "Making default (Makefile.svt) in `pwd`"; \
	make -f Makefile.svt)
	@ for dir in eadata objs programs scripts; \
		do (cd $$dir; \
			echo "Making default in `pwd`"; \
			make default); \
	done

#----------------------------------------------------------------------
# clean: remove all object, executable and make files
#----------------------------------------------------------------------

clean:
	@ (cd HDF; echo "Making clean (Makefile.svt) in `pwd`"; \
	make clean -f Makefile.svt)
	@ for dir in eadata objs programs scripts; \
		do (cd $$dir; \
			echo "Making clean in `pwd`"; \
			make clean; \
			/bin/rm -f Makefile); \
	done

#----------------------------------------------------------------------
# install: make all object and executable files and install
#----------------------------------------------------------------------

install: eadata/Makefile objs/Makefile programs/Makefile scripts/Makefile
	@ for dir in eadata objs programs scripts; \
		do (cd $$dir; \
			echo "Making install in `pwd`"; \
			make install); \
	done

tree: HDF/Makefile.svt
	cd HDF; make -f Makefile.svt tree

# rule for subtree makefiles
%/Makefile: $(SIM_CENTRAL_TREE)/src/%/RCS/Makefile,v
	(cd $*; co $(SIM_CENTRAL_TREE)/src/$*/RCS/Makefile,v)

HDF/Makefile.svt: $(SIM_CENTRAL_TREE)/src/HDF/RCS/Makefile.svt,v
	(cd HDF; co $(SIM_CENTRAL_TREE)/src/HDF/RCS/Makefile.svt,v)
