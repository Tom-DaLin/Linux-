#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
 
#define PORT 39002
#define MAX_FD_NUM 3
#define BUF_SIZE 512
#define ERR_EXIT(m)         \
    do                      \
    {                       \
        perror(m);          \
        exit(EXIT_FAILURE); \
    } while (0)
 
int main()
{
    //创建套接字
    int m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd < 0)
    {
        ERR_EXIT("create socket fail");
    }
 
    //初始化socket元素
    struct sockaddr_in server_addr;
    int server_len = sizeof(server_addr);
    memset(&server_addr, 0, server_len);
 
    server_addr.sin_family = AF_INET;
    //server_addr.sin_addr.s_addr = inet_addr("0.0.0.0"); //用这个写法也可以
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
 
    //绑定文件描述符和服务器的ip和端口号
    int m_bindfd = bind(m_sockfd, (struct sockaddr *)&server_addr, server_len);
    if (m_bindfd < 0)
    {
        ERR_EXIT("bind ip and port fail");
    }
 
    //进入监听状态，等待用户发起请求
    int m_listenfd = listen(m_sockfd, MAX_FD_NUM);
    if (m_listenfd < 0)
    {
        ERR_EXIT("listen client fail");
    }
 
    //定义客户端的套接字，这里返回一个新的套接字，后面通信时，就用这个m_connfd进行通信
    //struct sockaddr_in client_addr;
    //socklen_t client_len = sizeof(client_addr);
    //int m_connfd = accept(m_sockfd, (struct sockaddr *)&client_addr, &client_len);
 
    printf("client accept success\n");
 
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
 
    //接收客户端数据，并相应
    char buffer[BUF_SIZE];
    int array_fd[MAX_FD_NUM];
    //客户端连接数量
    int client_count = 0;

    fd_set tmpfd;
    int max_fd = m_sockfd;
    struct timeval timeout;
 
    for (int i = 0; i < MAX_FD_NUM; i++)
    {
        array_fd[i] = -1;
    }
    //array_fd[0] = m_sockfd;
 
    while (1)
    {
        FD_ZERO(&tmpfd);
        FD_SET(m_sockfd, &tmpfd);   // m_sockfd是服务器的socket，也加入到fd_set中，后面交给select进行监听。
        int i;
 
        //所有在线的客户端加入到fd中，并找出最大的socket
        for (i = 0; i < MAX_FD_NUM; i++)
        {
            if (array_fd[i] > 0)
            {
                FD_SET(array_fd[i], &tmpfd); //set array_fd in red_set
                if (max_fd < array_fd[i])
                {
                    max_fd = array_fd[i]; //get max_fd
                }
            }
        }
 
        int ret = select(max_fd + 1, &tmpfd, NULL, NULL, NULL);
        if (ret < 0)
        {
            ERR_EXIT("select fail");
        }
        else if (ret == 0)
        {
            //ERR_EXIT("select timeout"); //超时不是错误，不可断掉连接
            printf("select timeout\n");
            continue;
        }
 
        //表示有客户端连接
        if (FD_ISSET(m_sockfd, &tmpfd))
        {
            int m_connfd = accept(m_sockfd, (struct sockaddr *)&client_addr, &client_len);
            if (m_connfd < 0)
            {
                ERR_EXIT("server accept fail");
            }
 
            //客户端连接数已满
            if (client_count >= MAX_FD_NUM)
            {
                printf("max connections arrive！！！\n");
                // char buff[]="max connections arrive！！！";
                // send(m_connfd, buff, sizeof(buff) - 1, 0);
                close(m_connfd);
                continue;
            }
 
            //客户端数量加1
            client_count++;
            printf("we got a new connection, client_socket=%d, client_count=%d, ip=%s, port=%d\n", m_connfd, client_count, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
 
            for (i = 0; i < MAX_FD_NUM; i++)
            {
                if (array_fd[i] == -1)
                {
                    array_fd[i] = m_connfd;
                    break;
                }
            }
        }
 
        //遍历所有的客户端连接，找到发送数据的那个客户端描述符
        for (i = 0; i < MAX_FD_NUM; i++)
        {
            if (array_fd[i] < 0)
            {
                continue;
            }
            //有客户端发送过来的数据
            else
            {
                if (FD_ISSET(array_fd[i], &tmpfd))
                {
                    memset(buffer, 0, sizeof(buffer)); //重置缓冲区
                    int recv_len = recv(array_fd[i], buffer, sizeof(buffer) - 1, 0);
                    if (recv_len < 0)
                    {
                        ERR_EXIT("recv data fail");
                    }
                    //客户端断开连接
                    else if (recv_len == 0)
                    {
                        client_count--;
                        //打印断开的客户端数据
                        printf("client_socket=[%d] close, client_count=[%d], ip=%s, port=%d\n\n", array_fd[i], client_count, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                        close(array_fd[i]);
                        FD_CLR(array_fd[i], &tmpfd);
                        array_fd[i] = -1;
                    }
                    else
                    {
                        printf("server recv:%s\n", buffer);
                        strcat(buffer, "+ACK");
                        send(array_fd[i], buffer, sizeof(buffer) - 1, 0);
                    }
                }
            }
        }
    }
 
    //关闭套接字
    close(m_sockfd);
 
    printf("server socket closed!!!\n");
 
    return 0;
}