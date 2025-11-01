//
// Created by dunca on 01/11/2025.
//

#include "ast.h"
#include "../lexer/tokens.h"

// Implements recursive descent parsing
namespace Parser {

	//class to iterate over the vector of tokens
	class VectorAndIterator {
	using VectorPointer = std::unique_ptr<std::vector<Token::Token>>;
	private:
		VectorPointer m_vectorRef;
		int m_index {0};
	public:
		explicit VectorAndIterator(VectorPointer vec) : m_vectorRef(std::move(vec)) {};

		int index() const { return m_index; }
		void setIndex(int index) { m_index = index; }

		VectorAndIterator& operator++() {
			if (m_index < m_vectorRef->size()) {
				++m_index;
			} else {
				throw std::out_of_range("VectorAndIterator::operator++ going out of range");
			}
			return *this;
		}

		VectorAndIterator& operator--() {
			if (m_index > 0) {
				--m_index;
			} else {
				throw std::out_of_range("VectorAndIterator::operator-- going out of range");
			}
			return *this;
		}

		VectorAndIterator& operator+=(int add) {
			if (m_index + add < m_vectorRef->size()) {
				m_index += add;
			} else {
				throw std::out_of_range("VectorAndIterator::operator+= going out of range");
			}
			return *this;
		}

		VectorAndIterator& operator-=(int sub) {
			if (m_index - sub > 0) {
				m_index -= sub;
			} else {
				throw std::out_of_range("VectorAndIterator::operator-= going out of range");
			}
			m_index -= sub;
			return *this;
		}

		Token::Token& operator[](const int index) const {
			return (*m_vectorRef)[index];
		}

		Token::Token& takeCurrent() {
			Token::Token& tmp{(*m_vectorRef)[m_index]};
			++m_index;
			return tmp;
		}
	};

	void expect(Token::Token expected, VectorAndIterator tokens) {
		Token::Token actual {tokens.takeCurrent()};
		if (actual != expected) {}
	}

	Ast::ReturnKeyword parseStatement(Ast::Statement& statement) {
		return Ast::ReturnKeyword{2};
	}
}