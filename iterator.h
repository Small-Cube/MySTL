/*
* 基础迭代器的实现
*/
#ifndef ITERATOR_H_
#define ITERATOR_H_
#include<cstddef> 
namespace MyStl
{
	// 五种迭代器类型
	//这些class只作为标记，所以不需要什么成员
    //这种迭代器所指的对象不允许外界改变，只读
	struct input_iterator_tag {};  
	//唯写
	struct output_iterator_tag {};
	//允许写入型算法，如replace()在此种迭代器区间上进行读写
	struct forward_iterator_tag : public input_iterator_tag {};
	//可双向移动，某些算法要求
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	//前四种迭代器只供应一部分指针算术运算，前三种支持operator++，第四种再加上operator--。
	//第五种全都支持，
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};

	template <class _T, class _Distance>
	struct input_iterator
	{
		typedef input_iterator_tag  iterator_category;
		typedef _T                  value_type;
		typedef _Distance           difference_type;
		typedef _T*                 pointer;
		typedef _T&                 reference;
	};

	struct output_iterator
	{
		typedef output_iterator_tag iterator_category;
		typedef void                value_type;
		typedef void                difference_type;
		typedef void                pointer;
		typedef void                reference;
	};


	template <class _T, class _Distance>
	struct forward_iterator
	{
		typedef forward_iterator_tag iterator_category;
		typedef _T                    value_type;
		typedef _Distance             difference_type;
		typedef _T*                   pointer;
		typedef _T&                   reference;
	};

	template <class _T, class _Distance>
	struct bidirectional_iterator
	{
		typedef bidirectional_iterator_tag iterator_category;
		typedef _T                          value_type;
		typedef _Distance                   difference_type;
		typedef _T*                         pointer;
		typedef _T&                         reference;
	};

	template <class _T, class _Distance>
	struct random_access_iterator
	{
		typedef random_access_iterator_tag iterator_category;
		typedef _T                          value_type;
		typedef _Distance                   difference_type;
		typedef _T*                         pointer;
		typedef _T&                         reference;
	};

	//泛化接口，非连续容器实现的迭代器应该继承这个标准迭代器
	template <class _Category, class _T, class _Distance = ptrdiff_t,
		class _Pointer = _T*, class _Reference = _T&>
		struct iterator
	{
		typedef _Category   iterator_category;
		typedef _T          value_type;
		typedef _Distance   difference_type;
		typedef _Pointer    pointer;
		typedef _Reference  reference;
	};
	//iterator型别萃取
	template <class _Iterator>
	struct iterator_traits
	{
		typedef typename _Iterator::iterator_category iterator_category;
		typedef typename _Iterator::value_type        value_type;
		typedef typename _Iterator::difference_type   difference_type;
		typedef typename _Iterator::pointer           pointer;
		typedef typename _Iterator::reference         reference;
	};

	// 针对原生指针而设计的traits偏特化版。
	template <class _T>
	struct iterator_traits<_T*>
	{
		//原生指针的迭代器型别是random_access_iterator_tag
		typedef random_access_iterator_tag  iterator_category;
		typedef _T                          value_type;
		typedef ptrdiff_t                   difference_type;
		typedef _T*                         pointer;
		typedef _T&                         reference;
	};

	// 针对原生top-level const指针设计的traits偏特化版。
	template <class _T>
	struct iterator_traits<const _T*>
	{
		typedef random_access_iterator_tag  iterator_category;
		typedef _T                          value_type; //注意value_type是_T
		typedef ptrdiff_t                   difference_type;
		typedef const _T*                   pointer;
		typedef const _T&                   reference;
	};

	//取得某个迭代器的类型（category）
	template <class _Iterator>
	inline
		typename iterator_traits<_Iterator>::iterator_category iterator_category(const _Iterator&)
	{
		typedef typename iterator_traits<_Iterator>::iterator_category category;
		return category();
	}

	// 取得某个迭代器的distance_type（指针）
	template <class _Iterator>
	inline
		typename iterator_traits<_Iterator>::difference_type* distance_type(const _Iterator&)
	{
		return static_cast<typename iterator_traits<_Iterator>::difference_type*>(0);
	}

	// 取得某个迭代器的value_type（指针）
	template <class _Iterator>
	inline
		typename iterator_traits<_Iterator>::value_type* value_type(const _Iterator&)
	{
		return static_cast<typename iterator_traits<_Iterator>::value_type*>(0);
	}

	// 以下是整组distance函式
	template <class _InputIterator>
	inline
		typename iterator_traits<_InputIterator>::difference_type _distance(_InputIterator first,
			_InputIterator last, input_iterator_tag)
	{
		typename iterator_traits<_InputIterator>::difference_type n = 0;
		while (first != last)
		{
			++first; ++n;
		}
		return n;
	}

	template <class _RandomAccessIterator>
	inline
		typename iterator_traits<_RandomAccessIterator>::difference_type _distance(_RandomAccessIterator first,
			_RandomAccessIterator last, random_access_iterator_tag)
	{
		return last - first;
	}
	//函数distance,用来计算两个迭代器之间的距离，针对不同的迭代器类型
	//跳转到上面其中一个函数
	template <class _InputIterator>
	inline
		typename iterator_traits<_InputIterator>::difference_type distance(_InputIterator first,
			_InputIterator last)
	{
		typedef typename iterator_traits<_InputIterator>::iterator_category category;
		return _distance(first, last, category());
	}



	// 以下是整组 advance 函式
	
	template <class _InputIterator, class _Distance>
	inline void _advance(_InputIterator& i, _Distance n, input_iterator_tag)
	{
		while (n--) ++i;
	}

	template <class BidirectionalIterator, class _Distance>
	inline void _advance(BidirectionalIterator& i, _Distance n,
		bidirectional_iterator_tag)
	{
		if (n >= 0)
			while (n--) ++i;
		else
			while (n++) --i;
	}

	template <class _RandomAccessIterator, class _Distance>
	inline void _advance(_RandomAccessIterator& i, _Distance n,
		random_access_iterator_tag)
	{
		i += n;
	}
	//该函数有两个参数，迭代器p和数值n，函数内部将p累进n次
	//根据迭代器型别跳转到上面其中之一的函数。
	template <class _InputIterator, class _Distance>
	inline void advance(_InputIterator& i, _Distance n)
	{
		_advance(i, n, iterator_category(i));
	}
}//end of MYSTL
#endif //end of ITERATOR_H
