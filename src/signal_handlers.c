#include "signal_handlers.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "built_in.h"
#include "commands.h"

#define _GNU_SOURCE
struct sigset_t{
  unsigned int __sigbits[4];
};
struct sigaction{
  void(*sa_handler)(int);

  struct sigset_t sa_mask;
  int sa_flags;
};
struct sigaction act_INT;
struct sigaction act_TSTP;

int Mpid_num;
void catch_sigint(int signalNo)
{
  // TODO: File this!
  int cur_num = getpid();
  if(cur_num!=Mpid_num&&cur_num!=pid_num)
	exit(1);
  printf("cur_num : %d",cur_num);
  printf("\nShell doesn't close!!!\n");

}

void catch_sigtstp(int signalNo)
{
  // TODO: File this!
  printf("\nIt doesn't move!!!\n");
}

void signal_setting(){
  Mpid_num = getpid();
//  ANUM = Mpid_num;
  printf("Mpid_Num : %d \npid_num : %d\n",Mpid_num,pid_num);

//  if(pid_num!=Mpid_num){
 // if(back_num==0){
	act_INT.sa_handler = catch_sigint;
	sigemptyset(&act_INT.sa_mask);
	sigaction(SIGINT,&act_INT,NULL);

	act_TSTP.sa_handler = catch_sigtstp;
	sigemptyset(&act_TSTP.sa_mask);
	sigaction(SIGTSTP,&act_TSTP,NULL);
//  }
//  else if(back_num==1){
//	signal(SIGINT,SIG_IGN);
//	signal(SIGTSTP,SIG_IGN);
//}
/* else if(pid_num==1){
	act_INT.sa_handler = catch_sigint;
        sigemptyset(&act_INT.sa_mask);
        sigaction(SIGINT,&act_INT,NULL);

        act_TSTP.sa_handler = catch_sigtstp;
        sigemptyset(&act_TSTP.sa_mask);
        sigaction(SIGTSTP,&act_TSTP,NULL);

  }*/// else{
//	signal(SIGINT,SIG_IGN);
//	signal(SIGTSTP,SIG_IGN);
//  }
}
