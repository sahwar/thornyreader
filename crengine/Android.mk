LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := crengine
LOCAL_CFLAGS    := $(APP_CFLAGS)
LOCAL_CPPFLAGS  := $(APP_CPPFLAGS)
LOCAL_ARM_MODE := $(APP_ARM_MODE)

LOCAL_STATIC_LIBRARIES  := thornyreader libmobi
LOCAL_LDLIBS            += -llog -lz
LOCAL_CPP_FEATURES      += exceptions

LOCAL_CFLAGS            += -DHAVE_CONFIG_H
LOCAL_CFLAGS            += -DLINUX=1
LOCAL_CFLAGS            += -D_LINUX=1
LOCAL_CFLAGS            += -DFT2_BUILD_LIBRARY=1
LOCAL_CFLAGS            += -DCR3_ANTIWORD_PATCH=1
LOCAL_CFLAGS            += -DENABLE_ANTIWORD=1

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../thornyreader/include \
    $(LOCAL_PATH)/../ \
    $(LOCAL_PATH)/libpng \
    $(LOCAL_PATH)/freetype/include

LOCAL_SRC_FILES := \
	src/CreBridge.cpp \
	src/trlog.cpp \
	src/bookmark.cpp \
	src/lvtoc.cpp \
	src/chmfmt.cpp \
    src/cp_stats.cpp \
    src/crtxtenc.cpp \
    src/epubfmt.cpp \
    src/hyphman.cpp \
    src/lstridmap.cpp \
    src/lvbmpbuf.cpp \
    src/lvdocview.cpp \
    src/crcss.cpp \
    src/lvdrawbuf.cpp \
    src/lvfnt.cpp \
    src/lvfntman.cpp \
    src/lvimg.cpp \
    src/lvpagesplitter.cpp \
    src/lvrend.cpp \
    src/lvstream.cpp \
    src/lvstring.cpp \
    src/lvstsheet.cpp \
    src/lvstyles.cpp \
    src/lvtextfm.cpp \
    src/lvtinydom.cpp \
    src/lvxml.cpp \
    src/pdbfmt.cpp \
    src/props.cpp \
    src/rtfimp.cpp \
    src/txtselector.cpp \
    src/wordfmt.cpp \
    src/docxhandler.cpp \
    src/mobihandler.cpp \
    src/RectHelper.cpp \
    src/crconfig.cpp

LOCAL_SRC_FILES += \
    libpng/pngerror.c  \
    libpng/pngget.c  \
    libpng/pngpread.c \
    libpng/pngrio.c \
    libpng/pngrutil.c \
    libpng/pngvcrd.c \
    libpng/png.c \
    libpng/pngwrite.c \
    libpng/pngwutil.c \
    libpng/pnggccrd.c \
    libpng/pngmem.c \
    libpng/pngread.c \
    libpng/pngrtran.c \
    libpng/pngset.c \
    libpng/pngtrans.c \
    libpng/pngwio.c \
    libpng/pngwtran.c

LOCAL_SRC_FILES += \
    libjpeg/jcapimin.c \
    libjpeg/jchuff.c \
    libjpeg/jcomapi.c \
    libjpeg/jctrans.c \
    libjpeg/jdcoefct.c \
    libjpeg/jdmainct.c \
    libjpeg/jdpostct.c \
    libjpeg/jfdctfst.c \
    libjpeg/jidctred.c \
    libjpeg/jutils.c \
    libjpeg/jcapistd.c \
    libjpeg/jcinit.c \
    libjpeg/jcparam.c \
    libjpeg/jdapimin.c \
    libjpeg/jdcolor.c \
    libjpeg/jdmarker.c \
    libjpeg/jdsample.c \
    libjpeg/jfdctint.c \
    libjpeg/jmemmgr.c \
    libjpeg/jccoefct.c \
    libjpeg/jcmainct.c \
    libjpeg/jcphuff.c \
    libjpeg/jdapistd.c \
    libjpeg/jddctmgr.c \
    libjpeg/jdmaster.c \
    libjpeg/jdtrans.c \
    libjpeg/jidctflt.c \
    libjpeg/jmemnobs.c \
    libjpeg/jccolor.c \
    libjpeg/jcmarker.c \
    libjpeg/jcprepct.c \
    libjpeg/jdatadst.c \
    libjpeg/jdhuff.c \
    libjpeg/jdmerge.c \
    libjpeg/jerror.c \
    libjpeg/jidctfst.c \
    libjpeg/jquant1.c \
    libjpeg/jcdctmgr.c \
    libjpeg/jcmaster.c \
    libjpeg/jcsample.c \
    libjpeg/jdatasrc.c \
    libjpeg/jdinput.c \
    libjpeg/jdphuff.c \
    libjpeg/jfdctflt.c \
    libjpeg/jidctint.c \
    libjpeg/jquant2.c

LOCAL_SRC_FILES += \
    freetype/src/autofit/autofit.c \
    freetype/src/bdf/bdf.c \
    freetype/src/cff/cff.c \
    freetype/src/base/ftbase.c \
    freetype/src/base/ftbbox.c \
    freetype/src/base/ftbdf.c \
    freetype/src/base/ftbitmap.c \
    freetype/src/base/ftgasp.c \
    freetype/src/cache/ftcache.c \
    freetype/src/base/ftglyph.c \
    freetype/src/base/ftgxval.c \
    freetype/src/gzip/ftgzip.c \
    freetype/src/base/ftinit.c \
    freetype/src/lzw/ftlzw.c \
    freetype/src/base/ftmm.c \
    freetype/src/base/ftpatent.c \
    freetype/src/base/ftotval.c \
    freetype/src/base/ftpfr.c \
    freetype/src/base/ftstroke.c \
    freetype/src/base/ftsynth.c \
    freetype/src/base/ftsystem.c \
    freetype/src/base/fttype1.c \
    freetype/src/base/ftwinfnt.c \
    freetype/src/base/ftxf86.c \
    freetype/src/winfonts/winfnt.c \
    freetype/src/pcf/pcf.c \
    freetype/src/pfr/pfr.c \
    freetype/src/psaux/psaux.c \
    freetype/src/pshinter/pshinter.c \
    freetype/src/psnames/psmodule.c \
    freetype/src/raster/raster.c \
    freetype/src/sfnt/sfnt.c \
    freetype/src/smooth/smooth.c \
    freetype/src/truetype/truetype.c \
    freetype/src/type1/type1.c \
    freetype/src/cid/type1cid.c \
    freetype/src/type42/type42.c
	
LOCAL_SRC_FILES += \
    chmlib/src/chm_lib.c \
    chmlib/src/lzx.c 

LOCAL_SRC_FILES += \
    antiword/asc85enc.c \
    antiword/blocklist.c \
    antiword/chartrans.c \
    antiword/datalist.c \
    antiword/depot.c \
    antiword/doclist.c \
    antiword/fail.c \
    antiword/finddata.c \
    antiword/findtext.c \
    antiword/fontlist.c \
    antiword/fonts.c \
    antiword/fonts_u.c \
    antiword/hdrftrlist.c \
    antiword/imgexam.c \
    antiword/listlist.c \
    antiword/misc.c \
    antiword/notes.c \
    antiword/options.c \
    antiword/out2window.c \
    antiword/pdf.c \
    antiword/pictlist.c \
    antiword/prop0.c \
    antiword/prop2.c \
    antiword/prop6.c \
    antiword/prop8.c \
    antiword/properties.c \
    antiword/propmod.c \
    antiword/rowlist.c \
    antiword/sectlist.c \
    antiword/stylelist.c \
    antiword/stylesheet.c \
    antiword/summary.c \
    antiword/tabstop.c \
    antiword/unix.c \
    antiword/utf8.c \
    antiword/word2text.c \
    antiword/worddos.c \
    antiword/wordlib.c \
    antiword/wordmac.c \
    antiword/wordole.c \
    antiword/wordwin.c \
    antiword/xmalloc.c

include $(BUILD_EXECUTABLE)