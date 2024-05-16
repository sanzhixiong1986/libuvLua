﻿using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;

using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class network : MonoBehaviour
{
    public string server_ip;
    public int port;


    private Socket client_socket = null;
    private bool is_connect = false;
    private Thread recv_thread = null;

    private const int RECV_LEN = 8192;
    private byte[] recv_buf = new byte[RECV_LEN];
    private int recved;
    private byte[] long_pkg = null;
    private int long_pkg_size = 0;

    // event queque
    private Queue<cmd_msg> net_events = new Queue<cmd_msg>();
    // event listener, stype--> 监听者;
    public delegate void net_message_handler(cmd_msg msg);
    // 事件和监听的map
    private Dictionary<int, net_message_handler> event_listeners = new Dictionary<int, net_message_handler>();

    public static network _instance;
    public static network instance
    {
        get
        {
            return _instance;
        }
    }

    void Awake()
    {
        _instance = this;
        DontDestroyOnLoad(this.gameObject);
    }

    // Use this for initialization
    void Start()
    {
        this.connect_to_server();
    }

    void OnDestroy()
    {
        // Debug.Log("network onDestroy!");
        this.close();
    }

    void OnApplicaitonQuit()
    {
        // Debug.Log("OnApplicaitonQuit");
        this.close();
    }

    // Update is called once per frame
    void Update()
    {
        lock (this.net_events)
        {
            while (this.net_events.Count > 0)
            {
                cmd_msg msg = this.net_events.Dequeue();
                //  收到了一个命令包;
                if (this.event_listeners.ContainsKey(msg.stype))
                {
                    this.event_listeners[msg.stype](msg);
                }
                // end 
            }
        }
    }

    void on_conntect_timeout()
    {
    }

    void on_connect_error(string err)
    {
    }

    void connect_to_server()
    {
        try
        {
            this.client_socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            IPAddress ipAddress = IPAddress.Parse(this.server_ip);
            IPEndPoint ipEndpoint = new IPEndPoint(ipAddress, this.port);

            IAsyncResult result = this.client_socket.BeginConnect(ipEndpoint, new AsyncCallback(this.on_connected), this.client_socket);
            bool success = result.AsyncWaitHandle.WaitOne(5000, true);
            if (!success)
            { // timeout;
                this.on_conntect_timeout();
            }
        }
        catch (System.Exception e)
        {
            Debug.Log(e.ToString());
            this.on_connect_error(e.ToString());
        }
    }

    void on_recv_tcp_cmd(byte[] data, int start, int data_len)
    {
        cmd_msg msg;
        proto_man.unpack_cmd_msg(data, start, data_len, out msg);
        if (msg != null)
        {
            lock (this.net_events)
            { // recv thread
                this.net_events.Enqueue(msg);
            }
        }
    }

    void on_recv_tcp_data()
    {
        byte[] pkg_data = (this.long_pkg != null) ? this.long_pkg : this.recv_buf;
        while (this.recved > 0)
        {
            int pkg_size = 0;
            int head_size = 0;

            if (!tcp_packer.read_header(pkg_data, this.recved, out pkg_size, out head_size))
            {
                break;
            }

            if (this.recved < pkg_size)
            {
                break;
            }

            // unsigned char* raw_data = pkg_data + head_size;
            int raw_data_start = head_size;
            int raw_data_len = pkg_size - head_size;

            on_recv_tcp_cmd(pkg_data, raw_data_start, raw_data_len);
            // end 

            if (this.recved > pkg_size)
            {
                this.recv_buf = new byte[RECV_LEN];
                Array.Copy(pkg_data, pkg_size, this.recv_buf, 0, this.recved - pkg_size);
                pkg_data = this.recv_buf;
            }

            this.recved -= pkg_size;

            if (this.recved == 0 && this.long_pkg != null)
            {
                this.long_pkg = null;
                this.long_pkg_size = 0;
            }
        }
    }

    void thread_recv_worker()
    {
        if (this.is_connect == false)
        {
            return;
        }

        while (true)
        {
            if (!this.client_socket.Connected)
            {
                break;
            }

            try
            {
                int recv_len = 0;
                if (this.recved < RECV_LEN)
                {
                    recv_len = this.client_socket.Receive(this.recv_buf, this.recved, RECV_LEN - this.recved, SocketFlags.None);
                }
                else
                {
                    if (this.long_pkg == null)
                    {
                        int pkg_size;
                        int head_size;
                        tcp_packer.read_header(this.recv_buf, this.recved, out pkg_size, out head_size);
                        this.long_pkg_size = pkg_size;
                        this.long_pkg = new byte[pkg_size];
                        Array.Copy(this.recv_buf, 0, this.long_pkg, 0, this.recved);
                    }
                    recv_len = this.client_socket.Receive(this.long_pkg, this.recved, this.long_pkg_size - this.recved, SocketFlags.None);
                }

                if (recv_len > 0)
                {
                    this.recved += recv_len;
                    this.on_recv_tcp_data();
                }
            }
            catch (System.Exception e)
            {
                Debug.Log(e.ToString());
                this.client_socket.Disconnect(true);
                this.client_socket.Shutdown(SocketShutdown.Both);
                this.client_socket.Close();
                this.is_connect = false;
                break;
            }
        }
    }

    void on_connected(IAsyncResult iar)
    {
        try
        {
            Socket client = (Socket)iar.AsyncState;
            client.EndConnect(iar);

            this.is_connect = true;
            this.recv_thread = new Thread(new ThreadStart(this.thread_recv_worker));
            this.recv_thread.Start();

            Debug.Log("connect to server success" + this.server_ip + ":" + this.port + "!");
        }
        catch (System.Exception e)
        {
            Debug.Log(e.ToString());
            this.on_connect_error(e.ToString());
            this.is_connect = false;
        }
    }

    void close()
    {
        if (!this.is_connect)
        {
            return;
        }

        // abort recv thread
        if (this.recv_thread != null)
        {
            this.recv_thread.Abort();
        }
        // end

        if (this.client_socket != null && this.client_socket.Connected)
        {
            this.client_socket.Close();
        }
    }

    private void on_send_data(IAsyncResult iar)
    {
        try
        {
            Socket client = (Socket)iar.AsyncState;
            client.EndSend(iar);
        }
        catch (System.Exception e)
        {
            Debug.Log(e.ToString());
        }
    }

    public void send_protobuf_cmd(int stype, int ctype, ProtoBuf.IExtensible body)
    {
        byte[] cmd_data = proto_man.pack_protobuf_cmd(stype, ctype, body);
        if (cmd_data == null)
        {
            return;
        }

        byte[] tcp_pkg = tcp_packer.pack(cmd_data);

        // end 
        this.client_socket.BeginSend(tcp_pkg, 0, tcp_pkg.Length, SocketFlags.None, new AsyncCallback(this.on_send_data), this.client_socket);
        // end 
    }

    public void send_json_cmd(int stype, int ctype, string json_body)
    {
        byte[] cmd_data = proto_man.pack_json_cmd(stype, ctype, json_body);
        if (cmd_data == null)
        {
            return;
        }

        byte[] tcp_pkg = tcp_packer.pack(cmd_data);
        // end 
        this.client_socket.BeginSend(tcp_pkg, 0, tcp_pkg.Length, SocketFlags.None, new AsyncCallback(this.on_send_data), this.client_socket);
        // end 
    }

    public void add_service_listener(int stype, net_message_handler handler)
    {
        if (this.event_listeners.ContainsKey(stype))
        {
            this.event_listeners[stype] += handler;
        }
        else
        {
            this.event_listeners.Add(stype, handler);
        }
    }

    public void remove_service_listener(int stype, net_message_handler handler)
    {
        if (!this.event_listeners.ContainsKey(stype))
        {
            return;
        }
        this.event_listeners[stype] -= handler;
        if (this.event_listeners[stype] == null)
        {
            this.event_listeners.Remove(stype);
        }
    }
}
