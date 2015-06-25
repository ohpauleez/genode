SRC_CC  += main.cc
LIBS    += ld alarm server
INC_DIR += $(REP_DIR)/src/drivers/timer/include

vpath main.cc $(REP_DIR)/src/drivers/timer
