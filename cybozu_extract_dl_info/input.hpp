#pragma once
#include <iostream>
#include <exception>
#include <stdexcept>
#include <type_traits>
#include <limits>
/**
@brief  \~japanese 複数条件のあるTMP用if
\~english multi-conditional if struct for TMP.
\~japanese  std::enable_ifと組み合わせて使います。
\~english   This class is used in conjunction with std::enable_if.
\~
@code
template<typename T>
using bar = first_enabled_t<
std::enbale_if<cond1, type1>,
std::enbale_if<cond2, type2>,
std::enbale_if<cond3, type3>,
default_type
>;
@endcode
*/
template<typename ...Args>
struct first_enabled {};

template<typename T, typename ...Args>
struct first_enabled<std::enable_if<true, T>, Args...> { using type = T; };
template<typename T, typename ...Args>
struct first_enabled<std::enable_if<false, T>, Args...> : first_enabled<Args...> {};
template<typename T, typename ...Args>
struct first_enabled<T, Args...> { using type = T; };

template<typename ...Args>
using first_enabled_t = typename first_enabled<Args...>::type;

//! for int8_t/uint8_t
template<typename T, std::enable_if_t<std::is_arithmetic<T>::value, std::nullptr_t> = nullptr>
using arithmetic_t = first_enabled_t <
	std::enable_if<1 != sizeof(T), T>,
	std::enable_if<std::is_signed<T>::value, int>,
	unsigned int
>;
template<typename T_> using limit = std::numeric_limits<T_>;//create new type. C++11:alias declaration
/**
* @brief 標準入力から入力を受ける
* @details [long description]
*
* @param echo_str 入力を受ける前に表示する文字列。表示しない場合はnullptrか空白文字のみで構成された文字列へのポインタを渡す
* @param max 入力値を制限する。最大値を指定
* @param min 入力値を制限する。最小値を指定
* @return 入力した数字
* @exception none
*/
template<typename T, std::enable_if_t<std::is_arithmetic<T>::value, std::nullptr_t> = nullptr>//Tが整数か浮動小数点型でないならばコンパイルエラーを出す
inline T input(const char* echo_str, const T max = limit<T>::max(), const T min = limit<T>::lowest()) noexcept {
	arithmetic_t<T> buf;
	try {
		std::cin.exceptions(std::ios::failbit | std::ios::badbit);
		if (nullptr != echo_str && '\0' != echo_str[0]) std::cout << echo_str << std::endl;//文字列が空じゃなければ出力
		std::cin >> buf;//入力を受ける
		if (max < buf || buf < min) throw std::out_of_range("input is iligal");//範囲チェック
	}
	catch (std::exception& er) {
		std::cerr << er.what() << std::endl;//エラーメッセージ表示
		return input("再入力してください。", max, min);//エラー時は再帰する方向で
	}
	return static_cast<T>(buf);
}