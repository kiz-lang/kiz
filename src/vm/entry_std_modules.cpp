#include "../models/models.hpp"
#include "../libs/io/include/io_lib.hpp"

namespace kiz {

void Vm::entry_std_modules() {
    auto std_modules_insert = [](const std::string& name, model::NativeFunction* o){
        o->make_ref();
        std_modules.insert(name, o);
    };
    std_modules_insert("io", model::create_nfunc(io_lib::init_module, "__init__"));
}
} // namespace model