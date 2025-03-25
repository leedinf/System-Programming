/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#define MAXARGS   128
typedef struct proc{
    int pid;
    char cmd[50];
    int state;
    int fb;
}proc;

proc jobs[100];
/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 
void exec_pipe(char* cmdline, int, char***, int);
int jobs_count;
int get_max(proc *jobs);
int get_index(proc* , pid_t);
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigchld_handler(int sig);
int get_fg_pid(struct proc *jobs);

int main() 
{
    char cmdline[MAXLINE]; /* Command line */    
    int status;
    jobs_count=0;

    Signal(SIGINT, sigint_handler);
	Signal(SIGTSTP,sigtstp_handler);
	Signal(SIGCHLD,sigchld_handler);

    while (1) {
	/* Read */
	printf("CSE4100-SP-P2> ");                   
	fgets(cmdline, MAXLINE, stdin);
    
    char *start = cmdline;
    char *end;
    if(!strncmp(cmdline,"exit",4)) return 0;
    for(int j=0;j<sizeof(cmdline);j++){
        if(cmdline[j] == '\"') cmdline[j]=' ';
    }

	/* Evaluate */
    if ((*start) ) {
        eval(start);
    }    //문제없음

	if (feof(stdin))
	    exit(0);
    } 
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    int status;

    // 파이프 파싱 arguments////
    char **s[MAXARGS];
    char bufs[MAXLINE];
    ///////////////////////////
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); 

    if (argv[0] == NULL)  
	return;   /* Ignore empty lines */
    // for(int j=0;j<sizeof(cmdline);j++){
    //     if(cmdline[j]=='&'){
    //         cmdline[j]=' ';
    //         bg=1;
    //     }
    // }

    char *start = cmdline;
    char *end;
    char tempstr[100];
    char nullchar = '\0';       // pipe 붙여서 쓸 때 오류 수정하기 위해 붙여서 쓰면 띄어서 쓴 걸로 수정해줌.
    if(strchr(cmdline,'|')){
        int i=0;

        while ((end = strchr(start, '|')) != NULL) {
            *end = '\0'; // 파이프 기호를 널 문자로 바꿔 명령어를 분리
            s[i] = malloc(MAXARGS * sizeof(char**)); // 메모리 할당
            strcpy(tempstr,start);
            tempstr[strlen(tempstr)]=' ';
            strcat(tempstr,&nullchar);
            
            printf("%s",tempstr);
            parseline(tempstr, s[i]); // 명령어 파싱
            start = end + 1; // 다음 명령어 시작 위치로 이동
            i++;
        }       
        s[i] = malloc(MAXARGS * sizeof(char*)); // 메모리 할당

        parseline(start, s[i]); // 명령어 파싱
        exec_pipe(cmdline, i+1,s,bg);
    }
    else if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
        if((pid=fork()) == 0){
            if (execvp(argv[0], argv) < 0) {	//ex) /bin/ls ls -al & 여기서 성공하면 아예 다른 프로그램이 됨 !
                printf("%s: Command not found.\n", argv[0]); // 실패하면 여기서 이거 출력하고 exit됨 !
                exit(1);
            }
        }

        if (!bg){ 
	        // int status;
            jobs_count=get_max(jobs)+1;
            jobs[jobs_count].pid = pid;
            strcpy(jobs[jobs_count].cmd,cmdline);
            jobs[jobs_count].state = 1;
            jobs[jobs_count].fb = 1;
            pause();
        }
	    else{//when there is background process!
            jobs_count=get_max(jobs)+1;
            jobs[jobs_count].pid = pid;
            strcpy(jobs[jobs_count].cmd,cmdline);
            jobs[jobs_count].state = 1;
            jobs[jobs_count].fb = 0;
	        printf("[%d]\t%d\n", jobs_count, pid);
        }
    }
    else if(!strcmp(argv[0],"cd")){
        char s[128];
        if(argv[1] == NULL) return;
        if(chdir(argv[1])){
            if(argv[1] != NULL) printf("bash: cd: %s: No such file or directory\n",argv[1]);
        }
    }
    else if(!strncmp(argv[0],"jobs",4))
	{
		for(int i=0;i<100;i++){
            if(jobs[i].pid){
                if(jobs[i].state) printf("[%d]\trunning\t\t%s",i,jobs[i].cmd);
                else printf("[%d]\tsuspended\t\t%s",i,jobs[i].cmd);
            }
        }
		return;

	}
    if(!strcmp(argv[0],"kill")){
		//Kill(pid,SIGINT);
		//delete jobs;
        int i = atoi(argv[1]+1);

        if(jobs[i].pid){
            Kill(jobs[i].pid, SIGKILL);
            jobs[i].pid=0;
        }
        else{
            printf("No Such Job\n");
            return;
        }
    }
    if(!strcmp(argv[0],"fg")){
		
        int i= atoi(argv[1]+1);
        if(jobs[i].pid==0){
            printf("No Such Job\n");
            return;
        }
		printf("[%d] running %s\n",i,jobs[i].cmd);
        jobs[i].state = 1;
        jobs[i].fb = 1;
        Kill(jobs[i].pid,SIGCONT);
        
        pause();

		return;
    }
    if(!strcmp(argv[0],"bg")){
        int i= atoi(argv[1]+1);
        if(jobs[i].pid==0){
            printf("No Such Job\n");
            return;
        }
        printf("[%d] running %s\n",i,jobs[i].cmd);
        jobs[i].state = 1;
        jobs[i].fb = 0;
        Kill(pid,SIGCONT);	
        return;
    }
	
    return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "quit")) /* quit command */
	exit(0);  
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
	return 1;
    if (!strcmp(argv[0], "cd")) return 1;
    if (!strcmp(argv[0], "jobs")) return 1;
    if (!strcmp(argv[0], "bg")) return 1;
    if (!strcmp(argv[0], "fg")) return 1;
    if (!strcmp(argv[0], "kill")) return 1;
    
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */
    
    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	argv[--argc] = NULL;
    for(int i=0;i<argc;i++){
        for(int j=0;j<strlen(argv[i]);j++){
            if(argv[i][j]=='&'){
                argv[i][j]='\0';
                bg=1;
            }
        }
    }

    return bg;
}
/* $end parseline */

void exec_pipe(char* cmdline, int n, char*** cmd,int bg){
    pid_t pid;
    int fd[2];
    int in=0;
    int status;
    int i;
    for(i=0;i<n;i++){
        int j=0;

        pipe(fd);//f[0]를 읽고    f[1] 에 쓴다
                //0 stdin 1 stdout
        pid= fork();
        if(pid==0){//자식 : 부모에게 output 넘겨줌
            
            //input control
            if(in!=0){// 전 파이프 있으면
                dup2(in, 0);
                close(in);//현재 인풋으로 넣고 삭제
            }
            //output control
            if(i<n-1){//마지막거 빼고
                close(fd[0]);//안쓰는거 지움 pipe in
                dup2(fd[1],1);//stdout 대신 fd[1]; pipe out 
                close(fd[1]);
            }
            if (execvp(cmd[i][0],cmd[i]) < 0) {	//ex) /bin/ls ls -al & 여기서 성공하면 아예 다른 프로그램이 됨 !
                printf("%d%s: Command not found.\n", n,cmd[i][0]); // 실패하면 여기서 이거 출력하고 exit됨 !
                exit(1);
            }
        }

        else{
            if(i==n-1){
                if (!bg){ 
	                jobs_count=get_max(jobs)+1;
                    jobs[jobs_count].pid = pid;
                    strcpy(jobs[jobs_count].cmd,cmdline);
                    jobs[jobs_count].state = 1;
                    jobs[jobs_count].fb = 1;
                    pause();
	            }
	            else{//when there is background process!
	                jobs_count=get_max(jobs)+1;
                    jobs[jobs_count].pid = pid;
                    strcpy(jobs[jobs_count].cmd,cmdline);
                    jobs[jobs_count].state = 1;
                    jobs[jobs_count].fb = 0;
	                printf("[%d]\t%d\n", jobs_count, pid);
                }
            }
            close(fd[1]); //쓰는거 닫음 stdout 할거임
            if(in !=0){
                close(in); // 이전거 중복되는거 방지
            }

            in = fd[0]; // fd[1]로 받은거 넘겨줌
        }
    }
    if(in!=0) close(in);

}


void sigint_handler(int sig)
{
	pid_t pid;
    int k;

	pid = get_fg_pid(jobs);

	if(pid>0)
	{
        k=get_index(jobs,pid);
        jobs[k].pid=0;
        jobs[jobs_count].cmd[0] = '\0';
        jobs[jobs_count].state = 0;
        jobs[jobs_count].fb = 0;
		Kill(pid,SIGINT);
        printf("\n");
	}
	return;

}
void sigtstp_handler(int sig)
{

	pid_t pid;
    int k;
	pid = get_fg_pid(jobs);
	if(pid>0)
	{
        k=get_index(jobs,pid);
        jobs[jobs_count].state = 0;
        jobs[jobs_count].fb = 0;
		Kill(pid,SIGSTOP);
        printf("\n");
	}
	return;
}
void sigchld_handler(int sig)
{
    pid_t pid;
    int status;
    int k;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status)) {
            //printf("자식 프로세스 %d가 종료되었습니다.\n", pid);
            k = get_index(jobs,pid);
            jobs[k].pid=0;
        } else if (WIFSIGNALED(status)) {
            //printf("자식 프로세스 %d가 시그널에 의해 종료되었습니다.\n", pid);
        } else if (WIFSTOPPED(status)) {
            //printf("자식 프로세스 %d가 중지되었습니다.\n", pid);
        }
    }

}
int get_fg_pid(struct proc *jobs){

	int i;
	proc job;
	for(int i=0;i<100;i++){
        if(jobs[i].fb == 1){
            return jobs[i].pid;
        }
    }
	
	return -1;

}
int get_max(proc *jobs){
    for(int i=100;i>=0;i--){
        if(jobs[i].pid) return i;
    }
    return 0;
}
int get_index(proc *jobs, pid_t pid){
    for(int i=0;i<100;i++){
        if(jobs[i].pid == pid) return i;
    }
    return -1;
}