// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "expression_def.hpp"
#include "config.hpp"

namespace client {
	namespace parser
	{
		BOOST_SPIRIT_INSTANTIATE(expression_type, iterator_type, context_type);
	}
}
