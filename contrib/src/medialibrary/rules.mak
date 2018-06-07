MEDIALIB_HASH := 3e6cc1576435653aee83abdf3338fd1e97236e7f
MEDIALIB_VERSION := git-$(MEDIALIB_HASH)
MEDIALIB_GITURL := https://code.videolan.org/videolan/medialibrary/-/archive/$(MEDIALIB_HASH)/medialibrary-$(MEDIALIB_HASH).tar.gz

PKGS += medialibrary
ifeq ($(call need_pkg,"medialibrary"),)
PKGS_FOUND += medialibrary
endif

$(TARBALLS)/medialib-$(MEDIALIB_VERSION).tar.gz:
	$(call download,$(MEDIALIB_GITURL))

.sum-medialib: medialib-$(MEDIALIB_VERSION).tar.gz
	$(warning $@ not implemented)
	touch $@

medialib: medialib-$(MEDIALIB_VERSION).tar.gz .sum-medialib
	rm -rf $@-$(MEDIALIB_VERSION)
	mkdir -p $@-$(MEDIALIB_VERSION)
	tar xvzf "$<" --strip-components=1 -C $@-$(MEDIALIB_VERSION)
	$(MOVE)

.medialib: medialib
	$(RECONF)
	cd $< && $(HOSTVARS_PIC) ./configure --disable-tests --without-libvlc $(HOSTCONF)
	cd $< && $(MAKE)
	cd $< && $(MAKE) install
	touch $@

