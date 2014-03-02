TARGET = maelstrom
LIBS   = libc libc_log libc_fs libc_lwip_nic_dhcp \
		stdcxx sdl sdl_mixer sdl_net libm

include $(REP_DIR)/ports/maelstrom.inc
MAELSTROM_DIR = $(REP_DIR)/contrib/$(MAELSTROM)

INC_DIR += $(MAELSTROM_DIR)
INC_DIR += $(addprefix $(MAELSTROM_DIR)/, screenlib maclib netlogic)

SRC_CC  = $(notdir $(wildcard $(MAELSTROM_DIR)/*.cpp))
SRC_CC += $(notdir $(wildcard $(MAELSTROM_DIR)/screenlib/*.cpp))
SRC_CC += $(notdir $(wildcard $(MAELSTROM_DIR)/maclib/Mac_*.cpp))
SRC_CC += $(notdir $(wildcard $(MAELSTROM_DIR)/netlogic/*.cpp))

vpath %.cpp $(MAELSTROM_DIR)
vpath %.cpp $(MAELSTROM_DIR)/screenlib
vpath %.cpp $(MAELSTROM_DIR)/maclib
vpath %.cpp $(MAELSTROM_DIR)/netlogic

CC_OPT = -DVERSION=\"$(MAELSTROM_VER)\"

CC_WARN += -Wno-write-strings
