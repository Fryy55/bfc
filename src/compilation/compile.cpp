#include "compile.hpp"

#include "DataManager.hpp"
#include "printCompileError.hpp"
#include "generateContext.hpp"

#include <fmt/base.h>

#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;


static bool parseBatch(std::string_view data) noexcept {
	auto dm = DataManager::get();
	static auto& column = dm->getColumn();
	static auto& line = dm->getLine();
	static auto& context = dm->getContext();
	static auto& loops = DataManager::get()->getLoops();
	static auto& tokens = DataManager::get()->getTokens();


	for (std::size_t i = 0z; i < data.size(); ++i) {
		++column;

		switch (data[i]) {
			case '>':
				tokens.emplace_back(Token::Type::Right);
				break;

			case '<':
				tokens.emplace_back(Token::Type::Left);
				break;

			case '+':
				tokens.emplace_back(Token::Type::Plus);
				break;

			case '-':
				tokens.emplace_back(Token::Type::Minus);
				break;

			case '.':
				tokens.emplace_back(Token::Type::Out);
				break;

			case ',':
				tokens.emplace_back(Token::Type::In);
				break;

			case '[':
				loops.emplace(tokens.size());
				tokens.emplace_back(Token::Type::OpenLoop);
				break;

			case ']':
				if (loops.empty()) {
					printCompileError(
						generateContext(data, i),
						"A `]` command was encountered without a matching `[`"
					);
					return false;
				}
				tokens.emplace_back(Token::Type::CloseLoop, loops.top());
				loops.pop();
				break;

			case '\n':
				column = 0z;
				++line;
				break;

			default:
				break; // ignore as it's like a comment type shit
		}
	}


	auto rit = data.rbegin();
	context = {};
	for (std::int_fast16_t i = context.size() - 1u; 0u <= i && rit != data.rend(); --i)
		context[i] = *(rit++);

	return true;
}

int compile() noexcept {
	CLIArgs const& args = DataManager::get()->getArgs();
	auto source = args.source.value();

	if (args.cmd) {
		std::puts("-- Compiling source from the command line");
		if (!parseBatch(source))
			return 1;
	} else {
		fmt::println("-- Compiling source from file '{}'", source);
		std::ifstream I{ fs::path{ args.source.value() } };
		if (!I) {
			fmt::println("\e[31m-- Error: can't open file '{}'\e[0m", source);
			return 1;
		}

		alignas(std::hardware_constructive_interference_size)
			std::array<char, 64z * 1024u> buf;
		while (I.read(buf.data(), buf.size())) {
			if (!parseBatch({ buf.data(), buf.size() }))
				return 1;
		}
		if (!parseBatch({ buf.data(), static_cast<std::size_t>(I.gcount()) }))
			return 1;

		if (!DataManager::get()->getLoops().empty()) {
			printCompileError(
				generateContext(
					{ buf.data(), static_cast<std::size_t>(I.gcount()) },
					I.gcount() - 1z
				),
				"A `[` command somewhere isn't closed; expected `]`"
			);
			return 1;
		}
	}

	return 0;
}