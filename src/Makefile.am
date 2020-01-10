SUBDIRS = data src po

EXTRA_DIST =				\
	intltool-extract.in		\
	intltool-merge.in		\
	intltool-update.in

CLEANFILES =			\
	intltool-extract	\
	intltool-merge		\
	intltool-update

WORKDIR := $(shell pwd)
SPECFILE = tsclient.spec

rpm: dist-bzip2
	mkdir -p $(WORKDIR)/rpm/SOURCES $(WORKDIR)/rpm/BUILD \
	$(WORKDIR)/rpm/RPMS $(WORKDIR)/rpm/SRPMS; \
	cp $(PACKAGE)-$(VERSION)*tar.bz2 $(WORKDIR)/rpm/SOURCES; \
	rpmbuild --define "_sourcedir $(WORKDIR)/rpm/SOURCES" \
	--define "_builddir $(WORKDIR)/rpm/BUILD" \
	--define "_srcrpmdir $(WORKDIR)/rpm/SRPMS" \
	--define "_rpmdir $(WORKDIR)/rpm/RPMS" -ba $(SPECFILE) && \
	cp $(WORKDIR)/rpm/RPMS/*/$(PACKAGE)-$(VERSION)* . && \
	cp $(WORKDIR)/rpm/SRPMS/$(PACKAGE)-$(VERSION)* .
