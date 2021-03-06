#--------------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation.  All rights reserved.
#--------------------------------------------------------------------------------
# Bootstrap Makefile for: Operations Manager UNIX/Linux agent
# 2007-08-23
#--------------------------------------------------------------------------------

# Set up architecture and Build Root Directory
# PF (i.e. PlatForm) is either linux, solaris
SHELL=/bin/bash
PWD:=$(shell pwd)
SCX_BRD=$(shell dirname $(PWD))
PF_POSIX=true

# Get the root of the SCXPAL directory (and other easy locations)
SCXPAL_DIR := $(shell cd ../../pal; pwd)
SCXOMI_DIR := $(shell cd ../../omi/Unix; pwd -P)

SCXOMI_DEV_ROOT := $(SCXOMI_DIR)/output
SCXOMI_INCLUDE := $(SCXOMI_DEV_ROOT)/include
SCXOMI_LIBS := $(SCXOMI_DEV_ROOT)/lib

include $(SCXPAL_DIR)/build/config.mak
include $(SCX_BRD)/build/config.mak

ifndef OMI_CONFIGURE_QUALS
$(error "OMI_CONFIGURE_QUALS is not set.  Please re-run configure")
endif

# Include the real stuff
include $(SCX_BRD)/build/Makefile.version

ifndef SCX_BUILDVERSION_STATUS
$(error "Is Makefile.version missing?  Please re-run configure")
endif

# Figure out if we're doing a production build or a developer build

ifeq ($(SCX_BUILDVERSION_STATUS),Release_Build)
	BUILD_PRODUCTION=1
else
ifeq ($(SCX_BUILDVERSION_STATUS),Developer_Build)
	BUILD_PRODUCTION=0
else
$(error Unrecognized build type: $(SCX_BUILDVERSION_STATUS))
endif # Developer Build
endif # Release Build

all : ccenable pal_all omi_all installerPackage devel-deps ccdisable
installerPackage: pal_all omi_all

pal_all :
	$(MAKE) -C $(SCXPAL_DIR)/build all

clean: clean-status doxygen_clean
	-$(RMDIR) $(INTERMEDIATE_DIR)/include
	-$(RMDIR) $(INTERMEDIATE_DIR)/source
	-$(RMDIR) $(INTERMEDIATE_DIR)/test
	-$(RMDIR) $(SCX_BRD)/installer/intermediate
	-$(RMDIR) $(INTERMEDIATE_DIR)/lib*.a
	-$(RMDIR) $(INTERMEDIATE_DIR)/*.exp
	-$(RMDIR) $(INTERMEDIATE_DIR)/*.map
	-$(RMDIR) $(INTERMEDIATE_DIR)/*.cov
	-$(RMDIR) $(TARGET_DIR)
	-$(RM) $(SCX_BRD)/source/code/shared/tools/omi_preexec/strerror.cpp
	-$(RM) $(SCX_UNITTEST_ROOT)/providers/providertestutils.cpp
	-find $(SCX_BRD) -name \*~ -exec rm {} \;

# Clean the PAL as well as OM with distclean target
distclean: clean
	-$(MAKE) -C $(SCXPAL_DIR)/build clean
	-$(MAKE) -C $(SCXOMI_DIR) distclean
	-$(RMDIR) $(SCXOMI_DIR)/output*
	-$(RM) $(SCX_BRD)/build/config.mak
	-$(RM) $(SCXPAL_DIR)/build/config.mak
	-$(RM) $(SCXPAL_DIR)/build/Makefile.config_cache
	-$(RM) $(SCX_BRD)/build/Makefile.config_cache
	-$(RM) $(SCX_BRD)/build/Makefile.version
	-$(RM) -rf $(SCX_BRD)/intermediate $(SCX_BRD)/target
	-$(RM) -rf $(SCXPAL_DIR)/intermediate $(SCXPAL_DIR)/target

clean-status:
	$(ECHO) "========================= Performing make clean"

# Include the rest of the stuff we need

include $(SCX_BRD)/build/Makefile.pal
include $(SCX_BRD)/build/Makefile.macros
include $(SCX_BRD)/build/Makefile.pf.$(PF)
include $(SCX_BRD)/build/Makefile.rules
include $(SCX_BRD)/build/Makefile.components
include $(SCX_BRD)/build/Makefile.kits
include $(SCX_BRD)/build/Makefile.tests
include $(SCX_BRD)/build/Makefile.stub

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
ifeq (,$(findstring cache,$(MAKECMDGOALS)))
-include $(DEPFILES) $(DEPFILES_TEST)
endif
endif

#-------------------------------- End of File -----------------------------------
