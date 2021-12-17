OBJ_DIR=$(BUILDDIR)/build/obj/lib_ipc_event
target=$(LIBDIR)/liblog.so

CXXFLAGS += -fPIC  \
        -I../h	

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
