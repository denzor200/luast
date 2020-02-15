#pragma once

#include <boost/spirit/home/x3.hpp>
#include "ast.hpp"
#include <iostream>

namespace client {
	namespace x3 = boost::spirit::x3;

	// TODO: информацию по приоритетам вынести отдельно
	// TODO: зарегать все возможные операторы
	typedef std::pair<ast::optoken, int> priority;
	static const std::vector<priority> priority_table = {
		{ast::op_plus, 3},
		{ast::op_minus, 3},
		{ast::op_times, 2},
		{ast::op_divide, 2},
		{ast::op_negative, 1},
		{ast::op_positive, 1},
		{ast::op_degree, 0},
	};
	inline int get_priority(ast::optoken value) {
		for (priority p : priority_table) {
			if (p.first == value)
				return p.second;
		}
		return -1;
	}

	namespace ast
	{
		///////////////////////////////////////////////////////////////////////////
		//  The AST Printer
		///////////////////////////////////////////////////////////////////////////
		struct printer
		{
			typedef std::string result_type;

			int priority = -1;

			std::string operator()(unknown) const { return ""; }
			std::string operator()(unsigned int n) const { return std::to_string(n); }
			std::string operator()(const name& v) const { return v; }
			std::string operator()(const std::string& v) const { return v; }
			std::string operator()(nil) const { return "nil"; }

			std::string operator()(operation const& x) const
			{
				int parent_priority = priority;
				const std::string& po = boost::apply_visitor(*this, x.operand_);
				switch (x.operator_)
				{
				case ast::op_or: return " or " + po;
				case ast::op_plus: return " + " + po;
				case ast::op_minus: return " - " + po;
				case ast::op_times: return " * " + po;
				case ast::op_divide: return " / " + po;
				case ast::op_degree: return " ^ " + po;
				default: BOOST_ASSERT_MSG(0, "Invalid operation value");
				}
				return "";
			}

			std::string operator()(unary const& x) const
			{
				int parent_priority = priority;
				const std::string& po = boost::apply_visitor(*this, x.operand_);
				switch (x.sign)
				{
				case ast::op_negative: return " - " + po;
				case ast::op_positive: return " + " + po;
				default: BOOST_ASSERT_MSG(0, "Invalid operation value");
				}
				return "";
			}

			std::string operator()(expression const& x) const
			{
				int parent_priority = priority;
				std::string po = boost::apply_visitor(*this, x.first);
				for (operation const& oper : x.rest)
				{
					po += (*this)(oper);
				}
				return po;
			}
		};

		///////////////////////////////////////////////////////////////////////////
		//  The AST evaluator
		///////////////////////////////////////////////////////////////////////////
		struct eval
		{
			typedef int result_type;

			int operator()(unknown) const { BOOST_ASSERT(0); return 0; }
			int operator()(unsigned int n) const { return n; }

			// TODO: реализовать более умное эвалуирование
			// Сейчас оно даже ошибки не выдвет на этот случай..
			int operator()(const name& v) const { return 0; }
			int operator()(const std::string& v) const { return 0; }
			int operator()(nil) const { return 0; }

			int operator()(operation const& x, int lhs) const
			{
				int rhs = boost::apply_visitor(*this, x.operand_);
				switch (x.operator_)
				{
				case ast::op_or: return lhs || rhs;
				case ast::op_plus: return lhs + rhs;
				case ast::op_minus: return lhs - rhs;
				case ast::op_times: return lhs * rhs;
				case ast::op_divide: return lhs / rhs;
				case ast::op_degree: return static_cast<int>(std::pow(lhs, rhs));
				default: BOOST_ASSERT_MSG(0, "Invalid operation value");
				}
				return 0;
			}

			int operator()(unary const& x) const
			{
				int rhs = boost::apply_visitor(*this, x.operand_);
				switch (x.sign)
				{
				case ast::op_negative: return -rhs;
				case ast::op_positive: return +rhs;
				}
				BOOST_ASSERT(0);
				return 0;
			}

			int operator()(expression const& x) const
			{
				int state = boost::apply_visitor(*this, x.first);
				for (operation const& oper : x.rest)
				{
					state = (*this)(oper, state);
				}
				return state;
			}
		};
	}
}


