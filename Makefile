# Build soundpipe-c demo

.PHONY: all clean debug

default: all

# Version just for tracking

VERSION = 0.0.1

# Directory prefixes

SP_PREFIX ?= ../soundpipe
SP_INCS ?= dr_wav md5 soundpipe
BUILD_PREFIX ?= ./build
SRC_PREFIX ?= ./src

# compilable objects
MODS =
EXECS = straightpipe multipipe

SRCMODS_DIR ?= $(SRC_PREFIX)/modules
SRCEXECS_DIR ?= $(SRC_PREFIX)/external

BLDMODS_DIR ?= $(BUILD_PREFIX)/modules
BLDEXECS_DIR ?= $(BUILD_PREFIX)/external

# paths for the C-compiler 

#CMPATHS += $(addprefix $(SRC_PREFIX)/modules/, $(addsuffix .c, $(MODS)))
#CEPATHS += $(addprefix $(SRC_PREFIX)/external/, $(addsuffix .c, $(EXECS)))
OMPATHS += $(addprefix $(BLDMODS_DIR)/, $(addsuffix .o, $(MODS)))
OEPATHS += $(addprefix $(BLDEXECS_DIR)/, $(addsuffix .o, $(EXECS)))
EPATHS += $(addprefix $(BLDEXECS_DIR)/, $(EXECS))

# compiler flags

CFLAGS += -I$(BUILD_PREFIX) -I$(SP_PREFIX)

# optionally build library for C double data type rather than C float data type

ifeq ($(USE_DOUBLE), 1)
CFLAGS+=-DUSE_DOUBLE
SPFLOAT=double
else
SPFLOAT=float
endif

# C compiler flags

#CFLAGS += -DSP_VERSION=$(VERSION) -O3 -DSPFLOAT=${SPFLOAT} -std=c99
CFLAGS += -DSP_VERSION=$(VERSION) -O3 -DSPFLOAT=${SPFLOAT}
CFLAGS += -Iinclude -I/usr/local/include

# switches and static libraries

#CLFLAGS += -L$(SP_PREFIX) -lsoundpipe -static
CLFLAGS +=
CLLIBS += ../soundpipe/libsoundpipe.a

# this shouldn’t do anything but create any needed folders if needed

$(SP_PREFIX) \
$(SRC_PREFIX) \
$(SRC_PREFIX)/modules \
$(SRC_PREFIX)/external \
$(BUILD_PREFIX) \
$(BUILD_PREFIX)/modules \
$(BUILD_PREFIX)/external:
	mkdir -p $@

# compile the sources
# Note %.o : %.c means build the corresponding .o for each .c

$(BLDMODS_DIR)/%.o: $(SRCMODS_DIR)/%.c $(SP_PREFIX)/*.h \
  | $(BLDMODS_DIR) $(SRCMODS_DIR) $(SP_PREFIX)
	$(CC) -Wall $(CFLAGS) -c $< -o $@

$(BLDEXECS_DIR)/%.o: $(SRCEXECS_DIR)/%.c $(SP_PREFIX)/*.h \
  | $(BLDEXECS_DIR) $(SRCEXECS_DIR) $(SP_PREFIX)
	$(CC) -Wall $(CFLAGS) -c $< -o $@

# link the executables

$(BLDEXECS_DIR)/%: $(BLDEXECS_DIR)/%.o | $(SRCEXECS_DIR)
	$(CC) $(CLFLAGS) -o $@ $< $(CLLIBS) -lm

# Make automatically deletes “intermediate” files.
# This target will keep them.

.SECONDARY: $(OEPATHS)

all: $(EPATHS)
#all: $(OMPATHS) $(OEPATHS)
#all: $(BUILD_PREFIX)/external


# debug feature

debug:
	$(foreach v, $V, $(warning $v = $($v)))


# clean up the project (not right)

clean:
	rm -rf $(BUILD_PREFIX)
