#include <tyghbn/add_one.hpp>

namespace tyghbn {

template <>
std::string add_one(std::string x) {
    return x + "1";
}

} // namespace tyghbn
