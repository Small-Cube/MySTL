/*
* pair的实现
*/

#ifndef PAIR_H_
#define PAIR_H_

namespace MySTL
{
	template <class _T1, class _T2>
	struct pair
	{
		typedef _T1 first_type;
		typedef _T2 second_type;
		_T1 first;		// 注意，它是 public
		_T2 second;		// 注意，它是 public
		pair() : first(_T1()), second(_T2()) {}
		pair(const _T1& a, const _T2& b) : first(a), second(b) {}
		// 以pair A 为pair B的初值。A, B 的对应元素型别可以不同，只要能转换就好。
		template <class U1, class U2>
		pair(const pair<U1, U2>& p) : first(p.first), second(p.second) {}
	};

	template <class _T1, class _T2>
	inline bool operator==(const pair<_T1, _T2>& x, const pair<_T1, _T2>& y)
	{
		return x.first == y.first && x.second == y.second;
		// 第一元素和第二元素都相等，才视为相等。
	}

	template <class _T1, class _T2>
	inline bool operator<(const pair<_T1, _T2>& x, const pair<_T1, _T2>& y)
	{
		return x.first < y.first || (!(y.first < x.first) && x.second < y.second);
		// x的第一元素小于y的第一元素，或，x的第一元素不大于y的第一元素而
		// x的第二元素小于y的第二元素，才视为x小于y。
	}

	// 根据两个数值，制造出一个pair
	template <class _T1, class _T2>
	inline pair<_T1, _T2> make_pair(const _T1& x, const _T2& y)
	{
		return pair<_T1, _T2>(x, y);
	}
}//end of MyStl
#endif // end of PAIR_H_