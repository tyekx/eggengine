#include "ResourceEnums.h"

namespace Netcode::Graphics {

	ResourceState operator|(ResourceState lhs, ResourceState rhs) {
		return static_cast<ResourceState>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}
}