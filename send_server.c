/*************************************************************************
	> File Name: send_server.c
    	> Author: zhe  
	> Mail: 1477817066@qq.com
	> Created Time: 2018年10月16日 星期二 19时33分34秒
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <dirent.h>

#define SUCCESS 1
#define DEFIATE 0
#define filename_len 128
#define readbuf 100
#define sendbuf 100



/*向客户端发送文件*/
int PUSH_FILE(int *sockfd);
/*获取文件*/
int GET_FILE(int *sockfd);
/*获取普通文件*/
int GET_REGFILE(char *Filename, int *sockfd);
/*拷贝普通文件*/
int COPY_REGFILE(char *Filename, int *sockfd);
/*打印文件目录*/
int ECHO_FILE(int *sockfd);

/*向客户端发送文件*/
int PUSH_FILE(int *sockfd){

    char buff[100];
    int nrd;

	char buf[100];
	memset(buf, 0, 100);
    
    if((nrd = recv(*sockfd, buff, sizeof(buff), 0)) > 0){
        COPY_REGFILE(buff, sockfd);
    }else{
    	return DEFIATE;
    }
    
    return SUCCESS;
}
/*获取文件*/
int GET_FILE(int *sockfd){

    char buff[100];
    int nrd;
    char buf[100];
	
    memset(buf, 0, 100);
    strcpy(buf, "请选择要下载的文件.");
    send(*sockfd, buf, sizeof(buf),0);
    
    /*向客户端发送文件信息*/
    //ECHO_FILE(sockfd);

    if((nrd = recv(*sockfd, buff, sizeof(buff), 0)) > 0){
        GET_REGFILE(buff, sockfd);
    }else{
    	return DEFIATE;
    }
    
    return SUCCESS;
}


/*打印文件目录*/
int ECHO_FILE(int *sockfd){
    char buff[100];
    DIR *dir;
    struct dirent *dir_ent;
    char sucBuffer[10];


    memset(sucBuffer, 0, sizeof(sucBuffer));
    memset(buff, 0, sizeof(buff));/*将buff数组请0*/

    if((dir = opendir(".")) == NULL){
        printf("open dir error!\n");
        return DEFIATE;
    }/*打开客户端出传来的文件.*/
    
    while ((dir_ent = readdir(dir)) != NULL){
        /*不打印隐藏文件*/
        if(dir_ent->d_name[0] == '.'){
            ;
        }else{
            strcpy(buff, dir_ent->d_name);
            send(*sockfd, buff, sizeof(buff), 0);
            usleep(1);
            memset(buff, 0, sizeof(buff));
        }
    }

    strcpy(sucBuffer, "suc");
    send(*sockfd, sucBuffer, sizeof(sucBuffer), 0);
    usleep(1);

	return SUCCESS;
}

/*客户端获取文件*/
int GET_REGFILE(char *Filename, int *sockfd){
    
    char buff[100];
    int fd;
    int readfd;
    char sucBuffer[10];

    memset(sucBuffer, 0, sizeof(sucBuffer));

    /*判断文件是否存在*/
    if(-1 == access(Filename, R_OK)){
        printf("Can't open this file.\n");
        return DEFIATE;
    }

    /*打开文件*/
    if((fd = open(Filename, O_RDONLY)) == -1){
        printf("Can't open this file.\n");
        return DEFIATE;
    }

    /*传输文件*/
    while((readfd = read(fd, buff, sizeof(buff))) > 0){
        send(*sockfd, buff, readfd, 0);
        usleep(1);
    }

    printf("Sever send File SUCCESS.\n");

    strcpy(sucBuffer, "suc");
    send(*sockfd, sucBuffer, sizeof(sucBuffer), 0);
    usleep(1);

    close (fd);

    return SUCCESS;
}


/*客户端上传文件*/
int COPY_REGFILE(char *Filename, int *sockfd){
 
    char buff[100];
    int read;
    int fd;
    int writefd;

    memset(buff, 0, sizeof(buff));
    /*创建文件*/
    if(-1 == (fd = open(Filename, O_WRONLY | O_CREAT | O_TRUNC, 0666))){
        printf("Open file error.\n");
        return DEFIATE;
    }

    /*传输文件*/
    while ((read = recv(*sockfd, buff, sizeof(buff), 0)) > 0){
         if(strcmp(buff, "suc") == 0){
            break;
        }
        if((writefd = write(fd, buff, read)) < 0){
            printf("Write file error.\n");
            return DEFIATE;
        }
        memset(buff, 0, sizeof(buff));
    }

    printf("Sever get file success.\n");

    close(fd);

    return SUCCESS;
}


int main(int argc, char *argv[]){
    int Server_fd;/*服务器套接字*/
    int Client_fd;/*客户端套接字*/
    int len;

    struct sockaddr_in my_addr;
    struct sockaddr_in remote_addr;
    int Sin_size;
    char buf[BUFSIZ];
    
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(8000);

    /*创建服务器套接字*/
    if((Server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Socket error.\n");
        exit(0);
    }

    /*绑定端口*/
    if((bind(Server_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))) < 0){
        printf("bind error.\n");
        exit(0);
    }

    
    /*监听端口*/
    if(listen (Server_fd, 10) < 0){
        printf("listen error.\n");
        exit(0);
    }

    Sin_size = sizeof(struct sockaddr_in);
    
    while(1){
        /*接收客户端发来的链接请求*/
        if((Client_fd = accept(Server_fd, (struct sockaddr *)&remote_addr, &Sin_size)) < 0){
            printf("accept error.\n");
             exit(0);
        }

        printf("Connect from [%s:%d]\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
        char get_string[5];       
        while(1){
            memset(get_string, 0, 5);
            int len = 0;
            if((len = recv(Client_fd, get_string, sizeof(get_string), 0) > 0)){
                if ( strncmp("get", get_string, 3)  == 0 ){
                    GET_FILE(&Client_fd);
                }
                else if( strncmp("push", get_string, 4) == 0 ){
                    PUSH_FILE(&Client_fd);
                }
                else  if( strncmp("list", get_string, 4) == 0 ){
                    ECHO_FILE(&Client_fd);
                }
            }
        }
    }
        
    close(Server_fd);
    close(Client_fd);
    return 0;
}

