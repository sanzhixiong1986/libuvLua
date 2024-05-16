using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class game : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        this.Invoke("test", 3.0f);

        network.instance.add_service_listener(1, this.on_service_event);
    }

    void on_service_event(cmd_msg msg)
    {
        switch (msg.ctype)
        {
            case 2:
                gprotocol.LoginRes res = proto_man.protobuf_deserialize<gprotocol.LoginRes>(msg.body);
                Debug.Log("########## res = " + res.status);
                break;
        }
    }

    private void OnDestroy()
    {
        if (network.instance)
        {
            network.instance.remove_service_listener(1, this.on_service_event);
        }
    }

    void test()
    {
        gprotocol.LoginReq req = new gprotocol.LoginReq();
        req.name = "blake";
        req.email = "blake@bycw.edu";
        req.age = 34;
        req.int_set = 8;
        network.instance.send_protobuf_cmd(1, 1, req);
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
