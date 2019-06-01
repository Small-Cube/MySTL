/*
* һ�����������������ṩ��
* �ӿ���simple_alloc
*/

#ifndef ALLOC_H_
#define ALLOC_H_

#include <iostream>
#include <cstddef>
#include <cstdlib>

namespace MySTL
{

	/*
	* ��һ��������
	* ��Ҫ�������ڴ��㹻����õ�һ������
	* �򵱵ڶ������������޿ɷ����ڴ�ʱ���õ�һ������������������new-handler���ƿ��Ƿ���
	* �취�õ�һЩ�ɷ����ڴ棬�����׳�bad_alloc�쳣
	*/
	// �����ǵ�һ����������
	//û��ģ�������������ʵ���������instû������
	template <int inst>
	class _malloc_alloc_template
	{
	private:
		//���漸�����������������ڴ治������
		static void* oom_malloc(size_t);
		static void* oom_realloc(void *, size_t);
		static void(*_malloc_alloc_oom_handler)();

	public:
		static void* allocate(size_t n)
		{
			// ��һ��������ֱ��ʹ�� malloc()
			void* result = malloc(n);
			//�޷�����Ҫ�������oom_malloc
			if (0 == result) result = oom_malloc(n);
			return result;
		}

		static void deallocate(void *p, size_t /* n */)
		{
			// ��һ��������ֱ��ʹ�� free()
			free(p);	             
		}

		static void* reallocate(void *p, size_t /* old_sz */, size_t new_sz)
		{
			//��һ��������ֱ��ʹ�� realloc()
			//realloc���������޸�һ��ԭ���Ѿ�������ڴ��Ĵ�С������ʹһ���ڴ���������С��
			void * result = realloc(p, new_sz);	
			//�޷�����ʱ����oom_realloc
			if (0 == result) result = oom_realloc(p, new_sz);
			return result;
		}

		// �������� C++ �� set_new_handler().
		//����set_malloc_handler��������һ��void (*)()���͵Ĳ���f����������Ϊvoid (*)()��
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

	//����������ĺ���ʵ��
	template <int inst>
	void * _malloc_alloc_template<inst>::oom_malloc(size_t n)
	{
		void(*my_malloc_handler)();
		void *result;

		for (;;)
		{	// ���ϳ����ͷš����á����ͷš������á�
			my_malloc_handler = _malloc_alloc_oom_handler;
			using std::bad_alloc;
			if (0 == my_malloc_handler)
			{
				std::cerr << "out of memory!" << std::endl;
				exit(1);
			}
			(*my_malloc_handler)();		// ���д��������ͼ�ͷ��ڴ档
			result = malloc(n);			// �ٴγ��������ڴ档
			if (result) return(result);
		}
	}
	//����������ĺ���ʵ��
	template <int inst>
	void * _malloc_alloc_template<inst>::oom_realloc(void *p, size_t n)
	{
		void(*my_malloc_handler)();
		void *result;

		for (;;)
		{   // ���ϳ����ͷš����á����ͷš������á�
			my_malloc_handler = _malloc_alloc_oom_handler;
			if (0 == my_malloc_handler)
			{
				std::cerr << "out of memory!" << std::endl;
				exit(1);
			}
			(*my_malloc_handler)();	// ���д��������ͼ�ͷ��ڴ档
			result = realloc(p, n);	// �ٴγ��������ڴ档
			if (result) return(result);
		}
	}

	typedef _malloc_alloc_template<0> malloc_alloc;  
	//��һ�������������Ͻ���
	/*��һ����������malloc(),free(),realloc(),��C����ִ��ʵ���ڴ�
	*���á��ͷš������õȲ�����������������C++ new handler���Ʋ���ֱ��
	*ʹ�ã�����Ϊ���ڴ治����::operator new�������ڴ�ġ�
	*����ڴ治����׳��쳣��������exit(1)��ֹ����
	*/
	 //����Ϊ�������ӿ���
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

	// �����ǵڶ�����������
	//Դ�������߳���ص�һ������������ʡ�ԡ�
	template <int inst>
	class _default_alloc_template
	{

	private:
		static const int _ALIGN = 8;            // С��������ϵ��߽�
		static const int _MAX_BYTES = 128;      // С�����������
		static const int _NFREELISTS = 16;      //free-lists ����(_MAX_BYTES / _ALIGN)
		
		//��bytes�ϵ�Ϊ8�ı���
		static size_t ROUND_UP(size_t bytes)    
		{
			//��δ����ǳ��õ��ڴ�����ĳ��ù��ߣ�������bytes������ALLGN�ı���
			
			return (((bytes)+_ALIGN - 1) & ~(_ALIGN - 1));
		}
		//free list�ڵ�
		union obj                           
		{
			union obj* free_list_link;
			char client_data[1];    /* The client sees this. */
		};

	private:
		//����ָ�룬���������ָ��ָ��һ��obj��
		static obj* free_list[_NFREELISTS]; //16����������
		//����bytes��С����ʹ�õ������
		static  size_t FREELIST_INDEX(size_t bytes)  
		{
			return (((bytes)+_ALIGN - 1) / _ALIGN - 1);
		}

		//allocate()ʱ���������޿��ÿռ䣬����refill()��
		//����һ����СΪn�Ķ��󣬲����ܼ����СΪn���������鵽��������
		static void* refill(size_t n);
		//���ڴ��ȡ�ռ�,nobjsΪ�����ò���������һ����Ŀռ䣬��������
		//nobjs��size,���������ô��������ԣ����ֵ���ܻ����
		static char* chunk_alloc(size_t size, int& nobjs); 

	    //�ڴ��ˮλ��־
		static char* start_free; //��ʼλ�ã�ֻ��chunk_alloc�б仯
		static char* end_free;   //��ֹλ�ã�ֻ��chunk_alloc�б仯
		static size_t heap_size; //��С

	public:
		static void * allocate(size_t n) /* n must be > 0 */
		{
			obj** my_free_list;
			obj* result;
			//����128�����õ�һ��������
			if (n > (size_t)_MAX_BYTES)          
			{
				return(malloc_alloc::allocate(n));
			}
			//����ʹ��16�����������е���һ��
			my_free_list = free_list + FREELIST_INDEX(n);  
			result = *my_free_list;
			if (result == 0)
			{
				//����ʧ�ܵ���refill()
				void *r = refill(ROUND_UP(n));      
				return r;
			}
			//������������
			*my_free_list = result->free_list_link;
			return (result);
		};

		static void deallocate(void* p, size_t n) /* p may not be 0 */
		{
			obj* q = (obj*)p;
			obj** my_free_list;

			if (n > (size_t)_MAX_BYTES)     //���õ�һ��������
			{
				malloc_alloc::deallocate(p, n);
				return;
			}

			my_free_list = free_list + FREELIST_INDEX(n);
			//��������������������
			q->free_list_link = *my_free_list;
			*my_free_list = q;
		}

		static void * reallocate(void *p, size_t old_sz, size_t new_sz);
	};

	typedef _default_alloc_template<0> alloc;
	typedef _default_alloc_template<0> single_client_alloc;
	//�ٶ�size���ϵ���8�ı���
	template <int inst>
	char* _default_alloc_template<inst>::chunk_alloc(size_t size, int& nobjs) 
	{
		char * result;
		size_t total_bytes = size * nobjs;     //������С
		size_t bytes_left = end_free - start_free;//�ڴ������
		//�ڴ��ʣ��ռ���ȫ����
		if (bytes_left >= total_bytes)  
		{
			result = start_free;
			start_free += total_bytes;
			return(result);
		}
		//�������㣬���ǿ��Թ�Ӧһ������������
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
			//�ڴ����������һ������
			//����ѿռ��СΪ2������������һ����������������
			size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
			// ���������ڴ�ص���ͷ
			if (bytes_left > 0)
			{
				//��Ѱ�ʺϵ���������
				obj** my_free_list = free_list + FREELIST_INDEX(bytes_left);
				//������������������Щ��ͷ
				((obj *)start_free)->free_list_link = *my_free_list;
				*my_free_list = (obj *)start_free;
			}
			//����ѿռ䣬�����ڴ��
			start_free = (char *)malloc(bytes_to_get);
			if (0 == start_free) //�ѿռ䲻��
			{
				int i;
				obj** my_free_list, *p;
				//��Ѱ���������к�����δʹ�����㹻����
				for (i = (int)size; i <= _MAX_BYTES; i += _ALIGN)
				{
					my_free_list = free_list + FREELIST_INDEX(i);
					p = *my_free_list;
					if (0 != p)
					{
						*my_free_list = p->free_list_link;
						start_free = (char *)p;
						end_free = start_free + i;
						//�ݹ�����Լ�����������nobjs
						return(chunk_alloc(size, nobjs));
						//�����ڴ�ز������ͷ���������ʵ�������������
					}
				}
				end_free = 0; //ɽ��ˮ����������û���ڴ����
			//��ʱ���õ�һ�����������������쳣�����Ƿ��н���취
				start_free = (char*)malloc_alloc::allocate(bytes_to_get);
			}
			heap_size += bytes_to_get;
			end_free = start_free + bytes_to_get;
			return(chunk_alloc(size, nobjs));
		}
	}


	//���allocate()����������û�п������飬��ô������������
	//�µĿռ�ȡ���ڴ�أ���chunk_alloc()��ɣ���Ĭ��ȡ��20���½ڵ㣨���飩
	//�ڴ�ز�����ܻ�С��20��
	template <int inst>
	void* _default_alloc_template<inst>::refill(size_t n) //�ٶ�n�Ѿ��ϵ�Ϊ8�ı���
	{
		int nobjs = 20;
		//���Դ��ڴ�ط�������ռ�ȡ��nobjs�����飬nobjs������
		char* chunk = chunk_alloc(n, nobjs); 
		obj** my_free_list;
		obj* result;
		obj* current_obj, *next_obj;
		int i;
		//���ǡ��ֻ���䵽һ�����飬ֱ�ӷ��ظ������ߣ���������û���½ڵ�
		if (1 == nobjs) return(chunk);
		//����������������Խ���ʣ��ռ�
		my_free_list = free_list + FREELIST_INDEX(n);

		//�������ڴ�ؽ�����������
		result = (obj *)chunk;//��һ�鷵�ظ��Ͷ�
	    //��������������ɴ��ڴ�ط���Ķ���ռ�
		*my_free_list = next_obj = (obj *)(chunk + n);
		for (i = 1; ; i++) //���0�����鷵�ظ�������
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
