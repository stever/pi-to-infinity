CFLAGS = \
	-Wall \
    -g \
    -std=c99 \
	-DDEBUG \
	-DSTANDALONE \
    -DTARGET_POSIX \
    -DUSE_EXTERNAL_OMX \
    -DHAVE_LIBBCM_HOST \
    -DHAVE_LIBOPENMAX=2 \
    -DUSE_EXTERNAL_LIBBCM_HOST \
    -DUSE_VCHIQ_ARM \
    -DOMX \
    -DOMX_SKIP64BIT \
    -D_LINUX \
    -D_REENTRANT \
    -D_LARGEFILE64_SOURCE \
    -D_FILE_OFFSET_BITS=64 \
    -D__STDC_CONSTANT_MACROS \
    -D__STDC_LIMIT_MACROS \
    -fPIC \
    -DPIC \
    -U_FORTIFY_SOURCE \
    -ftree-vectorize \
    -pipe \
    -Wno-psabi \
    -Wno-deprecated-declarations
LDFLAGS = \
	-Wl,--no-whole-archive \
	-rdynamic
INCLUDES = \
	-I$(SDKSTAGE)/opt/vc/include/
LIBS = \
	-L$(SDKSTAGE)/opt/vc/lib/ \
	-lgpm \
	-lm \
	-lrt \
	-lGLESv2 \
	-lEGL \
	-lopenmaxil \
	-lbcm_host \
	-lvcos \
	-lpthread \
	-lvchiq_arm
SOURCES = \
	main.c \
	egl_display.c \
	gfx_engine.c \
	keyboard.c \
	mouse.c \
	targa.c \
	player.c
OBJECTS = $(SOURCES:.c=.o)
EXE = game

.c.o:
	@rm -f $@ 
	$(CC) $(CFLAGS) $(INCLUDES) -g -c $< -o $@

default: $(OBJECTS)
	$(CC) -o $(EXE) $(LDFLAGS) $(LIBS) /opt/vc/lib/libilclient.a $(OBJECTS)

clean:
	for i in $(OBJECTS); do (if test -e "$$i"; then ( rm $$i ); fi ); done
	@rm -f $(EXE) $(LIB)
