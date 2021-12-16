OBJ_DIR=$(BUILDDIR)/build/obj/lib_ipc_event
target=$(LIBDIR)/libipc_event_api.so

# CXXFLAGS += -fPIC  \
#             -I./  \
#             -I../h  \
#             -I$(RootPath)/common/h   \
#             -I$(RootPath)/libraries/export/slog  \
#             -I$(RootPath)/libraries/export/interprocess  \
#             -I$(RootPath)/libraries/export/timer        \
#             -I$(RootPath)/libraries/export/cjson        \
#             -I$(RootPath)/libraries/export/fileOpr      \
#             -I$(RootPath)/libraries/export/ota          \
#             -I$(RootPath)/libraries/export/commInfoShm

CXXFLAGS += -fPIC  \
        -I../h	\
		-I$(BUILDDIR)/libs/lib_log

# include $(RootPath)/Makefile.srcInc
include $(BUILDDIR)/Makefile.srcInc

LDFLAGS += -L$(LIBDIR)

LDFLAGS += -lpthread

# LDFLAGS += -lipc_evt_api    \
#             -lslog   \
#             -lcjson  \
#             -lfileOpr  \
#             -lcommInfoShm

$(target): $(CXXOBJS)  
	@echo "CXX $(target)"
	@$(CXX) -shared  $(CXXOBJS) $(LDFLAGS) -s -o $@

# install:
# 	cp -f $(EXE) $(LIBDIR)
