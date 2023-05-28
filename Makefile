CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -Werror -Wformat-security -Wstack-protector -fstack-protector-all #-Wl,-z,relro,-z,now

# Your target and source files
TARGET = parser
SRC = main.cpp toojpeg.cpp

build: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o .\build\$@ $^

clean:
	rm -f $(TARGET)

run:
	.\build\$(TARGET).exe
