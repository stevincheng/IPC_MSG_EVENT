
OBJ_DIR=$(BUILDDIR)/build/obj/log_manager
target=$(BINDIR)/log_manager


CXXFLAGS += -I./  \
			-I../h	\
			-I$(BUILDDIR)/libs/system_config/h	\
			-I$(BUILDDIR)/libs/lib_log/h 

LDFLAGS += -lpthread 

LDFLAGS += -L$(LIBDIR)
LDFLAGS += -llog 


include $(BUILDDIR)/Makefile.srcInc

$(target):$(CXXOBJS)
	@echo "CXX $(target)"
	$(CXX) $(CXXOBJS) $(LDFLAGS) -s -o $@
