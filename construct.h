/*
* ���������ȫ�ֺ�������construct(), destroy(),
* uninitialized_copy(),uninitialized_fill(),
* uninitialized_fill_n()�е�ǰ������
* �������Ĺ���������
*/
#ifndef CONSTRUCT_H_
#define CONSTRUCT_H_
#include<new>
#include "type_traits.h"
#include "iterator.h"
namespace MySTL {
	//�������
	//����һ��ָ���һ����ֵ������ֵ�趨��ָ����ָ�Ŀռ���
	template <class _T1, class _T2>
	inline void construct(_T1* p, const _T2& value)
	{
		new (p) _T1(value); 	// placement new; ���� T1::T1(value);
	}
	//destroy ����������
	/*destroy�ĵ�һ���汾����һ���汾����һ��ָ��
	***��ָ����ָ֮��������ֱ�ӵ��øö������������
	*/
	template<class _T>
	inline void destroy(_T* pointer) {
		pointer->~_T();//����~_T();
	}
	/*destroy�ĵڶ����汾���ڶ����汾��������������
	***��������������Χ�ڵĶ�������
	***�����Χ���ܴܺ����ÿ�������������������trivial destructor
	***��ôһֱ�����������Ч���кܴ�Ӱ�죬����������Ҫʹ��value_type()
	***��õ�������ָ������ͱ���������ȡ�����жϸö�������������Ƿ���
	***trivial destructor���������ʲôҲ�����ͽ�����������ѭ��������Χ
	***��ÿһ��������õ�һ���汾destroy
	*/
	//trivial destructorָĬ��������non-trivial destructor�Զ���
	// Ԫ�ص�ֵ���ͣ�value type��û��trivial destructor
	template <class _ForwardIterator>
	inline
		void _destroy_aux(_ForwardIterator first, _ForwardIterator last, __false_type)
	{
		for (; first < last; ++first) //���������������
			destroy(&*first);
	}
	// Ԫ�ص�ֵ���ͣ�value type����trivial destructor
	//ʲôҲ����
	template <class _ForwardIterator>
	inline
		void _destroy_aux(_ForwardIterator first, _ForwardIterator last, __true_type)
	{
	}
	// �ж�Ԫ�ص�ֵ���ͣ�value type���Ƿ��� trivial destructor
	template <class _ForwardIterator, class _T>
	inline void _destroy(_ForwardIterator first, _ForwardIterator last, _T*)
	{
		//��ȡ_T���ͱ����ԣ���ȡ��ش�����"type_traits.h"
		typedef typename __type_traits<_T>::has_trivial_destructor trivial_destructor;
		_destroy_aux(first, last, trivial_destructor());
	}
	// ������ destroy() �ڶ��汾�������������������˺�ʽ���跨�ҳ�Ԫ�ص�ֵ���ͣ�
	// �������� __type_traits<> ��ȡ���ʵ���ʩ��
	template <class _ForwardIterator>
	inline void destroy(_ForwardIterator first, _ForwardIterator last)
	{
		_destroy(first, last, value_type(first));
	}

	// ������destroy() �ڶ��汾��Ե�����Ϊ char* �� wchar_t* ���ػ���
	inline void destroy(char*, char*) {}
	inline void destroy(wchar_t*, wchar_t*) {}
	
}
#endif //end of CONSTRUCT_H_
