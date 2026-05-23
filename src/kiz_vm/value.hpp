#pragma once

struct Object;

enum class ValueKind: uint8_t {
    Int, Decimal, ObjectPtr, 
    FunctionPtr, String,
};

struct Value {
    ValueKind tag;
    uint64_t data;

    static auto from_int(uint64_t val) noexcept -> Value {
        return {ValueKind::Int, val};
    }
    static auto from_decimal(double val) noexcept -> Value {
        Value res{};
        res.tag = ValueKind::Decimal;
        *reinterpret_cast<double*>(&res.data) = val;
        return res;
    }
    static auto from_object(Object* obj) noexcept -> Value {
        return {ValueKind::ObjectPtr, reinterpret_cast<uint64_t>(obj)};
    }
    static auto from_func(uint64_t addr) noexcept -> Value {
        return {ValueKind::FunctionPtr, addr};
    }
    static auto from_string(const char* str) noexcept -> Value {
        return {ValueKind::String, reinterpret_cast<uint64_t>(str)};
    }

    auto is_int() const noexcept -> bool {
        return tag == ValueKind::Int;
    }
    auto is_decimal() const noexcept -> bool {
        return tag == ValueKind::Decimal;
    }
    auto is_objectptr() const noexcept -> bool {
        return tag == ValueKind::ObjectPtr;
    }
    auto is_functionptr() const noexcept -> bool {
        return tag == ValueKind::FunctionPtr;
    }
    auto is_string() const noexcept -> bool {
        return tag == ValueKind::String;
    }

    auto as_int() const -> uint64_t {
        assert(is_int());
        return data;
    }
    auto as_decimal() const -> double {
        $Assert(is_decimal());
        return *reinterpret_cast<const double*>(&data);
    }
    auto as_objectptr() const -> Ptr<Object> {
        $Assert(is_objectptr());
        return Ptr(reinterpret_cast<Object*>(data));
    }
    auto as_functionptr() const -> uint64_t {
        $Assert(is_functionptr());
        return data;
    }
    auto as_string() const -> Str {
        $Assert(is_string());
        return Str(reinterpret_cast<char*>(data));
    }
}