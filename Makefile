#======================================================================
# @(#) $Id$
# Makefile for all SIM source code
#======================================================================

#----------------------------------------------------------------------
# default: make all object and executable files
#----------------------------------------------------------------------

default: objs/Makefile programs/Makefile scripts/Makefile
	@ for dir in objs programs scripts; \
		do (cd $$dir; \
			echo "Making default in `pwd`"; \
			make default); \
	done

#----------------------------------------------------------------------
# clean: remove all object, executable and make files
#----------------------------------------------------------------------

clean:
	@ for dir in objs programs scripts; \
		do (cd $$dir; \
			echo "Making clean in `pwd`"; \
			make clean; \
			rm -f Makefile); \
	done

#----------------------------------------------------------------------
# install: make all object and executable files and install
#----------------------------------------------------------------------

install: objs/Makefile programs/Makefile scripts/Makefile
	@ for dir in objs programs scripts; \
		do (cd $$dir; \
			echo "Making install in `pwd`"; \
			make install); \
	done

# rule for subtree makefiles
%/Makefile: $(SIM_CENTRAL_TREE)/%/RCS/Makefile,v
	(cd $*; co $(SIM_CENTRAL_TREE)/$*/RCS/Makefile,v)
