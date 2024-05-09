#ifndef __NETBUS_H__
#define __NETBUS_H__

class netbus {
public:
	static netbus* instance();

public:
	//5.6新增加init方法
	void init();
	//end
	void start_tcp_server(int port);
	void start_ws_server(int port);
	void start_upd_server(int port);
	void run();
};

#endif
