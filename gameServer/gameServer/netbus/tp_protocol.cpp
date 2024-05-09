#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tp_protocol.h"

#include "../utils/cache_alloc.h"

extern cache_allocer* wbuf_allocer;

/************************************************************************/
/* 确定是否已经头的16位数字                                             */
/************************************************************************/
bool tp_protocol::read_header(unsigned char* data, int data_len, int* pkg_size, int* out_header_size){

	//断包操作
	if (data_len < 2){
		return false;
	}
	//读取包的大小
	*pkg_size = (data[0] | (data[1] << 8));
	//头的大小
	*out_header_size = 2;
	//end
	return true;
}
/************************************************************************/
/* 读取包的操作 重点                                                    */
/************************************************************************/
unsigned char* tp_protocol::package(const unsigned char* raw_data, int len, int* pkg_len){
	//头字节为两个字节
	int head_size = 2;
	//总得字节长度
	*pkg_len = (head_size + len);
	//创建一个数据大小的字符串
	unsigned char* data_buf = (unsigned char*)cache_alloc(wbuf_allocer, (*pkg_len));
	//获得头字节的长度
	data_buf[0] = (unsigned char)((*pkg_len) & 0x000000ff);
	//获得整个包的剩下的长度
	data_buf[1] = (unsigned char)(((*pkg_len) & 0x0000ff00) >> 8);
	//把数据的内存放入出来
	memcpy(data_buf + head_size, raw_data, len);
	//返回复制出来的数据
	return data_buf;
}

/************************************************************************/
/* 创建内存的数据  重点                                                 */
/************************************************************************/
void tp_protocol::relese_package(unsigned char* tp_pkg){
	cache_free(wbuf_allocer, tp_pkg);
}