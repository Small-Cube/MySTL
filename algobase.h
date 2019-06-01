//�����㷨��������Ҫ�������
#ifndef ALGOBASE_H_
#define ALGOBASE_H_


//#include "pair"
#include "iterator.h"
#include "type_traits.h"
#include <new>
#include <iostream>

namespace MySTL {
	//ȡ�������н�С��/�ϴ���
	//���ʱͳһȡǰ��
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
		return comp(b, a) ? b : a;	// �� comp ��������С�Ƚϡ���׼
	}
	template <class _T, class _Compare>
	inline const _T& max(const _T& a, const _T& b, _Compare comp)
	{
		return comp(a, b) ? b : a;	// �� comp ��������С�Ƚϡ���׼
	}
	//������copy��ʵ�֣�
	//���input_iterator��forward_iterator��bidirectional_iterator
	//����������������һ����ʼλ��һ����ֹλ�ã������µ�λ��
	//_copy��_copy_dispatch�ķ���
	template <class _InputIterator, class _OutputIterator>
	inline
		_OutputIterator _copy(_InputIterator first, _InputIterator last,
			_OutputIterator result, MySTL::input_iterator_tag)
	{
		//�Ƚϵ������Ƿ���Ⱦ���ѭ���Ƿ����
		for (; first != last; ++result, ++first)
			*result = *first;
		return result;
	}
	//�������������Ŀ죬_copy_d������_copy��ǿ��
	template <class _RandomAccessIterator, class _OutputIterator, class _Distance>
	inline
		_OutputIterator _copy_d(_RandomAccessIterator first, _RandomAccessIterator last,
			_OutputIterator result, _Distance*)
	{
		//�Ե���������n����ѭ���Ƿ����ִ��
		for (_Distance n = last - first; n > 0; --n, ++result, ++first)
			*result = *first;
		return result;
	}

	//���random_access_iterator������ʵ�����
	//�������溯��
	template <class _RandomAccessIterator, class _OutputIterator>
	inline
		_OutputIterator _copy(_RandomAccessIterator first, _RandomAccessIterator last,
			_OutputIterator result, random_access_iterator_tag)
	{
		return _copy_d(first, last, result, distance_type(first));
	}

	//������_copy_dispatch
	template <class _InputIterator, class _OutputIterator>
	struct _copy_dispatch
	{
		_OutputIterator operator()(_InputIterator first, _InputIterator last,
			_OutputIterator result)
		{
			return _copy(first, last, result, iterator_category(first));
		}
	};

	//�ػ��汾һ��ָ����ָ������ ƽ����ֵ�����
	//������_copy_dispatch��һ���ػ��汾
	template <class _T>
	inline
		_T* _copy_t(const _T* first, const _T* last, _T* result, __true_type)
	{
		memmove(result, first, sizeof(_T) * (last - first));
		return result + (last - first);
	}
	//�ػ��汾����ָ����ָ������ ��ƽ����ֵ�����
	//������_copy_dispatch��һ���ػ��汾
	template <class _T>
	inline
		_T* _copy_t(const _T* first, const _T* last, _T* result, __false_type)
	{
		return _copy_d(first, last, result, (ptrdiff_t*)0);
	}
	//�����ͱ�����������������ػ��汾����ʵ���õ���_copy_d����memmove
	template <class _T>
	struct _copy_dispatch<_T*, _T*>
	{
		_T* operator()(_T* first, _T* last, _T* result)
		{
			typedef typename __type_traits<_T>::has_trivial_assignment_operator t;
			return _copy_t(first, last, result, t());
		}
	};
	//�����ͱ�����������������ػ��汾
	template <class _T>
	struct _copy_dispatch<const _T*, _T*>
	{
		_T* operator()(const _T* first, const _T* last, _T* result)
		{
			typedef typename __type_traits<_T>::has_trivial_assignment_operator t;
			return _copy_t(first, last, result, t());
		}
	};
	//������ʱ�������￪ʼ����Ȼ��һ��һ�����ϵ������ҵ����ù�ϵ��
	//�����ӿڣ�
	template <class _InputIterator, class _OutputIterator>
	inline
		_OutputIterator copy(_InputIterator first, _InputIterator last,
			_OutputIterator result)
	{
		return _copy_dispatch<_InputIterator, _OutputIterator>()(first, last, result);
	}
	//�ػ��ӿڣ�����memmove()����ʵ��copy
	inline
		char* copy(const char* first, const char* last, char* result)
	{
		memmove(result, first, last - first);
		return result + (last - first);
	}
	//�ػ��ӿڣ�����memmove()����ʵ��copy
	inline
		wchar_t* copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
	{
		memmove(result, first, sizeof(wchar_t) * (last - first));
		return result + (last - first);
	}
	//������copy_backward��ʵ�֣�
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

	//copy_backward�ӿڣ�
	template <class _BidirectionalIterator1, class _BidirectionalIterator2>
	inline _BidirectionalIterator2 copy_backward(_BidirectionalIterator1 first,
		_BidirectionalIterator1 last,
		_BidirectionalIterator2 result)
	{
		return _copy_backward_dispatch<_BidirectionalIterator1,
			_BidirectionalIterator2>()(first, last, result);
	}


	//������Ԫ�ظ�����ֵ
	//���ڴ洦�������õ�
	template <class _ForwardIterator, class _T>
	void fill(_ForwardIterator first, _ForwardIterator last, const _T& value)
	{
		for (; first != last; ++first)	//�����߹�������Χ
			*first = value;
	}

	//������ǰn��Ԫ�ظ�����ֵ���������һ��������Ԫ�ص���һԪ�ص�����
	template <class _OutputIterator, class _Size, class _T>
	_OutputIterator fill_n(_OutputIterator first, _Size n, const _T& value)
	{
		for (; n > 0; --n, ++first)		// ����n��Ԫ��
			*first = value;	// ע�⣬assignment �Ǹ�д��overwrite�������ǰ��壨insert��
		return first;
	}

}


#endif//end of algobase
