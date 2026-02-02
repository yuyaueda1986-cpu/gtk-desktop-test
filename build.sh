#!/bin/bash
# Build script for gtk-dashboard
# Use this when meson is not available

set -e

SRCDIR="src"
BUILDDIR="builddir"
TARGET="gtk-dashboard"

CFLAGS="-Wall -std=c11 $(pkg-config --cflags gtk4 json-glib-1.0)"
LDFLAGS="$(pkg-config --libs gtk4 json-glib-1.0)"

mkdir -p "$BUILDDIR"

echo "Compiling..."
for src in main.c app.c json_parser.c widget_factory.c style_manager.c shape_renderer.c; do
    echo "  $src"
    gcc $CFLAGS -c "$SRCDIR/$src" -o "$BUILDDIR/${src%.c}.o"
done

echo "Linking..."
gcc -o "$BUILDDIR/$TARGET" \
    "$BUILDDIR/main.o" \
    "$BUILDDIR/app.o" \
    "$BUILDDIR/json_parser.o" \
    "$BUILDDIR/widget_factory.o" \
    "$BUILDDIR/style_manager.o" \
    "$BUILDDIR/shape_renderer.o" \
    $LDFLAGS -lm

echo "Build complete: $BUILDDIR/$TARGET"
