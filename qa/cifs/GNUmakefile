TOPDIR = ../..
include $(TOPDIR)/src/include/builddefs

TESTDIR = $(PCP_VAR_DIR)/testsuite/cifs
ROOTFILES = $(shell echo *.tgz)

default default_pcp setup:

install install_pcp: $(ROOTFILES)
	$(INSTALL) -m 755 -d $(TESTDIR)
	$(INSTALL) -m 644 -f $(ROOTFILES) $(TESTDIR)
	$(INSTALL) -m 644 -f GNUmakefile.install $(TESTDIR)/GNUmakefile

include $(BUILDRULES)
