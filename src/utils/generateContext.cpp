#include "generateContext.hpp"

#include "DataManager.hpp"

#include <fmt/format.h>


std::string generateContext(std::string_view data, std::size_t i) noexcept {
	auto& context = DataManager::get()->getContext();
	auto ret = i < context.size() ? // if i = 3 and size = 4 we need 1 char of context
		fmt::format(
			"{}{}",
			std::string_view{ context.begin() + i, context.end() },
			data.substr(0, i + 1z)
		)
		:
		std::string{ data.substr(i - context.size(), context.size() + 1z) };

	std::uint8_t realContextSize = 0u;
	for (
		auto rit = ret.rbegin();
		std::isprint(static_cast<unsigned char>(*rit)) && rit != ret.rend();
		++rit
	) ++realContextSize;

	return std::move(ret).substr(ret.size() - realContextSize, realContextSize);
}