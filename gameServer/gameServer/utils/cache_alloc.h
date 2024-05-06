#ifndef __CACHE_ALLOC_H__
#define __CACHE_ALLOC_H__

#ifdef __cplusplus
extern "C"{
#endif
	//创建内存并且返回内存数据
	struct cache_allocer* create_cache_allocer(int capacity, int elem_size);
	//销毁内存
	void destroy_cache_allocer(struct cache_allocer* allocer);
	//
	void* cache_alloc(struct cache_allocer* allocer, int elem_size);
	//
	void cache_free(struct cache_allocer* allocer, void* mem);
#ifdef __cplusplus

}
#endif // _cplusplus

#endif
