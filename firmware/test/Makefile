# Simple Makefile for building gate_delay tests with gcc

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
SRC_DIR = ../src
INC_DIR = ../include
TEST_DIR = .
TARGETS = test_adc_to_delay test_timer

SRC_FILES = $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp))
TEST_FILES = $(wildcard $(TEST_DIR)/*.cpp)

all: test

test: $(TARGETS)

test_adc_to_delay: $(TEST_DIR)/test_adc_to_delay.cpp $(SRC_FILES)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -DTEST_SUITE $(TEST_DIR)/test_adc_to_delay.cpp $(SRC_FILES) -o test_adc_to_delay

test_timer: $(TEST_DIR)/test_timer.cpp $(SRC_FILES)
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -DTEST_SUITE $(TEST_DIR)/test_timer.cpp $(SRC_FILES) -o test_timer

clean:
	rm -f $(TARGETS) adc_to_delay_results.csv

.PHONY: all clean test
