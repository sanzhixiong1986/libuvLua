#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tp_protocol.h"

#include "../utils/cache_alloc.h"

extern cache_allocer* wbuf_allocer_tp = nullptr;

//读取对应得头部数据
bool tp_protocol::read_header(unsigned char* data, int data_len, int* pkg_size, int* out_header_size){

	//断包操作
	if (data_len < 2){
		return false;
	}

	*pkg_size = (data[0] | (data[1] << 8));
	*out_header_size = 2;

	return true;
}

unsigned char* tp_protocol::package(const unsigned char* raw_data, int len, int* pkg_len){
	//头字节为两个字节
	int head_size = 2;
	//总得字节长度
	*pkg_len = (head_size + len);

	unsigned char* data_buf = (unsigned char*)cache_alloc(wbuf_allocer_tp, (*pkg_len));
	data_buf[0] = (unsigned char)((*pkg_len) & 0x000000ff);
	data_buf[1] = (unsigned char)(((*pkg_len) & 0x0000ff00) >> 8);
	memcpy(data_buf + head_size, raw_data, len);
	
	return data_buf;
}

void tp_protocol::relese_package(unsigned char* tp_pkg){
	cache_free(wbuf_allocer_tp, tp_pkg);
}