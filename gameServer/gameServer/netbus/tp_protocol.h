#ifndef __TP_PROTOCOL_H__
#define __TP_PROTOCOL_H__

class tp_protocol {
public:

	//读取头部得字节数据
	static bool read_header(unsigned char* data, int data_len, int* pkg_size, int* out_header_size);
	//解析数据
	static unsigned char* package(const unsigned char* raw_data, int len, int* pkg_len);
	//得到数据部分
	static void relese_package(unsigned char* tp_pkg);
};

#endif
