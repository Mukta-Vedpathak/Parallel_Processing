# Compiler
CXX = g++

# Directories
PART1_DIR = PART1
PART2_DIR = PART2
IMAGES_DIR = images

# Part 1
PART1_SRC = $(PART1_DIR)/part1.cpp libppm.cpp
PART1_OUT = $(IMAGES_DIR)/output_part1.ppm

# Part 2
PART2_1_SRC = $(PART2_DIR)/part2_1.cpp libppm.cpp
PART2_1_OUT = $(IMAGES_DIR)/output_part2_1.ppm

PART2_2_SRC = $(PART2_DIR)/part2_2.cpp libppm.cpp
PART2_2_OUT = $(IMAGES_DIR)/output_part2_2.ppm

PART2_3_SRC = $(PART2_DIR)/part2_3.cpp libppm.cpp
PART2_3_OUT = $(IMAGES_DIR)/output_part2_3.ppm

# Default target
all: part1 part2_1 part2_2 part2_3

# Part 1 target
part1: $(PART1_SRC)
	$(CXX) -o part1 $(PART1_SRC)
	./part1 $(IMAGES_DIR)/1.ppm $(PART1_OUT)

# Part 2_1 target
part2_1: $(PART2_1_SRC)
	$(CXX) -o part2_1 $(PART2_1_SRC)
	./part2_1 $(IMAGES_DIR)/1.ppm $(PART2_1_OUT)

# Part 2_2 target
part2_2: $(PART2_2_SRC)
	$(CXX) -o part2_2 $(PART2_2_SRC)
	./part2_2 $(IMAGES_DIR)/1.ppm $(PART2_2_OUT)

# Part 2_3 target
part2_3: $(PART2_3_SRC)
	$(CXX) -o part2_3 $(PART2_3_SRC)
	./part2_3 $(IMAGES_DIR)/1.ppm $(PART2_3_OUT)

# Clean target
clean:
	rm -f part1 part2_1 part2_2 part2_3 $(IMAGES_DIR)/output_part*.ppm
