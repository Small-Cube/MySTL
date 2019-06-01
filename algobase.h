//基本算法，根据需要不断添加
#ifndef ALGOBASE_H_
#define ALGOBASE_H_


//#include "pair"
#include "iterator.h"
#include "type_traits.h"
#include <new>
#include <iostream>

namespace MySTL {
	//取两对象中较小者/较大者
	//相等时统一取前者
	template <class _T>
	inline const _T& min(const _T& a, const _T& b)
	{
		return b < a ? b : a;
	}
	template <class _T>
	inline const _T& max(const _T& a, const _T& b)
	{
		return  a < b ? b : a;
	}
	template <class _T, class _Compare>
	inline const _T& min(const _T& a, const _T& b, _Compare comp)
	{
		return comp(b, a) ? b : a;	// 由 comp 决定「大小比较」标准
	}
	template <class _T, class _Compare>
	inline const _T& max(const _T& a, const _T& b, _Compare comp)
	{
		return comp(a, b) ? b : a;	// 由 comp 决定「大小比较」标准
	}
	//以下是copy的实现：
	//针对input_iterator、forward_iterator、bidirectional_iterator
	//输入两个迭代器，一个起始位置一个终止位置，返回新的位置
	//_copy是_copy_dispatch的泛化
	template <class _InputIterator, class _OutputIterator>
	inline
		_OutputIterator _copy(_InputIterator first, _InputIterator last,
			_OutputIterator result, MySTL::input_iterator_tag)
	{
		//比较迭代器是否相等决定循环是否继续
		for (; first != last; ++result, ++first)
			*result = *first;
		return result;
	}
	//这个函数比上面的快，_copy_d是上面_copy的强化
	template <class _RandomAccessIterator, class _OutputIterator, class _Distance>
	inline
		_OutputIterator _copy_d(_RandomAccessIterator first, _RandomAccessIterator last,
			_OutputIterator result, _Distance*)
	{
		//以迭代器距离n决定循环是否继续执行
		for (_Distance n = last - first; n > 0; --n, ++result, ++first)
			*result = *first;
		return result;
	}

	//针对random_access_iterator随机访问迭代器
	//调用上面函数
	template <class _RandomAccessIterator, class _OutputIterator>
	inline
		_OutputIterator _copy(_RandomAccessIterator first, _RandomAccessIterator last,
			_OutputIterator result, random_access_iterator_tag)
	{
		return _copy_d(first, last, result, distance_type(first));
	}

	//泛化版_copy_dispatch
	template <class _InputIterator, class _OutputIterator>
	struct _copy_dispatch
	{
		_OutputIterator operator()(_InputIterator first, _InputIterator last,
			_OutputIterator result)
		{
			return _copy(first, last, result, iterator_category(first));
		}
	};

	//特化版本一：指针所指对象有 平凡赋值运算符
	//是上面_copy_dispatch的一个特化版本
	template <class _T>
	inline
		_T* _copy_t(const _T* first, const _T* last, _T* result, __true_type)
	{
		memmove(result, first, sizeof(_T) * (last - first));
		return result + (last - first);
	}
	//特化版本二：指针所指对象有 非平凡赋值运算符
	//是上面_copy_dispatch的一个特化版本
	template <class _T>
	inline
		_T* _copy_t(const _T* first, const _T* last, _T* result, __false_type)
	{
		return _copy_d(first, last, result, (ptrdiff_t*)0);
	}
	//根据型别来调用上面的两个特化版本，其实调用的是_copy_d或者memmove
	template <class _T>
	struct _copy_dispatch<_T*, _T*>
	{
		_T* operator()(_T* first, _T* last, _T* result)
		{
			typedef typename __type_traits<_T>::has_trivial_assignment_operator t;
			return _copy_t(first, last, result, t());
		}
	};
	//根据型别来调用上面的两个特化版本
	template <class _T>
	struct _copy_dispatch<const _T*, _T*>
	{
		_T* operator()(const _T* first, const _T* last, _T* result)
		{
			typedef typename __type_traits<_T>::has_trivial_assignment_operator t;
			return _copy_t(first, last, result, t());
		}
	};
	//看代码时，从这里开始看，然后一步一步向上迭代，找到调用关系。
	//泛化接口：
	template <class _InputIterator, class _OutputIterator>
	inline
		_OutputIterator copy(_InputIterator first, _InputIterator last,
			_OutputIterator result)
	{
		return _copy_dispatch<_InputIterator, _OutputIterator>()(first, last, result);
	}
	//特化接口：调用memmove()函数实现copy
	inline
		char* copy(const char* first, const char* last, char* result)
	{
		memmove(result, first, last - first);
		return result + (last - first);
	}
	//特化接口：调用memmove()函数实现copy
	inline
		wchar_t* copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
	{
		memmove(result, first, sizeof(wchar_t) * (last - first));
		return result + (last - first);
	}
	//以下是copy_backward的实现：
	template <class _BidirectionalIterator1, class _BidirectionalIterator2>
	inline _BidirectionalIterator2 _copy_backward(_BidirectionalIterator1 first,
		_BidirectionalIterator1 last,
		_BidirectionalIterator2 result) {
		while (first != last) *--result = *--last;
		return result;
	}

	template <class _BidirectionalIterator1, class _BidirectionalIterator2>
	struct _copy_backward_dispatch
	{
		_BidirectionalIterator2 operator()(_BidirectionalIterator1 first,
			_BidirectionalIterator1 last,
			_BidirectionalIterator2 result) {
			return _copy_backward(first, last, result);
		}
	};

	template <class _T>
	inline
		_T* _copy_backward_t(const _T* first, const _T* last, _T* result,
			__true_type)
	{
		const ptrdiff_t N = last - first;
		memmove(result - N, first, sizeof(_T) * N);
		return result - N;
	}

	template <class _T>
	inline
		_T* _copy_backward_t(const _T* first, const _T* last, _T* result,
			__false_type)
	{
		return _copy_backward(first, last, result);
	}

	template <class _T>
	struct _copy_backward_dispatch<_T*, _T*>
	{
		_T* operator()(_T* first, _T* last, _T* result)
		{
			typedef typename __type_traits<_T>::has_trivial_assignment_operator t;
			return _copy_backward_t(first, last, result, t());
		}
	};

	template <class _T>
	struct _copy_backward_dispatch<const _T*, _T*>
	{
		_T* operator()(const _T* first, const _T* last, _T* result)
		{
			typedef typename __type_traits<_T>::has_trivial_assignment_operator t;
			return _copy_backward_t(first, last, result, t());
		}
	};

	//copy_backward接口：
	template <class _BidirectionalIterator1, class _BidirectionalIterator2>
	inline _BidirectionalIterator2 copy_backward(_BidirectionalIterator1 first,
		_BidirectionalIterator1 last,
		_BidirectionalIterator2 result)
	{
		return _copy_backward_dispatch<_BidirectionalIterator1,
			_BidirectionalIterator2>()(first, last, result);
	}


	//区间内元素改填新值
	//在内存处理工具中用到
	template <class _ForwardIterator, class _T>
	void fill(_ForwardIterator first, _ForwardIterator last, const _T& value)
	{
		for (; first != last; ++first)	//迭代走过整个范围
			*first = value;
	}

	//区间内前n个元素改填新值，返回最后一个被改填元素的下一元素迭代器
	template <class _OutputIterator, class _Size, class _T>
	_OutputIterator fill_n(_OutputIterator first, _Size n, const _T& value)
	{
		for (; n > 0; --n, ++first)		// 经过n个元素
			*first = value;	// 注意，assignment 是覆写（overwrite）而不是安插（insert）
		return first;
	}

}


#endif//end of algobase
