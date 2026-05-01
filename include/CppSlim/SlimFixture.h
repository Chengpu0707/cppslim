#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "StatementExecutor.h"
#include "SlimList.h"

// ---------------------------------------------------------------------------
// Type alias: strip cv-qualifiers and reference from T
// ---------------------------------------------------------------------------
template<typename T>
using slim_clean_t = std::remove_cv_t<std::remove_reference_t<T>>;

// ---------------------------------------------------------------------------
// SlimConvert<T> — bidirectional conversion between const char* and native type
// ---------------------------------------------------------------------------
template<typename T, typename = void>
struct SlimConvert;

template<>
struct SlimConvert<int> {
    static int         from(const char* s) noexcept { return s ? std::atoi(s) : 0; }
    static std::string to(int v)                    { return std::to_string(v); }
};

template<>
struct SlimConvert<long> {
    static long        from(const char* s) noexcept { return s ? std::atol(s) : 0L; }
    static std::string to(long v)                   { return std::to_string(v); }
};

template<>
struct SlimConvert<double> {
    static double from(const char* s) noexcept { return s ? std::atof(s) : 0.0; }
    static std::string to(double v) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%g", v);
        return buf;
    }
};

template<>
struct SlimConvert<float> {
    static float from(const char* s) noexcept {
        return s ? static_cast<float>(std::atof(s)) : 0.0f;
    }
    static std::string to(float v) { return SlimConvert<double>::to(static_cast<double>(v)); }
};

template<>
struct SlimConvert<bool> {
    static bool from(const char* s) noexcept {
        return s && (std::strcmp(s, "true") == 0 ||
                     std::strcmp(s, "yes")  == 0 ||
                     std::strcmp(s, "1")    == 0);
    }
    static std::string to(bool v) { return v ? "true" : "false"; }
};

template<>
struct SlimConvert<std::string> {
    static std::string from(const char* s)      { return s ? s : ""; }
    static std::string to(std::string v)        { return v; }
};

// ---------------------------------------------------------------------------
// slimListToVector — constructor helper: SlimList* → vector<string>
// ---------------------------------------------------------------------------
inline std::vector<std::string> slimListToVector(SlimList* args) {
    std::vector<std::string> v;
    if (!args) return v;
    int n = args->getLength();
    v.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const char* s = args->getStringAt(i);
        v.push_back(s ? s : "");
    }
    return v;
}

// ---------------------------------------------------------------------------
// MemberFunctionTraits — extract ReturnType and ArgTypes from a member fn ptr
// ---------------------------------------------------------------------------
template<typename F>
struct MemberFunctionTraits;

template<typename T, typename R, typename... Args>
struct MemberFunctionTraits<R(T::*)(Args...)> {
    using ReturnType = R;
    using ArgTypes   = std::tuple<Args...>;
};

template<typename T, typename R, typename... Args>
struct MemberFunctionTraits<R(T::*)(Args...) const> {
    using ReturnType = R;
    using ArgTypes   = std::tuple<Args...>;
};

// ---------------------------------------------------------------------------
// SlimFixture<Derived> — CRTP base providing Create/Destroy and result storage
// ---------------------------------------------------------------------------
template<typename Derived>
class SlimFixture {
public:
    mutable std::string slimResult_;

    static void* create(StatementExecutor* errorHandler, SlimList* args) noexcept {
        try {
            return new Derived(slimListToVector(args));
        } catch (const std::exception& e) {
            errorHandler->constructorError(e.what());
            return nullptr;
        } catch (...) {
            errorHandler->constructorError("Unknown exception in constructor");
            return nullptr;
        }
    }

    static void destroy(void* instance) noexcept {
        delete static_cast<Derived*>(instance);
    }

    const char* storeResult(std::string s) const {
        slimResult_ = std::move(s);
        return slimResult_.c_str();
    }
};

// ---------------------------------------------------------------------------
// BoundMethod<T, MethodPtr> — bridges a pointer-to-member to the C Method ABI
// ---------------------------------------------------------------------------
template<typename T, auto MethodPtr>
struct BoundMethod {
private:
    using Traits   = MemberFunctionTraits<decltype(MethodPtr)>;
    using Ret      = typename Traits::ReturnType;
    using ArgTypes = typename Traits::ArgTypes;

    template<std::size_t... I>
    static const char* callImpl(T* self, SlimList* args, std::index_sequence<I...>) {
        if constexpr (std::is_void_v<Ret>) {
            (self->*MethodPtr)(
                SlimConvert<slim_clean_t<std::tuple_element_t<I, ArgTypes>>>::from(
                    args->getStringAt(static_cast<int>(I)))...);
            return "";
        } else {
            return static_cast<SlimFixture<T>*>(self)->storeResult(
                SlimConvert<slim_clean_t<Ret>>::to(
                    (self->*MethodPtr)(
                        SlimConvert<slim_clean_t<std::tuple_element_t<I, ArgTypes>>>::from(
                            args->getStringAt(static_cast<int>(I)))...)));
        }
    }

public:
    static const char* call(void* inst, SlimList* args) noexcept {
        T* self = static_cast<T*>(inst);
        try {
            return callImpl(self, args,
                std::make_index_sequence<std::tuple_size_v<ArgTypes>>{});
        } catch (const std::exception& e) {
            static thread_local std::string err;
            err = std::string("__EXCEPTION__:message:<<") + e.what() + ".>>";
            return err.c_str();
        } catch (...) {
            return "__EXCEPTION__:message:<<Unknown exception in fixture method.>>";
        }
    }
};
