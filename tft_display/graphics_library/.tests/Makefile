BIN = \
	obj/main.o \
	obj/test_driver.o \
	obj/ext/graphics_library.o \
	obj/test_clear.o \
	obj/test_rect.o \
	obj/test_frame.o \
	obj/test_text.o \
	obj/test_bitmap.o \
	obj/test_color_bitmap.o

BIN_RENDERER = \
	obj/render/main.o \
	obj/ext/graphics_library.o \
	obj/test_driver.o \
	obj/render/screen.o

# Valid tests are:
#   ALL_AT_ONCE
#   ROW_BY_ROW
#   PIXEL_BY_PIXEL

CFLAGS = -g -O0 -DGFX_TESTING -DGFX_TEST_$(MAKE_TEST)

run: obj/tests
	obj/tests

render: obj/renderer
	obj/renderer

obj/renderer: $(BIN_RENDERER)
	gcc -o $@ $(BIN_RENDERER) -lm

obj/tests: $(BIN)
	gcc -o $@ $(BIN)

obj/%.o: %.c
	mkdir -p ${@D}
	gcc -o $@ -c $< $(CFLAGS)

obj/ext/%.o: ../%.c
	mkdir -p ${@D}
	gcc -o $@ -c $< $(CFLAGS)

clean:
	rm -r obj

.PHONY: clean

