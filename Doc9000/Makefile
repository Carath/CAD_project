##########################################################
# Global compiler settings:
include ../compiler_settings.mk

##########################################################
# Settings:

# Executable name:
EXE = doctor

# Used static library name:
NEURAL_LIB = ../NeuralLib

# Produced static library name:
DOC_LIB = ../DocLib

# Source and objects files location:
SRC_DIR = src
OBJ_DIR = obj

# Create the OBJ_DIR folder, if necessary:
$(shell mkdir -p $(OBJ_DIR))

##########################################################
# Libraries:

# Used high performance library:
ifeq ($(HIGH_PERF_LIB), OPENBLAS)

	HIGH_PERF_HEAD_DIR = -I$(HIGH_PERF_PATH)/include
	HIGH_PERF_LIB_DIR = -L$(HIGH_PERF_PATH)/lib
	HIGH_PERF_LINKING = -lopenblas
endif

# Choice of SQL API:
SQL_API = MySQL

ifeq ($(SQL_API), MySQL)
	SQL_API_PATH = /usr
	SQL_API_HEAD_DIR = -I$(SQL_API_PATH)/include/mysql
	SQL_API_LIB_DIR = -L$(SQL_API_PATH)/lib/mysql
	SQL_API_LINKING = -lmysqlclient
endif

##########################################################
# Compiler options:

# Necessary for using get_time():
POSIX_OPT = _POSIX_C_SOURCE=199309L

# For better performance:
PROCESSOR_ARCH = -march=native

# N.B: gcc for C, g++ for C++, alternative: clang.
CC = gcc
CPPFLAGS =
CFLAGS = -std=c99 -Wall -O2 $(PROCESSOR_ARCH) -D$(POSIX_OPT) -D_$(NUMBER_TYPE) -D_$(HIGH_PERF_LIB) $(HIGH_PERF_HEAD_DIR) $(SQL_API_HEAD_DIR)
LDFLAGS =
LDLIBS = $(NEURAL_LIB).a $(HIGH_PERF_LIB_DIR) $(HIGH_PERF_LINKING) $(SQL_API_LIB_DIR) $(SQL_API_LINKING) -lm

##########################################################
# Compiling rules:

# The following names are not associated with files:
.PHONY: all clean

# All executables to be created:
all: $(EXE)

# Sources and objects files:
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Linking the program:
$(EXE): $(OBJ) $(DOC_LIB).a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compiling the source files:
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(DOC_LIB).a: $(OBJ)
	ar rcs $@ $^

##########################################################
# Cleaning with 'make clean' the object files:
clean:
	rm -fv $(EXE) $(DOC_LIB).a $(OBJ_DIR)/*
