#pragma once

#include "DataManager.hpp"

#include <algorithm>


class TokensIterator final {
public:
	constexpr TokensIterator() = default;

private:
	constexpr TokensIterator(Token* ptr) : m_ptr(ptr) {}

public:
	constexpr TokensIterator(TokensIterator const&) noexcept = default;
	constexpr TokensIterator& operator=(TokensIterator const&) noexcept = default;
	constexpr TokensIterator(TokensIterator&&) noexcept = default;
	constexpr TokensIterator& operator=(TokensIterator&&) noexcept = default;
	
	constexpr ~TokensIterator() noexcept = default;

public:
	static TokensIterator begin() noexcept {
		auto it = std::ranges::find_if(
			s_tokens,
			[](Token const& x) constexpr static noexcept { return x.valid; }
		);

		return it == s_tokens.end() ? nullptr : std::to_address(it);
	}

	static TokensIterator end() noexcept {
		return nullptr;
	}

	constexpr TokensIterator operator+(std::size_t x) const noexcept {
		return { m_ptr + x };
	}

	constexpr TokensIterator operator-(std::size_t x) const noexcept {
		return { m_ptr - x };
	}

	constexpr TokensIterator& operator++() noexcept {
		auto ptr = std::ranges::find_if(
			m_ptr + 1z, std::to_address(s_tokens.end()),
			[](Token const& x) constexpr static noexcept { return x.valid; }
		);

		m_ptr = (ptr == std::to_address(s_tokens.end())) ? nullptr : ptr;

		return *this;
	}

	constexpr TokensIterator operator++(int) noexcept {
		TokensIterator ret{ *this };

		++(*this);

		return ret;
	}

	constexpr Token& operator*() const noexcept { return *m_ptr; }

	constexpr Token* operator->() const noexcept { return m_ptr; }

	constexpr auto operator<=>(TokensIterator const& other) const noexcept = default;

	constexpr bool operator==(TokensIterator const& other) const noexcept = default;

	[[nodiscard]] constexpr std::size_t index() const noexcept {
		return m_ptr - s_tokens.data();
	}

private:
	using TokensStdIt = std::remove_cvref_t<decltype(DataManager::get()->getTokens())>::iterator;

public:
	constexpr auto operator<=>(TokensStdIt const& it) const noexcept {
		return m_ptr <=> std::to_address(it);
	}

	constexpr bool operator==(TokensStdIt const& it) const noexcept {
		return m_ptr == std::to_address(it);
	}

private: // Fields
	static inline auto& s_tokens = DataManager::get()->getTokens();

	Token* m_ptr = nullptr;
};