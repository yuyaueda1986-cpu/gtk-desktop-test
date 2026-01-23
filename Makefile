CC = gcc
PKGCONFIG = pkg-config
CFLAGS = -Wall $(shell $(PKGCONFIG) --cflags gtk+-3.0)
LDFLAGS = $(shell $(PKGCONFIG) --libs gtk+-3.0)

TARGET = gtk-desktop-test
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

check-deps:
	@echo "Checking dependencies..."
	@$(PKGCONFIG) --exists gtk+-3.0 && echo "  gtk+-3.0: OK" || echo "  gtk+-3.0: NOT FOUND"
	@echo ""
	@echo "To install dependencies:"
	@echo "  Ubuntu/Debian: sudo apt install libgtk-3-dev"
	@echo "  Fedora:        sudo dnf install gtk3-devel"
	@echo "  Arch Linux:    sudo pacman -S gtk3"

.PHONY: all clean check-deps
