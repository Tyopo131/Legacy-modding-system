#include "pch.h"

extern "C" _declspec(dllexport) void onEnable() {
	std::cout << "Hello world! This is very advanced\n";
}
extern "C" _declspec(dllexport) void onDisable() {
	std::cout << "Disabling mod...";
}
