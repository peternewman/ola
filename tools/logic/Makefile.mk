if HAVE_SALEAE_LOGIC
bin_PROGRAMS += tools/logic/logic_rdm_sniffer
endif

tools_logic_logic_rdm_sniffer_SOURCES = \
    tools/logic/logic-rdm-sniffer.cpp
tools_logic_logic_rdm_sniffer_LDADD = common/libolacommon.la \
                                      libs/sniffer/libolasniffer.la \
                                      $(libSaleaeDevice_LIBS)

EXTRA_DIST += tools/logic/README.md
