#include "../../src/models/models.hpp"
#include "../../src/vm/vm.hpp"
#include "include/builtin_functions.hpp"

namespace model {

// Int.__call__
Object* int_call(Object* self, const List* args) {
    auto a = builtin::get_one_arg(args);
    dep::BigInt val(0);
    if (auto s = dynamic_cast<String*>(a)) val = dep::BigInt(s->val);
    else if (!kiz::Vm::is_true(a)) val = dep::BigInt(0);
    return new Int(val);
}

// Int.__bool__
Object* int_bool(Object* self, const List* args) {
    const auto self_int = dynamic_cast<Int*>(self);
    if (self_int->val == dep::BigInt(0)) return load_false();
    return load_true();
}

// Int.__add__ 整数加法：self + args[0]（仅支持Int/Decimal）
Object* int_add(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments");
    assert(args->val.size() == 1 && "function Int.add need 1 arg");

    const auto self_int = dynamic_cast<Int*>(self);
    assert(self_int!=nullptr && "function Int.add need 1 arg typed Int");

    // 与Int相加
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return new Int(self_int->val + another_int->val);
    }
    // 与Decimal相加（返回Decimal）
    auto another_dec = dynamic_cast<Decimal*>(args->val[0]);
    if (another_dec) {
        dep::Decimal left_dec(self_int->val);
        return new Decimal(left_dec + another_dec->val);
    }
    // 仅允许Int/Decimal
    assert(false && "function Int.add second arg need be Int or Decimal");
};

// Int.__sub__ 整数减法：self - args[0]（仅支持Int/Decimal）
Object* int_sub(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_sub)");
    assert(args->val.size() == 1 && "function Int.sub need 1 arg");

    auto self_int = dynamic_cast<Int*>(self);
    // 与Int相减
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return new Int(self_int->val - another_int->val);
    }
    // 与Decimal相减（返回Decimal）
    auto another_dec = dynamic_cast<Decimal*>(args->val[0]);
    if (another_dec) {
        dep::Decimal left_dec(self_int->val);
        return new Decimal(left_dec - another_dec->val);
    }
    // 仅允许Int/Decimal
    assert(false && "function Int.sub second arg need be Int or Decimal");
};

// Int.__mul__ 整数乘法：self * args[0]（仅支持Int/Decimal）
Object* int_mul(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_mul)");
    assert(args->val.size() == 1 && "function Int.mul need 1 arg");

    auto self_int = dynamic_cast<Int*>(self);
    // 与Int相乘
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return new Int(self_int->val * another_int->val);
    }
    // 与Decimal相乘（返回Decimal）
    auto another_dec = dynamic_cast<Decimal*>(args->val[0]);
    if (another_dec) {
        dep::Decimal left_dec(self_int->val);
        return new Decimal(left_dec * another_dec->val);
    }
    // 仅允许Int/Decimal
    assert(false && "function Int.mul second arg need be Int or Decimal");
};

// Int.__neg__ 取反
Object* int_neg(Object* self, const List* args) {
    auto self_int = dynamic_cast<Int*>(self);
    assert(self_int!=nullptr);
    auto new_int = dep::BigInt(0) - self_int->val;
    return new Int(new_int);
}

// Int.__div__ 整数除法 self / args[0]（仅支持Int/Decimal，返回Decimal）
Object* int_div(Object* self, const List* args) {
    kiz::Vm::assert_argc(1, args);

    auto self_int = dynamic_cast<Int*>(self);
    // 与Int相除（返回Decimal，保留10位小数）
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        if (another_int->val == 0) throw NativeFuncError("CalculateError", "divisor cannot be zero");
        dep::Decimal left_dec(self_int->val);
        dep::Decimal right_dec(another_int->val);
        return new Decimal(left_dec.div(right_dec, 10));
    }
    // 与Decimal相除（返回Decimal）
    auto another_dec = dynamic_cast<Decimal*>(args->val[0]);
    if (another_dec) {
        if(another_dec->val == dep::Decimal(0)) throw NativeFuncError("CalculateError", "divisor cannot be zero");
        dep::Decimal left_dec(self_int->val);
        return new Decimal(left_dec.div(another_dec->val, 10));
    }
    // 仅允许Int/Decimal
    assert(false && "function Int.div second arg need be Int or Decimal");
};

// Int.__pow__ 整数幂运算：self ^ args[0]（self的args[0]次方，仅支持Int指数）
Object* int_pow(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_pow)");
    assert(args->val.size() == 1 && "function Int.pow need 1 arg");

    auto self_int = dynamic_cast<Int*>(self);
    auto exp_int = dynamic_cast<Int*>(args->val[0]);
    assert(exp_int != nullptr && "function Int.pow second arg need be Int");

    // 指数非负时返回Int，负指数返回Decimal（扩展支持）
    if (exp_int->val.is_negative()) {
        dep::Decimal base_dec(self_int->val);
        dep::BigInt abs_exp = exp_int->val.abs();
        dep::Decimal res = base_dec.pow(abs_exp);
        // 负指数：1 / res
        dep::Decimal one(1);
        return new Decimal(one.div(res, 10));
    } else {
        return new Int(self_int->val.pow(exp_int->val));
    }
};

// Int.__mod__ 整数取模：self % args[0]（仅支持Int）
Object* int_mod(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_mod)");
    assert(args->val.size() == 1 && "function Int.mod need 1 arg");

    auto another_int = dynamic_cast<Int*>(args->val[0]);
    assert(another_int != nullptr && "function Int.mod second arg need be Int");
    assert(another_int->val != dep::BigInt(0) && "mod by zero");

    auto self_int = dynamic_cast<Int*>(self);
    dep::BigInt remainder = self_int->val % another_int->val;
    // 修正余数符号（确保与除数同号）
    if (remainder != dep::BigInt(0)
        and self_int->val < dep::BigInt(0) != another_int->val < dep::BigInt(0)
    ) {
        remainder += another_int->val;
    }
    return new Int(dep::BigInt(remainder));
};

// Int.__eq__ 相等判断：self == args[0]（仅支持Int/Decimal）
Object* int_eq(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_eq)");
    assert(args->val.size() == 1 && "function Int.eq need 1 arg");

    auto self_int = dynamic_cast<Int*>(self);
    // 与Int比较
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return load_bool(self_int->val == another_int->val);
    }
    // 与Decimal比较
    auto another_dec = dynamic_cast<Decimal*>(args->val[0]);
    if (another_dec) {
        dep::Decimal cmp_val(self_int->val);
        return load_bool(cmp_val == another_dec->val);
    }
    // 仅允许Int/Decimal
    assert(false && "function Int.eq second arg need be Int or Decimal");
};

// Int.__lt__ 小于判断：self < args[0]（仅支持Int/Decimal）
Object* int_lt(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_lt)");
    assert(args->val.size() == 1 && "function Int.lt need 1 arg");

    auto self_int = dynamic_cast<Int*>(self);
    // 与Int比较
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return load_bool(self_int->val < another_int->val);
    }
    // 与Decimal比较
    auto another_dec = dynamic_cast<Decimal*>(args->val[0]);
    if (another_dec) {
        dep::Decimal cmp_val(self_int->val);
        return load_bool(cmp_val < another_dec->val);
    }
    // 仅允许Int/Decimal
    assert(false && "function Int.lt second arg need be Int or Decimal");
};

// Int.__gt__ 大于判断：self > args[0]（仅支持Int/Decimal）
Object* int_gt(Object* self, const List* args) {
    DEBUG_OUTPUT("You given " + std::to_string(args->val.size()) + " arguments (int_gt)");
    assert(args->val.size() == 1 && "function Int.gt need 1 arg");

    auto self_int = dynamic_cast<Int*>(self);
    // 与Int比较
    auto another_int = dynamic_cast<Int*>(args->val[0]);
    if (another_int) {
        return load_bool(self_int->val > another_int->val);
    }
    // 与Decimal比较
    auto another_dec = dynamic_cast<Decimal*>(args->val[0]);
    if (another_dec) {
        dep::Decimal cmp_val(self_int->val);
        return load_bool(cmp_val > another_dec->val);
    }
    // 仅允许Int/Decimal
    assert(false && "function Int.gt second arg need be Int or Decimal");
};

// Int.__hash__
Object* int_hash(Object* self, const List* args) {
    auto self_int = dynamic_cast<Int*>(self);
    return create_int(self_int->val);
}

Object* int_str(Object* self, const List* args) {
    auto self_int = dynamic_cast<Int*>(self);
    return create_str(self_int->val.to_string());
}


}  // namespace model