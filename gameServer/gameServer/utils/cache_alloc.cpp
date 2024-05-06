#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cache_alloc.h"

//一个node对象,链表的头对象
struct node
{
	struct node* next;
};

//核心的对象类
struct cache_allocer{
	unsigned char* cache_mem;//字符串
	int capacity;
	struct node* free_list;//子节点的对象
	int elem_size;
};

struct cache_allocer* create_cache_allocer(int capacity, int elem_size){
	//创建需要的头节点
	struct cache_allocer* allocer = (struct cache_allocer*)(sizeof(struct cache_allocer));
	memset(allocer, 0, sizeof(struct cache_allocer));
	//end

	//给对象赋值
	elem_size = (elem_size < sizeof(struct node)) ? sizeof(struct node) : elem_size;
	allocer->capacity = capacity;
	allocer->elem_size = elem_size;
	allocer->cache_mem = (unsigned char*)(capacity * elem_size);
	memset(allocer->cache_mem, 0, sizeof(capacity * elem_size));
	//end

	//给列表一个赋值
	allocer->free_list = NULL;
	
	//给链表赋值
	for (int i = 0; i < capacity;i++)
	{
		struct node* walk = (struct node*)(allocer->cache_mem + i*elem_size);
		walk->next = allocer->free_list;
		allocer->free_list = walk;
	}

	return allocer;
}

//消除的方法
void destroy_cache_allocer(struct cache_allocer* allocer){
	if (allocer->cache_mem != NULL){
		free(allocer->cache_mem);
	}
	free(allocer);
}

void* cache_alloc(struct cache_allocer* allocer, int elem_size){
	if (allocer->elem_size < elem_size){
		return NULL;
	}

	if (allocer->free_list != NULL){
		void* now = allocer->free_list;
		allocer->free_list = allocer->free_list->next;
		return now;
	}
	
	return malloc(elem_size); 
}

void cache_free(struct cache_allocer* allocer, void* mem){
	if (((unsigned char*)mem) >= allocer->cache_mem && ((unsigned char*)mem) < allocer->cache_mem + allocer->capacity* allocer->elem_size){
		struct node* node = (struct node*)mem;
		node->next = allocer->free_list;
		allocer->free_list = node;
		return;
	}
	free(mem);
}