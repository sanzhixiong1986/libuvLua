#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tp_protocol.h"

#include "../utils/cache_alloc.h"

extern cache_allocer* wbuf_allocer;

/************************************************************************/
/* ȷ���Ƿ��Ѿ�ͷ��16λ����                                             */
/************************************************************************/
bool tp_protocol::read_header(unsigned char* data, int data_len, int* pkg_size, int* out_header_size){

	//�ϰ�����
	if (data_len < 2){
		return false;
	}
	//��ȡ���Ĵ�С
	*pkg_size = (data[0] | (data[1] << 8));
	//ͷ�Ĵ�С
	*out_header_size = 2;
	//end
	return true;
}
/************************************************************************/
/* ��ȡ���Ĳ��� �ص�                                                    */
/************************************************************************/
unsigned char* tp_protocol::package(const unsigned char* raw_data, int len, int* pkg_len){
	//ͷ�ֽ�Ϊ�����ֽ�
	int head_size = 2;
	//�ܵ��ֽڳ���
	*pkg_len = (head_size + len);
	//����һ�����ݴ�С���ַ���
	unsigned char* data_buf = (unsigned char*)cache_alloc(wbuf_allocer, (*pkg_len));
	//���ͷ�ֽڵĳ���
	data_buf[0] = (unsigned char)((*pkg_len) & 0x000000ff);
	//�����������ʣ�µĳ���
	data_buf[1] = (unsigned char)(((*pkg_len) & 0x0000ff00) >> 8);
	//�����ݵ��ڴ�������
	memcpy(data_buf + head_size, raw_data, len);
	//���ظ��Ƴ���������
	return data_buf;
}

/************************************************************************/
/* �����ڴ������  �ص�                                                 */
/************************************************************************/
void tp_protocol::relese_package(unsigned char* tp_pkg){
	cache_free(wbuf_allocer, tp_pkg);
}