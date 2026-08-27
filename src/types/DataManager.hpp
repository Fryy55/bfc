#pragma once

#include "Token.hpp"
#include "CLIArgs.hpp"

#include <vector>
#include <stack>


class DataManager final {
public:
	static DataManager* get() noexcept {
		static DataManager instance{};

		return &instance;
	}

private:
	DataManager() noexcept = default;

public:
	DataManager(DataManager const&) = delete;
	DataManager& operator=(DataManager const&) = delete;
	DataManager(DataManager&&) = delete;
	DataManager& operator=(DataManager&&) = delete;

private:
	~DataManager() noexcept = default;

public:
	std::stack<std::size_t>& getLoops() noexcept { return m_loops; }
	std::vector<Token>& getTokens() noexcept { return m_tokens; }
	CLIArgs const& getArgs() noexcept { return *m_args; }
	void setArgs(CLIArgs const& args) noexcept { m_args = &args; }
	std::size_t& getLine() noexcept { return m_line; }
	std::size_t& getColumn() noexcept { return m_column; }
	std::array<char, 64z>& getContext() noexcept { return m_context; }

private:
	std::stack<std::size_t> m_loops{};
	std::vector<Token> m_tokens{};
	CLIArgs const* m_args;
	std::size_t m_line = 1z;
	std::size_t m_column = 0z;
	std::array<char, 64z> m_context;
};