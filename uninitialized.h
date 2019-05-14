/*
*STL中有五个全局函数作用于未初始化的空间上，这个文件实现下面三个
* uninitialized_copy(),uninitialized_fill(),
* uninitialized_fill_n(),以及其他相关函数
*/
#ifndef UNINITIALIZED_H_
#define UNINITIALIZED_H_
#include "construct.h"
#include "algorithm.h"
namespace MySTL {
	/*
	* 以下是uninitialized_copy的实现
	*容器的全区间构造函数会首先配置内存区块，足以包含范围内所有元素
	**然后会调用uninitialized_copy()在该内存区块上构造元素
	*/
	//如果是POD数据类型，执行下面的函数
	/*POD数据类型是为了解决C++与C之间数据类型的兼容性
	*可以简单地理解为如果对象是采用默认构造函数、
	*默认析构函数、默认拷贝赋值函数就认为是POD，否则就不是
	*POD数据类型内存中是连续的
	*/
	template<class _InputIterator,class _ForwordIterator>
	_ForwordIterator _uninitialized_copy_aux(_InputIterator first, _InputIterator last,
		_ForwardIterator result, __true_type) {
		//copy函数在algobase.h里实现
		rerturn copy(first, last, result);
	}
	// 如果是non-POD 型别，执行流程就会转进到以下函数
	template <class _InputIterator, class _ForwardIterator>
	_ForwardIterator _uninitialized_copy_aux(_InputIterator first, _InputIterator last,
		_ForwardIterator result, __false_type)
	{
		_ForwardIterator cur = result;
		try
		{
			for (; first != last; ++first, ++cur)
				construct(&*cur, *first);	//必须一个一个元素地建构，无法批量进行
			return cur;
		}
		catch (...)
		{
			destroy(result, cur);
			throw;
		}
	}
	//_uninitialized_copy函数，利用萃取技术根据传入对象调用上面两个不同函数
	//函数功能：输入一个迭代器范围，在以result起始位置上，复制
	//从first到last迭代器指向的对象，然后放在result开始的位置。
	template <class _InputIterator, class _ForwardIterator, class _T>
	inline
		_ForwardIterator _uninitialized_copy(_InputIterator first, _InputIterator last,
			_ForwardIterator result, _T*)
	{
		typedef typename __type_traits<_T>::is_POD_type is_POD;
		//利用 is_POD() 所获得的结果，让编译程序做自变量推导。
		return _uninitialized_copy_aux(first, last, result, is_POD());

	}

	template <class _InputIterator, class _ForwardIterator>
	inline
		_ForwardIterator uninitialized_copy(_InputIterator first, _InputIterator last,
			_ForwardIterator result)
	{
		//利用 value_type() 取出 first 的 value type.
		return _uninitialized_copy(first, last, result, value_type(result));

	}

	// 以下是针对 const char* 的特化版本
	// 采用memmove()函数快速拷贝
	inline
		char* uninitialized_copy(const char* first, const char* last, char* result)
	{
		memmove(result, first, last - first);
		return result + (last - first);
	}

	/* 以下是uninitialized_copy_n的实现：
	*n是要构造的元素的个数
	*
	*/
	
	//迭代器类型是input_iterator_tag
	template <class _InputIterator, class _Size, class _ForwardIterator>
	pair<_InputIterator, _ForwardIterator> _uninitialized_copy_n(_InputIterator first,
		_Size count, _ForwardIterator result, input_iterator_tag)
	{
		_ForwardIterator cur = result;
		try
		{
			for (; count > 0; --count, ++first, ++cur)
				construct(&*cur, *first);
			return pair<_InputIterator, _ForwardIterator>(first, cur);
		}
		catch (...)
		{
			destroy(result, cur);
			throw;
		}
	}
	//迭代器类型是random_access_iterator_tag
	template <class _RandomAccessIterator, class _Size, class _ForwardIterator>
	inline
		pair<_RandomAccessIterator, _ForwardIterator> _uninitialized_copy_n(
			_RandomAccessIterator first, _Size count,
			_ForwardIterator result, random_access_iterator_tag)
	{
		_RandomAccessIterator last = first + count;
		return make_pair(last, uninitialized_copy(first, last, result));
	}

	//根据迭代器类型跳转到上面两个函数中一个
	template <class _InputIterator, class _Size, class _ForwardIterator>
	inline
		pair<_InputIterator, _ForwardIterator> uninitialized_copy_n(_InputIterator first,
			_Size count, _ForwardIterator result)
	{
		//根据迭代器型别跳转执行函数
		return _uninitialized_copy_n(first, count, result, iterator_category(first));
	}

	/* 以下是uninitialized_fill的实现：
	*void uninitialized_fill(_ForwardIterator first, _ForwardIterator last,	const _T& x) 
	*上为函数原型，也能将内存配置与对象构造行为分离开，如果在[first,last)上每个
	*迭代器都指向没有初始化的内存，那么这个函数将在这个范围内产生x的复制品。
	*
	*/

	// 如果是POD 型别，POD前面说过，会转进到以下函数
	template <class _ForwardIterator, class _T>
	inline
		void _uninitialized_fill_aux(_ForwardIterator first, _ForwardIterator last,
			const _T& x, __true_type)
	{
		fill(first, last, x);//算法fill()
	}

	// 如果是non-POD 型别，执行流程就会转进到以下函数
	template <class _ForwardIterator, class _T>
	void _uninitialized_fill_aux(_ForwardIterator first, _ForwardIterator last,
		const _T& x, __false_type)
	{
		_ForwardIterator cur = first;
		try
		{
			for (; cur != last; ++cur)
				construct(&*cur, x);	// 必须一个一个元素地建构，无法批量进行
		}
		catch (...)
		{
			destroy(first, cur);
			throw;
		}
	}
	//根据是否为POD类型调用上面的函数
	template <class _ForwardIterator, class _T, class _T1>
	inline void _uninitialized_fill(_ForwardIterator first, _ForwardIterator last,
		const _T& x, _T1*)
	{
		typedef typename __type_traits<_T1>::is_POD_type is_POD;
		_uninitialized_fill_aux(first, last, x, is_POD());

	}
	//根据迭代器类型
	template <class _ForwardIterator, class _T>
	inline
		void uninitialized_fill(_ForwardIterator first, _ForwardIterator last,
			const _T& x)
	{
		//利用 value_type() 取出 first 的 value type.
		_uninitialized_fill(first, last, x, value_type(first));
	}

	/*以下是uninitialized_fill_n的实现：
	*_ForwardIterator uninitialized_fill_n(_ForwardIterator first, _Size n,const _T& x)
	*在指定范围内为所有元素设定相同的初值
	*如果在[first,last)上每个
	*迭代器都指向没有初始化的内存，那么这个函数将在这个范围内产生x的复制品。
	*/

	// 如果是POD 型别，执行流程就会转进到以下函数
	template <class _ForwardIterator, class _Size, class _T>
	inline
		_ForwardIterator _uninitialized_fill_n_aux(_ForwardIterator first, _Size n,
			const _T& x, __true_type)
	{
		return MyStl::fill_n(first, n, x); //算法fill_n()
	}

	// 如果是non-POD 型别，执行流程就会转进到以下函数
	template <class _ForwardIterator, class _Size, class _T>
	_ForwardIterator _uninitialized_fill_n_aux(_ForwardIterator first, _Size n,
		const _T& x, __false_type)
	{
		_ForwardIterator cur = first;
		try
		{
			for (; n > 0; --n, ++cur)
				construct(&*cur, x);
			return cur;
		}
		catch (...)
		{
			destroy(first, cur);
			throw;
		}
	}
	//根据是否为POD转向上面两个函数
	template <class _ForwardIterator, class _Size, class _T, class _T1>
	inline _ForwardIterator _uninitialized_fill_n(_ForwardIterator first, _Size n,
		const _T& x, _T1*)
	{
		typedef typename __type_traits<_T1>::is_POD_type is_POD;
		return _uninitialized_fill_n_aux(first, n, x, is_POD());

	}
	//由value_type(first)得到迭代器型别
	template <class _ForwardIterator, class _Size, class _T>
	inline
		_ForwardIterator uninitialized_fill_n(_ForwardIterator first, _Size n,
			const _T& x)
	{
		return _uninitialized_fill_n(first, n, x, value_type(first));
	}

	/*__uninitialized_copy_copy、__uninitialized_fill_copy和__uninitialized_copy_fill函数
　　*这几个函数比较简单，都是调用上面的函数就可以实现功能
	*/
	  //同时拷贝两对迭代器内的内容
	 //拷贝[first1, last1)到[result, result + (last1 - first1))
     // 同时拷贝[first2, last2)到
    // [result + (last1 - first1), result + (last1 - first1) + (last2 - first2)]
	template <class _InputIterator1, class _InputIterator2, class _ForwardIterator>
	inline
		_ForwardIterator _uninitialized_copy_copy(_InputIterator1 first1, _InputIterator1 last1,
			_InputIterator2 first2, _InputIterator2 last2, _ForwardIterator result)
	{
		//拷贝第一对迭代器到result,返回地址是最后的地址
		_ForwardIterator mid = uninitialized_copy(first1, last1, result);
		try
		{
			//拷贝第二对迭代器到result,返回地址是最后的地址
			return uninitialized_copy(first2, last2, mid);
		}
		catch (...)
		{
			destroy(result, mid);
			throw;
		}
	}

	//用x填充[result,mid),之后将[first,last)迭代器所指的内容
	//拷贝到mid之后
	//
	template <class _ForwardIterator, class _T, class _InputIterator>
	inline
		_ForwardIterator _uninitialized_fill_copy(_ForwardIterator result, _ForwardIterator mid,
			const _T& x, _InputIterator first, _InputIterator last)
	{
		uninitialized_fill(result, mid, x);
		try
		{
			return uninitialized_copy(first, last, mid);
		}
		catch (...)
		{
			destroy(result, mid);
			throw;
		}
	}

	// 和上面函数很像，只不过调换顺序，先拷贝再填充
	template <class _InputIterator, class _ForwardIterator, class _T>
	inline
		void _uninitialized_copy_fill(_InputIterator first1, _InputIterator last1,
			_ForwardIterator first2, _ForwardIterator last2, const _T& x)
	{
		_ForwardIterator mid2 = uninitialized_copy(first1, last1, first2);
		try
		{
			uninitialized_fill(mid2, last2, x);
		}
		catch (...)
		{
			destroy(first2, mid2);
			throw;
		}
	}

}//end of MyStl
#endif// end of UNINITIALIZED_H_