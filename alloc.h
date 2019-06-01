/*
* 一、二级配置器，并提供了
* 接口类simple_alloc
*/

#ifndef ALLOC_H_
#define ALLOC_H_

#include <iostream>
#include <cstddef>
#include <cstdlib>

namespace MySTL
{

	/*
	* 第一级配置器
	* 当要求分配的内存足够大调用第一级配置
	* 或当第二级配置器已无可分配内存时调用第一级配置器，利用类似new-handler机制看是否有
	* 办法得到一些可分配内存，否则抛出bad_alloc异常
	*/
	// 以下是第一级配置器。
	//没有模板参数，所以其实尖括号里的inst没有用上
	template <int inst>
	class _malloc_alloc_template
	{
	private:
		//下面几个函数是用来处理内存不足的情况
		static void* oom_malloc(size_t);
		static void* oom_realloc(void *, size_t);
		static void(*_malloc_alloc_oom_handler)();

	public:
		static void* allocate(size_t n)
		{
			// 第一级配置器直接使用 malloc()
			void* result = malloc(n);
			//无法满足要求则改用oom_malloc
			if (0 == result) result = oom_malloc(n);
			return result;
		}

		static void deallocate(void *p, size_t /* n */)
		{
			// 第一级配置器直接使用 free()
			free(p);	             
		}

		static void* reallocate(void *p, size_t /* old_sz */, size_t new_sz)
		{
			//第一级配置器直接使用 realloc()
			//realloc函数用于修改一个原先已经分配的内存块的大小，可以使一块内存的扩大或缩小。
			void * result = realloc(p, new_sz);	
			//无法满足时调用oom_realloc
			if (0 == result) result = oom_realloc(p, new_sz);
			return result;
		}

		// 以下类似 C++ 的 set_new_handler().
		//函数set_malloc_handler，它接受一个void (*)()类型的参数f，返回类型为void (*)()。
		static void(*set_malloc_handler(void(*f)()))()
		{
			void(*old)() = _malloc_alloc_oom_handler;
			_malloc_alloc_oom_handler = f;
			return(old);
		}

	};

	// malloc_alloc out-of-memory handling
	template <int inst>
	void(*_malloc_alloc_template<inst>::_malloc_alloc_oom_handler)() = 0;

	//上面类里面的函数实现
	template <int inst>
	void * _malloc_alloc_template<inst>::oom_malloc(size_t n)
	{
		void(*my_malloc_handler)();
		void *result;

		for (;;)
		{	// 不断尝试释放、配置、再释放、再配置…
			my_malloc_handler = _malloc_alloc_oom_handler;
			using std::bad_alloc;
			if (0 == my_malloc_handler)
			{
				std::cerr << "out of memory!" << std::endl;
				exit(1);
			}
			(*my_malloc_handler)();		// 呼叫处理程序，企图释放内存。
			result = malloc(n);			// 再次尝试配置内存。
			if (result) return(result);
		}
	}
	//上面类里面的函数实现
	template <int inst>
	void * _malloc_alloc_template<inst>::oom_realloc(void *p, size_t n)
	{
		void(*my_malloc_handler)();
		void *result;

		for (;;)
		{   // 不断尝试释放、配置、再释放、再配置…
			my_malloc_handler = _malloc_alloc_oom_handler;
			if (0 == my_malloc_handler)
			{
				std::cerr << "out of memory!" << std::endl;
				exit(1);
			}
			(*my_malloc_handler)();	// 呼叫处理程序，企图释放内存。
			result = realloc(p, n);	// 再次尝试配置内存。
			if (result) return(result);
		}
	}

	typedef _malloc_alloc_template<0> malloc_alloc;  
	//第一级配置器到以上结束
	/*第一级配置器以malloc(),free(),realloc(),等C函数执行实际内存
	*配置、释放、重配置等操作，并且利用类似C++ new handler机制不能直接
	*使用，是因为其内存不是用::operator new来配置内存的。
	*如果内存不足会抛出异常或者利用exit(1)中止程序
	*/
	 //以下为配置器接口类
	template<class _T, class _Alloc>
	class simple_alloc
	{

	public:
		static _T *allocate(size_t n)
		{
			return 0 == n ? 0 : (_T*)_Alloc::allocate(n * sizeof(_T));
		}
		static _T *allocate(void)
		{
			return (_T*)_Alloc::allocate(sizeof(_T));
		}
		static void deallocate(_T *p, size_t n)
		{
			if (0 != n) _Alloc::deallocate(p, n * sizeof(_T));
		}
		static void deallocate(_T *p)
		{
			_Alloc::deallocate(p, sizeof(_T));
		}
	};

	// 以下是第二级配置器。
	//源码中有线程相关的一个参数，这里省略。
	template <int inst>
	class _default_alloc_template
	{

	private:
		static const int _ALIGN = 8;            // 小型区块的上调边界
		static const int _MAX_BYTES = 128;      // 小型区块的上限
		static const int _NFREELISTS = 16;      //free-lists 个数(_MAX_BYTES / _ALIGN)
		
		//将bytes上调为8的倍数
		static size_t ROUND_UP(size_t bytes)    
		{
			//这段代码是常用的内存调整的常用工具，将输入bytes调整到ALLGN的倍数
			
			return (((bytes)+_ALIGN - 1) & ~(_ALIGN - 1));
		}
		//free list节点
		union obj                           
		{
			union obj* free_list_link;
			char client_data[1];    /* The client sees this. */
		};

	private:
		//数组指针，里面放着是指针指向一个obj。
		static obj* free_list[_NFREELISTS]; //16个自由链表
		//根据bytes大小决定使用的链表号
		static  size_t FREELIST_INDEX(size_t bytes)  
		{
			return (((bytes)+_ALIGN - 1) / _ALIGN - 1);
		}

		//allocate()时自由链表无可用空间，调用refill()，
		//返回一个大小为n的对象，并可能加入大小为n的其他区块到自由链表
		static void* refill(size_t n);
		//从内存池取空间,nobjs为传引用参数，分配一个大的空间，可以容纳
		//nobjs个size,如果配置这么多个不可以，这个值可能会较少
		static char* chunk_alloc(size_t size, int& nobjs); 

	    //内存池水位标志
		static char* start_free; //起始位置，只在chunk_alloc中变化
		static char* end_free;   //终止位置，只在chunk_alloc中变化
		static size_t heap_size; //大小

	public:
		static void * allocate(size_t n) /* n must be > 0 */
		{
			obj** my_free_list;
			obj* result;
			//大于128，调用第一级配置器
			if (n > (size_t)_MAX_BYTES)          
			{
				return(malloc_alloc::allocate(n));
			}
			//决定使用16个自由链表中的哪一个
			my_free_list = free_list + FREELIST_INDEX(n);  
			result = *my_free_list;
			if (result == 0)
			{
				//分配失败调用refill()
				void *r = refill(ROUND_UP(n));      
				return r;
			}
			//调整自由链表
			*my_free_list = result->free_list_link;
			return (result);
		};

		static void deallocate(void* p, size_t n) /* p may not be 0 */
		{
			obj* q = (obj*)p;
			obj** my_free_list;

			if (n > (size_t)_MAX_BYTES)     //调用第一级配置器
			{
				malloc_alloc::deallocate(p, n);
				return;
			}

			my_free_list = free_list + FREELIST_INDEX(n);
			//调整自由链表，回收区块
			q->free_list_link = *my_free_list;
			*my_free_list = q;
		}

		static void * reallocate(void *p, size_t old_sz, size_t new_sz);
	};

	typedef _default_alloc_template<0> alloc;
	typedef _default_alloc_template<0> single_client_alloc;
	//假定size已上调至8的倍数
	template <int inst>
	char* _default_alloc_template<inst>::chunk_alloc(size_t size, int& nobjs) 
	{
		char * result;
		size_t total_bytes = size * nobjs;     //需分配大小
		size_t bytes_left = end_free - start_free;//内存池余量
		//内存池剩余空间完全满足
		if (bytes_left >= total_bytes)  
		{
			result = start_free;
			start_free += total_bytes;
			return(result);
		}
		//余量不足，但是可以供应一个及以上区块
		else if (bytes_left >= size)  
		{
			nobjs = int(bytes_left / size);
			total_bytes = size * nobjs;
			result = start_free;
			start_free += total_bytes;
			return(result);
		}
		else  
		{
			//内存池余量不足一个区块
			//请求堆空间大小为2倍需求量加上一个随分配次数递增量
			size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
			// 尝试利用内存池的零头
			if (bytes_left > 0)
			{
				//找寻适合的自由链表
				obj** my_free_list = free_list + FREELIST_INDEX(bytes_left);
				//调整自由链表，纳入这些零头
				((obj *)start_free)->free_list_link = *my_free_list;
				*my_free_list = (obj *)start_free;
			}
			//分配堆空间，补充内存池
			start_free = (char *)malloc(bytes_to_get);
			if (0 == start_free) //堆空间不足
			{
				int i;
				obj** my_free_list, *p;
				//找寻自由链表中含有尚未使用且足够大者
				for (i = (int)size; i <= _MAX_BYTES; i += _ALIGN)
				{
					my_free_list = free_list + FREELIST_INDEX(i);
					p = *my_free_list;
					if (0 != p)
					{
						*my_free_list = p->free_list_link;
						start_free = (char *)p;
						end_free = start_free + i;
						//递归调用自己，修正参数nobjs
						return(chunk_alloc(size, nobjs));
						//最终内存池残余的零头都被纳入适当的自由链表中
					}
				}
				end_free = 0; //山穷水尽，到处都没有内存可用
			//这时调用第一级配置器，利用其异常处理看是否有解决办法
				start_free = (char*)malloc_alloc::allocate(bytes_to_get);
			}
			heap_size += bytes_to_get;
			end_free = start_free + bytes_to_get;
			return(chunk_alloc(size, nobjs));
		}
	}


	//如果allocate()发现链表中没有可用区块，那么会调用这个函数
	//新的空间取自内存池（由chunk_alloc()完成），默认取得20个新节点（区块）
	//内存池不足可能会小于20个
	template <int inst>
	void* _default_alloc_template<inst>::refill(size_t n) //假定n已经上调为8的倍数
	{
		int nobjs = 20;
		//尝试从内存池分配所需空间取得nobjs个区块，nobjs是引用
		char* chunk = chunk_alloc(n, nobjs); 
		obj** my_free_list;
		obj* result;
		obj* current_obj, *next_obj;
		int i;
		//如果恰好只分配到一个区块，直接返回给调用者，自由链表没有新节点
		if (1 == nobjs) return(chunk);
		//否则调整自由链表以接纳剩余空间
		my_free_list = free_list + FREELIST_INDEX(n);

		//以下在内存池建立自由链表
		result = (obj *)chunk;//这一块返回给客端
	    //引导自由链表接纳从内存池分配的多余空间
		*my_free_list = next_obj = (obj *)(chunk + n);
		for (i = 1; ; i++) //编号0的区块返回给调用者
		{
			current_obj = next_obj;
			next_obj = (obj *)((char *)next_obj + n);
			if (nobjs - 1 == i)
			{
				current_obj->free_list_link = 0;
				break;
			}
			else
			{
				current_obj->free_list_link = next_obj;
			}
		}
		return(result);
	}

	template <int inst>
	void* _default_alloc_template<inst>::reallocate(void *p, size_t old_sz, size_t new_sz)
	{
		void * result;
		size_t copy_sz;

		if (old_sz > (size_t)_MAX_BYTES && new_sz > (size_t)_MAX_BYTES)
		{
			return(realloc(p, new_sz));
		}
		if (ROUND_UP(old_sz) == ROUND_UP(new_sz)) return(p);
		result = allocate(new_sz);
		copy_sz = new_sz > old_sz ? old_sz : new_sz;
		memcpy(result, p, copy_sz);
		deallocate(p, old_sz);
		return(result);
	}

	template <int inst>
	char* _default_alloc_template< inst>::start_free = 0;

	template <int inst>
	char* _default_alloc_template<inst>::end_free = 0;

	template <int inst>
	size_t _default_alloc_template<inst>::heap_size = 0;

	template <int inst>
	typename _default_alloc_template<inst>::obj*
		_default_alloc_template<inst> ::free_list[_NFREELISTS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
}    //end of MySTL

#endif//end of AALOC_H_
