# Kernel module name
obj-m := dev_driver.o

# Kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build

# Compiler flags for user program
CC := gcc
CFLAGS := -Wall -Wextra -O2

# Target files
TARGET := user_space_program

# Default rule: build both the kernel module and user program
all: kernel user

# Build the kernel module
kernel:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Build the user-space program
user: user_space_program.c
	$(CC) $(CFLAGS) user_space_program.c -o $(TARGET)

# Clean generated files
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f $(TARGET)
