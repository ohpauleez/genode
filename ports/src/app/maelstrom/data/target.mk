TARGET = maelstrom.tar

include $(REP_DIR)/ports/maelstrom.inc

maelstrom.tar:
	$(VERBOSE)cp -ur \
		$(REP_DIR)/contrib/$(MAELSTROM)/Images/ \
		$(REP_DIR)/contrib/$(MAELSTROM)/sounds/ \
		$(REP_DIR)/contrib/$(MAELSTROM)/icon.bmp \
		$(REP_DIR)/contrib/$(MAELSTROM)/Maelstrom_Fonts \
		$(REP_DIR)/contrib/$(MAELSTROM)/Maelstrom-Scores \
		$(REP_DIR)/contrib/$(MAELSTROM)/Maelstrom_Sprites \
			$(CURDIR)

	$(VERBOSE)tar cf $@ .

$(INSTALL_DIR)/$(TARGET): $(TARGET)
	$(VERBOSE)ln -sf $(CURDIR)/$(TARGET) $@
