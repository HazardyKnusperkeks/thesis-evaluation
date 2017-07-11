#ifndef COMMON_HPP
#define COMMON_HPP

#include <experimental/string_view>
#include <iomanip>
#include <tuple>

#if __has_include(<string_view>)
#include <string_view>
#else
namespace std {
	using experimental::string_view;
}
#endif

constexpr int stringViewToInt(const std::string_view& view) noexcept {
	int ret = 0;
	int mul = 1;
	
	static_assert('1' - '0' == 1, "");
	
	for ( const auto& zeichen : view ) {
		if ( zeichen == '-' ) {
			mul = -1;
		} //if ( zeichen == '-' )
		else {
			ret *= 10;
			ret += zeichen - '0';
		} //else -> if ( zeichen == '-' )
	} //for ( const auto& zeichen : view )
	
	return ret * mul;
}

namespace std {
	/**
	 * @brief Helper class to instantiate a std::unordered_map with a std::pair as key.
	 */
	template<typename T1, typename T2>
	struct hash<pair<T1, T2>>
	{
		auto operator()(const pair<T1, T2>& pair) const
			noexcept(noexcept(hash<T1>{}(pair.first) && noexcept(hash<T2>{}(pair.second))))
		{
			//Is this a good hash?
			return hash<T1>{}(pair.first) << 16 ^ hash<T2>{}(pair.second);
		}
	};
}

using Zeit = std::tuple<int, int, int, int>;

inline static Zeit zeitAusString(const std::string_view& view) {
	//time string: "10:14:26.518242"
	return std::make_tuple(stringViewToInt(view.substr(0, 2)), stringViewToInt(view.substr(3, 2)),
	                       stringViewToInt(view.substr(6, 2)), stringViewToInt(view.substr(9, 6)));
}

#endif
