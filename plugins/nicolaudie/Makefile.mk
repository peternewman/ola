# LIBRARIES
##################################################
if USE_NICOLAUDIE
lib_LTLIBRARIES += plugins/nicolaudie/libolanicolaudie.la

# Plugin description is generated from README.md
built_sources += plugins/nicolaudie/NicolaudiePluginDescription.h
nodist_plugins_nicolaudie_libolanicolaudie_la_SOURCES = \
    plugins/nicolaudie/NicolaudiePluginDescription.h
plugins/nicolaudie/NicolaudiePluginDescription.h: plugins/nicolaudie/README.md plugins/nicolaudie/Makefile.mk plugins/convert_README_to_header.sh
	sh $(top_srcdir)/plugins/convert_README_to_header.sh $(top_srcdir)/plugins/nicolaudie $(top_builddir)/plugins/nicolaudie/NicolaudiePluginDescription.h

plugins_nicolaudie_libolanicolaudie_la_SOURCES = \
    plugins/nicolaudie/NicolaudiePlugin.cpp \
    plugins/nicolaudie/NicolaudiePlugin.h \
    plugins/nicolaudie/NicolaudieDevice.cpp \
    plugins/nicolaudie/NicolaudieDevice.h
# \
#    plugins/nicolaudie/NicolaudiePort.h \
#    plugins/nicolaudie/NicolaudieThread.cpp \
#    plugins/nicolaudie/NicolaudieThread.h \
#    plugins/nicolaudie/NicolaudieWidget.cpp \
#    plugins/nicolaudie/NicolaudieWidget.h
plugins_nicolaudie_libolanicolaudie_la_LIBADD = \
    common/libolacommon.la \
    olad/plugin_api/libolaserverplugininterface.la
plugins_nicolaudie_libolanicolaudie_la_LIBADD += $(libLsHardDevKit_LIBS)

endif

EXTRA_DIST += plugins/nicolaudie/README.md
