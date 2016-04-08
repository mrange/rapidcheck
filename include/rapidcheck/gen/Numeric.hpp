#pragma once

#include "rapidcheck/detail/BitStream.h"
#include "rapidcheck/shrinkable/Create.h"
#include "rapidcheck/shrink/Shrink.h"
#include "rapidcheck/gen/Create.h"
#include "rapidcheck/gen/Exec.h"
#include "rapidcheck/gen/Select.h"
#include "rapidcheck/gen/Transform.h"
#include "rapidcheck/gen/detail/ScaleInteger.h"

namespace rc {
namespace gen {
namespace detail {

template <typename T>
Shrinkable<T> integral(const Random &random, int size) {
  return shrinkable::shrinkRecur(
      rc::detail::bitStreamOf(random).nextWithSize<T>(size),
      &shrink::integral<T>);
}

extern template Shrinkable<char> integral<char>(const Random &random, int size);
extern template Shrinkable<unsigned char>
integral<unsigned char>(const Random &random, int size);
extern template Shrinkable<short> integral<short>(const Random &random,
                                                  int size);
extern template Shrinkable<unsigned short>
integral<unsigned short>(const Random &random, int size);
extern template Shrinkable<int> integral<int>(const Random &random, int size);
extern template Shrinkable<unsigned int>
integral<unsigned int>(const Random &random, int size);
extern template Shrinkable<long> integral<long>(const Random &random, int size);
extern template Shrinkable<unsigned long>
integral<unsigned long>(const Random &random, int size);
extern template Shrinkable<long long> integral<long long>(const Random &random,
                                                          int size);
extern template Shrinkable<unsigned long long>
integral<unsigned long long>(const Random &random, int size);

Shrinkable<bool> boolean(const Random &random, int size);

template <typename T>
struct DefaultArbitrary {
  // If you ended up here, it means that RapidCheck wanted to generate an
  // arbitrary value of some type but you haven't declared a specialization of
  // Arbitrary for your type. Check the template stack trace to see which type
  // it is.
  static_assert(std::is_integral<T>::value,
                "No Arbitrary specialization for type T");

  static Gen<T> arbitrary() { return integral<T>; }
};

template<typename T>
constexpr auto genFloat () {
  return weightedOneOf<T> ({
    std::make_pair(24 , exec([](int a, int b, int c) -> T { return a + static_cast<T>(b) / (std::abs (static_cast<T>(c)) + 1); })),
    std::make_pair(1  , just(-std::numeric_limits<T>::quiet_NaN())),
//      std::make_pair(1  , just(-std::numeric_limits<T>::signaling_NaN())),
    std::make_pair(1  , just(-std::numeric_limits<T>::infinity())),
    std::make_pair(1  , just(std::numeric_limits<T>::infinity())),
    std::make_pair(1  , just(std::numeric_limits<T>::min())),
    std::make_pair(1  , just(static_cast<T>(-0.0))),  // In IEEE floats there's a negative 0 value
    std::make_pair(1  , just(std::numeric_limits<T>::epsilon())),
    std::make_pair(1  , just(static_cast<T>(0.0))),
    std::make_pair(1  , just(std::numeric_limits<T>::max())),
    });
}

template <>
struct DefaultArbitrary<float> {
  static Gen<float> arbitrary() { return genFloat<float>(); }
};

template <>
struct DefaultArbitrary<double> {
  static Gen<double> arbitrary() { return genFloat<double>(); }
};

template <>
struct DefaultArbitrary<long double> {
  static Gen<long double> arbitrary() { return genFloat<long double>(); }
};

template <>
struct DefaultArbitrary<bool> {
  static Gen<bool> arbitrary() { return boolean; }
};

} // namespace detail

template <typename T>
Gen<T> inRange(T min, T max) {
  return [=](const Random &random, int size) {
    if (max <= min) {
      std::string msg;
      msg += "Invalid range [" + std::to_string(min);
      msg += ", " + std::to_string(max) + ")";
      throw GenerationFailure(msg);
    }

    const auto rangeSize =
        detail::scaleInteger(static_cast<Random::Number>(max) -
                                 static_cast<Random::Number>(min) - 1,
                             size) +
        1;
    const auto value =
        static_cast<T>((Random(random).next() % rangeSize) + min);
    assert(value >= min && value < max);
    return shrinkable::shrinkRecur(
        value, [=](T x) { return shrink::towards<T>(x, min); });
  };
}

} // namespace gen
} // namespace rc
