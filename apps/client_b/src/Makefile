
OBJ_DIR=$(BUILDDIR)/build/obj/client_b
obj=server_main.cpp
target=$(BINDIR)/client_b

CXXFLAGS += -I./  \
        -I../h	\
		-I$(BUILDDIR)/libs/lib_log/h \
		-I$(BUILDDIR)/libs/lib_ipc_event/h

LDFLAGS += -L$(LIBDIR)
LDFLAGS += -lpthread \
			-lipc_event_api \
			-llog


include $(BUILDDIR)/Makefile.srcInc

$(target):$(CXXOBJS)
	@echo "CXX $(target)"
	$(CXX) $(CXXOBJS) $(LDFLAGS) -s -o $@
