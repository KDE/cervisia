GMOFILES = ko.gmo
POFILES = $(GMOFILES:.gmo=.po) 

localedir = $(kde_locale)

MAINTAINERCLEANFILES = $(GMOFILES)

GMSGFMT = @GMSGFMT@
MSGFMT = @MSGFMT@
XGETTEXT = @XGETTEXT@
EXTRA_DIST = $(PACKAGE).pot $(POFILES) $(GMOFILES)


SUFFIXES = .po .gmo

.po.mo:
	$(MSGFMT) -o $@ $<

.po.gmo:
	file=$(srcdir)/`echo $* | sed 's,.*/,,'`.gmo \
	&& rm -f $$file && $(GMSGFMT) -o $$file $< 

all: $(GMOFILES)

install: all
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	destdir=$(DESTDIR)$(localedir); \
	lang=`echo $$cat | sed 's/\.gmo//'`; \
	name=$(PACKAGE).mo \
	dir=$$destdir/$$lang/LC_MESSAGES; \
	$(mkinstalldirs) $$dir; \
	$(INSTALL_DATA) $$cat $$dir/$$name; \
	  echo "installing $$cat as $$dir/$$name"; \
	done

uninstall-local:
	@catalogs='$(GMOFILES)'; \
	for cat in $$catalogs; do \
	destdir=$(localedir); \
	lang=`echo $$cat | sed 's/\.gmo//'`; \
	name=$(PACKAGE).mo; \
	dir=$$destdir/$$lang/LC_MESSAGES; \
	rm -f $$cat $$dir/$$name; \
	echo "removing $$dir/$$name" ; \
	done

merge:
	@catalogs='$(POFILES)'; \
	for cat in $$catalogs; do \
		name=$(PACKAGE).pot ; \
		echo $$cat $$name; \
		msgmerge $$cat $$name > $$cat.new ; \
		if cmp $$cat $$cat.new >/dev/null; then \
			rm $$cat.new;  \
		else  \
			mv $$cat.new $$cat ; \
		fi; \
	done

