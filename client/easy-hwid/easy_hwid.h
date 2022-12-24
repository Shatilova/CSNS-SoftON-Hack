// small set of techniques to get unique ID of machine.
// all techniques are taken from the Internet.
//
// using of all included techniques can guarantee strong HWID
// that's not easy to change unintentionally or specially
#pragma once

#include <string>

namespace easy_hwid {
	std::string CPUHash();
	std::string VideoAdapter();
	std::string PhysicalDrive();
}