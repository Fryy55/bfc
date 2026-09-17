#pragma once

#include <filesystem>


class TempFile final {
public:
	TempFile(std::string_view filename) noexcept;
	
	TempFile(TempFile const&) noexcept = default;
	TempFile& operator=(TempFile const&) noexcept = default;
	TempFile(TempFile&&) noexcept = default;
	TempFile& operator=(TempFile&&) noexcept = default;
	
	~TempFile() noexcept;

public:
	operator bool() const noexcept { return m_valid; }

	[[nodiscard]] std::string getPath() const noexcept;
	[[nodiscard]] std::filebuf getWriteBuf() const noexcept;

private: // Fields
	std::filesystem::path m_path;
	bool m_valid = true;
};