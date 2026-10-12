#pragma once

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
#include <utility>

namespace Debug
{
	class FileList
	{
	public:
		FileList(std::filesystem::path directory, std::string extension)
			: directory(std::move(directory)), extension(std::move(extension)) {}

		void SetDirectory(const std::filesystem::path& path)
		{
			if (path != directory) {
				directory = path;
				Invalidate();
			}
		}

		const std::filesystem::path& GetDirectory() const { return directory; }
		void Invalidate() { dirty = true; }

		const std::vector<std::string>& GetFiles()
		{
			if (dirty) {
				files.clear();
				std::error_code error;
				std::filesystem::directory_iterator it(directory, error), end;
				while (!error && it != end) {
					if (it->path().extension() == extension) {
						files.push_back(it->path().filename().string());
					}
					it.increment(error);
				}
				std::sort(files.begin(), files.end());
				dirty = false;
			}
			return files;
		}

	private:
		std::filesystem::path directory;
		std::string extension;
		std::vector<std::string> files;
		bool dirty = true;
	};
}
