CC = g++
CFLAGS_BASE = -std=c++17 -Wall -Wextra -Werror -Wformat-security -Wstack-protector -fstack-protector-all

# Your target and source files
TARGET = parser
SRC = main.cpp toojpeg.cpp

ifdef OS
   OUT_PATH = .\build\$@
   CFLAGS = $(CFLAGS_BASE)
   RUN_TARGET = .\build\$(TARGET).exe
else
   OUT_PATH = $@
   CFLAGS = $(CFLAGS_BASE) -Wl,-z,relro,-z,now
   RUN_TARGET = ./$(TARGET)
endif

build: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT_PATH) $^

run:
	$(RUN_TARGET) -caff ./sample/3.caff
