#include "optimize.hpp"

#include "TokensIterator.hpp"

#include <fmt/ranges.h>

std::string_view foo(Token::Type x) noexcept {
	switch (x) {
		case Token::Type::Right:
			return "Right";

		case Token::Type::Left:
			return "Left";

		case Token::Type::Plus:
			return "Plus";

		case Token::Type::Minus:
			return "Minus";

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

	fmt::println("O0\n{}", fmt::join(tokens | std::views::enumerate, "\n"));

	if (args.o == 0u)
		return;

	if (args.o >= 1u) {
		while (true) {
			auto it = TokensIterator::begin();
			if (it == tokens.end() || it->type != Token::Type::OpenLoop)
				break;

			auto targetIndex = it->refVal;
			for (; it.index() != targetIndex + 1z; ++it)
				it->valid = false;
		}
	}
	fmt::println("\nO1\n{}", fmt::join(tokens | std::views::enumerate, "\n"));
	if (args.o >= 2u) {

	}
	fmt::println("\nO2\n{}", fmt::join(tokens | std::views::enumerate, "\n"));
	return;
}