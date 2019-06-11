/*
*list的实现
*/
#ifndef LIST_H_
#define LIST_H_

#include "allocator.h"
#include "iterator.h"
#include <initializer_list>
namespace MySTL {
	//节点结构
	template<class _T>
	struct _list_node {
		_list_node<_T>* prev; //指向前一个节点
		_list_node<_T>* next; //指向后一个节点
		_T data;              //数据
	};
	//list迭代器
	template<class _T, class _Ref, class _Ptr>
	struct _list_iterator  {
		typedef _list_iterator<_T, _T&, _T*>             iterator;//迭代器
		typedef _list_iterator<_T, const _T&, const _T*> const_iterator;//常量迭代器
		typedef _list_iterator<_T, _Ref, _Ptr>           self;//引用
		//没有继承迭代器的泛化接口，所以需要自己定义下面几种类型。
		typedef bidirectional_iterator_tag   iterator_category;
		typedef _T                           value_type;
		typedef _Ptr                         pointer;
		typedef _Ref                         reference;
		typedef _list_node<_T>*              link_type;
		typedef size_t                       size_type;
		typedef ptrdiff_t                    difference_type;
		
		link_type node;     //其实就是指向节点的指针

		// 迭代器的构造函数
		//如有参数，便根据参数设定迭代器与容器之间的联结关系
		_list_iterator(link_type x) : node(x) {}
		_list_iterator() {}
		_list_iterator(const iterator& x) : node(x.node) {}

		//迭代器操作
		bool operator==(const self&x) const { return node == x.node };
		bool operator!=(const self& x) const { return node != x.node; }
		//对迭代器取值
		reference operator*() const { return (*node).data; }
		pointer operator->() const { return &(operator*()); }
		//对迭代器累加就是前进一个节点
		self& operator++() {//前置++
			node = (link_type)((*node).next);
			return *this;
		}
		self operator++(int) {//后置++
			self temp = *this;
			++*this;          //这个++其实是重载的上面的++，但是*并不是重载的上面的*
			return temp;
		}

		// 对迭代器累减1，就是后退一个节点
		self& operator--()
		{
			node = (link_type)((*node).prev);
			return *this;
		}
		self operator--(int)
		{
			self tmp = *this;
			--*this;
			return tmp;
		}
	};
	//至此迭代器设计完成
	
	//以下为list的设计
	template <class _T, class _Alloc = alloc> // 默认使用 alloc 为配置器
	class list {
		protected:
			typedef void*                             void_pointer;
			typedef _list_node<_T>                    list_node;
			// 专属配置器，每次配置一个节点大小
			typedef simple_alloc<list_node, _Alloc>   list_node_allocator;
	public:
		typedef _T                       value_type;
		typedef value_type*              pointer;
		typedef const value_type*        const_pointer;
		typedef value_type&              reference;
		typedef const value_type&        const_reference;
		typedef list_node*               link_type;
		typedef size_t                   size_type;
		typedef ptrdiff_t                difference_type;
	public:
		typedef _list_iterator<_T, _T&, _T*>             iterator;
		typedef _list_iterator<_T, const _T&, const _T*> const_iterator;
	protected:
		//配置一个节点，返回其指针,使用的是二级配置器
		link_type get_node() { return list_node_allocator::allocate(); }
		//释放一个节点
		void put_node(link_type p) { return list_node_allocator::deallocate(p); }
		//配置一个带有元素值的节点
		link_type creat_node(const _T& x) {
			link_type p = get_node();
			try {
				//construct接受一个指针和一个初值，将初值设定到指针所指的空间上
				construct(&p->data, x);
			}
			catch(...){
				put_node(p);
				throw;
			}
			return p;
		}
		//释放一个节点，并摧毁对象
		void destroy_node(link_type p) {
			destroy(&p->data);
			put_node(p);
		}
	protected:
		//以下为三个初始化函数
		void empty_initialize()   //空链表
		{
			node = get_node();	// 配置一个节点空间，令 node 指向它。
			node->next = node;	// 令node 头尾都指向自己，不设元素值。
			node->prev = node;
		}
		//初始化为n个value元素
		void fill_initialize(size_type n, const _T& value)
		{
			empty_initialize();
			try
			{
				insert(begin(), n, value);
			}
			catch (...)
			{
				clear();
				put_node(node);
				throw;
			}
		}
		//初始化两个迭代器范围内的链表
		template <class _InputIterator>
		void range_initialize(_InputIterator first, _InputIterator last)
		{
			empty_initialize();
			try
			{
				insert(begin(), first, last);
			}
			catch (...)
			{
				clear();
				put_node(node);
				throw;
			}
		}
	protected:
		link_type node; // 永远指向尾后节点。该节点无元素值，代表空节点。
						// 其 next 节点永远是头节点。这样只需要一个指针就可以表示整个链表
	public:
		//构造函数
		list() { empty_initialize(); }  // 产生一个空链表。
		iterator begin() { return (link_type)((*node).next); }
		const_iterator begin() const { return (link_type)((*node).next); }
		iterator end() { return node; }// node 指向尾节点的下一位置
		const_iterator end() const { return node; }
		bool empty() const { return node->next == node; }
		size_type size() const {
			size_type result = size_type(distance(begin(), end()));//全局函数
			return result;
		}
		size_type max_size() const { return size_type(-1); } //2^32

		//取头结点的值
		reference front() { return *begin(); }
		const_reference front() const {return *begin()}
		//取尾节点的值
		reference back() { return *(--end());}
		const_reference back() const { return *(--end()); }
		//交换
		void swap(list<_T, _Alloc> &x) { std::swap(node, x.node); }
		
		//以下为insert的多个重载形式
		//在迭代器position所指节点的前面位置插入一个节点，值为x,
		iterator insert(iterator position, const _T& x) {
			link_type tmp = creat_node(x); //创建一个节点，值为x
			tmp->next = position.node;
			tmp->prev = position.node->prev;
			(link_type)(position.node->prev)->next = tmp;
			position.node->prev = tmp;
			return tmp;
		}

		// 在迭代器 position 所指位置之前安插一个节点，值为list值类型的默认值
		iterator insert(iterator position) { return insert(position, _T()); }
		//在迭代器 position 所指位置之前插入范围[first,last)内的值，在后面实现
		template <class _InputIterator>
		void insert(iterator position, _InputIterator first, _InputIterator last);
		// 在迭代器 position 所指位置之前安插n个节点，值为 x，在后面实现
		void insert(iterator pos, size_type n, const _T& x);

		void insert(iterator pos, int n, const _T& x)
		{
			insert(pos, (size_type)n, x);
		}
		void insert(iterator pos, long n, const _T& x)
		{
			insert(pos, (size_type)n, x);
		}

		//安插一个节点作为头结点
		void push_front(const _T& x) { insert(begin(), x); }

		// 安插一个节点，做为尾节点
		void push_back(const _T& x) { insert(end(), x); }

		// 移除迭代器 position 所指节点
		iterator erase(iterator position)
		{
			link_type next_node = link_type(position.node->next);
			link_type prev_node = link_type(position.node->prev);
			prev_node->next = next_node;
			next_node->prev = prev_node;
			destroy_node(position.node);
			return iterator(next_node);
		}
		//移除范围内迭代器，在后面实现
		iterator erase(iterator first, iterator last);

		//重新调整链表大小，new_size比原来大，扩充至new_size，并填以值x
		//new_size比原来小，缩减至new_size，摧毁靠后节点，三个函数都在后面实现
		void resize(size_type new_size, const _T& x);
		void resize(size_type new_size) { resize(new_size, _T()); }
		void clear();

		//移除头结点
		void pop_front() { erase(begin()); }
		//移除尾节点，end()其实是指向那个空节点的。
		void pop_back()
		{
			iterator tmp = end();
			erase(--tmp);
		}

		//构造函数，都使用fill_initialize来构造，因为不是默认类型，所以不能使用全局的初始化函数
		list(size_type n, const _T& value) { fill_initialize(n, value); }
		list(int n, const _T& value) { fill_initialize(n, value); }
		list(long n, const _T& value) { fill_initialize(n, value); }
		explicit list(size_type n) { fill_initialize(n, _T()); }
		//范围构造，
		template <class _InputIterator>
		list(_InputIterator first, _InputIterator last)
		{
			range_initialize(first, last);
		}

		list(const list<_T, _Alloc>& x)
		{
			range_initialize(x.begin(), x.end());
		}

		//列表构造
		list(const std::initializer_list<_T>& l)
		{
			range_initialize(l.begin(), l.end());
		}

		//析构函数
		~list()
		{
			clear();
			put_node(node);
		}
		//拷贝赋值构造函数
		list<_T, _Alloc>& operator=(const list<_T, _Alloc>& x);
	protected:
		// 将 [first,last) 内的所有元素搬移到position 处。是splice、sort、merge的基础
		void transfer(iterator position, iterator first, iterator last)
		{
			if (position != last) {
				(*(link_type((*last.node).prev))).next = position.node;
				(*(link_type((*first.node).prev))).next = last.node;
				(*(link_type((*position.node).prev))).next = first.node;
				link_type tmp = link_type((*position.node).prev);
				(*position.node).prev = (*last.node).prev;
				(*last.node).prev = (*first.node).prev;
				(*first.node).prev = tmp;
			}
		}

		public:
			// 将 x 接合于 position 所指位置之前
			// x 必须不同于 *this。
			void splice(iterator position, list& x)
			{
				if (!x.empty())
					transfer(position, x.begin(), x.end());
			}

			// 将 i 所指元素接合于 position 所指位置之前
			// position 和i 可指向同一个list。
			void splice(iterator position, list&, iterator i)
			{
				iterator j = i;
				++j;
				if (position == i || position == j) return;
				transfer(position, i, j);
			}

			// 将 [first,last) 内的所有元素接合于 position 所指位置之前
			// position 和[first,last)可指向同一个list
			// 但position不能位于[first,last)之内
			void splice(iterator position, list&, iterator first, iterator last)
			{
				if (first != last)
					transfer(position, first, last);
			}

			void remove(const _T& value);
			void unique();
			void merge(list& x);
			void reverse();
			void sort();
			template <class _Predicate> void remove_if(_Predicate);
			template <class _BinaryPredicate> void unique(_BinaryPredicate);
			template <class _StrictWeakOrdering> void merge(list&, _StrictWeakOrdering);
			template <class _StrictWeakOrdering> void sort(_StrictWeakOrdering);
			friend bool operator==(const list& x, const list& y);
	};

	template <class _T, class _Alloc>
	inline
		bool operator==(const list<_T, _Alloc>& x, const list<_T, _Alloc>& y)
	{
		typedef typename list<_T, _Alloc>::link_type link_type;
		link_type e1 = x.node;
		link_type e2 = y.node;
		link_type n1 = (link_type)e1->next;
		link_type n2 = (link_type)e2->next;
		for (; n1 != e1 && n2 != e2;
			n1 = (link_type)n1->next, n2 = (link_type)n2->next)
			if (n1->data != n2->data)
				return false;
		return n1 == e1 && n2 == e2;
	}

	template <class _T, class _Alloc>
	inline bool operator<(const list<_T, _Alloc>& x, const list<_T, _Alloc>& y)
	{
		return lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());
	}

	template <class _T, class _Alloc>
	inline void swap(list<_T, _Alloc>& x, list<_T, _Alloc>& y)
	{
		x.swap(y);
	}
	//insert实现
	template <class _T, class _Alloc> template <class _InputIterator>
	void list<_T, _Alloc>::insert(iterator position,
		_InputIterator first, _InputIterator last)
	{
		for (; first != last; ++first)
			insert(position, *first);
	}
	//insert实现
	template <class _T, class _Alloc>
	void list<_T, _Alloc>::insert(iterator position, size_type n, const _T& x)
	{
		for (; n > 0; --n)
			insert(position, x);
	}
	//erase实现
	template <class _T, class _Alloc>
	typename list<_T, _Alloc>::iterator list<_T, _Alloc>::erase(iterator first, iterator last)
	{
		while (first != last) erase(first++);
		return last;
	}
	
	//resize实现
	template <class _T, class _Alloc>
	void list<_T, _Alloc>::resize(size_type new_size, const _T& x)
	{
		iterator i = begin();
		size_type len = 0;
		for (; i != end() && len < new_size; ++i, ++len)
			;
		if (len == new_size)
			erase(i, end());
		else                          // i == end()
			insert(end(), new_size - len, x);
	}
	// 清除所有节点（整个链表）
	template <class _T, class _Alloc>
	void list<_T, _Alloc>::clear()
	{
		link_type cur = (link_type)node->next; // begin()
		while (cur != node)
		{	// 巡访每一个节点
			link_type tmp = cur;
			cur = (link_type)cur->next;
			destroy_node(tmp); 	// 摧毁（解构并释放）一个节点
		}
		// 恢复 node 原始状态
		node->next = node;
		node->prev = node;
	}

	template <class _T, class _Alloc>
	list<_T, _Alloc>& list<_T, _Alloc>::operator=(const list<_T, _Alloc>& rhs)
	{
		if (this != &rhs) //不是自赋值
		{
			iterator first1 = begin();
			iterator last1 = end();
			const_iterator first2 = rhs.begin();
			const_iterator last2 = rhs.end();
			while (first1 != last1 && first2 != last2) *first1++ = *first2++;
			if (first2 == last2)      //右边链表较短
				erase(first1, last1); //清除左边多余节点
			else //左边链表较短
				insert(last1, first2, last2); //插入右边多出的节点
		}
		return *this;
	}
	// 将数值为 value 之所有元素移除
	template <class _T, class _Alloc>
	void list<_T, _Alloc>::remove(const _T& value)
	{
		iterator first = begin();
		iterator last = end();
		while (first != last)
		{	// 巡访每一个节点
			iterator next = first;
			++next;
			if (*first == value) erase(first); 	// 找到就移除
			first = next;
		}
	}

	// 移除数值相同的连续元素
	template <class _T, class _Alloc>
	void list<_T, _Alloc>::unique()
	{
		iterator first = begin();
		iterator last = end();
		if (first == last) return;
		iterator next = first;
		while (++next != last)
		{
			if (*first == *next)
				erase(next);
			else
				first = next;
			next = first;
		}
	}
	// 将 x 合并到 *this 身上。两个 lists 的内容都必须先经过升序。
	template <class _T, class _Alloc>
	void list<_T, _Alloc>::merge(list<_T, _Alloc>& x)
	{
		iterator first1 = begin();
		iterator last1 = end();
		iterator first2 = x.begin();
		iterator last2 = x.end();

		// 注意：前提是，两个 lists 都已经过升序，
		while (first1 != last1 && first2 != last2)
			if (*first2 < *first1)
			{
				iterator next = first2;
				transfer(first1, first2, ++next);
				first2 = next;
			}
			else
				++first1;
		if (first2 != last2) transfer(last1, first2, last2);
	}

	// 将 *this 的内容逆向重置
	template <class _T, class _Alloc>
	void list<_T, _Alloc>::reverse()
	{
		// 以下判断，如果是空白串行，或仅有一个元素，就不做任何动作。
		// 使用 size() == 0 || size() == 1 来判断，虽然也可以，但是比较慢。
		if (node->next == node || link_type(node->next)->next == node) return;
		iterator first = begin();
		++first;
		while (first != end()) {
			iterator old = first;
			++first;
			transfer(begin(), old, first);
		}
	}
	// list 不能使用算法sort()，必须使用自己的sort() member function，
	// 因为算法sort()只接受RamdonAccessIterator. 
	// 本函数采用 quick sort. 
	template <class _T, class _Alloc>
	void list<_T, _Alloc>::sort()
	{
		// 以下判断，如果是空链表，或仅有一个元素，就不做任何动作。
		if (node->next == node || link_type(node->next)->next == node) return;

		// 一些新的 lists，做为中介数据存放区
		list<_T, _Alloc> carry;
		list<_T, _Alloc> counter[64];
		int fill = 0;
		while (!empty())
		{
			carry.splice(carry.begin(), *this, begin());
			int i = 0;
			while (i < fill && !counter[i].empty())
			{
				counter[i].merge(carry);
				carry.swap(counter[i++]);
			}
			carry.swap(counter[i]);
			if (i == fill) ++fill;
		}

		for (int i = 1; i < fill; ++i)
			counter[i].merge(counter[i - 1]);
		swap(counter[fill - 1]);
	}
	template <class _T, class _Alloc> template <class _Predicate>
	void list<_T, _Alloc>::remove_if(_Predicate pred)
	{
		iterator first = begin();
		iterator last = end();
		while (first != last) {
			iterator next = first;
			++next;
			if (pred(*first)) erase(first);
			first = next;
		}
	}

	template <class _T, class _Alloc> template <class _BinaryPredicate>
	void list<_T, _Alloc>::unique(_BinaryPredicate binary_pred) {
		iterator first = begin();
		iterator last = end();
		if (first == last) return;
		iterator next = first;
		while (++next != last) {
			if (binary_pred(*first, *next))
				erase(next);
			else
				first = next;
			next = first;
		}
	}

	template <class _T, class _Alloc> template <class _StrictWeakOrdering>
	void list<_T, _Alloc>::merge(list<_T, _Alloc>& x, _StrictWeakOrdering comp)
	{
		iterator first1 = begin();
		iterator last1 = end();
		iterator first2 = x.begin();
		iterator last2 = x.end();
		while (first1 != last1 && first2 != last2)
			if (comp(*first2, *first1)) {
				iterator next = first2;
				transfer(first1, first2, ++next);
				first2 = next;
			}
			else
				++first1;
		if (first2 != last2) transfer(last1, first2, last2);
	}

	template <class _T, class _Alloc> template <class _StrictWeakOrdering>
	void list<_T, _Alloc>::sort(_StrictWeakOrdering comp) {
		if (node->next == node || link_type(node->next)->next == node) return;
		list<_T, _Alloc> carry;
		list<_T, _Alloc> counter[64];
		int fill = 0;
		while (!empty()) {
			carry.splice(carry.begin(), *this, begin());
			int i = 0;
			while (i < fill && !counter[i].empty()) {
				counter[i].merge(carry, comp);
				carry.swap(counter[i++]);
			}
			carry.swap(counter[i]);
			if (i == fill) ++fill;
		}

		for (int i = 1; i < fill; ++i) counter[i].merge(counter[i - 1], comp);
		swap(counter[fill - 1]);
	}

}//end of MySTL
#endif//end of LIST_H