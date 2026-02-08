#include "../../src/models/models.hpp"
#include "../../src/vm/vm.hpp"
#include "include/builtin_methods.hpp"
#include "include/builtin_functions.hpp"

namespace model {

// Decimal.__call__：构造Decimal对象（支持字符串/Int/Decimal初始化）
Object* decimal_call(Object* self, const List* args) {
    auto a = builtin::get_one_arg(args);
    dep::Decimal val(0);

    // 从String初始化（如 "123.45", "-67.89e2"）
    if (auto s = dynamic_cast<String*>(a)) {
        val = dep::Decimal(s->val);
    }
    // 从Int初始化
    else if (auto i = dynamic_cast<Int*>(a)) {
        val = dep::Decimal(i->val);
    }
    // 从Decimal初始化（拷贝）
    else if (auto d = dynamic_cast<Decimal*>(a)) {
        val = d->val;
    }
    // 假值（Nil/Bool(false)）初始化为0
    else if (!kiz::Vm::is_true(a)) {
        val = dep::Decimal(0);
    }

    return new Decimal(val);
}

// Decimal.__bool__：非零判断（0为false，其余为true）
Object* decimal_bool(Object* self, const List* args) {
    const auto self_dec = dynamic_cast<Decimal*>(self);
    assert(self_dec != nullptr && "decimal_bool must be called by Decimal object");

    // 0的Decimal（mantissa=0，exponent=0）返回false
    return load_bool(!(self_dec->val == dep::Decimal(dep::BigInt(0))));
}

// Decimal.__add__：加法（self + args[0]），支持Int/Decimal
Object* decimal_add(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (decimal_add)");
    assert(args->val.size() == 1 && "function Decimal.add need 1 arg");

    const auto self_dec = dynamic_cast<Decimal*>(self);
    assert(self_dec != nullptr && "decimal_add must be called by Decimal object");

    // 与Int相加
    if (auto another_int = dynamic_cast<Int*>(args->val[0])) {
        dep::Decimal res = self_dec->val + another_int->val;
        return new Decimal(res);
    }
    // 与Decimal相加
    else if (auto another_dec = dynamic_cast<Decimal*>(args->val[0])) {
        dep::Decimal res = self_dec->val + another_dec->val;
        return new Decimal(res);
    }
    // 仅允许Int/Decimal
    assert(false && "function Decimal.add second arg need be Int or Decimal");
}

// Decimal.__sub__：减法（self - args[0]），支持Int/Decimal
Object* decimal_sub(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (decimal_sub)");
    assert(args->val.size() == 1 && "function Decimal.sub need 1 arg");

    const auto self_dec = dynamic_cast<Decimal*>(self);
    assert(self_dec != nullptr && "decimal_sub must be called by Decimal object");

    // 与Int相减
    if (auto another_int = dynamic_cast<Int*>(args->val[0])) {
        dep::Decimal res = self_dec->val - another_int->val;
        return new Decimal(res);
    }
    // 与Decimal相减
    else if (auto another_dec = dynamic_cast<Decimal*>(args->val[0])) {
        dep::Decimal res = self_dec->val - another_dec->val;
        return new Decimal(res);
    }
    // 仅允许Int/Decimal
    assert(false && "function Decimal.sub second arg need be Int or Decimal");
}

// Decimal.__mul__：乘法（self * args[0]），支持Int/Decimal
Object* decimal_mul(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (decimal_mul)");
    assert(args->val.size() == 1 && "function Decimal.mul need 1 arg");

    const auto self_dec = dynamic_cast<Decimal*>(self);
    assert(self_dec != nullptr && "decimal_mul must be called by Decimal object");

    // 与Int相乘
    if (auto another_int = dynamic_cast<Int*>(args->val[0])) {
        dep::Decimal res = self_dec->val * another_int->val;
        return new Decimal(res);
    }
    // 与Decimal相乘
    else if (auto another_dec = dynamic_cast<Decimal*>(args->val[0])) {
        dep::Decimal res = self_dec->val * another_dec->val;
        return new Decimal(res);
    }
    // 仅允许Int/Decimal
    assert(false && "function Decimal.mul second arg need be Int or Decimal");
}

// Decimal.__div__：除法（self / args[0]），支持Int/Decimal（默认保留10位小数）
Object* decimal_div(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (decimal_div)");
    assert(args->val.size() == 1 && "function Decimal.div need 1 arg");

    const auto self_dec = dynamic_cast<Decimal*>(self);
    assert(self_dec != nullptr && "decimal_div must be called by Decimal object");

    // 除数不能为0（提前检查）
    auto check_zero = [](const dep::Decimal& val) {
        return val == dep::Decimal(dep::BigInt(0));
    };

    // 与Int相除
    if (auto another_int = dynamic_cast<Int*>(args->val[0])) {
        dep::Decimal divisor(another_int->val);
        assert(!check_zero(divisor) && "decimal_div: division by zero");
        dep::Decimal res = self_dec->val.div(divisor, 10); // 保留10位小数
        return new Decimal(res);
    }
    // 与Decimal相除
    if (auto another_dec = dynamic_cast<Decimal*>(args->val[0])) {
        assert(!check_zero(another_dec->val) && "decimal_div: division by zero");
        dep::Decimal res = self_dec->val.div(another_dec->val, 10); // 保留10位小数
        return new Decimal(res);
    }
    // 仅允许Int/Decimal
    assert(false && "function Decimal.div second arg need be Int or Decimal");
}

// Decimal.__pow__：幂运算（self ^ args[0]），仅支持Int类型的指数（非负）
Object* decimal_pow(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (decimal_pow)");
    assert(args->val.size() == 1 && "function Decimal.pow need 1 arg");

    const auto self_dec = dynamic_cast<Decimal*>(self);
    assert(self_dec != nullptr && "decimal_pow must be called by Decimal object");

    // 指数仅支持Int（非负）
    auto exp_int = dynamic_cast<Int*>(args->val[0]);
    assert(exp_int != nullptr && "function Decimal.pow second arg need be Int");
    assert(!exp_int->val.is_negative() && "decimal_pow: negative exponent not supported");

    dep::Decimal res = self_dec->val.pow(exp_int->val);
    return new Decimal(res);
}

// Decimal.__eq__：相等判断（self == args[0]），支持Int/Decimal
Object* decimal_eq(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (decimal_eq)");
    assert(args->val.size() == 1 && "function Decimal.eq need 1 arg");

    const auto self_dec = dynamic_cast<Decimal*>(self);
    assert(self_dec != nullptr && "decimal_eq must be called by Decimal object");

    // 与Int比较
    if (auto another_int = dynamic_cast<Int*>(args->val[0])) {
        dep::Decimal cmp_val(another_int->val);
        return load_bool(self_dec->val == cmp_val);
    }
    // 与Decimal比较
    else if (auto another_dec = dynamic_cast<Decimal*>(args->val[0])) {
        return load_bool(self_dec->val == another_dec->val);
    }
    // 仅允许Int/Decimal
    assert(false && "function Decimal.eq second arg need be Int or Decimal");
}

// Decimal.__lt__：小于判断（self < args[0]），支持Int/Decimal
Object* decimal_lt(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (decimal_lt)");
    assert(args->val.size() == 1 && "function Decimal.lt need 1 arg");

    const auto self_dec = dynamic_cast<Decimal*>(self);
    assert(self_dec != nullptr && "decimal_lt must be called by Decimal object");

    // 与Int比较
    if (auto another_int = dynamic_cast<Int*>(args->val[0])) {
        dep::Decimal cmp_val(another_int->val);
        return load_bool(self_dec->val < cmp_val);
    }
    // 与Decimal比较
    if (auto another_dec = dynamic_cast<Decimal*>(args->val[0])) {
        return load_bool(self_dec->val < another_dec->val);
    }
    // 仅允许Int/Decimal
    assert(false && "function Decimal.lt second arg need be Int or Decimal");
}

// Decimal.__gt__：大于判断（self > args[0]），支持Int/Decimal
Object* decimal_gt(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (decimal_gt)");
    assert(args->val.size() == 1 && "function Decimal.gt need 1 arg");

    const auto self_dec = dynamic_cast<Decimal*>(self);
    assert(self_dec != nullptr && "decimal_gt must be called by Decimal object");

    // 与Int比较
    if (auto another_int = dynamic_cast<Int*>(args->val[0])) {
        dep::Decimal cmp_val(another_int->val);
        return load_bool(self_dec->val > cmp_val);
    }
    // 与Decimal比较
    if (auto another_dec = dynamic_cast<Decimal*>(args->val[0])) {
        return load_bool(self_dec->val > another_dec->val);
    }
    // 仅允许Int/Decimal
    assert(false && "function Decimal.gt second arg need be Int or Decimal");
}

// Decimal.__neg__：取反操作(-self)
Object* decimal_neg(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (decimal_neg)");
    assert(args->val.empty() && "function Decimal.neg need 0 arg"); // 取反无参数

    // 确保调用者是Decimal对象
    auto self_dec = dynamic_cast<Decimal*>(self);
    assert(self_dec != nullptr && "decimal_neg must be called by Decimal object");

    // 对Decimal值取反（0 - self_val 或直接用重载的-运算符）
    dep::Decimal neg_val = dep::Decimal(dep::BigInt(0)) - self_dec->val;

    return new Decimal(neg_val);
}

// Decimal.__hash__
Object* decimal_hash(Object* self, const List* args) {
    const auto self_dec = dynamic_cast<Decimal*>(self);
    return new Int(self_dec->val.hash());
}


// Decimal.safe_div：除法（self / args[0]），支持Int/Decimal（保留指定位小数）
Object* decimal_safe_div(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (decimal_safe_div)");
    assert(args->val.size() == 2 && "function Decimal.safe_div need 2 args: divisor, decimal_places");

    const auto self_dec = dynamic_cast<Decimal*>(self);

    // 解析保留小数位数（转为int，避免BigInt越界）
    const auto n_obj = dynamic_cast<Int*>(args->val[1]);
    assert(n_obj != nullptr && "decimal_safe_div second arg must be Int (decimal places)");
    // 确保n是小整数（避免超出int范围）
    assert(n_obj->val <= dep::BigInt(1000) && "decimal_safe_div: decimal places too large (max 1000)");
    const int n = static_cast<int>(n_obj->val.to_unsigned_long_long()); // 现在能正确解析20→20

    dep::Decimal divisor;
    // 处理除数为Int
    if (auto another_int = dynamic_cast<Int*>(args->val[0])) {
        divisor = dep::Decimal(another_int->val);
    }
    // 处理除数为Decimal
    else if (auto another_dec = dynamic_cast<Decimal*>(args->val[0])) {
        divisor = another_dec->val;
    }
    else {
        assert(false && "function Decimal.safe_div first arg need be Int or Decimal");
    }

    // 检查除数为0
    if (divisor == dep::Decimal(dep::BigInt(0))) {
        assert(false && "decimal_safe_div: division by zero");
    }

    // 调用修复后的div方法
    dep::Decimal res = self_dec->val.div(divisor, n);
    return new Decimal(res);
}

Object* decimal_str(Object* self, const List* args) {
    const auto self_dec = dynamic_cast<Decimal*>(self);
    return create_str(self_dec->val.to_string());
}

}  // namespace model