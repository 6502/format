
ALL: test_format

test_format: test_format.cpp format.hpp
	g++ -Wall -Wno-strict-aliasing -O2 -otest_format test_format.cpp
