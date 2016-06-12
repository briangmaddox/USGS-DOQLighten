# Generated automatically from Makefile.in by configure.
# $Id: Makefile,v 1.1.1.1 2002-02-19 19:07:48 bmaddox Exp $
# Makefile for the DOQ to geotiff program + support program
# Last modifled by $Author: bmaddox $ on $Date: 2002-02-19 19:07:48 $

prefix       = /home/bmaddox
host_os      = linux
srcdir       = .
top_srcdir   = .
enable_debug = no

# Set up the paths we'll be needing
INCPATHS = -I$(prefix)/include
LIBDIRS = -L$(prefix)/lib

# Libraries to link in
LIBS = -lImage -lgeotiff -ltiff -lFrodo

# Linker flags
LDFLAGS = $(LIBDIRS)
LOADLIBES = $(LIBS)

# Set up the DEBUG flag
ifeq ($(enable_debug),yes)
DEBUG = -Wall -g
else
DEBUG = -Wall -O2
endif

# Compiler defs
CC = gcc
CXX = g++
CXXFLAGS = -static $(DEBUG) $(INCPATHS)

# Where to install the binaries
BINDEST = $(prefix)/bin


all: doqlighten

doqlighten : utils.cc utils.h doqlighten.cc
	$(CXX) $(CXXFLAGS) utils.cc doqlighten.cc -o doqlighten $(LIBDIRS) $(LIBS)

clean::
	rm -f $(OBJS)  core doqlighten

realclean:
	rm -f Makefile config.cache config.status config.log

