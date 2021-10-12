#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <winsock2.h>

#include <Windows.h>

//	定义棋盘
char board[15][15];
//	角色
char role = '#';
//	落子坐标
char key_x = 7,key_y = 7;
//	套接字
int sockfd;
//	目标的结构体
struct sockaddr_in dest_addr;
int addrlen = sizeof(dest_addr);


typedef struct sockaddr* SP;

int init_board(const char* ip,short port)
{
	// 初始化网络库
    WSADATA wsaDATA;
    if(WSAStartup(MAKEWORD(2,2),&wsaDATA))
    {
        perror("WSAStartup");
        return EXIT_FAILURE;
    }
	
	//	初始化网络
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(0 > sockfd)
	{
		perror("socket");
		return EXIT_FAILURE;
	}

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(ip);
	
	//	初始化棋盘
	for(int i=0; i<15; i++)
	{
		for(int j=0; j<15; j++)
		{
			board[i][j] = '*';	
		}
	}
	
	return sockfd;
}

void show_board(void)
{
	system("clear");
	for(int i=0; i<15; i++)
	{
		for(int j=0; j<15; j++)
		{
			printf(" %c",board[i][j]);	
		}
		printf("\n");
	}
}

void get_key(void)
{
	char buf[256] = {};
	
	if(role == '@')
	{
		printf("等待对方落子...\n");
		int ret = recvfrom(sockfd,buf,sizeof(buf),0,(SP)&dest_addr,&addrlen);
		if(0 >= ret)
		{
			printf("对方逃跑了，恭喜你胜利了!\n");
			close(sockfd);
			exit(EXIT_SUCCESS);
		}

		sscanf(buf,"%hhd %hhd",&key_x,&key_y);
		board[key_x][key_y] = '@';
	}
	else
	{
		printf("请您落子...\n");
		for(;;)
		{

			printf("\33[%d;%dH",key_x+1,(key_y+1));
			switch(getch())
			{
				case 75:
					key_x > 0 && key_x--; break;
				case 77:
					key_x < 14 && key_x++; break;
				case 80:
					key_y < 14 && key_y++; break;
				case 72:
					key_y > 0 && key_y--; break;
				case 13:
					if('*' == board[key_x][key_y])
					{
						board[key_x][key_y] = '#';
						sprintf(buf,"%hhd %hhd",key_x,key_y);
						int ret = sendto(sockfd,buf,strlen(buf)+1,0,(SP)&dest_addr,addrlen);
						if(0 >= ret)
						{
							printf("你掉线了!\n");
							close(sockfd);
							exit(EXIT_FAILURE);
						}
						return;
					}
			}
		}
	}
}

int count_board(int go_x,int go_y)
{
	int count = 0;
	for(int x=key_x+go_x,y=key_y+go_y; 
		x>=0 && y>=0 && x<15 && y<15;
			x+=go_x,y+=go_y)
	{
		if(board[x][y] == board[key_x][key_y])
		{
			count++;	
		}
		else
		{
			break;	
		}
	}
	return count;
}

bool is_win(void)
{
	if(count_board(0,-1) + count_board(0,1)>=4)
	{
		return true;	
	}
	if(count_board(-1,0) + count_board(1,0)>=4)
	{
		return true;	
	}
	if(count_board(-1,-1) + count_board(1,1)>=4)
	{
		return true;	
	}
	if(count_board(1,-1) + count_board(-1,1)>=4)
	{
		return true;	
	}
	return false;
}

int main(int argc,const char* argv[])
{
	if(3 != argc)
	{
		printf("User: ./gobang ip port\n");
		return EXIT_SUCCESS;
	}

	// 初始化棋盘
	if(0 >= init_board(argv[1],atoi(argv[2])))
	{
		printf("初始化网络错误!\n");
		return EXIT_FAILURE;
	}

	for(int i=0; i<225; i++)
	{
		//	清屏，打印棋盘
		show_board();
		//	落子
		get_key();
		//	是否五子连珠
		if(is_win())
		{
			show_board();
			printf("游戏胜利，%c赢了\n",board[key_x][key_y]);
			return 0;
		}
		//	交换角色
		role = role == '@'?'#':'@';
	}
}


