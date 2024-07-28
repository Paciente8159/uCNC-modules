#!/usr/bin/env bash
export MAKE_TEST="ROW_BY_ROW"

while inotifywait -e close_write render/screen.c; do
	make render && kitty +kitten icat screen.png && kitty +kitten icat screen_dyn.png
done
