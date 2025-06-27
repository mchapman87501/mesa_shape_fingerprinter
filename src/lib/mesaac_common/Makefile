include ../../../top.mk

MODULES=mesaac_common
OBJS=$(MODULES:%=%.o)
SRCS=$(MODULES:%=%.cpp)

#CXXFLAGS = -O3 -D_FILE_OFFSET_BITS=64 -DHAVE_LARGEFILE_SUPPORT
CPPFLAGS := $(CPPFLAGS) -I../../../include -I/usr/local/openeye/include
LDFLAGS += -L/usr/local/openeye/lib
RANLIB = ranlib

TARGETLIB = libmesaac_common.a
PROGRAMS = 

default all: $(PROGRAMS) $(TARGETLIB)

clean::		
	$(RM) $(PROGRAMS) $(TARGETLIB)

install: install_headers install_libs

INCDIR = $(DESTDIR)$(incdir)/
install_headers:
	# $(INSTALL_DIR) $(INCDIR)
	# set -e; \
	# for f in *.h;  do \
	#     $(INSTALL) $$f $(INCDIR)/$$f; \
	# done

install_libs: $(TARGETLIB)
	$(INSTALL) $(TARGETLIB) $(DESTDIR)$(libdir)/$(TARGETLIB)
	$(RANLIB) $(DESTDIR)$(libdir)/$(TARGETLIB)

$(TARGETLIB): $(OBJS)
	$(AR) -ru $@ $(OBJS)
	$(RANLIB) $@
