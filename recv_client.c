/*************************************************************************
	> File Name: recv_client.c
	> Author: zhe  
	> Mail: 1477817066@qq.com
	> Created Time: 2018年10月16日 星期二 20时08分33秒
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
#define DEFIATE  0

int GET_FILE(int *sockfd);
int PUSH_FILE(int *sockfd);
int ECHO_THIS_FILE();
int ECHO_FILE(int *sockfd);
void QUIT(int *sd);

void QUIT(int *sd){
	close(*sd);

    exit(0);
}
/*下载文件*/
int GET_FILE(int *sockfd){
    char buff[100];
    int fd;
    int read;
	char Filename[100];
	char li[5];
	int writefd;
    char recv_buf[100];
	
	memset(li, 0, 5);
	memset(buff, 0, sizeof(buff));
	memset(Filename, 0, sizeof(Filename));
    memset(recv_buf, 0, sizeof(recv_buf));

	strcpy(li, "get");
	/*发送指令*/
	
    send(*sockfd, li, sizeof(li), 0);
    recv(*sockfd, recv_buf, sizeof(recv_buf), 0);
    printf("%s\n", recv_buf);



    scanf("%s", Filename);
    /*清空缓冲区*/
    setbuf(stdin,NULL);
    
    send(*sockfd, Filename, sizeof(Filename), 0);

    if(-1 == (fd = open(Filename, O_WRONLY | O_CREAT | O_TRUNC, 0666))){
        printf("Open file error.\n");
        return DEFIATE;
    }

    while ((read = recv(*sockfd, buff, sizeof(buff), 0)) > 0){
        if(strcmp(buff, "suc") == 0){
            break;
        }
        if((writefd = write(fd, buff, read)) < 0){
            printf("Write file error.\n");
            return DEFIATE;
        }
    }
    return SUCCESS;
}
/*上传文件*/
int PUSH_FILE(int *sockfd){

    char buff[100];
    int fd;
    int readfd;
    char Filename[100];
    char li[5];
    char sucBuffer[10];


    memset(sucBuffer, 0, sizeof(sucBuffer));
	
    /*向服务器端发送上传指令*/

    memset(li, 0, 5);
    memset(buff, 0, sizeof(buff));
    memset(Filename, 0, sizeof(Filename));

    ECHO_THIS_FILE();    
    printf("请选择要上传的文件.\n");
    scanf("%s", Filename);
    
    /*清空缓冲区*/
    setbuf(stdin,NULL);
    /*判断文件是否存在*/
    if(-1 == access(Filename, R_OK)){
        printf("Can't open this file.\n");
        return DEFIATE;
    }

	strcpy(li, "push");
	/*发送指令*/
	send(*sockfd, li, sizeof(li), 0);
	send(*sockfd, Filename, sizeof(Filename), 0);

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
        
    
    printf("Client send File SUCCESS.\n");

    strcpy(sucBuffer, "suc");
    send(*sockfd, sucBuffer, sizeof(sucBuffer), 0);
    usleep(1);

    close(fd);
    return SUCCESS;
}

int ECHO_FILE(int *sockfd){
	char buff[100];
	char li[5];
	int n = 0;
	
	memset(li, 0, 5);
	memset(buff, 0, sizeof(buff));

	strcpy(li, "list");
	/*发送指令*/
	send(*sockfd, "list", sizeof("list"), 0);

	while( (n = recv(*sockfd, buff, sizeof(buff), 0)) > 0 )
	{
        if(strcmp(buff, "suc") == 0){
            break;
        }
        
		printf("%s\n", buff);
    
        
	}

	return SUCCESS;
}
/*打印本地文件列表*/
int ECHO_THIS_FILE(){
    char buff[256];
	DIR *dir;
	struct dirent *dir_ent;

    memset(buff, 0, sizeof(buff)); /*将buff数组请0*/

    if((dir = opendir(".")) == NULL){
        printf("open dir error!\n");
        return DEFIATE;
    }/*打开客户端出传来的文件.*/
    
    while ((dir_ent = readdir(dir)) != NULL){
        /*不打印隐藏文件*/
        if(strncmp(dir_ent->d_name, ".", 1) == 0){
            continue;
        }
        printf("%s\n", dir_ent->d_name);
    }
	return SUCCESS;
}
/*文件名可以为相对路径, 可以为绝对路径.*/
int main(int argc, char *argv[]){
    //创建socket
    int Server_fd;
    int Client_fd;
    int len;
    struct sockaddr_in my_addr;
    struct sockaddr_in remote_addr;
    int Sin_size;
    char buf[BUFSIZ];
    char FILENAME[100];

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(8000);

    if((Server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Create socket error.\n");
        return DEFIATE;
    }
    if(connect(Server_fd, 
        (struct sockaddr *)&my_addr,
        sizeof(my_addr)) < 0){
            
            printf("connect error.\n");
            return DEFIATE;
    }

    char search[5];
    while(1){
        printf("Coonect 成功.上传push,下载get,服务器文件列表list,本地文件列表ls \n");
        memset(search, 0, sizeof(search));   
        scanf("%s", search);
            
        /*清空缓冲区*/
        setbuf(stdin,NULL);
            
        if(strncmp(search, "push", 4) == 0){
            PUSH_FILE(&Server_fd);
        }else if(strncmp(search, "get", 3) == 0){ //测试成功
            GET_FILE(&Server_fd);
        }else if(strncmp(search, "quit", 4) == 0){
            QUIT(&Server_fd);
        }else if(strncmp(search, "list", 4) == 0){
            ECHO_FILE(&Server_fd);
        }
        
    }
    return 0;

}


