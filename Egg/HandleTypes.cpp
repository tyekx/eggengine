#include "HandleTypes.h"

namespace Egg {

	Response::Response() : status{ 400 }, content{} { }
	Response::Response(int sc) : status{ sc }, content{} {}
	Response::Response(int sc, std::string c) : status{ sc }, content{ std::move(c) } { }

}

