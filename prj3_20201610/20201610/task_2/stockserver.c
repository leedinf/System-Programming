/*
 echoserveri.c - An iterative echo server 
 */ 
/* $begin echoserverimain */

// #define
#define NTHREADS 40
#define SBUFSIZE 2000

#include "csapp.h"
typedef struct{
    int ID;
    int left_stock;
    int price;
    int readcnt;
    sem_t mutex;
    sem_t w;
}item;

typedef struct {
 int *buf; /* Buffer array */
 int n; /* Maximum number of slots */
 int front; /* buf[(front+1)%n] is the first item */
 int rear; /* buf[rear%n] is the last item */
 sem_t mutex; /* Protects accesses to buf */
 sem_t slots; /* Counts available slots */
 sem_t items; /* Counts available items */
} sbuf_t;

typedef struct node{
    item item;
    struct node* left;
    struct node* right;
}node;

void echo(int connfd);

void sbuf_init(sbuf_t *sp, int n)
{
   sp->buf = Calloc(n, sizeof(int));
   sp->n = n;
   sp->front = sp->rear = 0;
   Sem_init(&sp->mutex,0,1);
   Sem_init(&sp->slots,0,n);
   Sem_init(&sp->items, 0, 0);
}

void sbuf_deinit(sbuf_t *sp)
{
   Free(sp->buf);
}
void sbuf_insert(sbuf_t *sp, int item)
{
   P(&sp->slots);
   P(&sp->mutex);
   sp->buf[(++sp->rear)%(sp->n)] = item;
   V(&sp->mutex);
   V(&sp->items);
}
int sbuf_remove(sbuf_t *sp)
{
   int item;
   P(&sp->items);
   P(&sp->mutex);
   item = sp->buf[(++sp->front)%(sp->n)];
   V(&sp->mutex);
   V(&sp->slots);
   return item;
}

void *thread(void *vargp);

static int byte_cnt; /* Byte counter */
static sem_t mutex; /* and the mutex that protects it */
static void init_echo_cnt(void)
{
    Sem_init(&mutex, 0, 1);
    byte_cnt = 0;
}

void load_stock();
void save_stock();
void print_stock();
void sigint_handler(int sig);
void sell(int a, int b);
int buy(int a, int b);
sbuf_t sbuf;
node* root;



void echo_cnt(int connfd){
    int n;
    char buf[MAXLINE];
    char obuf[MAXLINE];
    rio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;

    Pthread_once(&once, init_echo_cnt);
    Rio_readinitb(&rio, connfd);
    while ( ( n = Rio_readlineb(&rio, buf, MAXLINE) ) != 0 ) {
        // P(&mutex);
        byte_cnt += n;
        printf("thread %d received %d (%d total) bytes on fd %d\n",
         (int) pthread_self(), n, byte_cnt, connfd);
        obuf[0]='\0';
        if (strncmp(buf, "show", 4) == 0) {
            P(&root->item.mutex);
            root->item.readcnt++;
            if(root->item.readcnt == 1){
                P(&root->item.w);
            }
            V(&root->item.mutex);

            print_stock(root,obuf);
            P(&root->item.mutex);
            root->item.readcnt--;
            if (root->item.readcnt == 0) {
                V(&root->item.w);
            }
            V(&root->item.mutex);
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
            }
        } 
        else if (strncmp(buf, "sell ", 5) == 0) {
            int id, N;
            if (sscanf(buf + 5, "%d %d", &id, &N) == 2) {
                strcpy(obuf,"[sell] success\n");
                sell(id,N);
            }
        }
        Rio_writen(connfd,obuf,MAXLINE);
        // obuf[0]='\0'
        // save_stock(root,obuf);
        // V(&mutex);
    }

}


/////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv) 
{
    int listenfd, connfd, i;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    pthread_t tid;
    
    Signal(SIGINT, sigint_handler);
    char client_hostname[MAXLINE], client_port[MAXLINE];


    if (argc != 2) {
       fprintf(stderr, "usage: %s <port>\n", argv[0]);
       exit(0);
    }

    load_stock();

    listenfd = Open_listenfd(argv[1]);

    sbuf_init(&sbuf, SBUFSIZE);
    for ( i = 0; i < NTHREADS; i++ )/* Create a pool of worker threads */
        Pthread_create(&tid, NULL, thread, NULL);
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        sbuf_insert(&sbuf, connfd); /* Insert connfd in buffer */

        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, 
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        // Pthread_create(&tid, NULL, thread, NULL); 
    }
}
/////////////////////////////////////////////////////////////////////////



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
        Sem_init(&now->item.w,0,1);
        Sem_init(&now->item.mutex,0,1);
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
        P(&now->item.w);
        // printf("id: %d n: %d\n",now->item.ID,now->item.left_stock);
        now->item.left_stock += n;
        V(&now->item.w);
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
        //critical
        P(&now->item.w);
        if(now->item.left_stock>=n){
            now->item.left_stock -= n;
            V(&now->item.w);
            return 1;
        }
        else{
            V(&now->item.w);
            return 0;
        } 
        
    }

    return 0;
}
void save_stock(node* NODE, char* obuf){
    FILE* fp = Fopen("stock.txt","w");
    obuf[0]='\0';
    print_stock(NODE,obuf);
    fputs(obuf,fp);
    Fclose(fp);
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

void *thread(void *vargp){
    Pthread_detach(pthread_self());
    while(1){
        int connfd = sbuf_remove(&sbuf);
        echo_cnt(connfd);
        Close(connfd);
    }
}

void sigint_handler(int sig){
    char sigbuf[MAXLINE];
    sigbuf[0]='\0';
    save_stock(root,sigbuf);
    exit(0);
}
// {
//     //save_stock(root,obuf);
//     kill(getpid(), SIGINT);
//    return;
// }
/* $begin check_clients */
// void check_clients(pool *p) 
// {
//     int i, connfd, n;
//     char buf[MAXLINE]; 
//     char obuf[MAXLINE];
//     rio_t rio;

//     for (i = 0; (i <= p->maxi) && (p->nready > 0); i++) {
//        connfd = p->clientfd[i];
//        rio = p->clientrio[i];

//        /* If the descriptor is ready, echo a text line from it */
//        if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))) { 
//            p->nready--;
//            if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
//               printf("Server received %d bytes on fd %d\n", n, connfd);
//               //Rio_writen(connfd, buf, n); //line:conc:echoservers:endecho
//                 //printf("%s",buf);
//                 obuf[0]='\0';
//                 if (strncmp(buf, "show", 4) == 0) {
//                     print_stock(root,obuf);
//                     Rio_writen(connfd,obuf,MAXLINE);
//                 } 
//                 else if (strncmp(buf, "buy ", 4) == 0) {
//                     int id, N;
//                     if (sscanf(buf + 4, "%d %d", &id, &N) == 2) {
//                         if(buy(id,N)){
//                             strcpy(obuf,"[buy] success\n");
//                         }
//                         Rio_writen(connfd,obuf,MAXLINE);
//                     }
//                 } 
//                 else if (strncmp(buf, "sell ", 5) == 0) {
//                     int id, N;
//                     if (sscanf(buf + 5, "%d %d", &id, &N) == 2) {
//                         strcpy(obuf,"[sell] success\n");
//                         sell(id,N);
//                         Rio_writen(connfd,obuf,MAXLINE);
//                     }
//                 }
                
//            }

//            /* EOF detected, remove descriptor from pool */
//            else { 
//                 obuf[0]='\0';
//                 save_stock(root,obuf);
//                 printf("%d closed\n",connfd);
//               Close(connfd); //line:conc:echoservers:closeconnfd
//               FD_CLR(connfd, &p->read_set); //line:conc:echoservers:beginremove
//               p->clientfd[i] = -1;          //line:conc:echoservers:endremove
//            }
//        }
//     }
// }
/* $end echoserverimain */
// void command(){
    
// };