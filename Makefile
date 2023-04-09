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

c-py-test: cpytest.c src/c-py-test.c
	$(eval CFLAGS_ADDONS += $(shell pkg-config --cflags python3))
	$(eval LDFLAGS_ADDONS += $(shell pkg-config --libs python3-embed))
	$(eval CFLAGS_ADDONS += -I$(Python_NumPy_INCLUDE_DIR))
	$(eval CFLAGS_ADDONS += -DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION)
	$(eval CFLAGS_ADDONS += -I. -Iinc/)
	$(CC) $(CFLAGS_ADDONS) $^ $(LDFLAGS_ADDONS) -o c-py-test 

cpytest.c: src/cpytest.pyx
# move .pyx out so that the output files is in root
	mv $< .
	/usr/bin/cython -X language_level=3 -X wraparound=False -X boundscheck=False -X cdivision=True cpytest.pyx
	mv cpytest.pyx src/

clean-c-py-test:
	rm -f cpytest.c cpytest.h c-py-test