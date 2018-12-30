#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "signal_handlers.h"
#include "commands.h"
#include "built_in.h"
#include "utils.h"

int main()
{
  putenv("PATH=/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin");
  signal_setting();

  char buf[8096];

  while (1) {
    memset(buf,0,sizeof(buf));
    fgets(buf, 8096, stdin);

    struct single_command commands[512];
    int n_commands = 0;
    int background_num = 0;
    int buf_len = strlen(buf);

    for(int i =0 ;i<buf_len;i++){
	if(buf[i]=='&'){
		background_num = 1;
		buf[i] = 0;
		memset(running_command,0,512);
		strcpy(running_command,buf);
		break;
	}
    }

    mysh_parse_command(buf, &n_commands, &commands);

    if(background_num){
	pid_num = fork();
	if(pid_num){
		//back_num = 1;
		printf("%d\n",pid_num);
		free_commands(n_commands,&commands);
		n_commands = 0;
		signal_setting();
		continue;
	}
    }

    int ret = evaluate_command(n_commands, &commands);

    free_commands(n_commands, &commands);
    n_commands = 0;

    if(pid_num==0){
	if(ret==0)
		printf("%d done %s\n",getpid(),running_command);
	free_commands(n_commands, &commands);
	n_commands = 0;
	exit(1);
    }
    free_commands(n_commands, &commands);
    n_commands = 0;

    if (ret == 1) {
      break;
    }
  }

  return 0;
}
