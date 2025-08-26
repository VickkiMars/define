#include <iostream>
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;

void save_binary(const std::unordered_map<std::string, json> & map, const std::string& filename) {
	std::ofstream out(filename, std::ios::binary);
	size_t size = map.size();
	out.write(reinterpret_cast<const char*>(&size), sizeof(size));
	for (auto& [key, value] : map) {
		size_t ksize = key.size();
		out.write(reinterpret_cast<const char*>(&ksize), sizeof(ksize));
		out.write(key.data(), ksize);
		std::string val_str = value.dump();
		size_t vsize = val_str.size();
		out.write(reinterpret_cast<const char*>(&vsize), sizeof(vsize));
		out.write(val_str.data(), vsize);
	}
}



std::unordered_map<std::string, json> load_binary(const std::string filepath){
	std::unordered_map<std::string, json> map;
	std::ifstream in(filepath, std::ios::binary);
	if (!in.is_open()) {
		std::cerr << "Error: Could not open file\n";
		return map;
	}
	size_t size;
	in.read(reinterpret_cast<char*>(&size), sizeof(size));
	for (size_t i = 0; i < size; i++) {
		size_t ksize;
		in.read(reinterpret_cast<char*>(&ksize), sizeof(ksize));
		std::string key(ksize, '\0');
		in.read(&key[0], ksize);
		size_t vsize;
		in.read(reinterpret_cast<char*>(&vsize), sizeof(vsize));
		std::string val_str(vsize, '\0');
		in.read(&val_str[0], vsize);
		map[key] = json::parse(val_str);
	}
	return map;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Usage: ";
		std::cerr << " " << argv[0] << " --build	# Build binary index\n";
		std::cerr << " " << argv[0] << " WORD	#Search for a word\n";
		return 1;
	}

	std::string json_file = "word.json";
	std::string bin_file = json_file + ".bin";

	if (std::string(argv[1]) == "--build") {
		std::cout << "Loading file...\n";
		std::ifstream file(json_file);
		if (!file.is_open()) {
			std::cerr << "Error: Could not open " << json_file << "\n";
			return 1;
		}
		json data;
		file >> data;

		std::unordered_map<std::string, json> word_map;
		word_map.reserve(data.size());

		std::cout << "Building map...\n";
		for (const auto& entry : data) {
			if (entry.contains("word")) {
				word_map[entry["word"]] = entry;
			}
		}

		std::cout << "Saving to binary...\n";
		save_binary(word_map, bin_file);
		std::cout << "Done! Saved Index to " << bin_file << "\n";
	}
	else {
		std::string search_word = argv[1];
		std::transform(search_word.begin(), search_word.end(), search_word.begin(),
			[](unsigned char c){ return std::toupper(c); });
		std::cout << "Loading binary index...\n";
		auto word_map = load_binary(bin_file);
		auto it = word_map.find(search_word);
		if (it != word_map.end()) {
			std::cout << "\nDefinition of '" << search_word << "' :\n" << it->second["definition"] << "\n";
		} else {
			std::cout << "Word not found.\n";
		}
	}

	return 0;

}
