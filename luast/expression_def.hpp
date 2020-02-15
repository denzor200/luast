#pragma once

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include "ast.hpp"
#include "ast_adapted.hpp"
#include "expression.hpp"
#include "common.hpp"
#include "error_handler.hpp"

namespace client
{
	namespace x3 = boost::spirit::x3;
	///////////////////////////////////////////////////////////////////////////////
	//  The grammar
	///////////////////////////////////////////////////////////////////////////////
	namespace parser
	{
		using x3::uint_;
		using x3::bool_;
		using x3::char_;

		////////////////////////////////////////////////////////////////////////////
		// Tokens
		////////////////////////////////////////////////////////////////////////////

		x3::symbols<ast::optoken> additive_op;
		x3::symbols<ast::optoken> multiplicative_op;
		x3::symbols<ast::optoken> unary_op;
		x3::symbols<ast::optoken> degree_op;

		void add_keywords()
		{
			static bool once = false;
			if (once)
				return;
			once = true;

			additive_op.add
			("+", ast::op_plus)
				("-", ast::op_minus)
				;

			multiplicative_op.add
			("*", ast::op_times)
				("/", ast::op_divide)
				;

			unary_op.add
			("+", ast::op_positive)
				("-", ast::op_negative)
				//("!", ast::op_not)
				;

			degree_op.add
			("^", ast::op_degree)
				;

			add_keywords2();
		}

		struct expression_class;
		struct additive_expr_class;
		struct multiplicative_expr_class;
		struct unary_expr_class;
		struct unary_helper_expr_class;
		struct degree_expr_class;
		struct degree_helper_expr_class;
		struct primary_expr_class;

		typedef x3::rule<expression_class, ast::expression> expression_type;
		typedef x3::rule<additive_expr_class, ast::expression> additive_expr_type;
		typedef x3::rule<multiplicative_expr_class, ast::expression> multiplicative_expr_type;
		typedef x3::rule<unary_expr_class, ast::operand> unary_expr_type;
		typedef x3::rule<unary_helper_expr_class, ast::unary> unary_helper_expr_type;
		typedef x3::rule<degree_expr_class, ast::expression> degree_expr_type;
		typedef x3::rule<degree_helper_expr_class, ast::operation> degree_helper_expr_type;
		typedef x3::rule<primary_expr_class, ast::operand> primary_expr_type;

		expression_type const expression("expression");
		additive_expr_type const additive_expr("additive_expr");
		multiplicative_expr_type const multiplicative_expr("multiplicative_expr");
		unary_expr_type const unary_expr("unary_expr");
		unary_helper_expr_type const unary_helper_expr("unary_helper_expr");
		degree_expr_type const degree_expr("degree_expr");
		degree_helper_expr_type const degree_helper_expr("degree_helper_expr");
		primary_expr_type const primary_expr("primary_expr");

		auto const expression_def =
			additive_expr
			>> *(kw_or > additive_expr)
			;

		auto const additive_expr_def =
			multiplicative_expr
			>> *(additive_op > multiplicative_expr)
			;

		auto const multiplicative_expr_def =
			unary_expr
			>> *(multiplicative_op > unary_expr)
			;

		// unary_expr_def должен отдавать на выходе sign всегда
		// unary_helper_expr_def тогда будет не нужен
		// нужно найти способ задать знак по умолчанию
		// НЕЛЬЗЯ! Ложный вызов перегруженного оператора..
		auto const unary_expr_def =
			degree_expr
			| unary_helper_expr
			;

		// Пришлось отделить unary_helper_expr от unary_expr
		// Потому что компилятор никак не хотел автоматически преобразовывать
		//  вполне корректный ast::signed_ в ast::operand
		auto const unary_helper_expr_def =
			(unary_op > degree_expr)
			;

		// TODO: чем >> отличается от >
		// <pow_expression> ::=  <pow_operand> '^' <pow_expression> | <pow_operand>
		auto const degree_expr_def =
			primary_expr
			>> -degree_helper_expr
			;

		auto const degree_helper_expr_def =
			degree_op > degree_expr
			;

		auto const primary_expr_def =
			uint_
			| kw_bool
			| kw_nil
			| identifier
			| quoted
			| '(' > expression > ')'
			;

		BOOST_SPIRIT_DEFINE(
			expression
			, additive_expr
			, multiplicative_expr
			, unary_expr
			, unary_helper_expr
			, degree_expr
			, degree_helper_expr
			, primary_expr
		);

		struct expression_class: error_handler_base, x3::annotate_on_success
		{
		};
	}
}

namespace client
{
	parser::expression_type const& expression()
	{
		parser::add_keywords();
		return parser::expression;
	}
}

