#include "stdafx.h"
#include <cstdlib>
struct loaded_mod {
	HMODULE handle;
	std::string name;
	float ver;
	operator HMODULE() {
		return handle;
	}
};
constexpr float currentVer = (float)0.1;

#if _DEBUG
#define LOG(x) std::cout << x
#else
#define LOG(x)
#endif
int main() {
	using namespace nlohmann::json_literals;
	namespace fs = std::filesystem;
	using json_iterator = nlohmann::json_abi_v3_11_3::detail::iter_impl<nlohmann::json>;
	fs::path fileName = "mods.json";
	if (!fs::exists(fileName)) {

		const char* generatedJson =
			R"**(
{
	"mods": [
		
	]
}
)**";
		std::ofstream createFile(fileName);
		try {
			if (!createFile.is_open() || !createFile) {
				std::cerr << "[INT-ERROR] The object for creating the file went wrong. (not your fault) :) ";
				return 1;
			}
			createFile << generatedJson;

		}
		catch (...) {
			std::cerr << "[INT-ERROR] Writing to the file went wrong (not your fault) :)";
			return 1;
		}

		createFile.close();
		std::cout << "Please input all of your mods inside the newly generated mods.json file.";
		std::cout << std::string(R"**(

The "name" key should specify an identifier for the mod. This will be used for error information.
The "path" key should contain the path to the DLL file containing the mod.
The "ver" key specifies what version of the modding system this mod is designed for. (current version is 
)**") + std::to_string(currentVer) +
R"**()
Enclose every mod with it's own set of curly braces.

Example:
{
	"mods": [
		{
			"path": "MyMod.dll",
			"name": "MyMod",
			"ver": 1
		}
	]
}
These can be in any order, not just the one shown in the example.
)**";
		std::cin.get();
		return 2;
	}
	nlohmann::json jsonFromFile;
	std::ifstream readFile(fileName);
	if (!readFile.is_open() || !readFile) {
		std::cerr << "[INT-ERROR] The object for reading JSON went wrong.";
		return EXIT_FAILURE;
	}
	try {
		readFile >> jsonFromFile;
	}
	catch (...) {
		std::cerr << "[ERROR] Invalid JSON (or something else went wrong)";
		std::cin.get();
		return 3;
	}
	readFile.close();

	json_iterator mods = jsonFromFile.find("mods");


	if (mods == jsonFromFile.end()) {
		std::cout << "[ERROR] You forgot the \"mods\" tag";
		std::cin.get();
		return 3;
	}

	LOG(mods.value());

	LOG("\n");
	LOG(jsonFromFile.find("mods")->size() + "\n\n\n");
	const size_t size = jsonFromFile.find("mods")->size();
	std::vector<nlohmann::json> acceptedMods;
	for (int timer = 0; timer < size; timer++) {
		bool hasVerKey = true;
		if (mods.value().at(timer).find("ver") == mods.value().at(timer).end()) {
			std::cout << "[WARN] Version key not found. Always specify a version. Assuming latest version.";
			hasVerKey = false;
		}
		if (((mods.value().at(timer).find("name") == mods.value().at(timer).end()) || (mods.value().at(timer).find("path") == mods.value().at(timer).end()))) {
			std::cerr << "[ERROR] Skipping loading mod " << timer << " because it is missing a required key.\n\n\n";
			continue;
		}
		else if (mods.value().at(timer).find("name").value().type() != nlohmann::json_abi_v3_11_3::detail::value_t::string || mods.value().at(timer).find("path").value().type() != nlohmann::json_abi_v3_11_3::detail::value_t::string) {
			std::cout << "[ERROR] Skipping loading mod " << timer << " because a required key is of the wrong type.\n\n\n";
			continue;
		}
		std::cout << "mod canidate found: "
			<< mods.value().at(timer).find("name").value();
		if (hasVerKey)
			std::cout << " made for modding system version " << mods.value().at(timer).find("ver").value();
		std::cout << "\n(json: " << mods.value().at(timer) << ")"
			<< "\n\n\n";

		acceptedMods.push_back(mods.value().at(timer));
	}


	//	for (int timer = 0; timer < size; timer++) {
	//		if (!(acceptedMods.size() < 1)) {
	//			for (int timer2 = 0; timer2 <= acceptedMods.at(timer).length(); timer2++)
	//			{
	//				if (acceptedMods.at(timer).at(timer2) == '\"') acceptedMods.at(timer).erase(timer2);
	//			}
	//		}
	//	}
	// Code disabled for causing issues

	if (acceptedMods.size()) {
		LOG("Loading mods\n\n");
	}
	else std::cout << "No mods loaded.";
	std::vector<loaded_mod> libs;

	for (int timer = 0; timer < acceptedMods.size(); timer++) {
		LOG(acceptedMods.at(timer) << "\n");
		auto latestVer = [&]() -> bool {

			// ----------
			loaded_mod temp;
			temp.ver = currentVer;
			temp.name = acceptedMods.at(timer).find("name").value();
			temp.handle = LoadLibraryA(std::string(acceptedMods.at(timer).find("name").value()).c_str());
			if (temp.handle == NULL) {
				std::cerr << "[ERROR] Mod not found (or something else went wrong, like invalid DLL)\n[ERROR] Detailed error: " << GetLastError();
				return false;
			}
			auto func = ((void (*)())GetProcAddress(temp.handle, "onEnable"));
			if (func != NULL) {
				libs.push_back(temp);
				func();
			}
			else {
				std::cerr << "[ERROR] Mod not found (or something else went wrong)\n";
				return false;
			}
			return true;
			// ----------  
			/*
			Lines are there so I don't confuse the lambda with the rest of the code,
			i'm just like that
			*/
			};



		
		if (acceptedMods.at(timer).find("ver") == acceptedMods.at(timer).end()) {
			if (!latestVer()) continue;
		}
		
		else if (acceptedMods.at(timer).find("ver").value() == "0.1-alpha") { if (!latestVer()) continue; }

		

		else {
			
			std::cerr << "[ERROR] Could not load mod " << acceptedMods.at(timer).find("name").value() << " because the version is invalid\n";
		}

		std::cin.get();
		for (int timer = 0; timer < libs.size(); timer++) {
			if (libs.at(timer).ver >= 0.1) {
				auto func = (void (*)())GetProcAddress(libs.at(timer).handle, "onDisable");
				func();
			}
			FreeLibrary(libs.at(timer));
		}
	}

}