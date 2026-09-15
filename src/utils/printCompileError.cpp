#include "printCompileError.hpp"

#include "DataManager.hpp"

#include <fmt/color.h>

#include <filesystem>


void printCompileError(std::string_view context, std::string_view error) noexcept {
	auto dm = DataManager::get();
	auto line = dm->getLine();
	auto column = dm->getColumn();
	static CLIArgs const& args = dm->getArgs();

	if (args.cmd) {
		fmt::println(
			fmt::fg(fmt::color::crimson),
			"-- Error: at line {} column {}: {}",
			line, column, fmt::styled(error, fmt::emphasis::bold)
		);
	} else {
		fmt::println(
			fmt::fg(fmt::color::crimson),
			"-- Error: at {}:{}:{}: {}",
			std::filesystem::canonical(args.source.value()).string(),
			line, column,
			fmt::styled(error, fmt::emphasis::bold)
		);
	}

	auto lineStr = fmt::format("{}", line);
	auto lineStrLen = lineStr.size();
	auto contextFillLen = context.size() - 1z;
	fmt::println(
		"    {} | {}\n"
		"    {} | {}{}",
		std::move(lineStr), fmt::styled(context, fmt::bg(fmt::color::black)),
		std::string(lineStrLen, ' '), std::string(contextFillLen, ' '),
		fmt::styled("^", fmt::fg(fmt::color::red) | fmt::emphasis::bold)
	);

	return;
}