/*
* pair��ʵ��
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
		_T1 first;		// ע�⣬���� public
		_T2 second;		// ע�⣬���� public
		pair() : first(_T1()), second(_T2()) {}
		pair(const _T1& a, const _T2& b) : first(a), second(b) {}
		// ��pair A Ϊpair B�ĳ�ֵ��A, B �Ķ�ӦԪ���ͱ���Բ�ͬ��ֻҪ��ת���ͺá�
		template <class U1, class U2>
		pair(const pair<U1, U2>& p) : first(p.first), second(p.second) {}
	};

	template <class _T1, class _T2>
	inline bool operator==(const pair<_T1, _T2>& x, const pair<_T1, _T2>& y)
	{
		return x.first == y.first && x.second == y.second;
		// ��һԪ�غ͵ڶ�Ԫ�ض���ȣ�����Ϊ��ȡ�
	}

	template <class _T1, class _T2>
	inline bool operator<(const pair<_T1, _T2>& x, const pair<_T1, _T2>& y)
	{
		return x.first < y.first || (!(y.first < x.first) && x.second < y.second);
		// x�ĵ�һԪ��С��y�ĵ�һԪ�أ���x�ĵ�һԪ�ز�����y�ĵ�һԪ�ض�
		// x�ĵڶ�Ԫ��С��y�ĵڶ�Ԫ�أ�����ΪxС��y��
	}

	// ����������ֵ�������һ��pair
	template <class _T1, class _T2>
	inline pair<_T1, _T2> make_pair(const _T1& x, const _T2& y)
	{
		return pair<_T1, _T2>(x, y);
	}
}//end of MyStl
#endif // end of PAIR_H_