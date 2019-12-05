#! /usr/bin/env python
# coding:utf-8
# tcp_server

import socket
import threading
import csv
bind_ip = '0.0.0.0'
bind_port = 10001
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind((bind_ip, bind_port))
server.listen(5)
print('[*] listen %s:%d' % (bind_ip, bind_port))

def handle_client(client_socket):
    bufsize=1024
    f=open("iot.csv", 'a')
    writer=csv.writer(f, lineterminator="\n")

    request1=client_socket.recv(bufsize)
    if("GET" in request1):
        client_socket.send("ERROR\r\n") #ブラウザからのアクセスがあった時GET....というメッセージを受信していたので。
        f.close()
        client_socket.close()
        exit(1)

    request2=client_socket.recv(bufsize)

    if("GET" in request2):
        client_socket.send("ERROR\r\n") 
        f.close()
        client_socket.close()
        exit(1)

    request3=client_socket.recv(bufsize)
    if("GET" in request3):
        client_socket.send("ERROR\r\n")
        f.close()
        client_socket.close()
        exit(1)

    request4=client_socket.recv(bufsize)
    if("GET" in request4):
        client_socket.send("ERROR\r\n") 
        f.close()
        client_socket.close()
        exit(1)

    if(request1 and request2 and request3 and request4):
        print('[*] recv: %s' % request1)
        print('[*] recv: %s' % request2)
        print('[*] recv: %s' % request3)
        print('[*] recv: %s' % request4)
        request=[str(request1), str(request2), str(request3), str(request4)]
        if(len(request)==4):
            client_socket.send("OK\r\n")
            writer.writerow(request)
        else:
            client_socket.send("ERROR\r\n")

    
    f.close()
    client_socket.close()

while True:
    client, addr = server.accept()
    print('[*] connected from: %s:%d' % (addr[0], addr[1]))
    client_handler=threading.Thread(target=handle_client, args=(client,))
    client_handler.start()