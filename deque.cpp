/*deque的实现
*
*
*
*/
#ifndef DEQUE_H_
#define DEQUE_H_
namespace MySTL {
	//全局函数，buffer_size用它来设定缓冲区的大小
	//如果n不为0传回n，表示buff size由用户定义，如果为0,表示使用默认值
	//那么如果sz小于512传回512/sz，如果sz不小于512传回1
	inline size_t _deque_buf_size(size_t n, size_t sz)
	{
		return n != 0 ? n : (sz < 512 ? size_t(512 / sz) : size_t(1));
	}
	//迭代器的实现
	template <class _T, class _Ref, class _Ptr>
	struct _deque_iterator {
		typedef _deque_iterator<_T, _T&, _T*>                 iterator;
		typedef _deque_iterator<_T, const _T&, const _T*>     const_iterator;
		//使用默认缓存区大小
		static size_t buffer_size() { return _deque_buf_size(0, sizeof(_T)); }
		// 未继承 std::iterator，所以必须自行撰写五个必要的迭代器相应型别
		typedef random_access_iterator_tag     iterator_category;
		typedef _T                             value_type;
		typedef _Ptr                           pointer;
		typedef _Ref                           reference;
		typedef size_t                         size_type;
		typedef ptrdiff_t                      difference_type;
		typedef _T**                           map_pointer;
		typedef _deque_iterator                self;

		// 保持与容器的联结
		_T* cur;	// 此迭代器所指之缓冲区中的当前(current）元素
		_T* first;	// 此迭代器所指之缓冲区的头
		_T* last;	// 此迭代器所指之缓冲区的尾（含备用空间）
		map_pointer node; //指向控制中心

		//迭代器构造函数
		_deque_iterator(_T* x, map_pointer y)
			: cur(x), first(*y), last(*y + buffer_size()), node(y) {}
		_deque_iterator() : cur(0), first(0), last(0), node(0) {}
		_deque_iterator(const iterator& x)
			: cur(x.cur), first(x.first), last(x.last), node(x.node) {}

		//迭代器操作
		reference operator*() const { return *cur };
		pointer operator->() const { return &(operator*()); }
		//迭代器之间距离，指间隔的元素数量
		difference_type operator-(const self& x) const
		{
			return difference_type(buffer_size()) * (node - x.node - 1) +
				(cur - first) + (x.last - x.cur);
		}

		self& operator++()   //前置
		{
			++cur;				     // 切换至下一个元素。
			if (cur == last)         // 如果已达所在缓冲区的尾端，
			{
				set_node(node + 1);	 // 就切换至下一个节点（亦即缓冲区）
				cur = first;	     //  的第一个元素。
			}
			return *this;
		}
		self operator++(int) //后置
		{
			self tmp = *this;
			++*this;
			return tmp;
		}

		self& operator--()
		{
			if (cur == first)       // 如果已达所在缓冲区的头端，
			{
				set_node(node - 1);	// 就切换至前一个节点（亦即缓冲区）
				cur = last;			// 的最后一个元素。
			}
			--cur;				    // 切换至前一个元素。
			return *this;
		}
		self operator--(int)
		{
			self tmp = *this;
			--*this;
			return tmp;
		}
		self& operator+=(difference_type n)
		{
			difference_type offset = n + (cur - first);
			// 目标位置在同一缓冲区内
			if (offset >= 0 && offset < difference_type(buffer_size()))
				cur += n;
			else // 目标位置不在同一缓冲区内
			{
				difference_type node_offset =
					offset > 0 ? offset / difference_type(buffer_size())
					: -difference_type((-offset - 1) / buffer_size()) - 1;
				// 切换至正确的节点（亦即缓冲区）
				set_node(node + node_offset);
				// 切换至正确的元素
				cur = first + (offset - node_offset * difference_type(buffer_size()));
			}
			return *this;
		}
		self operator+(difference_type n) const
		{
			self tmp = *this;
			return tmp += n; // operator+=
		}
		self& operator-=(difference_type n) { return *this += -n; }
		//迭代器的后移
		self operator-(difference_type n) const
		{
			self tmp = *this;
			return tmp -= n; //operator-=
		}

		reference operator[](difference_type n) const { return *(*this + n); }

		bool operator==(const self& x) const { return cur == x.cur; }
		bool operator!=(const self& x) const { return !(*this == x); }
		bool operator<(const self& x) const
		{
			return (node == x.node) ? (cur < x.cur) : (node < x.node);
		}
		bool operator>=(const self& x) const
		{
			return !(*this > x);
		}
		bool operator>(const self& x) const
		{
			return x < *this;
		}
		bool operator<=(const self& x) const
		{
			return !(x < *this);
		}

		explicit operator bool() const
		{
			return cur != 0;
		}

		//跳到新缓冲区
		void set_node(map_pointer new_node)
		{
			node = new_node;
			first = *new_node;
			last = first + difference_type(buffer_size());
		}
	};

	// 默认使用 alloc 为配置器
	template <class _T, class _Alloc = alloc, size_t _BufSiz = 0>
	class deque
	{
	public:
		typedef _T                  value_type;
		typedef value_type*         pointer;
		typedef const value_type*   const_pointer;
		typedef value_type&         reference;
		typedef const value_type&   const_reference;
		typedef size_t              size_type;
		typedef ptrdiff_t           difference_type;
		typedef _deque_iterator<_T, _T&, _T*>                iterator;
		typedef _deque_iterator<_T, const _T&, const _T*>    const_iterator;

	protected:
		// 元素的指针的指针
		typedef pointer* map_pointer;
		// 专属空间配置器，每次配置一个元素大小
		typedef simple_alloc<value_type, _Alloc> data_allocator;
		// 专属空间配置器，每次配置一个指针大小
		typedef simple_alloc<pointer, _Alloc> map_allocator;

		static size_type buffer_size()
		{
			return _deque_buf_size(_BufSiz, sizeof(value_type));
		}
		static size_type initial_map_size() { return size_type(8); }

	protected:// Data members
		iterator start;		// 表现第一个节点。
		iterator finish;	// 表现最后一个节点。
		map_pointer map;	// 指向map，map是块连续空间，
							// 其内的每个元素都是一个指针（称为节点），指向一块缓冲区。
		size_type map_size;	// map内可容纳多少指标。

	public:
		iterator begin() { return start; }
		iterator end() { return finish; }
		const_iterator begin() const { return start; }
		const_iterator end() const { return finish; }
		reference operator[](size_type n)
		{
			return start[difference_type(n)]; //_deque_iterator<>::operator[]
		}
		const_reference operator[](size_type n) const
		{
			return start[difference_type(n)];
		}
		reference front() { return *start; } //_deque_iterator<>::operator*
		reference back()
		{
			iterator tmp = finish;
			--tmp;	             //_deque_iterator<>::operator--
			return *tmp;
		}
		const_reference front() const { return *start; }
		const_reference back() const
		{
			const_iterator tmp = finish;
			--tmp;
			return *tmp;
		}

		size_type size() const { return finish - start; }
		size_type max_size() const { return size_type(-1); }
		bool empty() const { return finish == start; }

	public:
		//构造函数
		//start(), finish()调用迭代器的默认构造函数
		deque() : start(), finish(), map(0), map_size(0)
		{
			create_map_and_nodes(0);
		}

		deque(const deque& x)
			: start(), finish(), map(0), map_size(0)
		{
			create_map_and_nodes(x.size());
			try
			{
				uninitialized_copy(x.begin(), x.end(), start);
			}
			catch (...)
			{
				destroy_map_and_nodes();
				throw;
			}
		}

		deque(size_type n, const value_type& value)
			: start(), finish(), map(0), map_size(0)
		{
			fill_initialize(n, value);
		}
		deque(int n, const value_type& value)
			: start(), finish(), map(0), map_size(0)
		{
			fill_initialize(n, value);
		}
		deque(long n, const value_type& value)
			: start(), finish(), map(0), map_size(0)
		{
			fill_initialize(n, value);
		}

		explicit deque(size_type n)
			: start(), finish(), map(0), map_size(0)
		{
			fill_initialize(n, value_type());
		}
		template <class _InputIterator>
		deque(_InputIterator first, _InputIterator last)
			: start(), finish(), map(0), map_size(0)
		{
			range_initialize(first, last, iterator_category(first));
		}

		deque(const std::initializer_list<_T>& il) : start(), finish(),
			map(0), map_size(0)
		{
			range_initialize(il.begin(), il.end(), forward_iterator_tag());
		}

		//析构函数
		~deque()
		{
			destroy(start, finish);
			destroy_map_and_nodes();
		}

		deque& operator= (const deque& x)
		{
			const size_type len = size();
			if (&x != this)
			{
				if (len >= x.size())
					erase(copy(x.begin(), x.end(), start), finish);
				else
				{
					const_iterator mid = x.begin() + difference_type(len);
					copy(x.begin(), mid, start);
					insert(finish, mid, x.end());
				}
			}
			return *this;
		}

		void swap(deque& x)
		{
			std::swap(start, x.start);
			std::swap(finish, x.finish);
			std::swap(map, x.map);
			std::swap(map_size, x.map_size);
		}

		public:
			void push_back(const value_type& t)
			{
				if (finish.cur != finish.last - 1)// 最后缓冲区尚有一个以上的备用空间
				{
					construct(finish.cur, t);// 直接在备用空间上建构元素
					++finish.cur;// 调整最后缓冲区的使用状态
				}
				else // 最后缓冲区已无（或只剩一个）元素备用空间。
					push_back_aux(t);
			}

			void push_front(const value_type& t)
			{
				if (start.cur != start.first)// 第一缓冲区尚有备用空间
				{
					construct(start.cur - 1, t); // 直接在备用空间上建构元素
					--start.cur;// 调整第一缓冲区的使用状态
				}
				else // 第一缓冲区已无备用空间
					push_front_aux(t);
			}

			void pop_back()
			{
				if (finish.cur != finish.first)// 最后缓冲区有一个（或更多）元素
				{
					--finish.cur;		// 调整指针，相当于排除了最后元素
					destroy(finish.cur);// 将最后元素解构
				}
				else // 最后缓冲区没有任何元素
					pop_back_aux();	// 这里将进行缓冲区的释放工作
			}

			void pop_front()
			{
				if (start.cur != start.last - 1)// 第一缓冲区有一个（或更多）元素
				{
					destroy(start.cur);	// 将第一元素解构
					++start.cur;// 调整指针，相当于排除了第一元素
				}
				else// 第一缓冲区仅有一个元素
					pop_front_aux();// 这里将进行缓冲区的释放工作
			}

	public:
		// 在position 处安插一个元素，其值为 x
		iterator insert(iterator position, const value_type& x)
		{
			if (position.cur == start.cur) // 如果安插点是deque 最前端
			{
				push_front(x);				// 交给push_front 去做
				return start;
			}
			else if (position.cur == finish.cur)// 如果安插点是deque 最尾端
			{
				push_back(x);					  // 交给push_back 去做
				iterator tmp = finish;
				--tmp;
				return tmp;
			}
			else  //安插点在中间区域
			{
				return insert_aux(position, x);	 // 交给 insert_aux 去做
			}
		}

		iterator insert(iterator position) { return insert(position, value_type()); }

		void insert(iterator pos, size_type n, const value_type& x);
		void insert(iterator pos, int n, const value_type& x)
		{
			insert(pos, (size_type)n, x);
		}
		void insert(iterator pos, long n, const value_type& x)
		{
			insert(pos, (size_type)n, x);
		}

		template <class _InputIterator>
		void insert(iterator pos, _InputIterator first, _InputIterator last)
		{
			insert(pos, first, last, iterator_category(first));
		}

	public:
		void resize(size_type new_size, const value_type& x)
		{
			const size_type len = size();
			if (new_size < len)
				erase(start + new_size, finish);
			else
				insert(finish, new_size - len, x);
		}
		void resize(size_type new_size) { resize(new_size, value_type()); }

	public:
		// 清除 pos 所指的元素。pos 为清除点。
		iterator erase(iterator pos)
		{
			iterator next = pos;
			++next;
			difference_type index = pos - start;	// 清除点之前的元素个数
			if (index < difference_type(size() >> 1))// 如果清除点之前的元素比较少，
			{
				copy_backward(start, pos, next);// 就搬移清除点之前的元素
				pop_front();// 搬移完毕，最前一个元素赘余，去除之
			}
			else // 清除点之后的元素比较少，
			{
				copy(next, finish, pos);// 就搬移清除点之后的元素
				pop_back(); // 搬移完毕，最后一个元素赘余，去除之
			}
			return start + index;
		}

		iterator erase(iterator first, iterator last);

		void clear();

	protected:
		//构造\析构的辅助函数
		void create_map_and_nodes(size_type num_elements);
		void destroy_map_and_nodes();
		void fill_initialize(size_type n, const value_type& value);

		template <class _InputIterator>
		void range_initialize(_InputIterator first, _InputIterator last,
			input_iterator_tag);

		template <class _ForwardIterator>
		void range_initialize(_ForwardIterator first, _ForwardIterator last,
			forward_iterator_tag);

	protected:
		// push/pop操作的辅助函数
		void push_back_aux(const value_type& t);
		void push_front_aux(const value_type& t);
		void pop_back_aux();
		void pop_front_aux();

	protected:
		// 插入操作辅助函数
		template <class _InputIterator>
		void insert(iterator pos, _InputIterator first, _InputIterator last,
			input_iterator_tag);

		template <class _ForwardIterator>
		void insert(iterator pos, _ForwardIterator first, _ForwardIterator last,
			forward_iterator_tag);

		iterator insert_aux(iterator pos, const value_type& x);
		void insert_aux(iterator pos, size_type n, const value_type& x);

		template <class _ForwardIterator>
		void insert_aux(iterator pos, _ForwardIterator first, _ForwardIterator last,
			size_type n);

		iterator reserve_elements_at_front(size_type n)
		{
			size_type vacancies = start.cur - start.first;
			if (n > vacancies)
				new_elements_at_front(n - vacancies);
			return start - difference_type(n);
		}

		iterator reserve_elements_at_back(size_type n)
		{
			size_type vacancies = (finish.last - finish.cur) - 1;
			if (n > vacancies)
				new_elements_at_back(n - vacancies);
			return finish + difference_type(n);
		}

		void new_elements_at_front(size_type new_elements);
		void new_elements_at_back(size_type new_elements);

		void destroy_nodes_at_front(iterator before_start);
		void destroy_nodes_at_back(iterator after_finish);

	protected:
		//map和节点的分配
		void reserve_map_at_back(size_type nodes_to_add = 1)
		{
			if (nodes_to_add + 1 > map_size - (finish.node - map))
				// 如果 map 尾端的节点备用空间不足
				// 符合以上条件则必须重换一个map（配置更大的，拷贝原来的，释放原来的）
				reallocate_map(nodes_to_add, false);
		}

		void reserve_map_at_front(size_type nodes_to_add = 1)
		{
			if (nodes_to_add > size_type(start.node - map))
				// 如果 map 前端的节点备用空间不足
				// 符合以上条件则必须重换一个map（配置更大的，拷贝原来的，释放原来的）
				reallocate_map(nodes_to_add, true);
		}

		void reallocate_map(size_type nodes_to_add, bool add_at_front);

		pointer allocate_node() { return data_allocator::allocate(buffer_size()); }
		void deallocate_node(pointer n)
		{
			data_allocator::deallocate(n, buffer_size());
		}


	public:
		bool operator==(const deque<_T, _Alloc, 0>& x) const
		{
			return size() == x.size() && equal(begin(), end(), x.begin());
		}
		bool operator!=(const deque<_T, _Alloc, 0>& x) const
		{
			return size() != x.size() || !equal(begin(), end(), x.begin());
		}
		bool operator<(const deque<_T, _Alloc, 0>& x) const
		{
			return lexicographical_compare(begin(), end(), x.begin(), x.end());
		}
		bool operator>=(const deque<_T, _Alloc, 0>& x) const
		{
			return !(*this < x);
		}
		bool operator>(const deque<_T, _Alloc, 0>& x) const
		{
			return x < *this;
		}
		bool operator<=(const deque<_T, _Alloc, 0>& x) const
		{
			return !(x < *this);
		}
	};

	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::insert(iterator pos,
		size_type n, const value_type& x)
	{
		if (pos.cur == start.cur)
		{
			iterator new_start = reserve_elements_at_front(n);
			uninitialized_fill(new_start, start, x);
			start = new_start;
		}
		else if (pos.cur == finish.cur) {
			iterator new_finish = reserve_elements_at_back(n);
			uninitialized_fill(finish, new_finish, x);
			finish = new_finish;
		}
		else
			insert_aux(pos, n, x);
	}

	template <class _T, class _Alloc, size_t _BufSize>
	typename deque<_T, _Alloc, _BufSize>::iterator
		deque<_T, _Alloc, _BufSize>::erase(iterator first, iterator last)
	{
		if (first == start && last == finish) // 如果清除区间就是整个 deque
		{
			clear();							// 直接呼叫 clear() 即可
			return finish;
		}
		else
		{
			difference_type n = last - first;			// 清除区间的长度
			difference_type elems_before = first - start;// 清除区间前方的元素个数
			if (elems_before < (size() - n) / 2)// 如果前方的元素比较少，
			{
				copy_backward(start, first, last);		// 向后搬移前方元素（覆盖清除区间）
				iterator new_start = start + n;			// 标记 deque 的新起点
				destroy(start, new_start);				// 搬移完毕，将赘余的元素解构
														// 以下将赘余的缓冲区释放
				for (map_pointer cur = start.node; cur < new_start.node; ++cur)
					data_allocator::deallocate(*cur, buffer_size());
				start = new_start;	// 设定 deque 的新起点
			}
			else // 如果清除区间后方的元素比较少
			{
				copy(last, finish, first);			// 向前搬移后方元素（覆盖清除区间）
				iterator new_finish = finish - n;	// 标记 deque 的新尾点
				destroy(new_finish, finish);		// 搬移完毕，将赘余的元素解构
													// 以下将赘余的缓冲区释放
				for (map_pointer cur = new_finish.node + 1; cur <= finish.node; ++cur)
					data_allocator::deallocate(*cur, buffer_size());
				finish = new_finish;	// 设定 deque 的新尾点
			}
			return start + elems_before;
		}
	}

	// 注意，最终需要保留一个缓冲区。这是deque 的策略，也是deque 的初始状态。
	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::clear()
	{
		// 以下针对头尾以外的每一个缓冲区（它们一定都是饱满的）
		for (map_pointer node = start.node + 1; node < finish.node; ++node)
		{
			// 将缓冲区内的所有元素解构
			destroy(*node, *node + buffer_size());
			// 释放缓冲区内存
			data_allocator::deallocate(*node, buffer_size());
		}

		if (start.node != finish.node) // 至少有头尾两个缓冲区 
		{
			destroy(start.cur, start.last);	// 将头缓冲区的目前所有元素解构
			destroy(finish.first, finish.cur); // 将尾缓冲区的目前所有元素解构
											   // 以下释放尾缓冲区。注意，头缓冲区保留。
			data_allocator::deallocate(finish.first, buffer_size());
		}
		else// 只有一个缓冲区
			destroy(start.cur, finish.cur);	// 将此唯一缓冲区内的所有元素解构
											// 注意，并不释放缓冲区空间。这唯一的缓冲区将保留。
		finish = start;	// 调整状态
	}

	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::create_map_and_nodes(size_type num_elements)
	{
		// 需要节点数=(元素个数/每个缓冲区可容纳的元素个数)+1
		// 如果刚好整除，会多配一个节点。
		size_type num_nodes = num_elements / buffer_size() + 1;

		// 一个 map 要管理几个节点。最少8个，最多是 “所需节点数加2”
		// （前后各预留一个，扩充时可用）。
		map_size = max(initial_map_size(), num_nodes + 2);
		map = map_allocator::allocate(map_size);
		// 以上配置出一个 “具有 map_size个节点” 的map。

		// 以下令nstart和nfinish指向map所拥有之全部节点的最中央区段。
		// 保持在最中央，可使头尾两端的扩充能量一样大。每个节点可对应一个缓冲区。
		map_pointer nstart = map + (map_size - num_nodes) / 2;
		map_pointer nfinish = nstart + num_nodes - 1;

		map_pointer cur;
		try
		{
			// 为map内的每个现用节点配置缓冲区。所有缓冲区加起来就是deque的空间
			// （最后一个缓冲区可能留有一些余裕）。
			for (cur = nstart; cur <= nfinish; ++cur)
				*cur = allocate_node();
		}
		catch (...)
		{
			// "commit or rollback" 语意：若非全部成功，就一个不留。
			for (map_pointer n = nstart; n < cur; ++n)
				deallocate_node(*n);
			map_allocator::deallocate(map, map_size);
			throw;
		}
		// 为deque内的两个迭代器start和end 设定正确的内容。
		start.set_node(nstart);
		finish.set_node(nfinish);
		start.cur = start.first;		// first, cur都是public
		finish.cur = finish.first + num_elements % buffer_size();
		// 前面说过，如果刚好整除，会多配一个节点。
		// 此时即令cur指向这多配的一个节点（所对映之缓冲区）的起头处。
	}

	// 只在出现异常时由catch分支调用
	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::destroy_map_and_nodes()
	{
		for (map_pointer cur = start.node; cur <= finish.node; ++cur)
			deallocate_node(*cur);
		map_allocator::deallocate(map, map_size);
	}

	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::fill_initialize(size_type n,
		const value_type& value)
	{
		create_map_and_nodes(n);	 // 把deque的结构都产生并安排好
		map_pointer cur;
		try
		{
			// 为每个节点的缓冲区设定初值
			for (cur = start.node; cur < finish.node; ++cur)
				uninitialized_fill(*cur, *cur + buffer_size(), value);
			// 最后一个节点的设定稍有不同（因为尾端可能有备用空间，不必设初值）
			uninitialized_fill(finish.first, finish.cur, value);
		}
		catch (...)
		{
			// commit or rollback
			for (map_pointer n = start.node; n < cur; ++n)
				destroy(*n, *n + buffer_size());
			destroy_map_and_nodes();
			throw;
		}

	}



	template <class _T, class _Alloc, size_t _BufSize>
	template <class _InputIterator>
	void deque<_T, _Alloc, _BufSize>::range_initialize(_InputIterator first,
		_InputIterator last, input_iterator_tag)
	{
		create_map_and_nodes(0);
		for (; first != last; ++first)
			push_back(*first);
	}

	template <class _T, class _Alloc, size_t _BufSize>
	template <class _ForwardIterator>
	void deque<_T, _Alloc, _BufSize>::range_initialize(_ForwardIterator first,
		_ForwardIterator last, forward_iterator_tag)
	{
		size_type n = distance(first, last);
		create_map_and_nodes(n);
		try
		{
			uninitialized_copy(first, last, start);
		}
		catch (...)
		{
			destroy_map_and_nodes();
			throw;
		}
	}

	// 只有当 finish.cur == finish.last – 1 时才会被调用
	// 也就是说只有当最后一个缓冲区只剩一个备用元素空间时才会被调用
	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::push_back_aux(const value_type& t)
	{
		value_type t_copy = t;
		reserve_map_at_back();		//  若符合某种条件则必须重换一个map
		*(finish.node + 1) = allocate_node();	// 配置一个新节点（缓冲区）
		try
		{
			construct(finish.cur, t_copy);		// 针对目标元素设值
			finish.set_node(finish.node + 1);	// 改变finish，令其指向新节点
			finish.cur = finish.first;			// 设定 finish 的状态
		}
		catch (...)
		{
			deallocate_node(*(finish.node + 1));
			throw;
		}
	}

	// 只有当start.cur == start.first时才会被调用
	// 也就是说只有当第一个缓冲区没有任何备用元素时才会被调用
	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::push_front_aux(const value_type& t)
	{
		value_type t_copy = t;
		reserve_map_at_front();		//  若符合某种条件则必须重换一个map
		*(start.node - 1) = allocate_node();	// 配置一个新节点（缓冲区）
		try
		{
			start.set_node(start.node - 1);		// 改变start，令其指向新节点
			start.cur = start.last - 1;			// 设定 start的状态
			construct(start.cur, t_copy);		// 针对目标元素设值
		}
		catch (...)
		{
			// "commit or rollback" 语意：若非全部成功，就一个不留。
			start.set_node(start.node + 1);
			start.cur = start.first;
			deallocate_node(*(start.node - 1));
			throw;
		}

	}

	// 只有当finish.cur == finish.first时才会被呼叫。
	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::pop_back_aux()
	{
		deallocate_node(finish.first);	// 释放最后一个缓冲区
		finish.set_node(finish.node - 1);	// 调整 finish 的状态，使指向
		finish.cur = finish.last - 1;		//  上一个缓冲区的最后一个元素
		destroy(finish.cur);				// 将该元素解构。
	}

	// 只有当start.cur == start.last - 1时才会被呼叫。
	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::pop_front_aux() {
		destroy(start.cur);				// 将第一缓冲区的第一个元素解构。
		deallocate_node(start.first);		// 释放第一缓冲区。
		start.set_node(start.node + 1);	// 调整 start 的状态，使指向
		start.cur = start.first;			//  下一个缓冲区的第一个元素。
	}


	template <class _T, class _Alloc, size_t _BufSize>
	template <class _InputIterator>
	void deque<_T, _Alloc, _BufSize>::insert(iterator pos,
		_InputIterator first, _InputIterator last,
		input_iterator_tag)
	{
		copy(first, last, inserter(*this, pos));
	}

	template <class _T, class _Alloc, size_t _BufSize>
	template <class _ForwardIterator>
	void deque<_T, _Alloc, _BufSize>::insert(iterator pos,
		_ForwardIterator first,
		_ForwardIterator last,
		forward_iterator_tag)
	{
		size_type n = 0;
		distance(first, last, n);
		if (pos.cur == start.cur)
		{
			iterator new_start = reserve_elements_at_front(n);
			try
			{
				uninitialized_copy(first, last, new_start);
				start = new_start;
			}
			catch (...)
			{
				destroy_nodes_at_front(new_start);
				throw;
			}
		}
		else if (pos.cur == finish.cur)
		{
			iterator new_finish = reserve_elements_at_back(n);
			try
			{
				uninitialized_copy(first, last, finish);
				finish = new_finish;
			}
			catch (...)
			{
				destroy_nodes_at_back(new_finish);
				throw;
			}
		}
		else
			insert_aux(pos, first, last, n);
	}


	template <class _T, class _Alloc, size_t _BufSize>
	typename deque<_T, _Alloc, _BufSize>::iterator
		deque<_T, _Alloc, _BufSize>::insert_aux(iterator pos, const value_type& x)
	{
		difference_type index = pos - start;	// 安插点之前的元素个数
		value_type x_copy = x;
		if (index < difference_type(size() / 2)) // 如果安插点之前的元素个数比较少
		{
			push_front(front());			// 在最前端加入与第一元素同值的元素。
			iterator front1 = start;		// 以下标示记号，然后进行元素搬移...
			++front1;
			iterator front2 = front1;
			++front2;
			pos = start + index;
			iterator pos1 = pos;
			++pos1;
			copy(front2, pos1, front1);		// 元素搬移
		}
		else // 安插点之后的元素个数比较少
		{
			push_back(back());			// 在最尾端加入与最后元素同值的元素。
			iterator back1 = finish;	// 以下标示记号，然后进行元素搬移...
			--back1;
			iterator back2 = back1;
			--back2;
			pos = start + index;
			copy_backward(pos, back2, back1);	// 元素搬移
		}
		*pos = x_copy;	// 在安插点上设定新值
		return pos;
	}

	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::insert_aux(iterator pos,
		size_type n, const value_type& x)
	{
		const difference_type elems_before = pos - start;
		size_type length = size();
		value_type x_copy = x;
		if (elems_before < length / 2) {
			iterator new_start = reserve_elements_at_front(n);
			iterator old_start = start;
			pos = start + elems_before;
			try
			{
				if (elems_before >= difference_type(n))
				{
					iterator start_n = start + difference_type(n);
					uninitialized_copy(start, start_n, new_start);
					start = new_start;
					copy(start_n, pos, old_start);
					fill(pos - difference_type(n), pos, x_copy);
				}
				else
				{
					_uninitialized_copy_fill(start, pos, new_start, start, x_copy);
					start = new_start;
					fill(old_start, pos, x_copy);
				}
			}
			catch (...)
			{
				destroy_nodes_at_front(new_start);
				throw;
			}
		}
		else
		{
			iterator new_finish = reserve_elements_at_back(n);
			iterator old_finish = finish;
			const difference_type elems_after = difference_type(length) - elems_before;
			pos = finish - elems_after;
			try
			{
				if (elems_after > difference_type(n)) {
					iterator finish_n = finish - difference_type(n);
					uninitialized_copy(finish_n, finish, finish);
					finish = new_finish;
					copy_backward(pos, finish_n, old_finish);
					fill(pos, pos + difference_type(n), x_copy);
				}
				else
				{
					_uninitialized_fill_copy(finish, pos + difference_type(n),
						x_copy, pos, finish);
					finish = new_finish;
					fill(pos, old_finish, x_copy);
				}
			}
			catch (...)
			{
				destroy_nodes_at_back(new_finish);
				throw;
			}
		}
	}



	template <class _T, class _Alloc, size_t _BufSize>
	template <class _ForwardIterator>
	void deque<_T, _Alloc, _BufSize>::insert_aux(iterator pos,
		_ForwardIterator first,
		_ForwardIterator last,
		size_type n)
	{
		const difference_type elems_before = pos - start;
		size_type length = size();
		if (elems_before < length / 2)
		{
			iterator new_start = reserve_elements_at_front(n);
			iterator old_start = start;
			pos = start + elems_before;
			try
			{
				if (elems_before >= difference_type(n)) {
					iterator start_n = start + difference_type(n);
					uninitialized_copy(start, start_n, new_start);
					start = new_start;
					copy(start_n, pos, old_start);
					copy(first, last, pos - difference_type(n));
				}
				else
				{
					ForwardIterator mid = first;
					advance(mid, difference_type(n) - elems_before);
					_uninitialized_copy_copy(start, pos, first, mid, new_start);
					start = new_start;
					copy(mid, last, old_start);
				}
			}
			catch (...)
			{
				destroy_nodes_at_front(new_start);
				throw;
			}
		}
		else
		{
			iterator new_finish = reserve_elements_at_back(n);
			iterator old_finish = finish;
			const difference_type elems_after = difference_type(length) - elems_before;
			pos = finish - elems_after;
			try
			{
				if (elems_after > difference_type(n)) {
					iterator finish_n = finish - difference_type(n);
					uninitialized_copy(finish_n, finish, finish);
					finish = new_finish;
					copy_backward(pos, finish_n, old_finish);
					copy(first, last, pos);
				}
				else
				{
					ForwardIterator mid = first;
					advance(mid, elems_after);
					_uninitialized_copy_copy(mid, last, pos, finish, finish);
					finish = new_finish;
					copy(first, mid, pos);
				}
			}
			catch (...)
			{
				destroy_nodes_at_back(new_finish);
				throw;
			}
		}
	}

	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::new_elements_at_front(size_type new_elements)
	{
		size_type new_nodes = (new_elements + buffer_size() - 1) / buffer_size();
		reserve_map_at_front(new_nodes);
		size_type i;
		try
		{
			for (i = 1; i <= new_nodes; ++i)
				*(start.node - i) = allocate_node();
		}
		catch (...)
		{
			for (size_type j = 1; j < i; ++j)
				deallocate_node(*(start.node - j));
			throw;
		}

	}

	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::new_elements_at_back(size_type new_elements) {
		size_type new_nodes = (new_elements + buffer_size() - 1) / buffer_size();
		reserve_map_at_back(new_nodes);
		size_type i;
		try
		{
			for (i = 1; i <= new_nodes; ++i)
				*(finish.node + i) = allocate_node();
		}
		catch (...)
		{
			for (size_type j = 1; j < i; ++j)
				deallocate_node(*(finish.node + j));
			throw;
		}

	}

	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::destroy_nodes_at_front(iterator before_start)
	{
		for (map_pointer n = before_start.node; n < start.node; ++n)
			deallocate_node(*n);
	}

	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::destroy_nodes_at_back(iterator after_finish)
	{
		for (map_pointer n = after_finish.node; n > finish.node; --n)
			deallocate_node(*n);
	}

	template <class _T, class _Alloc, size_t _BufSize>
	void deque<_T, _Alloc, _BufSize>::reallocate_map(size_type nodes_to_add,
		bool add_at_front)
	{
		size_type old_num_nodes = finish.node - start.node + 1;
		size_type new_num_nodes = old_num_nodes + nodes_to_add;

		map_pointer new_nstart;
		if (map_size > 2 * new_num_nodes)
		{
			new_nstart = map + (map_size - new_num_nodes) / 2
				+ (add_at_front ? nodes_to_add : 0);
			if (new_nstart < start.node)
				copy(start.node, finish.node + 1, new_nstart);
			else
				copy_backward(start.node, finish.node + 1, new_nstart + old_num_nodes);
		}
		else
		{
			size_type new_map_size = map_size + max(map_size, nodes_to_add) + 2;
			// 配置一块空间，准备给新map使用。
			map_pointer new_map = map_allocator::allocate(new_map_size);
			new_nstart = new_map + (new_map_size - new_num_nodes) / 2
				+ (add_at_front ? nodes_to_add : 0);
			// 把原map 内容拷贝过来。
			copy(start.node, finish.node + 1, new_nstart);
			// 释放原map
			map_allocator::deallocate(map, map_size);
			// 设定新map的起始地址与大小
			map = new_map;
			map_size = new_map_size;
		}

		// 重新设定迭代器 start 和 finish
		start.set_node(new_nstart);
		finish.set_node(new_nstart + old_num_nodes - 1);
	}

	template <class _T, class _Alloc, size_t _BufSiz>
	inline void swap(deque<_T, _Alloc, _BufSiz>& x, deque<_T, _Alloc, _BufSiz>& y)
	{
		x.swap(y);
	}
}
#endif //end of DEQUE_H