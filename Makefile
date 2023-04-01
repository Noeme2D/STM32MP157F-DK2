INC_DIR=inc
SRC_DIR=src

CFLAGS_ADDONS = $(CFLAGS)
LDFLAGS_ADDONS = $(LDFLAGS)

CFLAGS_ADDONS += -I$(INC_DIR)/

CFLAGS_ADDONS += $(shell pkg-config --cflags libdrm)
LDFLAGS_ADDONS += $(shell pkg-config --libs libdrm)

CFLAGS_ADDONS += $(shell pkg-config --cflags glesv2)
LDFLAGS_ADDONS += $(shell pkg-config --libs glesv2)

CFLAGS_ADDONS += $(shell pkg-config --cflags gbm)
LDFLAGS_ADDONS += $(shell pkg-config --libs gbm)

CFLAGS_ADDONS += $(shell pkg-config --cflags egl)
LDFLAGS_ADDONS += $(shell pkg-config --libs egl)

ls-gl-ext: ${SRC_DIR}/ls-gl-ext.c
	$(CC) $^ $(CFLAGS_ADDONS) $(LDFLAGS_ADDONS) -o $@
