CFLAGS_ADDONS = $(CFLAGS)
LDFLAGS_ADDONS = $(LDFLAGS)

ls-gl-ext: deps = libdrm glesv2 gbm egl

ls-gl-ext: src/ls-gl-ext.c src/graphics-common.c
	$(foreach dep, $(deps), $(eval CFLAGS_ADDONS += $(shell pkg-config --cflags $(dep))))
	$(foreach dep, $(deps), $(eval LDFLAGS_ADDONS += $(shell pkg-config --libs $(dep))))
	$(eval CFLAGS_ADDONS += -Iinc/)
	$(CC) $^ $(CFLAGS_ADDONS) $(LDFLAGS_ADDONS) -o $@

clean-ls-gl-ext:
	rm -f ls-gl-ext

gl-test: deps = libdrm glesv2 gbm egl

gl-test: src/gl-test.c src/graphics-common.c
	$(foreach dep, $(deps), $(eval CFLAGS_ADDONS += $(shell pkg-config --cflags $(dep))))
	$(foreach dep, $(deps), $(eval LDFLAGS_ADDONS += $(shell pkg-config --libs $(dep))))
	$(eval CFLAGS_ADDONS += -Iinc/)
	$(CC) $^ $(CFLAGS_ADDONS) $(LDFLAGS_ADDONS) -o $@

clean-gl-test:
	rm -f gl-test