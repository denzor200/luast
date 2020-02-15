#pragma once

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/optional.hpp>
#include <list>

namespace client {
	namespace x3 = boost::spirit::x3;
	namespace ast
	{
		///////////////////////////////////////////////////////////////////////////
		//  The AST
		///////////////////////////////////////////////////////////////////////////
		struct unknown {};
		struct nil {};
		struct unary;
		struct expression;

		// TODO: запретить неявно кастовать из std::string
		struct name : std::string, x3::position_tagged
		{
			using std::string::string;
		};

		struct operand : x3::variant<
			unknown
			, nil
			, unsigned int
			, name
			, std::string
			, x3::forward_ast<unary>
			, x3::forward_ast<expression>
		>
		{
			using base_type::base_type;
			using base_type::operator=;
		};

		enum optoken
		{
			op_or,
			op_plus,
			op_minus,
			op_times,
			op_divide,
			op_positive,
			op_negative,
			op_degree
		};

		struct unary
		{
			optoken sign;
			operand operand_;
		};

		struct operation : x3::position_tagged
		{
			optoken operator_;
			operand operand_;
		};

		struct expression : x3::position_tagged
		{
			operand first;
			std::list<operation> rest;
		};

		struct variable_with_index : x3::position_tagged
		{
			expression prefixexp;
			expression index;
		};

		struct variable_with_member : x3::position_tagged
		{
			expression prefixexp;
			name member;
		};

		struct variable : x3::variant<
			name
			, variable_with_index
			, variable_with_member
		>
		{
			using base_type::base_type;
			using base_type::operator=;
		};

		struct assignment : x3::position_tagged
		{
			std::list<variable> varlist;
			std::list<expression> explist;
		};

		struct functioncall : x3::position_tagged
		{
			expression prefixexp;
			boost::optional<name> method;
			std::list<expression> args;
		};

		struct funcname
		{
			name value;
			std::list<name> members;
			boost::optional<name> method;
		};

		struct label
		{
			name value;
		};

		struct break_ : x3::position_tagged {};

		struct goto_
		{
			name value;
		};

		struct args : x3::position_tagged {};

		struct do_end;
		struct while_;
		struct repeat_;
		struct if_;
		struct for_;
		struct foreach;
		struct function;
		struct local_function;

		struct local : x3::position_tagged
		{
			std::list<name> namelist;
			std::list<expression> explist;
		};

		struct retstat : std::list<expression> {};

		struct statement : x3::variant<
			assignment
			, functioncall
			, label
			, break_
			, goto_
			, boost::recursive_wrapper<do_end>
			, boost::recursive_wrapper<while_>
			, boost::recursive_wrapper<repeat_>
			, boost::recursive_wrapper<if_>
			, boost::recursive_wrapper<for_>
			, boost::recursive_wrapper<foreach>
			, boost::recursive_wrapper<function>
			, boost::recursive_wrapper<local_function>
			, local
			, retstat
		>
		{
			using base_type::base_type;
			using base_type::operator=;
		};

		struct do_end
		{
			statement block;
		};

		struct while_
		{
			expression exp;
			statement block;
		};

		struct repeat_
		{
			statement block;
			expression exp;
		};

		struct if_
		{
			expression exp;
			statement block;
			std::list<std::pair<expression, statement>> elseif;
			boost::optional<statement> else_block;
		};

		struct for_
		{
			name ctr;
			expression init;
			expression exit;
			expression step;
			statement block;
		};

		struct foreach
		{
			std::list<name> namelist;
			std::list<expression> explist;
			statement block;
		};

		struct function
		{
			funcname _name;
			std::list<name> parlist;
			boost::optional<args> _args;
			statement block;
		};

		struct local_function
		{
			name _name;
			std::list<name> parlist;
			boost::optional<args> _args;
			statement block;
		};

		// print function for debugging
		inline std::ostream& operator<<(std::ostream& out, unknown) { out << "unknown"; return out; }
	}
}

