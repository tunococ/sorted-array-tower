#include <sorted_array_tower/add_one.hpp>

namespace sorted_array_tower {

template <>
std::string add_one(std::string x) {
    return x + "1";
}

} // namespace sorted_array_tower
