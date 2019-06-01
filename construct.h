/*
* 定义了五个全局函数——construct(), destroy(),
* uninitialized_copy(),uninitialized_fill(),
* uninitialized_fill_n()中的前两个，
* 负责对象的构建和析构
*/
#ifndef CONSTRUCT_H_
#define CONSTRUCT_H_
#include<new>
#include "type_traits.h"
#include "iterator.h"
namespace MySTL {
	//构造对象
	//接受一个指针和一个初值，将初值设定到指针所指的空间上
	template <class _T1, class _T2>
	inline void construct(_T1* p, const _T2& value)
	{
		new (p) _T1(value); 	// placement new; 调用 T1::T1(value);
	}
	//destroy 将对象析构
	/*destroy的第一个版本，第一个版本接受一个指针
	***将指针所指之物析构，直接调用该对象的析构函数
	*/
	template<class _T>
	inline void destroy(_T* pointer) {
		pointer->~_T();//调用~_T();
	}
	/*destroy的第二个版本，第二个版本接受两个迭代器
	***将两个迭代器范围内的对象析构
	***这个范围可能很大，如果每个对象的析构函数都是trivial destructor
	***那么一直调用这个将对效率有很大影响，所以在这里要使用value_type()
	***获得迭代器所指对象的型别，再利用萃取机制判断该对象的析构函数是否是
	***trivial destructor，如果是则什么也不做就结束，不是则循环整个范围
	***对每一个对象调用第一个版本destroy
	*/
	//trivial destructor指默认析构，non-trivial destructor自定义
	// 元素的值类型（value type）没有trivial destructor
	template <class _ForwardIterator>
	inline
		void _destroy_aux(_ForwardIterator first, _ForwardIterator last, __false_type)
	{
		for (; first < last; ++first) //逐个调用析构函数
			destroy(&*first);
	}
	// 元素的值类型（value type）有trivial destructor
	//什么也不做
	template <class _ForwardIterator>
	inline
		void _destroy_aux(_ForwardIterator first, _ForwardIterator last, __true_type)
	{
	}
	// 判断元素的值类型（value type）是否有 trivial destructor
	template <class _ForwardIterator, class _T>
	inline void _destroy(_ForwardIterator first, _ForwardIterator last, _T*)
	{
		//萃取_T的型别特性，萃取相关代码在"type_traits.h"
		typedef typename __type_traits<_T>::has_trivial_destructor trivial_destructor;
		_destroy_aux(first, last, trivial_destructor());
	}
	// 以下是 destroy() 第二版本，接受两个迭代器。此函式是设法找出元素的值类型，
	// 进而利用 __type_traits<> 求取最适当措施。
	template <class _ForwardIterator>
	inline void destroy(_ForwardIterator first, _ForwardIterator last)
	{
		_destroy(first, last, value_type(first));
	}

	// 以下是destroy() 第二版本针对迭代器为 char* 和 wchar_t* 的特化版
	inline void destroy(char*, char*) {}
	inline void destroy(wchar_t*, wchar_t*) {}
	
}
#endif //end of CONSTRUCT_H_
