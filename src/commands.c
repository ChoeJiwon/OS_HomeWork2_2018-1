#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
//#include <socket.h>

#include "commands.h"
#include "built_in.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>

#define SOCK_PATH "Socket"

struct sockaddr_un{
  sa_family_t sun_family;
  char sun_path[512];
};

int running;
static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static void exec(struct single_command *com){
  execv(com->argv[0],com->argv);
  char PATH[100];
  char* saveptr=NULL;
  char* tok;

  sprintf(PATH,"%s",getenv("PATH"));
  tok=strtok_r(PATH,":",&saveptr);
  while(tok){
	char path[100];
	sprintf(path,"%s/%s",tok,com->argv[0]);
	execv(path,com->argv);
	tok=strtok_r(NULL,":",&saveptr);
  }
}

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
  if (n_commands > 0) {
    struct single_command* com = (*commands);

    assert(com->argc != 0);

    int built_in_pos = is_built_in_command(com->argv[0]);
    if (built_in_pos != -1) {
      if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
        if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
          fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
        }
      } else {
        fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
        return -1;
      }
    } else if (strcmp(com->argv[0], "") == 0) {
      return 0;
    } else if (strcmp(com->argv[0], "exit") == 0) {
      return 1;
    } else if(n_commands==1){
	int status = 0;
	int pid_num = getpid();
	running = fork();
	if(running==0){
		exec(com);
		printf("%s : command not found\n",com->argv[0]);
		exit(1);
	} else{
		waitpid(running,&status,0);
		if(status) return -1;
		return 0;
	}
    } else {
	void* status;
	pthread_t threads[2];
	int pid_num=fork();
	if(pid_num==0){
		for(int a=0;a<n_commands;a++,com++){
			pthread_create(&threads[1],NULL,&threads[1],(void *)com);
			pthread_join(threads[1],&status);
		}
		exit(0);
	} else{
		pthread_create(&threads[0],NULL,&threads[0],(void *)&n_commands);
		waitpid(pid_num,NULL,0);
		pthread_join(threads[0],NULL);
	}
    }
  }

  return 0;
}

void *server_thread(void *n){
  int serverSocket = 0;
  int clientSocket = 0;
  int serverAddrSize = 0;
  socklen_t clientAddrSize = 0;

  struct sockaddr_un serverAddress;
  struct sockaddr_un clientAddress;

  bzero(&serverAddress,sizeof(serverAddress));
  bzero(&clientAddress,sizeof(clientAddress));

  remove(SOCK_PATH);
  serverSocket = socket(PF_LOCAL,SOCK_STREAM,0);

  memset(&serverAddress,0,sizeof(serverAddress));
  serverAddress.sun_family = AF_LOCAL; 
  strcpy(serverAddress.sun_path, SOCK_PATH);
  serverAddrSize = sizeof(serverAddress);
  bind(serverSocket,(struct sockaddr *)&serverAddress, serverAddrSize);

  listen(serverSocket,5);
  int std=dup(1);
  char buf[512];
  memset(buf,0,sizeof(buf));
  memset(&clientAddress,0,sizeof(clientAddress));
  clientAddrSize = sizeof(clientAddress);

  int N=(int *)n;
  while(N--){
	clientSocket = accept(serverSocket,(struct sockaddr *)&clientAddress, &clientAddrSize);
	send(clientSocket,buf,strlen(buf)+1,0);

	memset(buf,0,sizeof(buf));
	recv(clientSocket,buf,512,0);

	close(clientSocket);
  }
  dup2(std,1);
  printf("%s",buf);
  close(serverSocket);
  remove(SOCK_PATH);
  return 0;
}

void *client_thread(void* com){
  int socketFd = 0;
  struct sockaddr_un address;
  int result = 0;
  char buf[512]={0};
  int addSize=0;

  socketFd = socket(PF_LOCAL,SOCK_STREAM,0);

  memset(&address,0,sizeof(address));
  address.sun_family = AF_LOCAL;
  strcpy(address.sun_path, SOCK_PATH);
  addSize=sizeof(address);
  result = connect(socketFd,(struct sockaddr*)&address, addSize);

  if(result == -1){
	printf("client : ERROR : Connection Failed\n");
	return (void *)-1;
  }

  int fd[2];
  pipe(fd);
  int pid_num = fork();
  int status;
  if(pid_num==0){
	recv(socketFd,buf,512,0);
	FILE *fp = fopen("temp","w");
	fprintf(fp,"%s",buf);
	fclose(fp);
	freopen("temp","r",stdin);
	dup2(fd[1],1);
	exec((struct single_command *)com);
	printf("%s : command not found\n",((struct single_command*)com)->argv[0]);
	exit(1);
  }
  waitpid(pid_num,&status,0);
  if(status!=0) return (void *)-1;
  memset(buf,0,sizeof(buf));
  read(fd[0],buf,512);
  write(socketFd,buf,strlen(buf)+1);
  close(socketFd);
  remove("temp");
  return 0;




}
void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
