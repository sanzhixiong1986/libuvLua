using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class network : MonoBehaviour
{
    public string server_ip;
    public int port;

    private Socket client_socket = null;
    private bool is_connect = false;
    private Thread recv_thread = null;
    private byte[] recv_buffer = new byte[8192];//装数据的内存

    private void Awake()
    {
        DontDestroyOnLoad(this.gameObject);
    }

    // Start is called before the first frame update
    void Start()
    {
        this.connect_to_server();
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    void on_conntect_timeout()
    {

    }

    void on_connect_error(string err)
    {

    }

    /// <summary>
    /// 链接服务器
    /// </summary>
    void connect_to_server()
    {
        try
        {
            //客户端链接
            this.client_socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            IPAddress ipAddress = IPAddress.Parse(this.server_ip);
            IPEndPoint ipEndpoint = new IPEndPoint(ipAddress, this.port);

            IAsyncResult result = this.client_socket.BeginConnect(ipEndpoint, new AsyncCallback(this.on_connected), this.client_socket);
            bool success = result.AsyncWaitHandle.WaitOne(5000, true);
            if (!success)
            {
                this.on_conntect_timeout();
            }
        }
        catch(System.Exception e)
        {
            Debug.Log(e.ToString());
            this.on_connect_error(e.ToString());
        }
    }

    void on_recv_data()
    {
        //如果没有链接就不往下走
        if(this.is_connect == false)
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
                int recv_len = this.client_socket.Receive(this.recv_buffer);
                if (recv_len > 0)
                {

                }
            }
            catch(System.Exception e)
            {
                Debug.Log(e.ToString());
                this.client_socket.Disconnect(true);//清理数据
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
            this.recv_thread = new Thread(new ThreadStart(this.on_recv_data));
            this.recv_thread.Start();

            Debug.Log("connect to server success" + this.server_ip + ":" + this.port + "!");
        }
        catch(System.Exception e)
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

        if(this.recv_thread != null)
        {
            this.recv_thread.Abort();
        }

        if(this.client_socket != null && this.client_socket.Connected)
        {
            this.client_socket.Close();
        }
    }
}
