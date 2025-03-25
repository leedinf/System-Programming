#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"list.h"
#include"hash.h"
#include"bitmap.h"

int main(){
    char input_string[200];
    char *token;
    char words[10][200];
    //struct list *list_array[10];
    struct list **list_array = (struct list**)malloc(sizeof(struct list*)*10);
    struct hash **hash_table = (struct hash**)malloc(sizeof(struct hash*)*10);
    struct bitmap **bitmap_array = (struct bitmap**)malloc(sizeof(struct bitmap*)*10);
    struct list* tmplist;
    struct list_elem* tmpelem;
    struct list_item* tmpitem;
    struct hash* tmphash;
    struct hash_elem* hashelem;
    struct bitmap *tmpmap;
    int token_count;//들어오는 단어의 개수
    while(1){
        token_count=0;
        fgets(input_string, sizeof(input_string),stdin);
        if(input_string[strlen(input_string) -1] =='\n') input_string[strlen(input_string)-1] = '\0';
        if(!strcmp(input_string,"quit")) return 0;

        token = strtok(input_string, " ");
        while(token != NULL){
            strcpy(words[token_count], token);
            token_count++;
            token = strtok(NULL, " ");
        }
    
        if(!strcmp(words[0],"create")){
            if(!strcmp(words[1],"list")){
                struct list *temp_list = (struct list*)malloc(sizeof(struct list));
                list_init(temp_list);
                list_array[words[2][4]-'0']=temp_list;
            }
            if(!strcmp(words[1],"hashtable")){
                struct hash *temp_hash = (struct hash*)malloc(sizeof(struct hash));
                hash_init(temp_hash,hash_func,hash_less,NULL);
                hash_table[words[2][4]-'0']=temp_hash;
            }
            if(!strcmp(words[1],"bitmap")){
                tmpmap = bitmap_create(atoi(words[3]));
                bitmap_array[words[2][2]-'0'] = tmpmap;
            }
        }
        
        if(!strcmp(words[0],"delete")){
            if(!strncmp(words[1],"list",4)){
                if(list_array[words[1][4]-'0']!=NULL){
                    tmplist=list_array[words[1][4]-'0'];
                    while(!list_empty(tmplist)){
                        list_remove(list_front(tmplist));
                    }
                }
            }
            if(!strncmp(words[1],"hash",4)){
                if(hash_table[words[1][4]-'0']!=NULL){
                    tmphash=hash_table[words[1][4]-'0'];
                    hash_destroy(tmphash,hash_destructor);
                }
            }
            if(!strncmp(words[1],"bm",2)){
                if(bitmap_array[words[1][2]-'0']!=NULL){
                    tmpmap = bitmap_array[words[1][2]-'0'];
                    bitmap_destroy(tmpmap);
                }
            }
        }
        if(!strcmp(words[0],"dumpdata")){
            if(!strncmp(words[1],"list",4)){
                if(list_array[words[1][4]-'0']!=NULL){
                    tmplist=list_array[words[1][4]-'0'];
                    if(!list_empty(tmplist)){
                        
                        tmpelem=list_front(tmplist);
                        while(tmpelem!=&tmplist->tail){
                            int a=list_entry(tmpelem, struct list_item,elem)->data;
                            printf("%d ",a);
                            tmpelem=tmpelem->next;
                        }
                        printf("\n");
                    }
                    //if(list_empty(tmplist)) printf("empty\n");
                }
                else{
                    //printf("NULL list\n");
                } 
            } 
            if(!strncmp(words[1],"hash",4)){
                if(hash_table[words[1][4]-'0']!=NULL){
                    tmphash = hash_table[words[1][4]-'0'];
                    if(!hash_empty(tmphash)){
                        struct hash_iterator *iter= (struct hash_iterator*)malloc(sizeof(struct hash_iterator));
                        hash_first(iter, tmphash);
                        while(hash_next(iter) != NULL ){
                            struct hash_elem *cur = hash_cur(iter);
                            if(cur == NULL) break; // 현재 요소가 NULL이면 반복 종료
                            printf("%d ", cur->value);
                        }
                        printf("\n");
                    }
                }
            } 
            if(!strncmp(words[1],"bm",2)){
                if(bitmap_array[words[1][2]-'0'] != NULL){
                    tmpmap = bitmap_array[words[1][2]-'0'];
                    for(size_t i=0;i<bitmap_size(tmpmap);i++){
                        printf("%d",bitmap_test(tmpmap,i));
                    }
                    printf("\n");
                }
            }
        }
        if(!strcmp(words[0],"bitmap_mark")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            bitmap_mark(tmpmap,atoi(words[2]));
        }
        if(!strcmp(words[0],"bitmap_all")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            if(bitmap_all(tmpmap,atoi(words[2]),atoi(words[3])))printf("%s\n","true");
            else printf("%s\n","false");
        }
        if(!strcmp(words[0],"bitmap_any")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            if(bitmap_any(tmpmap,atoi(words[2]),atoi(words[3])))printf("%s\n","true");
            else printf("%s\n","false");
        }
        if(!strcmp(words[0],"bitmap_contains")){
            bool val = false;
            if(!strcmp(words[4],"true")) val= true;
            else if(!strcmp(words[4],"false")) val = false;
            tmpmap = bitmap_array[words[1][2]-'0'];
            if(bitmap_contains(tmpmap,atoi(words[2]),atoi(words[3]),val)) printf("%s\n","true");
            else printf("%s\n","false");
            
        }
        if(!strcmp(words[0],"bitmap_count")){
            bool val = false;
            if(!strcmp(words[4],"true")) val= true;
            else if(!strcmp(words[4],"false")) val = false;
            tmpmap = bitmap_array[words[1][2]-'0'];
            printf("%zu\n",(bitmap_count(tmpmap,atoi(words[2]),atoi(words[3]),val)));

        }
        if(!strcmp(words[0],"bitmap_dump")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            bitmap_dump(tmpmap);
        }
        if(!strcmp(words[0],"bitmap_flip")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            bitmap_flip(tmpmap,atoi(words[2]));
        }
        if(!strcmp(words[0],"bitmap_expand")){
            tmpmap = bitmap_array[words[1][2]-'0'];

        }
        if(!strcmp(words[0],"bitmap_none")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            if(bitmap_none(tmpmap,atoi(words[2]),atoi(words[3]))) printf("true\n");
            else printf("false\n");
        }
        if(!strcmp(words[0],"bitmap_reset")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            bitmap_reset(tmpmap,atoi(words[2]));
        }
        if(!strcmp(words[0],"bitmap_scan")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            bool val = false;
            if(!strcmp(words[4],"true")) val= true;
            else if(!strcmp(words[4],"false")) val = false;
            printf("%zu\n",bitmap_scan(tmpmap,atoi(words[2]),atoi(words[3]),val));
        }
        if(!strcmp(words[0],"bitmap_scan_and_flip")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            bool val = false;
            if(!strcmp(words[4],"true")) val= true;
            else if(!strcmp(words[4],"false")) val = false;
            printf("%zu\n",bitmap_scan_and_flip(tmpmap,atoi(words[2]),atoi(words[3]),val));

        }
        if(!strcmp(words[0],"bitmap_set")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            bool val = false;
            if(!strcmp(words[3],"true")) val= true;
            else if(!strcmp(words[3],"false")) val = false;
            bitmap_set(tmpmap,atoi(words[2]),val);
        }
        if(!strcmp(words[0],"bitmap_set_all")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            bool val = false;
            if(!strcmp(words[2],"true")) val= true;
            else if(!strcmp(words[2],"false")) val = false;
            bitmap_set_all(tmpmap,val);
        }
        if(!strcmp(words[0],"bitmap_set_multiple")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            bool val = false;
            if(!strcmp(words[4],"true")) val= true;
            else if(!strcmp(words[4],"false")) val = false;
            bitmap_set_multiple(tmpmap,atoi(words[2]),atoi(words[3]),val);
        }
        
        if(!strcmp(words[0],"bitmap_size")){
            tmpmap = bitmap_array[words[1][2]-'0'];
            printf("%zu\n",bitmap_size(tmpmap));
        }
        if(!strcmp(words[0],"bitmap_test")){
            tmpmap = bitmap_array[words[1][2]-'0'];

            if(bitmap_test(tmpmap,atoi(words[2]))) printf("true\n");
            else printf("false\n");
        }
        if(!strcmp(words[0],"bitmap_expand")){
            tmpmap = bitmap_array[words[1][2]-'0'];

            tmpmap = bitmap_expand(tmpmap,atoi(words[2]));

            bitmap_array[words[1][2]-'0'] = tmpmap;
        }
        if(!strcmp(words[0],"hash_insert")){
            tmphash = hash_table[words[1][4]-'0'];
            hashelem = (struct hash_elem*)malloc(sizeof(struct hash_elem));
            hashelem->value = atoi(words[2]);
            hash_insert(tmphash,hashelem);
        }
        if(!strcmp(words[0],"hash_apply")){
            tmphash = hash_table[words[1][4]-'0'];
            if(!strcmp(words[2],"square")){
                hash_apply(tmphash,square);
            }
            if(!strcmp(words[2],"triple")){
                hash_apply(tmphash,triple);
            }
        }
        if(!strcmp(words[0],"hash_delete")){
            tmphash = hash_table[words[1][4]-'0'];
            hashelem = (struct hash_elem*)malloc(sizeof(struct hash_elem));
            hashelem->value = atoi(words[2]);
            hash_delete(tmphash,hashelem);
        }
        if(!strcmp(words[0],"hash_empty")){
            tmphash = hash_table[words[1][4]-'0'];
            if(hash_empty(tmphash)) printf("%s\n","true");
            else printf("%s\n","false");
        }
        if(!strcmp(words[0],"hash_size")){
            tmphash = hash_table[words[1][4]-'0'];
            printf("%zu\n",hash_size(tmphash));
        }
        if(!strcmp(words[0],"hash_find")){
            tmphash = hash_table[words[1][4]-'0'];
            hashelem = (struct hash_elem*)malloc(sizeof(struct hash_elem));
            hashelem->value=atoi(words[2]);
            hashelem=hash_find(tmphash,hashelem);
            if(hashelem != NULL) printf("%d\n",hashelem->value);
        }
        if(!strcmp(words[0],"hash_replace")){
            tmphash = hash_table[words[1][4]-'0'];
            hashelem = (struct hash_elem*)malloc(sizeof(struct hash_elem));
            hashelem->value=atoi(words[2]);
            hash_replace(tmphash,hashelem);
        }
        if(!strcmp(words[0],"hash_clear")){
            tmphash = hash_table[words[1][4]-'0'];
            hash_clear(tmphash,hash_destructor);
        }

        if(!strcmp(words[0],"list_push_front")){
            tmplist=list_array[words[1][4]-'0'];

            tmpelem=(struct list_elem*)malloc(sizeof(struct list_elem));
            tmpitem=list_entry(tmpelem,struct list_item,elem);
            tmpitem->data = atoi(words[2]);
            list_push_front(list_array[words[1][4]-'0'],tmpelem);
        }
        if(!strcmp(words[0],"list_push_back")){
            tmplist=list_array[words[1][4]-'0'];

            tmpelem=(struct list_elem*)malloc(sizeof(struct list_elem));
            tmpitem=list_entry(tmpelem,struct list_item,elem);
            tmpitem->data = atoi(words[2]);
            list_push_back(list_array[words[1][4]-'0'],tmpelem);
        }
        if(!strcmp(words[0],"list_front")){
            tmplist=list_array[words[1][4]-'0'];

            printf("%d\n",list_entry(list_front(tmplist),struct list_item,elem)->data);
        }
        if(!strcmp(words[0],"list_back")){
            tmplist=list_array[words[1][4]-'0'];

            printf("%d\n",list_entry(list_back(tmplist),struct list_item,elem)->data);
        }
        if(!strcmp(words[0],"list_pop_front")){
            tmplist=list_array[words[1][4]-'0'];
            list_pop_front(tmplist);
        }
        if(!strcmp(words[0],"list_pop_back")){
            tmplist=list_array[words[1][4]-'0'];
            list_pop_back(tmplist);
        }
        if(!strcmp(words[0],"list_insert")){
            tmplist=list_array[words[1][4]-'0'];
            tmpelem=(struct list_elem*)malloc(sizeof(struct list_elem));
            tmpitem=list_entry(tmpelem,struct list_item,elem);
            tmpitem->data = atoi(words[3]);
            struct list_elem *belem=list_head(tmplist);
            for(int i=0;i<=atoi(words[2]);i++){
                if(belem->next !=NULL){
                    belem=belem->next;
                }
            }
            list_insert(belem,tmpelem);
        }
        if(!strcmp(words[0],"list_insert_ordered")){
            tmplist=list_array[words[1][4]-'0'];
            tmpelem=(struct list_elem*)malloc(sizeof(struct list_elem));
            tmpitem=list_entry(tmpelem,struct list_item,elem);
            tmpitem->data =atoi(words[2]);

            list_insert_ordered(tmplist,tmpelem,data_less,NULL);
        }
        if(!strcmp(words[0],"list_empty")){
            tmplist=list_array[words[1][4]-'0'];
            if(list_empty(tmplist)) printf("true\n");
            else printf("false\n");
        }
        if(!strcmp(words[0],"list_size")){
            tmplist=list_array[words[1][4]-'0'];
            printf("%zu\n",list_size(tmplist));
        }
        if(!strcmp(words[0],"list_max")){
            tmplist=list_array[words[1][4]-'0'];
            printf("%d\n",list_entry(list_max(tmplist,data_less,NULL),struct list_item,elem)->data);
        }
        if(!strcmp(words[0],"list_min")){
            printf("%d\n",list_entry(list_min(tmplist,data_less,NULL),struct list_item,elem)->data);
        }
        
        if(!strcmp(words[0],"list_remove")){
            tmplist=list_array[words[1][4]-'0'];
            tmpelem=list_head(tmplist);
            for(int i=0;i<=atoi(words[2]);i++){
                tmpelem=tmpelem->next;
            }
            list_remove(tmpelem);
        }
        if(!strcmp(words[0],"list_reverse")){
            tmplist=list_array[words[1][4]-'0'];
            list_reverse(tmplist);
        }
        if(!strcmp(words[0],"list_shuffle")){
            tmplist=list_array[words[1][4]-'0'];
            list_shuffle(tmplist);
        }
        if(!strcmp(words[0],"list_sort")){
            tmplist=list_array[words[1][4]-'0'];
            list_sort(tmplist,data_less,NULL);
        }
        if(!strcmp(words[0],"list_splice")){
            tmplist=list_array[words[1][4]-'0'];
            tmpelem=list_head(tmplist);
            struct list *getlist=list_array[words[3][4]-'0'];
            struct list_elem *getelem1=list_head(getlist);
            struct list_elem *getelem2=list_head(getlist);
            for(int i=0;i<=atoi(words[2]);i++){
                tmpelem=tmpelem->next;
            }
            for(int i=0;i<=atoi(words[4]);i++){
                getelem1=getelem1->next;
            }
            for(int i=0;i<=atoi(words[5]);i++){
                getelem2=getelem2->next;
            }

            list_splice(tmpelem,getelem1,getelem2);
        }
        if(!strcmp(words[0],"list_swap")){
            tmplist=list_array[words[1][4]-'0'];
            struct list_elem *getelem1=list_head(tmplist);
            struct list_elem *getelem2=list_head(tmplist);
            for(int i=0;i<=atoi(words[2]);i++){
                getelem1=getelem1->next;
            }
            for(int i=0;i<=atoi(words[3]);i++){
                getelem2=getelem2->next;
            }
            list_swap(getelem1,getelem2);

        }
        if(!strcmp(words[0],"list_unique")){
            tmplist=list_array[words[1][4]-'0'];
            if(token_count==3){
                struct list *getlist = list_array[words[2][4]-'0'];
                list_unique(tmplist, getlist, data_less, NULL);
            }
            else if(token_count == 2){
                list_unique(tmplist, NULL , data_less, NULL);
            }
        }
    }
}
