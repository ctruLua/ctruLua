FREETYPE             := freetype
FREETYPE_VERSION     := $(FREETYPE)-2.6
FREETYPE_SRC         := $(FREETYPE_VERSION).tar.bz2
FREETYPE_DOWNLOAD    := "http://download.savannah.gnu.org/releases/freetype/freetype-2.6.tar.bz2"

LIBEXIF              := libexif
LIBEXIF_VERSION      := $(LIBEXIF)-0.6.21
LIBEXIF_SRC          := $(LIBEXIF_VERSION).tar.bz2
LIBEXIF_DOWNLOAD     := "http://sourceforge.net/projects/libexif/files/libexif/0.6.21/libexif-0.6.21.tar.bz2"

LIBJPEGTURBO         := libjpeg-turbo
LIBJPEGTURBO_VERSION := $(LIBJPEGTURBO)-1.4.1
LIBJPEGTURBO_SRC     := $(LIBJPEGTURBO_VERSION).tar.gz
LIBJPEGTURBO_DOWNLOAD := "http://sourceforge.net/projects/libjpeg-turbo/files/1.4.1/libjpeg-turbo-1.4.1.tar.gz"

LIBPNG               := libpng
LIBPNG_VERSION       := $(LIBPNG)-1.6.17
LIBPNG_SRC           := $(LIBPNG_VERSION).tar.xz
LIBPNG_DOWNLOAD      := "http://prdownloads.sourceforge.net/libpng/libpng-1.6.17.tar.xz"

SQLITE               := sqlite
SQLITE_VERSION       := $(SQLITE)-autoconf-3081002
SQLITE_SRC           := $(SQLITE_VERSION).tar.gz
SQLITE_DOWNLOAD      := "http://www.sqlite.org/2015/sqlite-autoconf-3081002.tar.gz"

ZLIB                 := zlib
ZLIB_VERSION         := $(ZLIB)-1.2.8
ZLIB_SRC             := $(ZLIB_VERSION).tar.gz
ZLIB_DOWNLOAD        := "http://prdownloads.sourceforge.net/libpng/zlib-1.2.8.tar.gz"

LIBOGG               := libogg
LIBOGG_VERSION       := $(LIBOGG)-1.3.2
LIBOGG_SRC           := $(LIBOGG_VERSION).tar.gz
LIBOGG_DOWNLOAD      := "http://downloads.xiph.org/releases/ogg/libogg-1.3.2.tar.gz"

LIBVORBIS            := libvorbis
LIBVORBIS_VERSION    := $(LIBVORBIS)-1.3.5
LIBVORBIS_SRC        := $(LIBVORBIS_VERSION).tar.gz
LIBVORBIS_DOWNLOAD   := "http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.5.tar.gz"

export PORTLIBS        := $(CURDIR)/build
export PATH            := $(DEVKITARM)/bin:$(PATH)
export PKG_CONFIG_PATH := $(PORTLIBS)/lib/pkgconfig
export CFLAGS          := -march=armv6k -mtune=mpcore -mfloat-abi=hard -O3 \
                          -mword-relocations -fomit-frame-pointer -ffast-math
export CPPFLAGS        := -I$(PORTLIBS)/include
export LDFLAGS         := -L$(PORTLIBS)/lib

.PHONY: all old_all install install-zlib clean \
        $(FREETYPE) \
        $(LIBEXIF) \
        $(LIBJPEGTURBO) \
        $(LIBPNG) \
        $(SQLITE) \
        $(ZLIB)
all: zlib install-zlib freetype libexif libjpeg-turbo libpng sqlite install
	@echo "Finished!"

create_build_dir:
	@[ -d $(CURDIR)/build ] || mkdir -p $(CURDIR)/build
	@[ -d $(CURDIR)/build/include ] || mkdir -p $(CURDIR)/build/include
	@[ -d $(CURDIR)/build/lib ] || mkdir -p $(CURDIR)/build/lib

old_all:
	@echo "Please choose one of the following targets:"
	@echo "  $(FREETYPE) (requires zlib to be installed)"
	@echo "  $(LIBEXIF)"
	@echo "  $(LIBJPEGTURBO)"
	@echo "  $(LIBPNG) (requires zlib to be installed)"
	@echo "  $(SQLITE)"
	@echo "  $(ZLIB)"

$(FREETYPE): $(FREETYPE_SRC)
	@[ -d $(FREETYPE_VERSION) ] || tar -xf $<
	@cd $(FREETYPE_VERSION) && \
	 ./configure --prefix=$(PORTLIBS) --host=arm-none-eabi --disable-shared --enable-static --without-harfbuzz
	@$(MAKE) -C $(FREETYPE_VERSION)
	@make create_build_dir
	@cp -srf $(CURDIR)/$(FREETYPE_VERSION)/include/. $(CURDIR)/build/include
	@cp -sf $(CURDIR)/$(FREETYPE_VERSION)/objs/.libs/libfreetype.a $(CURDIR)/build/lib/libfreetype.a

$(LIBEXIF): $(LIBEXIF_SRC)
	@[ -d $(LIBEXIF_VERSION) ] || tar -xf $<
	@cd $(LIBEXIF_VERSION) && \
	 ./configure --prefix=$(PORTLIBS) --host=arm-none-eabi --disable-shared --enable-static
	@$(MAKE) -C $(LIBEXIF_VERSION)

$(LIBJPEGTURBO): $(LIBJPEGTURBO_SRC)
	@[ -d $(LIBJPEGTURBO_VERSION) ] || tar -xf $<
	@cd $(LIBJPEGTURBO_VERSION) && \
	 ./configure --prefix=$(PORTLIBS) --host=arm-none-eabi --disable-shared --enable-static
	@$(MAKE) CFLAGS+="\"-Drandom()=rand()\"" -C $(LIBJPEGTURBO_VERSION)
	@make create_build_dir
	@cp -sf $(CURDIR)/$(LIBJPEGTURBO_VERSION)/*.h $(CURDIR)/build/include
	@cp -sf $(CURDIR)/$(LIBJPEGTURBO_VERSION)/.libs/libjpeg.a $(CURDIR)/build/lib/libjpeg.a

$(LIBPNG): $(LIBPNG_SRC)
	@[ -d $(LIBPNG_VERSION) ] || tar -xf $<
	@cd $(LIBPNG_VERSION) && \
	 ./configure --prefix=$(PORTLIBS) --host=arm-none-eabi --disable-shared --enable-static
	@$(MAKE) -C $(LIBPNG_VERSION)
	@make create_build_dir
	@cp -sf $(CURDIR)/$(LIBPNG_VERSION)/*.h $(CURDIR)/build/include
	@cp -sf $(CURDIR)/$(LIBPNG_VERSION)/.libs/*.a $(CURDIR)/build/lib/libpng.a

# sqlite won't work with -ffast-math
$(SQLITE): $(SQLITE_SRC)
	@[ -d $(SQLITE_VERSION) ] || tar -xf $<
	@cd $(SQLITE_VERSION) && \
	 CFLAGS="$(filter-out -ffast-math,$(CFLAGS)) -DSQLITE_OS_OTHER=1" ./configure --disable-shared --disable-threadsafe --disable-dynamic-extensions --host=arm-none-eabi --prefix=$(PORTLIBS)
	# avoid building sqlite3 shell
	@$(MAKE) -C $(SQLITE_VERSION) libsqlite3.la

$(ZLIB): $(ZLIB_SRC)
	@[ -d $(ZLIB_VERSION) ] || tar -xf $<
	@cd $(ZLIB_VERSION) && \
	 CHOST=arm-none-eabi ./configure --static --prefix=$(PORTLIBS)
	@$(MAKE) -C $(ZLIB_VERSION)
	@make create_build_dir
	@cp -sf $(CURDIR)/$(ZLIB_VERSION)/*.h $(CURDIR)/build/include
	@cp -sf $(CURDIR)/$(ZLIB_VERSION)/libz.a $(CURDIR)/build/lib/libz.a

$(LIBOGG): $(LIBOGG_SRC)
	@[ -d $(LIBOGG_VERSION) ] || tar -xf $<
	@cd $(LIBOGG_VERSION) && \
	 ./configure --prefix=$(PORTLIBS) --host=arm-none-eabi --disable-shared --enable-static
	@$(MAKE) -C $(LIBOGG_VERSION)
	@make create_build_dir
	@cp -srf $(CURDIR)/$(LIBOGG_VERSION)/include/. $(CURDIR)/build/include
	@cp -sf $(CURDIR)/$(LIBOGG_VERSION)/src/.libs/*.a $(CURDIR)/build/lib

$(LIBVORBIS): $(LIBVORBIS_SRC)
	@[ -d $(LIBVORBIS_VERSION) ] || tar -xf $<
	@cd $(LIBVORBIS_VERSION) && \
	 ./configure --prefix=$(PORTLIBS) --host=arm-none-eabi --disable-shared --enable-static
	@$(MAKE) -C $(LIBVORBIS_VERSION)
	@make create_build_dir
	@cp -srf $(CURDIR)/$(LIBVORBIS_VERSION)/include/. $(CURDIR)/build/include
	@cp -sf $(CURDIR)/$(LIBVORBIS_VERSION)/lib/.libs/*.a $(CURDIR)/build/lib

# Downloads
$(ZLIB_SRC):
	wget -O $@ $(ZLIB_DOWNLOAD)

$(FREETYPE_SRC):
	wget -O $@ $(FREETYPE_DOWNLOAD)

$(LIBEXIF_SRC):
	wget -O $@ $(LIBEXIF_DOWNLOAD)

$(LIBJPEGTURBO_SRC):
	wget -O $@ $(LIBJPEGTURBO_DOWNLOAD)

$(LIBPNG_SRC):
	wget -O $@ $(LIBPNG_DOWNLOAD)

$(SQLITE_SRC):
	wget -O $@ $(SQLITE_DOWNLOAD)

$(LIBOGG_SRC):
	wget -O $@ $(LIBOGG_DOWNLOAD)

$(LIBVORBIS_SRC):
	wget -O $@ $(LIBVORBIS_DOWNLOAD)

install-zlib:
	@$(MAKE) -C $(ZLIB_VERSION) install

install:
	@[ ! -d $(FREETYPE_VERSION) ] || $(MAKE) -C $(FREETYPE_VERSION) install
	@[ ! -d $(LIBEXIF_VERSION) ] || $(MAKE) -C $(LIBEXIF_VERSION) install
	@[ ! -d $(LIBJPEGTURBO_VERSION) ] || $(MAKE) -C $(LIBJPEGTURBO_VERSION) install
	@[ ! -d $(LIBPNG_VERSION) ] || $(MAKE) -C $(LIBPNG_VERSION) install
	@[ ! -d $(SQLITE_VERSION) ] || $(MAKE) -C $(SQLITE_VERSION) install-libLTLIBRARIES install-data
	@[ ! -d $(LIBOGG_VERSION) ] || $(MAKE) -C $(LIBOGG_VERSION) install
	@[ ! -d $(LIBVORBIS_VERSION) ] || $(MAKE) -C $(LIBVORBIS_VERSION) install

clean:
	@$(RM) -r $(FREETYPE_VERSION)
	@$(RM) -r $(LIBEXIF_VERSION)
	@$(RM) -r $(LIBJPEGTURBO_VERSION)
	@$(RM) -r $(LIBPNG_VERSION)
	@$(RM) -r $(SQLITE_VERSION)
	@$(RM) -r $(ZLIB_VERSION)
	@$(RM) -r $(LIBOGG_VERSION)
	@$(RM) -r $(LIBVORBIS_VERSION)
	@rm -rf $(CURDIR)/build
	@rm -f $(CURDIR)/*.tar.*
