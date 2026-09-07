#pragma once

#include <filesystem>

inline std::filesystem::path GetAssetDirectory(const std::filesystem::path& p)
{
	return std::filesystem::path("assets") / p;
}