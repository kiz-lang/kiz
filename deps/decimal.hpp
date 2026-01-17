#pragma once
#include "bigint.hpp"
#include <string>
#include <cassert>
#include <algorithm>
#include <functional>

namespace dep {

class Decimal {
private:
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
     * @brief 对齐两个Decimal的指数，返回对齐后的公共指数
     * @param a 输入Decimal
     * @param b 输入Decimal
     * @param a_mant 输出：a对齐后的尾数
     * @param b_mant 输出：b对齐后的尾数
     * @return 公共指数
     */
    static int align_exponent(const Decimal& a, const Decimal& b, BigInt& a_mant, BigInt& b_mant) {
        int exp_diff = a.exponent_ - b.exponent_;
        if (exp_diff == 0) {
            a_mant = a.mantissa_;
            b_mant = b.mantissa_;
            return a.exponent_;
        } else if (exp_diff > 0) {
            // a的指数更大，b的尾数需要乘以10^exp_diff
            a_mant = a.mantissa_;
            b_mant = b.mantissa_ * BigInt::fast_pow_unsigned(BigInt(10), BigInt(exp_diff));
            return a.exponent_;
        } else {
            // b的指数更大，a的尾数需要乘以10^(-exp_diff)
            a_mant = a.mantissa_ * BigInt::fast_pow_unsigned(BigInt(10), BigInt(-exp_diff));
            b_mant = b.mantissa_;
            return b.exponent_;
        }
    }

public:
    // ========================= 构造函数 =========================
    Decimal() : mantissa_(0), exponent_(0) {}

    // 从BigInt构造（指数为0）
    explicit Decimal(const BigInt& mantissa) : mantissa_(mantissa), exponent_(0) {
        normalize();
    }

    // 从整数构造
    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    explicit Decimal(T val) : mantissa_(BigInt(static_cast<size_t>(std::abs(val)))), exponent_(0) {
        if (val < 0) {
            mantissa_ = -mantissa_;
        }
        normalize();
    }

    // 从字符串构造（支持 "123", "-45.678", "0.001", "123e-2" 等格式）
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
            mantissa_ = -mantissa_;
        }

        normalize();
    }

    // 移动/拷贝构造
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

    /// 转换为字符串
    [[nodiscard]] std::string to_string() const {
        if (mantissa_ == BigInt(0)) {
            return "0";
        }

        std::string mant_str = mantissa_.abs().to_string();
        bool is_neg = mantissa_.is_negative();
        int total_exp = exponent_;

        // 计算小数点位置：pos = 字符串长度 + total_exp
        int dot_pos = static_cast<int>(mant_str.size()) + total_exp;
        std::string res;

        if (dot_pos <= 0) {
            // 小于1的小数：0.00...mant_str
            res = "0." + std::string(-dot_pos, '0') + mant_str;
        } else if (dot_pos >= static_cast<int>(mant_str.size())) {
            // 整数或带末尾零的数：mant_str + 00...
            res = mant_str + std::string(dot_pos - static_cast<int>(mant_str.size()), '0');
        } else {
            // 带小数的数：拆分整数和小数部分
            res = mant_str.substr(0, dot_pos) + "." + mant_str.substr(dot_pos);
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

    /// 哈希函数（用于STL容器）
    [[nodiscard]] size_t hash() const {
        size_t mant_hash = std::hash<std::string>()(mantissa_.to_string());
        size_t exp_hash = std::hash<int>()(exponent_);
        return mant_hash ^ (exp_hash << 1);
    }

    // ========================= 比较运算符 =========================
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

    // ========================= 算术运算符（Decimal与Decimal） =========================
    /// 加法
    Decimal operator+(const Decimal& other) const {
        BigInt a_mant, b_mant;
        int exp = align_exponent(*this, other, a_mant, b_mant);
        BigInt sum_mant = a_mant + b_mant;
        Decimal res(sum_mant);
        res.exponent_ = exp;
        res.normalize();
        return res;
    }

    /// 减法
    Decimal operator-(const Decimal& other) const {
        BigInt a_mant, b_mant;
        int exp = align_exponent(*this, other, a_mant, b_mant);
        BigInt sub_mant = a_mant - b_mant;
        Decimal res(sub_mant);
        res.exponent_ = exp;
        res.normalize();
        return res;
    }

    /// 乘法
    Decimal operator*(const Decimal& other) const {
        BigInt mul_mant = mantissa_ * other.mantissa_;
        Decimal res(mul_mant);
        res.exponent_ = exponent_ + other.exponent_;
        res.normalize();
        return res;
    }

    /// 除法（无限精度，若除不尽则尾数会无限长，此处使用BigInt除法保证精度）
    Decimal operator/(const Decimal& other) const {
        assert(!(other.mantissa_ == BigInt(0)) && "Decimal division by zero");
        BigInt div_mant = mantissa_ * BigInt::fast_pow_unsigned(BigInt(10), BigInt(-other.exponent_));
        div_mant = div_mant / other.mantissa_;
        Decimal res(div_mant);
        res.exponent_ = exponent_ + other.exponent_; // 注意：这里是 +，因为分母的指数是负的
        res.normalize();
        return res;
    }

    /// 乘方（指数为BigInt，仅支持整数次幂）
    Decimal pow(const BigInt& exp) const {
        assert(!exp.is_negative() && "Decimal pow: negative exponent not supported");
        if (exp == BigInt(0)) {
            return Decimal(BigInt(1));
        }
        BigInt mant_pow = BigInt::fast_pow_unsigned(mantissa_.abs(), exp);
        if (mantissa_.is_negative() && (exp % BigInt(2) == BigInt(1))) {
            mant_pow = -mant_pow;
        }
        Decimal res(mant_pow);
        res.exponent_ = exponent_ * static_cast<int>(exp.to_unsigned_long_long());
        res.normalize();
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