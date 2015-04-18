#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
void init_daemon(void)
{
	int pid;
	int i;
	char *id;
	FILE *fp;
	pid_t p;
	pid=fork();
	if(pid>0) exit(0);//是父进程，结束父进程
	else if(pid<0) exit(1);//fork失败，退出
	//是第一子进程，后台继续执行
	setsid();//第一子进程成为新的会话组长和进程组长
	//并与控制终端分离
	pid=fork();
	if(pid > 0) exit(0);//是第一子进程，结束第一子进程 
	else if(pid<0) exit(1);//fork失败，退出
	//是第二子进程，继续
	//第二子进程不再是会话组长
	for(i=0;i<NOFILE;++i) close(i);//关闭打开的文件描述符
	chdir("/tmp");//改变工作目录到/tmp
	umask(0);//重设文件创建掩模
	p = getpid();
	id = (char *)malloc(16);
	sprintf(id,"%d",p);
	fp = fopen("/tmp/weather_home.pid","w+");
	fwrite(id,1,strlen(id),fp);
	fclose(fp);
	free(id);
	return;
}
