/*
 echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */
#include "csapp.h"
typedef struct{
    int ID;
    int left_stock;
    int price;
    int readcnt;
    sem_t mutex;
}item;

typedef struct{
    int maxfd;
    fd_set read_set;
    fd_set ready_set;
    int nready;
    int maxi;
    int clientfd[FD_SETSIZE];//1024
    rio_t clientrio[FD_SETSIZE];//1024
}pool;

typedef struct node{
    item item;
    struct node* left;
    struct node* right;
}node;

void echo(int connfd);
void init_pool(int, pool*);
void add_client(int connfd, pool *p);
void check_clients(pool *p);

void load_stock();
void save_stock();
void print_stock();
void sell(int a, int b);
int buy(int a, int b);
node* root;
void sigint_handler(int sig){
    char sigbuf[MAXLINE];
    sigbuf[0]='\0';
    save_stock(root,sigbuf);
    exit(0);
}
int main(int argc, char **argv) 
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    static pool pool;
    
    char client_hostname[MAXLINE], client_port[MAXLINE];

    Signal(SIGINT, sigint_handler);

    if (argc != 2) {
	    fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(0);
    }

    load_stock();

    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd, &pool);


    while (1) {
        // wait for listening / connected descriptor to become ready
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

        //if(FD_ISSET(STDIN_FILENO, &ready_set))
	    
        if(FD_ISSET(listenfd, &pool.ready_set)){
            clientlen = sizeof(struct sockaddr_storage); 
	        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            add_client(connfd, &pool);

            Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                        client_port, MAXLINE, 0);
            printf("Connected to (%s, %s)\n", client_hostname, client_port);
            
        }
        check_clients(&pool);
    }
    exit(0);
}


void init_pool(int listenfd, pool *p){
    // init :  connected fd 없음
    int i;
    p->maxi = -1;
    for(i=0; i<FD_SETSIZE; i++){
        p->clientfd[i] = -1;
    }

    // init : listenfd 는 유일한 select read set 
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, pool *p) 
{
    int i;  
    p->nready--;
    for (i = 0; i < FD_SETSIZE; i++)  /* Find an available slot */
	    if (p->clientfd[i] < 0) { 
	        /* Add connected descriptor to the pool */
	        p->clientfd[i] = connfd;                 //line:conc:echoservers:beginaddclient
	        Rio_readinitb(&p->clientrio[i], connfd); //line:conc:echoservers:endaddclient

	        /* Add the descriptor to descriptor set */
	        FD_SET(connfd, &p->read_set); //line:conc:echoservers:addconnfd

	        /* Update max descriptor and pool highwater mark */
	        if (connfd > p->maxfd) //line:conc:echoservers:beginmaxfd
	    	    p->maxfd = connfd; //line:conc:echoservers:endmaxfd
	        if (i > p->maxi)       //line:conc:echoservers:beginmaxi
	    	    p->maxi = i;       //line:conc:echoservers:endmaxi
	        break;
	    }
    if (i == FD_SETSIZE) /* Couldn't find an empty slot */
	    app_error("add_client error: Too many clients");
}
/* $end add_client */

/* $begin check_clients */
void check_clients(pool *p) 
{
    int i, connfd, n;
    char buf[MAXLINE]; 
    char obuf[MAXLINE];
    rio_t rio;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++) {
	    connfd = p->clientfd[i];
	    rio = p->clientrio[i];

	    /* If the descriptor is ready, echo a text line from it */
	    if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) { 
	        p->nready--;
	        if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
	    	    printf("Server received %d bytes on fd %d\n", n, connfd);
	    	    //Rio_writen(connfd, buf, n); //line:conc:echoservers:endecho
                //printf("%s",buf);
                obuf[0]='\0';
                if (strncmp(buf, "show", 4) == 0) {
                    print_stock(root,obuf);
                    Rio_writen(connfd,obuf,MAXLINE);
                } 
                else if (strncmp(buf, "buy ", 4) == 0) {
                    int id, N;
                    if (sscanf(buf + 4, "%d %d", &id, &N) == 2) {
                        if(buy(id,N)){
                            strcpy(obuf,"[buy] success\n");
                        }
                        else{
                            strcpy(obuf,"Not enough left stock\n");
                        }
                        Rio_writen(connfd,obuf,MAXLINE);
                    }
                } 
                else if (strncmp(buf, "sell ", 5) == 0) {
                    int id, N;
                    if (sscanf(buf + 5, "%d %d", &id, &N) == 2) {
                        strcpy(obuf,"[sell] success\n");
                        sell(id,N);
                        Rio_writen(connfd,obuf,MAXLINE);
                    }
                }

                
	        }

	        /* EOF detected, remove descriptor from pool */
	        else { 
                obuf[0]='\0';
                printf("%d closed\n",connfd);
	    	    Close(connfd); //line:conc:echoservers:closeconnfd
	    	    FD_CLR(connfd, &p->read_set); //line:conc:echoservers:beginremove
	    	    p->clientfd[i] = -1;          //line:conc:echoservers:endremove
	        }
	    }
    }
}
/* $end echoserverimain */
// void command(){
    
// };
void load_stock() {
    FILE* pFile = fopen("stock.txt", "r");
    if (pFile == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    
    int a, b, c;
    node* now;
    node* nn;
    node* pre;

    while (fscanf(pFile, "%d %d %d", &a, &b, &c) == 3) {
        now = malloc(sizeof(node));
        if (now == NULL) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }

        now->item.ID = a;
        now->item.left_stock = b;
        now->item.price = c;
        now->left = now->right = NULL;

        if (root == NULL) {
            root = now;
        } else {
            nn = root;
            pre = NULL;
            while (nn != NULL) {
                pre = nn;
                if (nn->item.ID < a) {
                    nn = nn->right;
                } else {
                    nn = nn->left;
                }
            }
            if (a < pre->item.ID) {
                pre->left = now;
            } else {
                pre->right = now;
            }
        }
    }

    fclose(pFile);
}
void save_stock(node* NODE, char* obuf){
    FILE* fp = Fopen("stock.txt","w");
    print_stock(NODE,obuf);
    fputs(obuf,fp);
    Fclose(fp);
}
void sell(int id, int n){
    //find id
    node* now;
    now=root;
    while(now->item.ID !=id){
        if(now->item.ID < id){
            now=now->right;
        }
        else{
            now=now->left;
        }
        if(now==NULL) break;
    }
    if(now!=NULL && now->item.ID == id){
        // printf("id: %d n: %d\n",now->item.ID,now->item.left_stock);
        now->item.left_stock += n;
        // printf("id: %d n: %d\n",now->item.ID,now->item.left_stock);
    }
}  
int buy(int id, int n){
    node* now;
    now=root;
    while(now->item.ID !=id){
        if(now->item.ID < id){
            now=now->right;
        }
        else{
            now=now->left;
        }
        if(now==NULL) break;
    }
    if(now!=NULL){
        if(now->item.left_stock>=n){
            now->item.left_stock -= n;
            return 1;
        }
        else return 0;
    }

    return 0;
}

void print_stock(node* NODE, char* obuf){
    node* now = NODE;
    char str[MAXLINE];
    if(now !=NULL){
        sprintf(str,"%d %d %d\n",now->item.ID,now->item.left_stock,now->item.price);
    }
    strcat(obuf,str);
    if(now->left !=NULL){
        print_stock(now->left,obuf);
    }
    if(now->right !=NULL){
        print_stock(now->right,obuf);
    }
}
