CXX_OPT = -g -ffreestanding -fPIC -fno-exceptions -fno-rtti  -nostdinc -nostdlib

LD_SCRIPT= $(PRG_DIR)/../policy.ld

-include *.d
-include ../*.d

LIBS = platform

%.o : %.cc
	$(MSG_COMP)$@
	$(VERBOSE)$(CXX) -c $(CC_MARCH) $(CXX_OPT) $(INCLUDES) $< -o $@

%.o : %.S
	$(MSG_ASSEM)$@
	$(VERBOSE)$(CXX) -c $(CC_MARCH) -fPIC -nostding -nostdlib -MMD $< -o $@

$(TARGET_POLICY).elf: ../table.o policy.o
	$(MSG_LINK)$@
	$(VERBOSE)$(LD) $(LD_MARCH) -T $(LD_SCRIPT) -Ttext=0 $(INCLUDES) $^ -o $@

$(INSTALL_DIR)/$(TARGET_POLICY): $(TARGET_POLICY).elf
	$(VERBOSE)$(OBJCOPY) -O binary $< $@

$(TARGET): $(INSTALL_DIR)/$(TARGET_POLICY)

clean cleanall: clean_policy

clean_policy:
	$(VERBOSE)$(RM) ../*.o ../*.d *.o *.d $(TARGET_POLICY).elf \
		$(BUILD_BASE_DIR)/bin/$(TARGET_POLICY)
