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
		return std::to_address(std::ranges::find_if(
			s_tokens, [](Token const& x) { return x.valid; }
		));
	}

	static TokensIterator end() noexcept {
		auto range = std::ranges::find_last_if(
			s_tokens, [](Token const& x) { return x.valid; }
		);

		return std::to_address(range.empty() ? range.begin() : range.begin() + 1z);
	}

	constexpr TokensIterator operator+(std::size_t x) const noexcept {
		return { m_ptr + x };
	}

	constexpr TokensIterator operator-(std::size_t x) const noexcept {
		return { m_ptr - x };
	}

	constexpr TokensIterator& operator++() noexcept {
		m_ptr = std::to_address(std::ranges::find_if(
			m_ptr + 1z, std::to_address(s_tokens.end()),
			[](Token const& x) { return x.valid; }
		));

		return *this;
	}

	constexpr TokensIterator operator++(int) noexcept {
		TokensIterator ret{ *this };

		++(*this);

		return ret;
	}

	constexpr Token& operator*() const noexcept { return *m_ptr; }

	constexpr Token* operator->() const noexcept { return m_ptr; }

	constexpr bool operator==(TokensIterator const& other) const noexcept {
		return m_ptr == other.m_ptr;
	}

	constexpr bool operator!=(TokensIterator const& other) const noexcept {
		return m_ptr != other.m_ptr;
	}

	[[nodiscard]] constexpr std::size_t index() const noexcept {
		return m_ptr - s_tokens.data();
	}

private:
	using TokensStdIt = std::remove_cvref_t<decltype(DataManager::get()->getTokens())>::iterator;

public:
	constexpr bool operator==(TokensStdIt const& it) const noexcept {
		return m_ptr == std::to_address(it);
	}

	constexpr bool operator!=(TokensStdIt const& it) const noexcept {
		return m_ptr != std::to_address(it);
	}

private: // Fields
	static inline auto& s_tokens = DataManager::get()->getTokens();

	Token* m_ptr = nullptr;
};