#pragma once
#include "bigint.hpp"
#include <string>
#include <cassert>
#include <algorithm>
#include <functional>

namespace dep {

class Decimal {
    BigInt mantissa_;   // 尾数（包含符号，归一化后无末尾零）
    int exponent_;      // 指数：value = mantissa_ * 10^exponent_

    /**
     * @brief 归一化：确保尾数无末尾零，保证表示唯一性
     * 例如：1200×10^-3 → 12×10^-1；-000 → 0×10^0
     */
    void normalize() {
        if (mantissa_ == BigInt(0)) {
            exponent_ = 0;
            return;
        }
        // 移除尾数末尾的零，同步调整指数
        BigInt ten(10);
        while (mantissa_ % ten == BigInt(0)) {
            mantissa_ /= ten;
            exponent_ += 1;
        }
    }

    /**
     * @brief 修复：对齐两个Decimal的指数（核心错误修复）
     * 正确逻辑：将两个数对齐到**更小的指数**（更低的量级），避免大数被缩小导致精度丢失
     * @param a 输入Decimal
     * @param b 输入Decimal
     * @param a_mant 输出：a对齐后的尾数
     * @param b_mant 输出：b对齐后的尾数
     * @return 公共指数（两个数中更小的那个）
     */
    static int align_exponent(const Decimal& a, const Decimal& b, BigInt& a_mant, BigInt& b_mant) {
        // 找到更小的指数（公共指数）
        int common_exp = std::min(a.exponent_, b.exponent_);
        // 计算每个数需要放大的倍数：10^(原指数 - 公共指数)
        int a_scale = a.exponent_ - common_exp;
        int b_scale = b.exponent_ - common_exp;

        BigInt ten(10);
        // 放大尾数，使两者指数都等于common_exp
        a_mant = a.mantissa_ * BigInt::fast_pow_unsigned(ten, BigInt(a_scale));
        b_mant = b.mantissa_ * BigInt::fast_pow_unsigned(ten, BigInt(b_scale));

        return common_exp;
    }

    Decimal(const BigInt& mantissa, int exponent)
        : mantissa_(mantissa), exponent_(exponent) {
    }

public:
    // ========================= 构造函数 =========================
    Decimal() : mantissa_(0), exponent_(0) {}

    // 从BigInt构造（指数为0）
    explicit Decimal(const BigInt& mantissa) : mantissa_(mantissa), exponent_(0) {
        normalize();
    }

    // 修复：整数构造函数（原逻辑没问题，但补充注释）
    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    explicit Decimal(T val) : mantissa_(BigInt(static_cast<size_t>(std::abs(val)))), exponent_(0) {
        if (val < 0) {
            mantissa_ = BigInt(0) - mantissa_;
        }
        normalize();
    }

    // 从字符串构造（原逻辑正确，无需修改）
    explicit Decimal(const std::string& s) {
        std::string str = s;
        bool is_neg = false;
        size_t start = 0;

        // 处理符号
        if (str[0] == '-') {
            is_neg = true;
            start = 1;
        } else if (str[0] == '+') {
            start = 1;
        }

        // 处理指数部分（e/E）
        size_t exp_pos = str.find_first_of("eE");
        int exp = 0;
        if (exp_pos != std::string::npos) {
            exp = std::stoi(str.substr(exp_pos + 1));
            str = str.substr(0, exp_pos);
        }

        // 处理小数点
        size_t dot_pos = str.find('.', start);
        if (dot_pos == std::string::npos) {
            // 无小数点：整数
            mantissa_ = BigInt(str.substr(start));
            exponent_ = exp;
        } else {
            // 有小数点：拆分整数和小数部分
            std::string int_part = str.substr(start, dot_pos - start);
            std::string frac_part = str.substr(dot_pos + 1);

            // 拼接尾数（整数部分+小数部分）
            std::string mant_str = (int_part.empty() ? "0" : int_part) + frac_part;
            mantissa_ = BigInt(mant_str);
            // 指数 = 输入指数 - 小数部分长度
            exponent_ = exp - static_cast<int>(frac_part.size());
        }

        // 应用符号
        if (is_neg) {
            mantissa_ = BigInt(0) - mantissa_;
        }

        normalize();
    }

    // 移动/拷贝构造（默认）
    Decimal(const Decimal& other) = default;
    Decimal(Decimal&& other) noexcept = default;
    Decimal& operator=(const Decimal& other) = default;
    Decimal& operator=(Decimal&& other) noexcept = default;
    ~Decimal() = default;

    // ========================= 基础方法 =========================
    /// 取绝对值
    [[nodiscard]] Decimal abs() const {
        Decimal res = *this;
        res.mantissa_ = res.mantissa_.abs();
        return res;
    }

    /// 取整数部分（截断小数部分）
    [[nodiscard]] BigInt integer_part() const {
        if (exponent_ >= 0) {
            // 指数非负：尾数 × 10^exponent
            return mantissa_ * BigInt::fast_pow_unsigned(BigInt(10), BigInt(exponent_));
        } else {
            // 指数为负：尾数 ÷ 10^(-exponent)
            BigInt ten_pow = BigInt::fast_pow_unsigned(BigInt(10), BigInt(-exponent_));
            return mantissa_ / ten_pow;
        }
    }

    /// 修复：to_string() 方法（小数点位置计算错误）
    [[nodiscard]] std::string to_string() const {
        if (mantissa_ == BigInt(0)) {
            return "0";
        }

        std::string mant_str = mantissa_.abs().to_string();
        bool is_neg = mantissa_.is_negative();
        int exp = exponent_;

        // 修复核心：正确计算小数点位置
        // 逻辑：数值 = mant_str × 10^exp → 等价于 mant_str 的小数点向左（exp负）/右（exp正）移|exp|位
        std::string res;
        if (exp >= 0) {
            // 指数非负：尾数后补exp个零（整数）
            res = mant_str + std::string(exp, '0');
        } else {
            // 指数为负：需要插入小数点
            int abs_exp = std::abs(exp);
            if (abs_exp >= static_cast<int>(mant_str.size())) {
                // 小数点在最前面，补零：0.000...mant_str
                res = "0." + std::string(abs_exp - mant_str.size(), '0') + mant_str;
            } else {
                // 小数点在中间：拆分整数和小数部分
                res = mant_str.substr(0, mant_str.size() - abs_exp) + "." + mant_str.substr(mant_str.size() - abs_exp);
            }
        }

        // 移除末尾的零和多余的小数点
        if (res.find('.') != std::string::npos) {
            res.erase(res.find_last_not_of('0') + 1, std::string::npos);
            if (res.back() == '.') {
                res.pop_back();
            }
        }

        // 应用符号
        if (is_neg) {
            res = "-" + res;
        }

        return res;
    }

    /// 哈希函数
    [[nodiscard]] BigInt hash() const {
        size_t mant_hash = std::hash<std::string>()(mantissa_.to_string());
        size_t exp_hash = std::hash<int>()(exponent_);

        BigInt mant_big(mant_hash);
        BigInt exp_big(exp_hash);
        BigInt shift_base(1);
        shift_base = shift_base.pow(BigInt(64));

        return mant_big * shift_base + exp_big;
    }

    // ========================= 比较运算符（逻辑正确，无需修改） =========================
    bool operator==(const Decimal& other) const {
        if (exponent_ != other.exponent_) {
            BigInt a_mant, b_mant;
            align_exponent(*this, other, a_mant, b_mant);
            return a_mant == b_mant;
        }
        return mantissa_ == other.mantissa_;
    }

    bool operator!=(const Decimal& other) const {
        return !(*this == other);
    }

    bool operator<(const Decimal& other) const {
        BigInt a_mant, b_mant;
        align_exponent(*this, other, a_mant, b_mant);
        return a_mant < b_mant;
    }

    bool operator>(const Decimal& other) const {
        return other < *this;
    }

    bool operator<=(const Decimal& other) const {
        return *this < other || *this == other;
    }

    bool operator>=(const Decimal& other) const {
        return *this > other || *this == other;
    }

    // ========================= 算术运算符（保持原有） =========================
    Decimal operator+(const Decimal& other) const {
        BigInt a_mant, b_mant;
        int exp = align_exponent(*this, other, a_mant, b_mant);
        BigInt sum_mant = a_mant + b_mant;
        Decimal res;  // 使用默认构造函数
        res.mantissa_ = sum_mant;
        res.exponent_ = exp;
        res.normalize();  // 只在最后 normalize 一次
        return res;
    }

    Decimal operator-(const Decimal& other) const {
        BigInt a_mant, b_mant;
        int exp = align_exponent(*this, other, a_mant, b_mant);
        BigInt sub_mant = a_mant - b_mant;
        Decimal res;
        res.mantissa_ = sub_mant;
        res.exponent_ = exp;
        res.normalize();
        return res;
    }

    // pow 方法也需要修复
    [[nodiscard]] Decimal pow(const BigInt& exp) const {
        assert(!exp.is_negative() && "Decimal pow: negative exponent not supported");
        if (exp == BigInt(0)) {
            return Decimal(BigInt(1));
        }
        BigInt mant_pow = BigInt::fast_pow_unsigned(mantissa_.abs(), exp);
        if (mantissa_.is_negative() && (exp % BigInt(2) == BigInt(1))) {
            mant_pow = BigInt(0) - mant_pow;
        }
        Decimal res;
        res.mantissa_ = mant_pow;
        // 修复：指数应该乘以 exp，而不是使用 static_cast<int>
        // 需要将 BigInt 转换为 int，但要注意溢出
        BigInt exp_int = exp;
        res.exponent_ = exponent_ * static_cast<int>(exp_int.to_unsigned_long_long());
        res.normalize();
        return res;
    }

    Decimal operator*(const Decimal& other) const {
        BigInt mul_mant = mantissa_ * other.mantissa_;
        Decimal res(mul_mant);
        res.exponent_ = exponent_ + other.exponent_;
        res.normalize();
        return res;
    }

    Decimal operator/(const Decimal& other) const {
        return this->div(other, 10); // 默认保留10位小数
    }

    [[nodiscard]] Decimal div(const Decimal& other, int n=10) const { // n改为int类型
        if (other.mantissa_ == BigInt(0)) {
            throw KizStopRunningSignal();
        }
        assert(n >= 0 && "n must be non-negative");

        BigInt a_mant, b_mant;
        // 对齐指数（抵消a/b的指数影响）
        align_exponent(*this, other, a_mant, b_mant);

        BigInt ten(10);
        // 补零扩展被除数：乘以 10^n，用于保留n位小数
        BigInt scale = BigInt::fast_pow_unsigned(ten, BigInt(n));
        BigInt dividend = a_mant * scale;

        // 整数除法（截断余数）
        BigInt quotient = dividend / b_mant;

        // 修复核心：先创建空对象，避免构造函数提前normalize
        Decimal res; // 空构造：mantissa_=0，exponent_=0
        res.mantissa_ = quotient; // 原始商（如5000000000），未被normalize
        res.exponent_ = -n; // 设置指数为 -n（如-10）
        res.normalize(); // 此时normalize会正确简化：5000000000 → 5，exponent_-10→-1

        return res;
    }

    // ========================= 算术运算符（Decimal与BigInt） =========================
    Decimal operator+(const BigInt& other) const {
        return *this + Decimal(other);
    }

    Decimal operator-(const BigInt& other) const {
        return *this - Decimal(other);
    }

    Decimal operator*(const BigInt& other) const {
        return *this * Decimal(other);
    }

    Decimal operator/(const BigInt& other) const {
        return *this / Decimal(other);
    }

    // ========================= 赋值运算符 =========================
    Decimal& operator+=(const Decimal& other) {
        *this = *this + other;
        return *this;
    }

    Decimal& operator-=(const Decimal& other) {
        *this = *this - other;
        return *this;
    }

    Decimal& operator*=(const Decimal& other) {
        *this = *this * other;
        return *this;
    }

    Decimal& operator/=(const Decimal& other) {
        *this = *this / other;
        return *this;
    }

    Decimal& operator+=(const BigInt& other) {
        return *this += Decimal(other);
    }

    Decimal& operator-=(const BigInt& other) {
        return *this -= Decimal(other);
    }

    Decimal& operator*=(const BigInt& other) {
        return *this *= Decimal(other);
    }

    Decimal& operator/=(const BigInt& other) {
        return *this /= Decimal(other);
    }
};

// ========================= 全局函数（BigInt与Decimal运算） =========================
inline Decimal operator+(const BigInt& lhs, const Decimal& rhs) {
    return rhs + lhs;
}

inline Decimal operator-(const BigInt& lhs, const Decimal& rhs) {
    return Decimal(lhs) - rhs;
}

inline Decimal operator*(const BigInt& lhs, const Decimal& rhs) {
    return rhs * lhs;
}

inline Decimal operator/(const BigInt& lhs, const Decimal& rhs) {
    return Decimal(lhs) / rhs;
}

} // namespace dep