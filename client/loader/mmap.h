// MMAP implementation
// original author is unknown
// works perfectly with Win 7, 8, 8.1, and 10

#pragma once

#include <Windows.h>
#include <string>
#include <optional>

std::string MMAP(const std::string& process_name, unsigned char* raw);