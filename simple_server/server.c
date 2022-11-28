/*
 * 程序名：server.cpp，此程序用于演示socket通信的服务端
 * 作者：C语言技术网(www.freecplus.net) 日期：20190525
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Using:./server port\nExample:./server 5005\n\n");
    return -1;
  }

  // 第1步：创建服务端的socket，和文件描述符一样
  int listenfd;
  // 初始化时只指定了所用的底层协议族为AF_INET(本地域通信)，传输层使用SOCK_STREAM（字节流协议），即TCP协议
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
  {
    perror("socket");
    return -1;
  }

  // 第2步：把服务端用于通信的地址和端口绑定到socket上。
  struct sockaddr_in servaddr; // 服务端地址信息的数据结构。
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;                // 协议族，在socket编程中只能是AF_INET。
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 任意ip地址。
  // servaddr.sin_addr.s_addr = inet_addr("192.168.190.134"); // 指定ip地址。
  servaddr.sin_port = htons(atoi(argv[1])); // 指定通信端口。
  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
  {
    perror("bind");
    close(listenfd);
    return -1;
  }

  // 第3步：把socket设置为监听模式。
  if (listen(listenfd, 5) != 0) // listen函数创建一个listen监听队列用于存放用户连接！！！
  {
    perror("listen");
    close(listenfd);
    return -1;
  }
  printf("Fininsh listening, try to accept...\n");

  // 第4步：接受客户端的连接。
  int clientfd;                             // 客户端的socket。
  int socklen = sizeof(struct sockaddr_in); // struct sockaddr_in的大小
  struct sockaddr_in clientaddr;            // 客户端的地址信息。

  // accept从listen监听队列中取出一个用户连接，当监听队列为空时，accept陷入阻塞！！！
  clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
  printf("客户端（%s）已连接。\n", inet_ntoa(clientaddr.sin_addr));

  // 第5步：与客户端通信，接收客户端发过来的报文后，回复ok。
  char buffer[1024];
  while (1)
  {
    int iret;
    memset(buffer, 0, sizeof(buffer));
    if ((iret = recv(clientfd, buffer, sizeof(buffer), 0)) <= 0) // 接收客户端的请求报文。
    {
      printf("iret=%d, waitting next connection\n", iret);
      // clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
      // printf("客户端（%s）已连接。\n", inet_ntoa(clientaddr.sin_addr));
      break;
    }
    printf("接收：%s\n", buffer);

    strcpy(buffer, "ok");
    if ((iret = send(clientfd, buffer, strlen(buffer), 0)) <= 0) // 向客户端发送响应结果。
    {
      perror("send");
      break;
    }
    printf("发送：%s\n", buffer);
  }

  // 第6步：关闭socket，释放资源。
  close(listenfd);
  close(clientfd);
}