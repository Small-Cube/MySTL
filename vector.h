/*
* vector的实现
*/

#ifndef VECTOR_H_
#define VECTOR_H_

#include "allocator.h"
#include <initializer_list>

namespace MySTL
{
	template <class _T, class _Alloc = alloc>  // 默认使用 alloc 为配置器
	class vector
	{
	public:

		typedef _T                 value_type;
		typedef value_type*        pointer;
		typedef const value_type*  const_pointer;
		typedef const value_type*  const_iterator;
		typedef value_type&        reference;
		typedef const value_type&  const_reference;
		typedef size_t             size_type;
		typedef ptrdiff_t          difference_type;
		//注意vector的iterator是原生指针，经过iterator_traits
		//得到迭代器型别是random_access_iterator_category
		typedef value_type*        iterator;

	protected:
		// vector专属空间配置器，每次配置一个元素大小
		typedef simple_alloc<value_type, _Alloc> data_allocator;

		// vector采用简单的线性连续空间。以两个迭代器start和end分别指向头尾，
		// 并以迭代器end_of_storage指向容量尾端。容量(capacity)可能比(尾 - 头 = size)还大，
		// 多余即备用空间。
		iterator start;
		iterator finish;
		iterator end_of_storage;
		//插入操作辅助函数
		void insert_aux(iterator position, const _T& x);
		//释放整个capacity
		void deallocate()
		{
			if (start)
				data_allocator::deallocate(start, end_of_storage - start);
		}

		void fill_initialize(size_type n, const _T& value)
		{
			start = allocate_and_fill(n, value);   // 配置空间并设初值
			finish = start + n;				       // 调整水位
			end_of_storage = finish; 			   // 调整水位
		}
		//以上函数为protected
	public:
		iterator begin() { return start; }
		const_iterator begin() const { return start; }
		iterator end() { return finish; }
		const_iterator end() const { return finish; }
		size_type size() const { return size_type(end() - begin()); }
		size_type max_size() const { return size_type(-1) / sizeof(_T); } //2^32/T
		size_type capacity() const { return size_type(end_of_storage - begin()); }
		bool empty() const { return begin() == end(); }
		reference operator[](size_type n) { return *(begin() + n); }
		const_reference operator[](size_type n) const { return *(begin() + n); }

		//构造函数：
		vector() : start(0), finish(0), end_of_storage(0) {} //空vector
		vector(size_type n, const _T& value) { fill_initialize(n, value); }
		vector(int n, const _T& value) { fill_initialize(n, value); }
		vector(long n, const _T& value) { fill_initialize(n, value); }
		explicit vector(size_type n) { fill_initialize(n, _T()); }
		//拷贝构造函数
		vector(const vector<_T, _Alloc>& x)
		{
			start = allocate_and_copy(x.end() - x.begin(), x.begin(), x.end());
			finish = start + (x.end() - x.begin());
			end_of_storage = finish;
		}

		template <class _InputIterator>
		vector(_InputIterator first, _InputIterator last) :
			start(0), finish(0), end_of_storage(0)
		{
			range_initialize(first, last, iterator_category(first));
		}
		//列表构造
		vector(const std::initializer_list<_T>& il)
		{
			range_initialize(il.begin(), il.end(), forward_iterator_tag());
		}

		//析构函数
		~vector()
		{
			destroy(start, finish);  // 全局函式
			deallocate();            // 先前定义好的成员函式
		}

		vector<_T, _Alloc>& operator=(const vector<_T, _Alloc>& x);

		//预留空间：如果要求预留的空间n比原来capacity不大，则不操作
		//否则，将原空间元素完全搬移到新空间
		void reserve(size_type n)
		{
			if (capacity() < n)
			{
				const size_type old_size = size();
				iterator tmp = allocate_and_copy(n, start, finish);
				destroy(start, finish);
				deallocate();
				start = tmp;
				finish = tmp + old_size;
				end_of_storage = start + n;
			}
		}

		// 取出第一个元素内容
		reference front() { return *begin(); }
		const_reference front() const { return *begin(); }
		// 取出最后一个元素内容
		reference back() { return *(end() - 1); }
		const_reference back() const { return *(end() - 1); }
		// 增加一个元素，做为最后元素
		void push_back(const _T& x)
		{
			if (finish != end_of_storage)// 还有备用空间
			{
				construct(finish, x); // 直接在备用空间中建构元素。
				++finish;             // 调整水位高度
			}
			else // 已无备用空间
				insert_aux(end(), x); //交由辅助函数完成
		}

		//交换两个vector
		//实际是指针交换，速度非常快
		void swap(vector<_T, _Alloc>& x)
		{
			std::swap(start, x.start);
			std::swap(finish, x.finish);
			std::swap(end_of_storage, x.end_of_storage);
		}
		//插入操作
		iterator insert(iterator position, const _T& x)
		{
			size_type n = position - begin();
			if (finish != end_of_storage && position == end())//有备用空间且是尾端插入
			{                                                 //不用调整空间布局
				construct(finish, x);// 全局函数。
				++finish;
			}
			else
				insert_aux(position, x);
			return begin() + n;
		}
		iterator insert(iterator position) { return insert(position, _T()); }
		//范围插入操作
		template <class _InputIterator>
		void insert(iterator position, _InputIterator first, _InputIterator last)
		{
			range_insert(position, first, last, iterator_category(first));
		}
		//插入n个相同元素
		void insert(iterator pos, size_type n, const _T& x);
		void insert(iterator pos, int n, const _T& x)
		{
			insert(pos, (size_type)n, x);
		}
		void insert(iterator pos, long n, const _T& x)
		{
			insert(pos, (size_type)n, x);
		}
		//删除最后元素
		void pop_back()
		{
			--finish;
			destroy(finish);//全局函数
		}
		// 将迭代器 position 所指之元素移除
		iterator erase(iterator position)
		{
			if (position + 1 != end()) // 如果 p 不是指向最后一个元素
									   // 将 p 之后的元素整体向前递移
				copy(position + 1, finish, position);

			--finish;           // 调整水位
			destroy(finish);	// 全局函数
			return position;
		}
		// 删除范围内元素
		iterator erase(iterator first, iterator last)
		{
			iterator i = copy(last, finish, first);//待删除范围之后元素整体前移
			destroy(i, finish);	// 全局函数
			finish = finish - (last - first);
			return first;
		}
		// size大小调整：调小——删除后端元素，调大——后端插入值为x的元素
		void resize(size_type new_size, const _T& x)
		{
			if (new_size < size())
				erase(begin() + new_size, end());
			else
				insert(end(), new_size - size(), x);
		}
		void resize(size_type new_size) { resize(new_size, _T()); }
		// 清除全部元素。注意，并未释放空间，以备可能未来还会新加入元素。
		void clear() { erase(begin(), end()); }

	protected:
		iterator allocate_and_fill(size_type n, const _T& x)
		{
			iterator result = data_allocator::allocate(n); // 配置n个元素空间
			try
			{
				uninitialized_fill_n(result, n, x); //全局函数
				return result;
			}
			catch (...)
			{
				// "commit or rollback" 
				data_allocator::deallocate(result, n);
				throw;
			}
		}

		template <class _ForwardIterator>
		iterator allocate_and_copy(size_type n, _ForwardIterator first, _ForwardIterator last)
		{
			iterator result = data_allocator::allocate(n);
			try
			{
				uninitialized_copy(first, last, result);
				return result;
			}
			catch (...)
			{
				// "commit or rollback" 
				data_allocator::deallocate(result, n);
				throw;
			}
		}


		template <class _InputIterator>
		void range_initialize(_InputIterator first, _InputIterator last, input_iterator_tag)
		{
			for (; first != last; ++first)
				push_back(*first);
		}

		// 仅被构造函数调用
		template <class _ForwardIterator>
		void range_initialize(_ForwardIterator first, _ForwardIterator last, forward_iterator_tag)
		{
			size_type n = distance(first, last);
			start = allocate_and_copy(n, first, last);
			finish = start + n;
			end_of_storage = finish;
		}

		template <class _InputIterator>
		void range_insert(iterator pos, _InputIterator first,
			_InputIterator last, input_iterator_tag);

		template <class _ForwardIterator>
		void range_insert(iterator pos, _ForwardIterator first,
			_ForwardIterator last, forward_iterator_tag);

	}; // end of vector

	template <class _T, class _Alloc>
	inline
		bool operator==(const vector<_T, _Alloc>& x, const vector<_T, _Alloc>& y)
	{
		return x.size() == y.size() && equal(x.begin(), x.end(), y.begin());
	}

	template <class _T, class _Alloc>
	inline
		bool operator!=(const vector<_T, _Alloc>& x, const vector<_T, _Alloc>& y)
	{
		return !(x == y);
	}

	template <class _T, class _Alloc>
	inline
		bool operator<(const vector<_T, _Alloc>& x, const vector<_T, _Alloc>& y)
	{
		return lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
	}

	template <class _T, class _Alloc>
	inline
		bool operator>=(const vector<_T, _Alloc>& x, const vector<_T, _Alloc>& y)
	{
		return !(x < y);
	}

	template <class _T, class _Alloc>
	inline
		bool operator>(const vector<_T, _Alloc>& x, const vector<_T, _Alloc>& y)
	{
		return y < x;
	}

	template <class _T, class _Alloc>
	inline
		bool operator<=(const vector<_T, _Alloc>& x, const vector<_T, _Alloc>& y)
	{
		return !(y < x);
	}

	template <class _T, class _Alloc>
	inline void swap(vector<_T, _Alloc>& x, vector<_T, _Alloc>& y)
	{
		x.swap(y);
	}

	//拷贝赋值运算符
	template <class _T, class _Alloc>
	vector<_T, _Alloc>& vector<_T, _Alloc>::operator=(const vector<_T, _Alloc>& x)
	{
		if (&x != this)  //不是自赋值
		{
			if (x.size() > capacity()) // 如果目标物比我本身的容量capacity还大
			{
				iterator tmp = allocate_and_copy(x.end() - x.begin(), x.begin(), x.end());
				destroy(start, finish);	    // 把整个旧的vector 摧毁
				deallocate();			    // 释放旧空间
				start = tmp;				// 设定指向新空间
				end_of_storage = start + (x.end() - x.begin());
			}
			else if (size() >= x.size()) // 如果目标物大小 <= 我的大小size
			{
				iterator i = copy(x.begin(), x.end(), begin());
				destroy(i, finish);
			}
			else //我的size < 目标物大小 < 我的capacity
			{
				copy(x.begin(), x.begin() + size(), start);
				uninitialized_copy(x.begin() + size(), x.end(), finish);
			}
			finish = start + x.size();
		}
		return *this;
	}
	//插入辅助函数，是前面insert和push_back有可能调用
	template <class _T, class _Alloc>
	void vector<_T, _Alloc>::insert_aux(iterator position, const _T& x)
	{
		if (finish != end_of_storage) //还有备用空间
		{
			// 在备用空间起始处建构一个元素，并以vector 最后一个元素值为其初值。    
			construct(finish, *(finish - 1));
			// 调整水位。
			++finish;
			_T x_copy = x;
			copy_backward(position, finish - 2, finish - 1); //插入处之后整体后移一位
			*position = x_copy;
		}
		else // 已无备用空间
		{
			// 配置新空间
			// 配置原则：如果原大小为0，则配置 1（个元素大小）；
			// 如果原大小不为0，则配置原大小的两倍，
			// 前半段用来放置原资料，后半段准备用来放置新资料。
			const size_type old_size = size();
			const size_type len = old_size != 0 ? 2 * old_size : 1;

			iterator new_start = data_allocator::allocate(len); // 实际配置
			iterator new_finish = new_start;
			try
			{
				// 将原vector插入点之前的内容拷贝到新vector。
				new_finish = uninitialized_copy(start, position, new_start);
				// 为新元素设定初值x
				construct(new_finish, x);
				// 调整水位。
				++new_finish;
				// 将原vector插入点之后的内容拷贝到新vector。
				new_finish = uninitialized_copy(position, finish, new_finish);
			}
			catch (...)
			{
				// "commit or rollback" 
				destroy(new_start, new_finish);
				data_allocator::deallocate(new_start, len);
				throw;
			}
			// 解构并释放原 vector
			destroy(begin(), end());
			deallocate();
			// 调整迭代器，指向新vector
			start = new_start;
			finish = new_finish;
			end_of_storage = new_start + len;
		}
	}

	// 从 position 开始，安插 n 个元素，元素初值为 x
	template <class _T, class _Alloc>
	void vector<_T, _Alloc>::insert(iterator position, size_type n, const _T& x)
	{
		if (n != 0)// 当 n != 0  才进行以下所有动作
		{
			if (size_type(end_of_storage - finish) >= n)// 备用空间大于等于「新增元素个数」
			{
				_T x_copy = x;
				// 以下计算安插点之后的现有元素个数
				const size_type elems_after = finish - position;
				iterator old_finish = finish;
				if (elems_after > n)// 「安插点之后的现有元素个数」大于「新增元素个数」
				{
					uninitialized_copy(finish - n, finish, finish);
					finish += n;	// 将vector 尾端标记后移
					MySTL::copy_backward(position, old_finish - n, old_finish);
					MySTL::fill(position, position + n, x_copy);// 从安插点开始填入新值
				}
				else// 「安插点之后的现有元素个数」小于等于「新增元素个数」
				{
					uninitialized_fill_n(finish, n - elems_after, x_copy);
					finish += n - elems_after;
					uninitialized_copy(position, old_finish, finish);
					finish += elems_after;
					MySTL::fill(position, old_finish, x_copy);
				}
			}
			else// 备用空间小于「新增元素个数」（那就必须配置额外的内存）
			{
				// 首先决定新长度：旧长度的两倍，或旧长度+新增元素个数
				const size_type old_size = size();
				const size_type len = old_size + max(old_size, n);
				// 以下配置新的vector 空间
				iterator new_start = data_allocator::allocate(len);
				iterator new_finish = new_start;
				try
				{
					// 以下首先将旧vector 的安插点之前的元素复制到新空间
					new_finish = uninitialized_copy(start, position, new_start);
					// 以下再将新增元素（初值皆为 n）填入新空间
					new_finish = uninitialized_fill_n(new_finish, n, x);
					// 以下再将旧vector 的安插点之后的元素复制到新空间
					new_finish = uninitialized_copy(position, finish, new_finish);
				}
				catch (...)
				{
					// commit or rollback
					destroy(new_start, new_finish);
					data_allocator::deallocate(new_start, len);
					throw;
				}
				// 以下清除并释放旧的 vector 
				destroy(start, finish);
				deallocate();
				// 以下调整水位标记
				start = new_start;
				finish = new_finish;
				end_of_storage = new_start + len;
			}
		}
	}



	template <class _T, class _Alloc> template <class _InputIterator>
	void vector<_T, _Alloc>::range_insert(iterator pos, _InputIterator first,
		_InputIterator last, input_iterator_tag)
	{
		for (; first != last; ++first)
		{
			pos = insert(pos, *first);
			++pos;
		}
	}

	template <class _T, class _Alloc> template <class _ForwardIterator>
	void vector<_T, _Alloc>::range_insert(iterator position, _ForwardIterator first,
		_ForwardIterator last, forward_iterator_tag)
	{
		if (first != last)
		{
			size_type n = 0;
			distance(first, last, n);
			if (size_type(end_of_storage - finish) >= n)
			{
				const size_type elems_after = finish - position;
				iterator old_finish = finish;
				if (elems_after > n)
				{
					uninitialized_copy(finish - n, finish, finish);
					finish += n;
					copy_backward(position, old_finish - n, old_finish);
					copy(first, last, position);
				}
				else
				{
					_ForwardIterator mid = first;
					advance(mid, elems_after);
					uninitialized_copy(mid, last, finish);
					finish += n - elems_after;
					uninitialized_copy(position, old_finish, finish);
					finish += elems_after;
					copy(first, mid, position);
				}
			}
			else
			{
				const size_type old_size = size();
				const size_type len = old_size + max(old_size, n);
				iterator new_start = data_allocator::allocate(len);
				iterator new_finish = new_start;
				try
				{
					new_finish = uninitialized_copy(start, position, new_start);
					new_finish = uninitialized_copy(first, last, new_finish);
					new_finish = uninitialized_copy(position, finish, new_finish);
				}
				catch (...)
				{
					destroy(new_start, new_finish);
					data_allocator::deallocate(new_start, len);
					throw;
				}

				destroy(start, finish);
				deallocate();
				start = new_start;
				finish = new_finish;
				end_of_storage = new_start + len;
			}
		}
	}
}// end of MyStl
#endif  //end of VECTOR_H_
