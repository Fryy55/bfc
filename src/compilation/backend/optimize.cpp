#include "optimize.hpp"

#include "TokensIterator.hpp"

#include <fmt/ranges.h>

#include <ranges>

std::string_view foo(Token::Type x) noexcept {
	switch (x) {
		case Token::Type::Shift:
			return "Shift";

		case Token::Type::Add:
			return "Add";

		case Token::Type::Out:
			return "Out";

		case Token::Type::In:
			return "In";

		case Token::Type::OpenLoop:
			return "OpenLoop";

		case Token::Type::CloseLoop:
			return "CloseLoop";

		default:
			return "UNK";
	}
}

std::string format_as(Token const& t) {
	return fmt::format(
		"{} | {} | {}",
		t.valid ? "valid" : "invalid", foo(t.type), t.refVal
	);
}


void optimize() noexcept {
	auto dm = DataManager::get();
	CLIArgs const& args = dm->getArgs();
	auto& tokens = dm->getTokens();

	if (args.debugPrints)
		fmt::println("O0\n{}", fmt::join(std::views::zip(std::views::iota(0z), tokens), "\n"));

	if (args.o == 0u)
		return;

	if (args.o >= 1u) {
		while (true) {
			auto it = TokensIterator::begin();
			if (it == tokens.end() || it->type != Token::Type::OpenLoop)
				break;

			for (auto targetIndex = it->refVal; it.index() != targetIndex + 1z; ++it)
				it->valid = false;
		}

		if (args.debugPrints)
			fmt::println("\nO1\n{}", fmt::join(std::views::zip(std::views::iota(0z), tokens), "\n"));
	}

	if (args.o >= 2u) {
		using Type = Token::Type;

		auto lastTokenType = static_cast<Type>(0u);
		Token* lastToken = nullptr;
		auto updateLastToken = [&lastToken](Token* token) noexcept {
			if (lastToken && lastToken->refVal == 0)
				lastToken->valid = false;
			lastToken = token;
		};

		for (auto& token : TokensIterator{}) {
			if (token.type != lastTokenType) {
				switch (token.type) {
					case Type::Shift:
						updateLastToken(&token);
						lastTokenType = Type::Shift;
						break;

					case Type::Add:
						updateLastToken(&token);
						lastTokenType = Type::Add;
						break;

					default:
						updateLastToken(nullptr);
						lastTokenType = Type::None;
						break;
				}
				continue;
			}

			// of last type
			token.valid = false;
			lastToken->refVal += token.refVal;
		}

		updateLastToken(nullptr);

		if (args.debugPrints)
			fmt::println("\nO2\n{}", fmt::join(std::views::zip(std::views::iota(0), tokens), "\n"));
	}

	return;
}