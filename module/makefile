TOP_DIR=../../../../..
include $(TOP_DIR)/application/qqphoto/makeinclude/makefile.qqphoto

INC:=-I./ -I./spp_incl -I../comm $(INC) -I$(TOP_DIR)/extern/pdu_parser

INC_UPP_ASYN=-I$(TOP_DIR)/extern/upp_api/include
LIB_UPP_ASYN=$(TOP_DIR)/extern/upp_api/lib/libuppa_asyn.a \
	$(TOP_DIR)/extern/upp_api/lib/libuppurl.a

	
INC+=-I$(TOP_DIR)/plib/base_class/base_lib/include -I$(TOP_DIR)/plib/outerlib/pdu/include/ -I$(TOP_DIR)/plib/outerlib/l5/include -I$(TOP_DIR)/plib/outerlib/logapi/include
INC+=-I$(TOP_DIR)/comm/include
INC+=-I$(TOP_DIR)/application/qqphoto/comm/photoapi/include -I$(TOP_DIR)/application/qqphoto/comm/photobase/include -I$(TOP_DIR)/application/qqphoto/comm/photoutils/include
INC+=-I$(TOP_DIR)/extern/upp_api/include
INC+=-I$(TOP_DIR)/extern/libcomment/include
INC+=-I$(TOP_DIR)/application/qqphoto/comm/dclog/include
INC+=-I$(TOP_DIR)/extern/curl/include
INC+=-I$(TOP_DIR)/plib/application/qzdata/include
INC+=-I$(TOP_DIR)/application/qqphoto/comm/photo_log/include/
INC+=-I$(TOP_DIR)//plib/outerlib/qzdclog/include
INC+=-I$(TOP_DIR)/application/qqphoto/comm/asyn/httpreq
INC+=$(INC_DCLOGAPI)
INC+=-I$(TOP_DIR)/application/qqphoto/comm/photo_log/include

INC+=-I$(TOP_DIR)/plib/base_class/extern_libs/tbase/include
INC+=-I$(TOP_DIR)/plib/base_class/extern_libs/tdev/include
INC+=-I$(TOP_DIR)/plib/application/panel_auth/include/
INC+=-I$(TOP_DIR)/plib/base_class/extern_libs/mysql/include/mysql

INC+=-I$(TOP_DIR)/extern/

INC+=$(INC_UPP_ASYN)


LIB:=$(LIB_ASYNC_CGI) $(LIB)
LIB+=-L$(TOP_DIR)/comm/lib -lphotocomm
LIB+=-L$(TOP_DIR)/application/qqphoto/comm/photobase/lib -lphotobase
LIB+=$(TOP_DIR)/application/qqphoto/server/upp/comm/libuppcomm.a
LIB+=-L$(TOP_DIR)/extern/curl/lib -lcurl
LIB+=$(LIB_UPP_ASYN)
LIB+=-L$(TOP_DIR)/extern/asn/lib -lasn1c++ \
	-L$(TOP_DIR)/extern/outlib -lssl -lcrypto -lz -ldl -lrt
LIB+=$(TOP_DIR)/extern/libcomment/lib/libUGCAPI.a
LIB+=$(TOP_DIR)/application/qqphoto/comm/asyn/lib/libphotohttp.a
LIB+=-L$(TOP_DIR)/plib/outerlib/pdu/lib/ -lpdu_header -L$(TOP_DIR)/plib/outerlib/l5/lib -lqos_client -ll5helper $(TOP_DIR)/plib/outerlib/logapi/lib/libmsglog.a
LIB+=$(TOP_DIR)/extern/outlib/libz.a


LIB+=$(TOP_DIR)/application/qqphoto/comm/photo_log/lib/libphotolog.a
LIB+=$(LIB_DCLOGAPI)
LIB+=$(TOP_DIR)/plib/outerlib/qzdclog/lib/libtdclogapi.a
LIB+=$(TOP_DIR)/plib/outerlib/qzdclog/lib/libtdclogapi_mt.a
LIB+=$(TOP_DIR)/application/qqphoto/comm/photoutils/lib/libphotoutils.4.1.a

LIB+=$(TOP_DIR)/plib/application/qzdata/lib/libqzbitmap_l5client.a
LIB+=$(TOP_DIR)/plib/base_class/base_lib/lib/libnetwork.a
LIB+=$(TOP_DIR)/comm/lib/libphotocomm.a
##LIB+=$(TOP_DIR)/plib/outerlib/logapi/lib/libmsglog.a

BIN=spp_upp.so
SRCS_ALL=$(wildcard *.cpp)
SRCS=$(filter-out upp_gtest.cpp, $(SRCS_ALL))
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
DEPS=$(SRCS:.cpp=.d)

PATH_QZONE_RUNNING=/usr/local/qzone_v3.0/
C_FLAGS:=-g -m32 -DSUS_LINUX -D_MP_MODE -fPIC -shared -D_UPP_SVR_DEBUG_ -DQZONE_PROJECT_ROOT=\"$(PATH_QZONE_RUNNING)\"#-z defs

.PHONY: all dep clean

C_FLAGS+=-Wall -Wno-unused-parameter -Wno-unused-variable

all:DEPEND $(BIN)

DEPEND:
	make -C $(TOP_DIR)/application/qqphoto/comm/photobase/
	make -C $(TOP_DIR)/application/qqphoto/comm/photoutils/	
	make -C ../comm
	
$(BIN):$(OBJS) $(DEP_LIB)
	$(CXX) $(C_FLAGS)  $(OBJS) $(DEP_LIB) -g -o $@ $(LIB)

%.o:%.cpp
	@echo Compiling $< ==> $@
	$(CXX) $(C_FLAGS) -c $(INC) -g -o  $@ $<

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

$(OBJS_DIR)%.d:$(SRC_DIR)%.cpp
	$(CXX) -MM $(C_FLAGS) $(INC) $< > $@

clean:
	rm -f $(OBJS) $(BIN) $(DEPS)
	make -C ../comm clean
	make -C $(TOP_DIR)/application/qqphoto/comm/photobase clean
	make -C $(TOP_DIR)/application/qqphoto/comm/photoutils clean

commdep:
	make -C $(TOP_DIR)/comm/
	make -C $(TOP_DIR)/application/qqphoto/cgi/comm_cgi/
	make -C $(TOP_DIR)/application/qqphoto/application/

cleancommdep:
	make -C $(TOP_DIR)/comm clean
	make -C $(TOP_DIR)/application/qqphoto/cgi/comm_cgi clean
	make -C $(TOP_DIR)/application/qqphoto/application clean
	
