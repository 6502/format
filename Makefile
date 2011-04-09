
ALL: test_format

test_format: test_format.cpp format.hpp
	g++ -Wall -O2 -otest_format test_format.cpp
