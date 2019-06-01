/*
*STL�������ȫ�ֺ���������δ��ʼ���Ŀռ��ϣ�����ļ�ʵ����������
* uninitialized_copy(),uninitialized_fill(),
* uninitialized_fill_n(),�Լ�������غ���
*/
#ifndef UNINITIALIZED_H_
#define UNINITIALIZED_H_
#include "construct.h"
#include "algorithm.h"
#include "pair.h"
namespace MySTL {
	/*
	* ������uninitialized_copy��ʵ��
	*������ȫ���乹�캯�������������ڴ����飬���԰�����Χ������Ԫ��
	**Ȼ������uninitialized_copy()�ڸ��ڴ������Ϲ���Ԫ��
	*/
	//�����POD�������ͣ�ִ������ĺ���
	/*POD����������Ϊ�˽��C++��C֮���������͵ļ�����
	*���Լ򵥵����Ϊ��������ǲ���Ĭ�Ϲ��캯����
	*Ĭ������������Ĭ�Ͽ�����ֵ��������Ϊ��POD������Ͳ���
	*POD���������ڴ�����������
	*/
	template<class _InputIterator,class _ForwordIterator>
	_ForwordIterator _uninitialized_copy_aux(_InputIterator first, _InputIterator last,
		_ForwordIterator result, __true_type) {
		//copy������algobase.h��ʵ��
		return copy(first, last, result);
	}
	// �����non-POD �ͱ�ִ�����̾ͻ�ת�������º���
	template <class _InputIterator, class _ForwardIterator>
	_ForwardIterator _uninitialized_copy_aux(_InputIterator first, _InputIterator last,
		_ForwardIterator result, __false_type)
	{
		_ForwardIterator cur = result;
		try
		{
			for (; first != last; ++first, ++cur)
				construct(&*cur, *first);	//����һ��һ��Ԫ�صؽ������޷���������
			return cur;
		}
		catch (...)
		{
			destroy(result, cur);
			throw;
		}
	}
	//_uninitialized_copy������������ȡ�������ݴ�������������������ͬ����
	//�������ܣ�����һ����������Χ������result��ʼλ���ϣ�����
	//��first��last������ָ��Ķ���Ȼ�����result��ʼ��λ�á�
	template <class _InputIterator, class _ForwardIterator, class _T>
	inline
		_ForwardIterator _uninitialized_copy(_InputIterator first, _InputIterator last,
			_ForwardIterator result, _T*)
	{
		typedef typename __type_traits<_T>::is_POD_type is_POD;
		//���� is_POD() ����õĽ�����ñ���������Ա����Ƶ���
		return _uninitialized_copy_aux(first, last, result, is_POD());

	}

	template <class _InputIterator, class _ForwardIterator>
	inline
		_ForwardIterator uninitialized_copy(_InputIterator first, _InputIterator last,
			_ForwardIterator result)
	{
		//���� value_type() ȡ�� first �� value type.
		return _uninitialized_copy(first, last, result, value_type(result));

	}

	// ��������� const char* ���ػ��汾
	// ����memmove()�������ٿ���
	inline
		char* uninitialized_copy(const char* first, const char* last, char* result)
	{
		memmove(result, first, last - first);
		return result + (last - first);
	}

	/* ������uninitialized_copy_n��ʵ�֣�
	*n��Ҫ�����Ԫ�صĸ���
	*
	*/
	
	//������������input_iterator_tag
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
	//������������random_access_iterator_tag
	template <class _RandomAccessIterator, class _Size, class _ForwardIterator>
	inline
		pair<_RandomAccessIterator, _ForwardIterator> _uninitialized_copy_n(
			_RandomAccessIterator first, _Size count,
			_ForwardIterator result, random_access_iterator_tag)
	{
		_RandomAccessIterator last = first + count;
		return make_pair(last, uninitialized_copy(first, last, result));
	}

	//���ݵ�����������ת����������������һ��
	template <class _InputIterator, class _Size, class _ForwardIterator>
	inline
		pair<_InputIterator, _ForwardIterator> uninitialized_copy_n(_InputIterator first,
			_Size count, _ForwardIterator result)
	{
		//���ݵ������ͱ���תִ�к���
		return _uninitialized_copy_n(first, count, result, iterator_category(first));
	}

	/* ������uninitialized_fill��ʵ�֣�
	*void uninitialized_fill(_ForwardIterator first, _ForwardIterator last,	const _T& x) 
	*��Ϊ����ԭ�ͣ�Ҳ�ܽ��ڴ��������������Ϊ���뿪�������[first,last)��ÿ��
	*��������ָ��û�г�ʼ�����ڴ棬��ô����������������Χ�ڲ���x�ĸ���Ʒ��
	*
	*/

	// �����POD �ͱ�PODǰ��˵������ת�������º���
	template <class _ForwardIterator, class _T>
	inline
		void _uninitialized_fill_aux(_ForwardIterator first, _ForwardIterator last,
			const _T& x, __true_type)
	{
		fill(first, last, x);//�㷨fill()
	}

	// �����non-POD �ͱ�ִ�����̾ͻ�ת�������º���
	template <class _ForwardIterator, class _T>
	void _uninitialized_fill_aux(_ForwardIterator first, _ForwardIterator last,
		const _T& x, __false_type)
	{
		_ForwardIterator cur = first;
		try
		{
			for (; cur != last; ++cur)
				construct(&*cur, x);	// ����һ��һ��Ԫ�صؽ������޷���������
		}
		catch (...)
		{
			destroy(first, cur);
			throw;
		}
	}
	//�����Ƿ�ΪPOD���͵�������ĺ���
	template <class _ForwardIterator, class _T, class _T1>
	inline void _uninitialized_fill(_ForwardIterator first, _ForwardIterator last,
		const _T& x, _T1*)
	{
		typedef typename __type_traits<_T1>::is_POD_type is_POD;
		_uninitialized_fill_aux(first, last, x, is_POD());

	}
	//���ݵ���������
	template <class _ForwardIterator, class _T>
	inline
		void uninitialized_fill(_ForwardIterator first, _ForwardIterator last,
			const _T& x)
	{
		//���� value_type() ȡ�� first �� value type.
		_uninitialized_fill(first, last, x, value_type(first));
	}

	/*������uninitialized_fill_n��ʵ�֣�
	*_ForwardIterator uninitialized_fill_n(_ForwardIterator first, _Size n,const _T& x)
	*��ָ����Χ��Ϊ����Ԫ���趨��ͬ�ĳ�ֵ
	*�����[first,last)��ÿ��
	*��������ָ��û�г�ʼ�����ڴ棬��ô����������������Χ�ڲ���x�ĸ���Ʒ��
	*/

	// �����POD �ͱ�ִ�����̾ͻ�ת�������º���
	template <class _ForwardIterator, class _Size, class _T>
	inline
		_ForwardIterator _uninitialized_fill_n_aux(_ForwardIterator first, _Size n,
			const _T& x, __true_type)
	{
		return MySTL::fill_n(first, n, x); //�㷨fill_n()
	}

	// �����non-POD �ͱ�ִ�����̾ͻ�ת�������º���
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
	//�����Ƿ�ΪPODת��������������
	template <class _ForwardIterator, class _Size, class _T, class _T1>
	inline _ForwardIterator _uninitialized_fill_n(_ForwardIterator first, _Size n,
		const _T& x, _T1*)
	{
		typedef typename __type_traits<_T1>::is_POD_type is_POD;
		return _uninitialized_fill_n_aux(first, n, x, is_POD());

	}
	//��value_type(first)�õ��������ͱ�
	template <class _ForwardIterator, class _Size, class _T>
	inline
		_ForwardIterator uninitialized_fill_n(_ForwardIterator first, _Size n,
			const _T& x)
	{
		return _uninitialized_fill_n(first, n, x, value_type(first));
	}

	/*__uninitialized_copy_copy��__uninitialized_fill_copy��__uninitialized_copy_fill����
����*�⼸�������Ƚϼ򵥣����ǵ�������ĺ����Ϳ���ʵ�ֹ���
	*/
	  //ͬʱ�������Ե������ڵ�����
	 //����[first1, last1)��[result, result + (last1 - first1))
     // ͬʱ����[first2, last2)��
    // [result + (last1 - first1), result + (last1 - first1) + (last2 - first2)]
	template <class _InputIterator1, class _InputIterator2, class _ForwardIterator>
	inline
		_ForwardIterator _uninitialized_copy_copy(_InputIterator1 first1, _InputIterator1 last1,
			_InputIterator2 first2, _InputIterator2 last2, _ForwardIterator result)
	{
		//������һ�Ե�������result,���ص�ַ�����ĵ�ַ
		_ForwardIterator mid = uninitialized_copy(first1, last1, result);
		try
		{
			//�����ڶ��Ե�������result,���ص�ַ�����ĵ�ַ
			return uninitialized_copy(first2, last2, mid);
		}
		catch (...)
		{
			destroy(result, mid);
			throw;
		}
	}

	//��x���[result,mid),֮��[first,last)��������ָ������
	//������mid֮��
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

	// �����溯������ֻ��������˳���ȿ��������
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

}
#endif// end of UNINITIALIZED_H_