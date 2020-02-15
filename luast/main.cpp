// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "ast.hpp"
#include "expression.hpp"
#include "visitors.hpp"
#include "config.hpp"

#include <iostream>
#include <boost/assert.hpp>

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////

static void test_phrase(bool* status, const std::string& phrase, boost::optional<std::string> expect)
{
	typedef std::string::const_iterator iterator_type;
	typedef client::ast::expression ast_program;
	typedef client::ast::printer ast_print;
	typedef client::ast::eval ast_eval;

	ast_program program;                // Our program (AST)
	ast_print print;                    // Prints the program
	//ast_eval eval;                      // Evaluates the program

	iterator_type iter = phrase.cbegin();
	iterator_type end = phrase.cend();


	using boost::spirit::x3::with;
	using client::parser::error_handler_type;
	error_handler_type error_handler(iter, end, std::cerr); // Our error handler

	// Our parser
	auto const parser =
		// we pass our error handler to the parser so we can access
		// it later on in our on_error and on_sucess handlers
		with<client::parser::error_handler_tag>(std::ref(error_handler))
		[
			client::expression()
		];



	// Header
	std::cout << " * testing phrase:" << std::endl;
	std::cout << "\t\t" << phrase << std::endl;

	// Parsing
	boost::spirit::x3::ascii::space_type space;
	bool r = phrase_parse(iter, end, parser, space, program);


	// Output parsed value
	if (r && iter == end)
	{
		// TODO: здесь должен быть eval. Это не правильно - использовать print для получения значения
		std::cout << " * result:" << std::endl;
		std::cout << "\t\t" << print(program) << std::endl;
	}
	else
	{
		std::cout << " * result:" << std::endl;
		std::cout << "\t\t" << "<NULL>" << std::endl;
	}


	// Verify parsed value
	if (expect)
	{
		if ((r && iter == end) && print(program) == expect)
		{
			std::cout << "success" << std::endl;
			std::cout << std::endl;
		}
		else {
			std::cout << "fail" << std::endl;
			std::cout << std::endl;
			*status = false;
			return;
		}
	}
	else {
		if (!(r && iter == end))
		{
			std::cout << "success" << std::endl;
			std::cout << std::endl;
		}
		else {
			std::cout << "fail" << std::endl;
			std::cout << std::endl;
			*status = false;
			return;
		}
	}

}

static int test()
{
	bool status = true;

	std::cout << "/////////////////////////////////////////////////////////\n";
	std::cout << "Testing \"expression\" parser...\n";
	std::cout << "/////////////////////////////////////////////////////////\n\n";

	// Testing "quoted"
	test_phrase(&status, "     \"line1\\aline2\"     ", std::string("line1\aline2"));
	test_phrase(&status, "     \"line1\\bline2\"     ", std::string("line1\bline2"));
	test_phrase(&status, "     \"line1\\fline2\"     ", std::string("line1\fline2"));
	test_phrase(&status, "     \"line1\\nline2\"     ", std::string("line1\nline2"));
	test_phrase(&status, "     \"line1\\rline2\"     ", std::string("line1\rline2"));
	test_phrase(&status, "     \"line1\\tline2\"     ", std::string("line1\tline2"));
	test_phrase(&status, "     \"line1\\vline2\"     ", std::string("line1\vline2"));

	test_phrase(&status, "     \"line1\\\nline2\"     ", std::string("line1\nline2"));
	test_phrase(&status, "     \"line1\\\rline2\"     ", std::string("line1\nline2"));

	// error "unfinished string" test
	test_phrase(&status, "     \"line1\nline2\"     ", boost::optional<std::string>());
	test_phrase(&status, "     \"line1\rline2\"     ", boost::optional<std::string>());
	test_phrase(&status, "     \"line1				", boost::optional<std::string>());


	test_phrase(&status, "     \"line1\\\\line2\"     ", std::string("line1\\line2"));
	test_phrase(&status, "     \"line1\\\"line2\"     ", std::string("line1\"line2"));
	test_phrase(&status, "     \"line1\\\'line2\"     ", std::string("line1\'line2"));

	test_phrase(&status, "     \"line1'line2\"     ", std::string("line1'line2"));
	test_phrase(&status, "     'line1\"line2'     ", std::string("line1\"line2"));

	test_phrase(&status, "     \"line1\\\\nline2\"     ", std::string("line1\\nline2"));

	test_phrase(&status, "     \"line1\\x32line2\"     ", std::string("line12line2"));
	test_phrase(&status, "     \"line1\\xffline2\"     ", std::string("line1\xffline2"));
	// error "hexadecimal digit expected" test
	test_phrase(&status, "     \"line1\\x0line2\"     ", boost::optional<std::string>());

	test_phrase(&status, "     \"line1\\50line2\"     ", std::string("line12line2"));
	test_phrase(&status, "     \"line1\\255line2\"     ", std::string("line1\xffline2"));
	// error "decimal escape too large" test
	test_phrase(&status, "     \"line1\\256line2\"     ", boost::optional<std::string>());
	// error "invalid escape sequence" test
	test_phrase(&status, "     \"line1\\qline2\"     ", boost::optional<std::string>());
	test_phrase(&status, "     \"line1\\",				boost::optional<std::string>());

	// Test "out of range" case for hexaesc_reader parser
	test_phrase(&status, "     \"line1\\x",				boost::optional<std::string>());
	// Test "out of range" case for decesc parser
	test_phrase(&status, "     \"line1\\",				boost::optional<std::string>());
	// Test "out of range" case for unfinished_guard parser
	//		see error "unfinished string" test higher

	if (status)
	{
		std::cout << "/////////////////////////////////////////////////////////\n";
		std::cout << "Testing \"expression\" parser: ALL RIGHT\n";
		std::cout << "/////////////////////////////////////////////////////////\n\n";
	}
	else
	{
		std::cout << "/////////////////////////////////////////////////////////\n";
		std::cout << "Testing \"expression\" parser: FAILED\n";
		std::cout << "/////////////////////////////////////////////////////////\n\n";
	}

	return 0;
}

static int sandbox()
{
	std::cout << "/////////////////////////////////////////////////////////\n\n";
	std::cout << "Expression parser...\n\n";
	std::cout << "/////////////////////////////////////////////////////////\n\n";
	std::cout << "Type an expression...or [q or Q] to quit\n\n";

	typedef std::string::const_iterator iterator_type;
	typedef client::ast::expression ast_program;
	typedef client::ast::printer ast_print;
	typedef client::ast::eval ast_eval;

	std::string str;
	while (std::getline(std::cin, str))
	{
		if (str.empty() || str[0] == 'q' || str[0] == 'Q')
			break;

		ast_program program;                // Our program (AST)
		ast_print print;                    // Prints the program
		ast_eval eval;                      // Evaluates the program

		iterator_type iter = str.begin();
		iterator_type end = str.end();


		using boost::spirit::x3::with;
		using client::parser::error_handler_type;
		error_handler_type error_handler(iter, end, std::cerr); // Our error handler

		// Our parser
		auto const parser =
			// we pass our error handler to the parser so we can access
			// it later on in our on_error and on_sucess handlers
			with<client::parser::error_handler_tag>(std::ref(error_handler))
			[
				client::expression()
			];


		boost::spirit::x3::ascii::space_type space;
		bool r = phrase_parse(iter, end, parser, space, program);

		if (r && iter == end)
		{
			std::cout << "-------------------------\n";
			std::cout << "Parsing succeeded\n";
			std::cout << print(program);
			std::cout << "\nResult: " << eval(program) << std::endl;
			std::cout << "-------------------------\n";
		}
		else
		{
			std::cout << "-------------------------\n";
			std::cout << "Parsing failed\n";
			std::cout << "-------------------------\n";
		}
	}

	std::cout << "Bye... :-) \n\n";
	return 0;
}

static void help()
{
	// TODO: implement this
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Invalid args\n";
		help();
		return -1;
	}

	if (strcmp(argv[1], "test") == 0)
		return test();
	else if (strcmp(argv[1], "sandbox") == 0)
		return sandbox();

	std::cerr << "Invalid args\n";
	help();
	return -1;
}
