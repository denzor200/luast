#pragma once

#include <boost/spirit/home/x3.hpp>

// TODO: ast убрать от сюда??
#include "ast.hpp"
#include "ast_adapted.hpp"

// TODO: объ€вление объектов в h-файле не обойдетс€ без последствий
// подумай о создании _def.h файла

namespace client {

	//****************************
	// Custom parsers
	//****************************
	// ”чтите, что hexaesc_reader всегда возвращет true. »ли отваливаетс€
	struct hexaesc_reader : x3::parser<hexaesc_reader> {
		using attribute_type = int;

		static int hexavalue(int c) {
			if (std::isdigit(c)) return c - '0';
			else return std::tolower(c) - 'a' + 10;
		}

		template<typename Iterator, typename Context, typename RContext, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context const& context,
			RContext const& rcontext, Attribute& attr) const
		{
			int r = 0;

			for (int i = 0; i < 2; ++i)
			{
				if (first == last || !std::isxdigit(*first))
				{
					boost::throw_exception(
						x3::expectation_failure<Iterator>(
							first, "hexadecimal digit expected"));
				}
				
				r = (r << 4) + hexavalue(*first);
				first++;
			}

			attr = r;
			return true;
		};
	};

	struct decesc : x3::parser<decesc> {
		using attribute_type = int;

		template<typename Iterator, typename Context, typename RContext, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context const& context,
			RContext const& rcontext, Attribute& attr) const
		{
			bool success = false;
			int r = 0;

			for (int i = 0; i < 3; ++i)
			{
				if (first == last || !std::isdigit(*first))
					break;

				r = 10 * r + *first - '0';

				// помечаем, что как минимум один digit был считан
				success = true;
				first++;
			}

			if (success)
			{
				if (r > UCHAR_MAX)
				{
					boost::throw_exception(
						x3::expectation_failure<Iterator>(
							first, "decimal escape too large"));
				}
				attr = r;
				return true;
			}
			return false;
		}
	};

	struct unfinished_guard : x3::parser<unfinished_guard> {
		using attribute_type = char;

		template<typename Iterator, typename Context, typename RContext, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context const& context,
			RContext const& rcontext, Attribute& attr) const
		{
			if (first == last) {
				boost::throw_exception(
					x3::expectation_failure<Iterator>(
						first, "unfinished string"));
			}
			if (*first == '\n' || *first == '\r') {
				boost::throw_exception(
					x3::expectation_failure<Iterator>(
						first, "unfinished string"));
			}
			return false;
		}
	};

	struct esc_guard : x3::parser<esc_guard> {
		using attribute_type = char;

		template<typename Iterator, typename Context, typename RContext, typename Attribute>
		bool parse(Iterator& first, Iterator const& last, Context const& context,
			RContext const& rcontext, Attribute& attr) const
		{
			if (first != last && *first == '\\') {
				boost::throw_exception(
					x3::expectation_failure<Iterator>(
						first, "invalid escape sequence"));
			}
			return false;
		}
	};

	auto const hexaesc_reader_ = hexaesc_reader();
	auto const decesc_ = decesc();
	auto const unfinished_guard_ = unfinished_guard();
	auto const esc_guard_ = esc_guard();

	namespace parser
	{
		using x3::raw;
		using x3::lexeme;
		using x3::alpha;
		using x3::alnum;
		using x3::lit;
		using x3::char_;

		////////////////////////////////////////////////////////////////////////////
		// Tokens
		////////////////////////////////////////////////////////////////////////////

		x3::symbols<bool> symbool;
		x3::symbols<ast::optoken> or_op;
		x3::symbols<> nil_kw;
		x3::symbols<char const> unesc_char;

		void add_keywords2()
		{
			static bool once = false;
			if (once)
				return;
			once = true;

			symbool.add
			("true", true)
				("false", false)
				;

			or_op.add
			("or", ast::op_or)
				;

			nil_kw.add
			("nil")
				;

			unesc_char.add("\\a", '\a')("\\b", '\b')("\\f", '\f')("\\n", '\n')
				("\\r", '\r')("\\t", '\t')("\\v", '\v')
				;
			unesc_char.add
			("\\\n", '\n')("\\\r", '\n')
				;
			unesc_char.add
			("\\\\", '\\')("\\\"", '"')("\\'", '\'')
				;

			// TODO: реализовать спецсимвол z
		}

		struct identifier_class;
		struct quoted_class;
		struct kw_bool_class;
		struct kw_or_class;
		struct kw_nil_class;

		typedef x3::rule<identifier_class, ast::name> identifier_type;
		typedef x3::rule<quoted_class, std::string> quoted_type;
		typedef x3::rule<kw_bool_class, bool> kw_bool_type;
		typedef x3::rule<kw_or_class, ast::optoken> kw_or_type;
		typedef x3::rule<kw_nil_class, ast::nil> kw_nil_type;

		identifier_type const identifier = "identifier";
		quoted_type const quoted = "quoted";
		kw_bool_type const kw_bool = "kw_bool";
		kw_or_type const kw_or = "kw_or";
		kw_nil_type const kw_nil = "kw_nil";

		auto mkkw = [](const auto& val)
		{
			return lexeme[val >> !(alnum | '_')];
		};
		auto mkquoted = [](const char ch)
		{
			return lexeme[ch >> *(unfinished_guard_ | ("\\x" >> hexaesc_reader_ | unesc_char | '\\' >> decesc_ | esc_guard_) | (char_ - ch)) >> ch];
		};

		auto const reserved = mkkw(symbool | or_op | nil_kw);
		auto const identifier_def	= raw[lexeme[(alpha | '_') >> *(alnum | '_')] - reserved];
		auto const quoted_def		= mkquoted('"')
									| mkquoted('\'');
		auto const kw_bool_def	= mkkw(symbool);
		auto const kw_or_def	= mkkw(or_op);
		auto const kw_nil_def	= mkkw(nil_kw);

		BOOST_SPIRIT_DEFINE(identifier);
		BOOST_SPIRIT_DEFINE(quoted);
		BOOST_SPIRIT_DEFINE(kw_bool);
		BOOST_SPIRIT_DEFINE(kw_or);
		BOOST_SPIRIT_DEFINE(kw_nil);
	}
}

template<>
struct client::x3::get_info<client::hexaesc_reader>
{
	typedef std::string result_type;
	std::string operator()(client::hexaesc_reader const&) const
	{
		return "hexaesc_reader";
	}
};

template<>
struct client::x3::get_info<client::decesc>
{
	typedef std::string result_type;
	std::string operator()(client::decesc const&) const
	{
		return "decesc";
	}
};

template<>
struct client::x3::get_info<client::unfinished_guard>
{
	typedef std::string result_type;
	std::string operator()(client::unfinished_guard const&) const
	{
		return "unfinished_guard";
	}
};

template<>
struct client::x3::get_info<client::esc_guard>
{
	typedef std::string result_type;
	std::string operator()(client::esc_guard const&) const
	{
		return "esc_guard";
	}
};

