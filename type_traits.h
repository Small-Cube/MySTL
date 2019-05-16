/*先看看书理解了萃取的机制，再实现
* 萃取传入的T类型的型别特性，包括：
* has_trivial_default_constructor;
* has_trivial_copy_constructor;
* has_trivial_assignment_operator;
* has_trivial_destructor;
* is_POD_type; (pod = plain old data)
*/
#ifndef TYPE_TRAITS_H_
#define TYPE_TRAITS_H_
namespace MySTL {
	struct __true_type {};
	struct __false_type {};
 
	//has nontrivial xxx...可以自己定义
	template <class _type>
	struct __type_traits
	{
		typedef __false_type    has_trivial_default_constructor;
		typedef __false_type    has_trivial_copy_constructor;
		typedef __false_type    has_trivial_assignment_operator;
		typedef __false_type    has_trivial_destructor;
		typedef __false_type    is_POD_type;  // 所谓 POD 意指 Plain Old Data structure. 	
	};
 
	//以下都是内置型别的特化
	template<>
	struct __type_traits<char>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	template<>
	struct __type_traits<signed char>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	template<>
	struct __type_traits<unsigned char>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	template<>
	struct __type_traits<short>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	template<>
	struct __type_traits<unsigned short>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	template<>
	struct __type_traits<int>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	template<>
	struct __type_traits<unsigned int>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	template<>
	struct __type_traits<long>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	template<>
	struct __type_traits<unsigned long>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	template<>
	struct __type_traits<float>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	template<>
	struct __type_traits<double>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	template<>
	struct __type_traits<long double>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
	//原生指针的特化，在标准库里是一个一个特化的。
	template <class _T>
	struct __type_traits<_T*>
	{
		typedef __true_type    has_trivial_default_constructor;
		typedef __true_type    has_trivial_copy_constructor;
		typedef __true_type    has_trivial_assignment_operator;
		typedef __true_type    has_trivial_destructor;
		typedef __true_type    is_POD_type;
	};
 
}
#endif