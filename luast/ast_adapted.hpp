#pragma once

#include "ast.hpp"
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(client::ast::unary,
	sign, operand_
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::operation,
	operator_, operand_
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::expression,
	first, rest
)

