AUTOMAKE_OPTIONS = foreign

# List of shared libraries to produce
lib_LTLIBRARIES = \
  libcdbobjects.la

AM_CPPFLAGS = \
  -I$(includedir) \
  -isystem$(OFFLINE_MAIN)/include \
  -isystem$(ROOTSYS)/include

libcdbobjects_la_SOURCES = \
  CDBHistos.cc \
  CDBTF.cc \
  CDBTTree.cc

libcdbobjects_la_LDFLAGS = \
  `root-config --libs`

##############################################
# please add new classes in alphabetical order

pkginclude_HEADERS = \
  CDBHistos.h \
  CDBTF.h \
  CDBTTree.h

################################################
# linking tests

BUILT_SOURCES = testexternals.cc

noinst_PROGRAMS = \
  testexternals

testexternals_SOURCES = testexternals.cc
testexternals_LDADD = libcdbobjects.la

testexternals.cc:
	echo "//*** this is a generated file. Do not commit, do not edit" > $@
	echo "int main()" >> $@
	echo "{" >> $@
	echo "  return 0;" >> $@
	echo "}" >> $@

##############################################
# please add new classes in alphabetical order

clean-local:
	rm -f $(BUILT_SOURCES)
