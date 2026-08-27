#include "printCompileError.hpp"

#include "DataManager.hpp"

#include <fmt/format.h>

#include <filesystem>


void printCompileError(std::string context, std::string_view error) noexcept {
	auto dm = DataManager::get();
	auto line = dm->getLine();
	auto column = dm->getColumn();
	static CLIArgs const& args = dm->getArgs();

	if (args.cmd) {
		fmt::println("\e[31m-- Error: at line {} column {}: \e[1m{}\e[0m", line, column, error);
	} else {
		fmt::println(
			"\e[31m-- Error: at {}:{}:{}: \e[1m{}\e[0m",
			std::filesystem::canonical(args.source.value()).string(),
			line, column,
			error
		);
	}

	auto lineStr = fmt::format("{}", line);
	auto lineStrLen = lineStr.size();
	auto contextFillLen = context.size() - 1z;
	fmt::println(
		"    {} | \e[40m{}\e[0m\n"
		"    {} | {}\e[1;31m^\e[0m",
		std::move(lineStr), std::move(context),
		std::string(lineStrLen, ' '), std::string(contextFillLen, ' ')
	);

	return;
}