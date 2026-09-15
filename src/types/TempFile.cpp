#include "TempFile.hpp"

#include "DataManager.hpp"

#include <fmt/color.h>

#include <fstream>

namespace fs = std::filesystem;


TempFile::TempFile(std::string_view filename) noexcept {
	auto path = fmt::format("{}/{}", DataManager::get()->getArgs().tempDir, filename);

	std::error_code ec;
	m_path = fs::weakly_canonical(path, ec);

	if (ec) {
		m_valid = false;
		fmt::println(
			fmt::fg(fmt::color::crimson),
			"-- Error: couldn't access file path '{}'", std::move(path)
		);
	}

	fs::create_directories(m_path.parent_path(), ec);
	if (ec) {
		m_valid = false;
		fmt::println(
			fmt::fg(fmt::color::crimson),
			"-- Error: couldn't create parent directories for path '{}'", m_path.string()
		);
	}
}

TempFile::~TempFile() noexcept {
	std::error_code ec;
	fs::remove(m_path, ec);
	if (ec) {
		fmt::println(
			fmt::fg(fmt::color::yellow),
			"-- Warning: failed to remove file '{}': {}",
			m_path.string(), ec.message()
		);
	}
}


std::string TempFile::getPath() const noexcept {
	return m_path.string();
}

std::filebuf TempFile::getWriteBuf() const noexcept {
	std::filebuf ret{};
	ret.open(m_path, std::ios::out | std::ios::binary);

	return ret;
}