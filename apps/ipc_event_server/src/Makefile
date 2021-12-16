
OBJ_DIR=$(BUILDDIR)/build/obj/ipc_event_server
# obj=server_main.cpp
target=$(BINDIR)/ipc_event_server

CXXFLAGS += -I./  \
			-I../h \
			-I$(BUILDDIR)/libs/lib_log \
			-I$(BUILDDIR)/libs/lib_ipc_event/h

LDFLAGS += -lpthread 

include $(BUILDDIR)/Makefile.srcInc

$(target):$(CXXOBJS)
	@echo "CXX $(target)"
	$(CXX) $(CXXOBJS) $(LDFLAGS) -std=c++11 -s -o $@
