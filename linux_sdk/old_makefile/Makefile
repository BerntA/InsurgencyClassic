#
# SDK Makefile for x86 Linux
#
#

#############################################################################
# Developer configurable items
#############################################################################

# the name of the mod binary (_i486.so is appended to the end)
NAME=server
STATSNAME=server
# the location of the vcproj that builds the mod
MOD_PROJ=../dlls/hl_sdk.vcproj
STATS_PROJ=../stats/ins_statsman/ins_statsman.vcproj
# the name of the mod configuration (typically <proj name>_<build type><build target>)
MOD_CONFIG=server_ReleaseSDKWin32
STATS_CONFIG=ins_statsman_ReleaseWin32

# the directory the base binaries (tier0_i486.so, etc) are located
GAME_DIR=~/hl2server
MOD_NAME=Insurgency

# compiler options (gcc 3.4.1 or above is required)
CC=/usr/bin/colorgcc
CPLUS=/usr/bin/colorgcc
CLINK=/usr/bin/colorgcc
LIB_DIR=/usr/lib/gcc/i486-linux-gnu/4.0.3

# put any compiler flags you want passed here
USER_CFLAGS=

# link flags for your mod, make sure to include any special libraries here
LDFLAGS=-lm -ldl tier0_i486.so vstdlib_i486.so -Wl,-rpath,../bin,-rpath,.

# XERCES 2.6.0 or above ( http://xml.apache.org/xerces-c/ ) is used by the vcproj to makefile converter
# it must be installed before being able to run this makefile
# if you have xerces installed already you should be able to use the two lines below
XERCES_INC_DIR=/usr/include
XERCES_LIB_DIR=/usr/lib

# You shouldnt need to change this
CPP_LIB=$(LIB_DIR)/libstdc++.a $(LIB_DIR)/libgcc_eh.a

#############################################################################
# Things below here shouldn't need to be altered
#############################################################################
MAKE=make

# the dir we want to put binaries we build into
BUILD_DIR=.
# the place to put object files
BUILD_OBJ_DIR=$(BUILD_DIR)/obj

# the location of the source code
SOURCE_DIR=..

# the CPU target for the build, must be i486 for now
ARCH=i486
ARCH_CFLAGS=-mtune=pentium-mmx -march=pentium-mmx -msse

ifdef DEBUG
        X_CFLAGS=-g
	X_LDFLAGS=
else
        X_CFLAGS=-O3
	X_LDFLAGS=-s
endif

# -fpermissive is so gcc 3.4.x doesn't complain about some template stuff
BASE_CFLAGS=-fpermissive -D_LINUX -DNDEBUG -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp
SHLIBEXT=so
SHLIBLDFLAGS=-shared -Wl,-Map,$@_map.txt -Wl

#flags passed to the c compiler
CFLAGS=$(X_CFLAGS) $(USER_CFLAGS) $(DEFINES) $(ARCH_CFLAGS) $(BASE_CFLAGS) -Usprintf=use_Q_snprintf_instead_of_sprintf -Ustrncpy=use_Q_strncpy_instead -UPROTECTED_THINGS_ENABLE

# define list passed to make for the sub makefile
BASE_DEFINES=CC=$(CC) CPLUS=$(CPLUS) CPP_LIB="$(CPP_LIB)" \
	BUILD_DIR=$(BUILD_DIR) BUILD_OBJ_DIR=$(BUILD_OBJ_DIR) \
	SOURCE_DIR=$(SOURCE_DIR) SHLIBLDFLAGS="$(SHLIBLDFLAGS)" SHLIBEXT=$(SHLIBEXT) \
	CLINK=$(CLINK) CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS) $(X_LDFLAGS)" \
	ARCH=$(ARCH) GAME_DIR=$(GAME_DIR) MOD_CONFIG=$(MOD_CONFIG) NAME=$(NAME) \
	XERCES_INC_DIR=$(XERCES_INC_DIR) XERCES_LIB_DIR=$(XERCES_LIB_DIR) \
	STATS_CONFIG=$(STATS_CONFIG) STATSNAME=$(STATSNAME)

# Project Makefile
MAKE_MOD=Makefile.mod
MAKE_STATS=Makefile.stats
MAKE_VCPM=Makefile.vcpm
MAKE_PLUGIN=Makefile.plugin

all: check vcpm mod

check:
	@if [ -z "$(CC)" ]; then echo "Compiler not defined."; exit; fi
	@if [ ! -d $(BUILD_DIR) ];then mkdir $(BUILD_DIR);fi
	@cd $(BUILD_DIR)

vcpm:
	@if [ ! -f "tier0_i486.so" ]; then ln -s $(GAME_DIR)/bin/tier0_i486.so .; fi
	@if [ ! -f "vstdlib_i486.so" ]; then ln -s $(GAME_DIR)/bin/vstdlib_i486.so .; fi
	$(MAKE) -f $(MAKE_VCPM) $(BASE_DEFINES)

dovcpm:
	@if [ ! -f "tier0_i486.so" ]; then ln -s $(GAME_DIR)/bin/tier0_i486.so .; fi
	@if [ ! -f "vstdlib_i486.so" ]; then ln -s $(GAME_DIR)/bin/vstdlib_i486.so .; fi
	./vcpm $(MOD_PROJ)

makemod:
	$(MAKE) -f $(MAKE_MOD) $(BASE_DEFINES)

stats:
	@if [ ! -f "tier0_i486.so" ]; then ln -s $(GAME_DIR)/bin/tier0_i486.so .; fi
	@if [ ! -f "vstdlib_i486.so" ]; then ln -s $(GAME_DIR)/bin/vstdlib_i486.so .; fi
	./vcpm $(STATS_PROJ)
	$(MAKE) -f $(MAKE_STATS) $(BASE_DEFINES)

linktest:
	@echo "[0;31mTesting server binary symbols[0m"
	@rm -f obj/server/__linux_link_test.c
	@rm -f obj/server/__linux_link_test.o
	@echo "int main(void){return 0;}">obj/server/__linux_link_test.c
	$(CLINK) -Wl,-rpath,. -o obj/server/__linux_link_test.o $(LDFLAGS) $(NAME)_$(ARCH).$(SHLIBEXT) obj/server/__linux_link_test.c
	@rm -f obj/server/__linux_link_test.c
	@rm -f obj/server/__linux_link_test.o

install:
	cp $(NAME)_$(ARCH).$(SHLIBEXT) $(GAME_DIR)/$(MOD_NAME)/bin

mod: vcpm dovcpm makemod linktest install

plugin:
	$(MAKE) -f $(MAKE_PLUGIN) $(BASE_DEFINES)

cleanmod:
	 $(MAKE) -f $(MAKE_MOD) $(BASE_DEFINES) clean

cleanstats:
	 $(MAKE) -f $(MAKE_STATS) $(BASE_DEFINES) clean

clean:
	 $(MAKE) -f $(MAKE_VCPM) $(BASE_DEFINES) clean
	 $(MAKE) -f $(MAKE_PLUGIN) $(BASE_DEFINES) clean
	 $(MAKE) -f $(MAKE_MOD) $(BASE_DEFINES) clean
	 $(MAKE) -f $(MAKE_STATS) $(BASE_DEFINES) clean

