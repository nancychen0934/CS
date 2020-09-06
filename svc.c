#include "svc.h"
#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include <ctype.h>
#include <assert.h>

struct stage_item{
    char* file_name;
    int change_state;//0:add, 1:rm, 2:modify
    int hash_value;
};
struct file_item{
    char* filename;
    int hash_value;
    //char* commit_id;
    char* content;
};
struct SVC{
    struct commit** commit_pool;//branches[head]///??????
    int n_commits_pool;
    //struct file_item* files;
    struct branch* head;
    struct branch* branches;
    int n_branches;
    //char** stage_info;
    struct stage_item* stage_info;
    int n_stage_item;
    //stage
};

struct modify_record{
    char* filename;
    int pre_hash;
    int new_hash;
};

struct commit{
    struct commit* parent;
    //struct commit* subordinate;
    char* commit_id;
    char* commit_message;
    struct file_item* tracked_files;
    int n_tracked_files;
    
    char** commit_changes;
    int n_changes;
    struct modify_record* modify_records;
    int n_modify;
    
    struct branch* curr_branch;
    
};
struct branch{
    //struct commit* commits;
    //int curr_index; // commits[curr_commit]
    char* branch_id;
    struct commit* parent_commit;
    struct commit* aim_commit;
    
};

//print functions for check
// void print_stage(struct SVC* helper){
// 	printf("The stage includes: \n");
// 	for(int i=0; i<helper->n_stage_item; i++){
// 		printf("%s,  %d\n", helper->stage_info[i].file_name, helper->stage_info[i].change_state);
// 	}
// }

// void print_tracked_files(struct commit* curr_commit){
// 	//struct commit* curr_commit = helper->head->commits[helper->head->curr_index];
// 	printf("The current tracked files include: \n");
// 	for(int i=0; i<curr_commit->n_tracked_files; i++){
// 		printf("%s\n", curr_commit->tracked_files[i].filename);
// 	}
// }

// void print_changes(struct commit* curr_commit){
// 	//struct commit* curr_commit = helper->head->commits[helper->head->curr_index];
// 	printf("The current changes include: \n");
// 	for(int i=0; i<curr_commit->n_changes; i++){
// 		printf("%s\n", curr_commit->commit_changes[i]);
// 	}
// }

void change_file(char* filename){
    FILE *out;
    out = fopen(filename,"a+");
    if(out == NULL){
            exit(EXIT_FAILURE);
    }
    fprintf(out,"heelo\n");
    
    fclose(out);
}
void *svc_init(void) {
    struct SVC* svc = (struct SVC*)malloc(sizeof(struct SVC));
    svc->stage_info = NULL;
    
    svc->n_stage_item = 0;
    svc->branches = (struct branch*)malloc(sizeof(struct branch));
    svc->n_branches = 1;
    svc->branches[0].branch_id = (char*)malloc(sizeof(char)*(strlen("master")+1));
    strcpy(svc->branches[0].branch_id, "master");
    svc->branches[0].branch_id[6] = '\0';
    //svc->branches[0].curr_index = 0;
    //svc->branches[0].commits = NULL;
    svc->branches[0].parent_commit = NULL;
    svc->branches[0].aim_commit = NULL;
    svc->head = &(svc->branches[0]);
    
    svc->commit_pool = NULL;
    svc->n_commits_pool = 0;
    
    return svc;
}
//Following are free functions

//free the stage area
void free_stage(struct SVC* svc_helper){
    if(svc_helper->stage_info == NULL){
        return;
    }
    for(int i=0; i<svc_helper->n_stage_item; i++){
        free(svc_helper->stage_info[i].file_name);
        svc_helper->stage_info[i].file_name = NULL;
        //free(svc_helper->stage_info[i])
    }
    free(svc_helper->stage_info);
    svc_helper->stage_info = NULL;
    svc_helper->n_stage_item = 0;
    return;
}


void free_commits(void* helper){
    struct SVC* svc_helper = (struct SVC*)helper;
    for(int i=0; i<svc_helper->n_commits_pool; i++){
        
        free(svc_helper->commit_pool[i]->commit_id);
        svc_helper->commit_pool[i]->commit_id = NULL;
        
        free(svc_helper->commit_pool[i]->commit_message);
        svc_helper->commit_pool[i]->commit_message = NULL;
        
        //free tracked_files
        for(int j=0; j<svc_helper->commit_pool[i]->n_tracked_files; j++){
            free(svc_helper->commit_pool[i]->tracked_files[j].filename);
            svc_helper->commit_pool[i]->tracked_files[j].filename = NULL;
            //free(svc_helper->commit_pool[i]->tracked_files[j].commit_id);
            //svc_helper->commit_pool[i]->tracked_files[j].commit_id = NULL;
            free(svc_helper->commit_pool[i]->tracked_files[j].content);
            svc_helper->commit_pool[i]->tracked_files[j].content = NULL;
        }
        free(svc_helper->commit_pool[i]->tracked_files);
        svc_helper->commit_pool[i]->tracked_files = NULL;
        
        //free modify records
        for(int j=0; j<svc_helper->commit_pool[i]->n_modify; j++){
            free(svc_helper->commit_pool[i]->modify_records[j].filename);
            svc_helper->commit_pool[i]->modify_records[j].filename = NULL;
        }
        free(svc_helper->commit_pool[i]->modify_records);
        svc_helper->commit_pool[i]->modify_records = NULL;
        
        for(int j=0; j<svc_helper->commit_pool[i]->n_changes; j++){
            free(svc_helper->commit_pool[i]->commit_changes[j]);
            svc_helper->commit_pool[i]->commit_changes[j] = NULL;
        }
        free(svc_helper->commit_pool[i]->commit_changes);
        svc_helper->commit_pool[i]->commit_changes = NULL;
        
        free(svc_helper->commit_pool[i]);
        svc_helper->commit_pool[i] = NULL;
    }
    free(svc_helper->commit_pool);
    svc_helper->commit_pool = NULL;
}

void free_branches(void* helper){
    
    struct SVC* svc_helper = (struct SVC*)helper;
    //printf("it now has %d branches\n", svc_helper->n_branches);
    for(int i=0; i<svc_helper->n_branches; i++){
        //printf("the branch id is %s\n",svc_helper->branches[i].branch_id);
        free(svc_helper->branches[i].branch_id);
        //printf("the branch id is %s\n",svc_helper->branches[i].branch_id);
        svc_helper->branches[i].branch_id = NULL;
        //printf("the branch id is %s\n",svc_helper->branches[i].branch_id);
    }
    free(svc_helper->branches);
    svc_helper->branches = NULL;
}
void cleanup(void *helper) {
    // TODO: Implement
    free_stage(helper);
    free_commits(helper);
    free_branches(helper);
    free(helper);
    //helper = NULL;
}

int hash_file(void *helper, char *file_path) {
    // TODO: Implement
    FILE *fp = fopen(file_path,"r");
    if(file_path == NULL){
        return -1;
    }else if(fp == NULL){
        return -2;
    }else{
		int ch;
        int count = 0;
        int file_length = strlen(file_path);
        for(int i=0; i<file_length; i++){
			//int c = (int)file_path[i];
            count = (count + (unsigned int)file_path[i])%1000;
        }
        //printf("count is: %d\n",count);
        //int check = 0;
        while( (ch=fgetc(fp)) != EOF ){
            //check = 1;
        	
            count = (count + ch)%2000000000;
        }
        // if(check == 1){
        //     count -= 10;
        // }
        
        //count -= 10;
          
        fclose(fp);
        
        return count; 
    }
    
    
}


// thi is used for get the alphabetic order
int check_both_alpha(char a, char b){
	int check = 0;
	if(((a <= 'z' && a >= 'a')||(a <= 'Z' && a >= 'A')) && ((b <= 'z' && b >= 'a')||(b <= 'Z' && b >= 'A'))){
		check = 1;
	}else{
		check = 0;
	}
	return check;
}

int cmp_alpha(char c1, char c2)
{

    if((isupper(c1) && isupper(c2)) || (islower(c1) && islower(c2))){
		if(c1 < c2){
			return 0;
		}else if(c1 > c2){
			return 1;
		}else{
			return 2;
		}
	}else if(isupper(c1) && islower(c2)){
		
		if(c1+32 < c2){
			return 0;
		}else if(c1+32 > c2){
			return 1;
		}else{
			
			return 0;
		}
	}else{
		if(c1-32 < c2){
			return 0;
		}else if(c1-32 > c2){
			return 1;
		}else{
			return 1;
		}
	}
	
}

void string_order(char** slist, int count){
	char* temp;
	int check_alpha = 0;
	int len = 0;
	int check_order = 0;
	//printf("OK here\n");
	for(int i=0;i<count-1;i++){
		for(int j=i+1;j<count;j++){
			
			if(strlen(slist[i]) <= strlen(slist[j])){
				len = strlen(slist[i]);
			}else{
				len = strlen(slist[j]);
			}
			
			// the last char is the change_state so not being compared
			for(int k=0; k<len-1; k++){
				check_alpha = check_both_alpha(slist[i][k],slist[j][k]);
				
				if(check_alpha == 0){
					//printf("OK here1\n");
					if(slist[i][k] > slist[j][k]){
						temp = slist[i];
						slist[i] = slist[j];
						slist[j] = temp;
						//printf("OK here111\n");
						break;
					}else if(slist[i][k] < slist[j][k]){
						break;
					}
				}else{
					//printf("OK here2\n");
					check_order = cmp_alpha(slist[i][k],slist[j][k]);
					//printf("the fking order is %d\n", check_order);
					if(check_order == 1){
						//printf("OK here22\n");
						temp = slist[i];
						slist[i] = slist[j];
						slist[j] = temp;
						//printf("OK here222\n");
						break;
					}else if(check_order == 0){
						break;
					}
				}
			}
			
		}
	}
}

//this is used to save the file content into the file_item
void store_content(void *helper, char* file_name, struct file_item* item){
    //this function store all attributes except for the commit id;
    item->filename = (char*)malloc(sizeof(char)*100);
    strcpy(item->filename,file_name);
    
    item->hash_value = hash_file(helper, file_name);
    
	item->content = (char*)malloc(sizeof(char)*1024);
	FILE* fp = fopen(file_name,"r");
	char c;
	int count = 0;
	while(EOF != (c = fgetc( fp ))&&count<1024){
		item->content[count] = c;
        count++;
	}
	fclose(fp);
	//printf("the content of %s is: %s\n", item->filename, item->content);
}

void store_commit_id(struct commit* commit){
    int id = 0;
    for(int i=0; i<strlen(commit->commit_message); i++){
        id = (id + commit->commit_message[i]) % 1000;
    }
    for(int i=0; i<commit->n_changes; i++){
        if(commit->commit_changes[i][strlen(commit->commit_changes[i])-1] == '0'){
            id = id + 376591;
        }else if(commit->commit_changes[i][strlen(commit->commit_changes[i])-1] == '1'){
            id = id + 85973;
        }else{
            id = id + 9573681;
        }
        
        for(int j=0; j<strlen(commit->commit_changes[i])-1; j++){
            id = (id * (commit->commit_changes[i][j] % 37)) % 15485863 + 1;
        }
    }
    printf("here the id number is %d\n",id);
    sprintf(commit->commit_id,"%06x",id);
    printf("here the id number is %s\n",commit->commit_id);
    
}


//check if a file is being tracked by last commit
int check_tracked(char* filename, struct commit* commit){
    if(commit->tracked_files == NULL){
        return 0;
    }else{
        for(int i=0; i<commit->n_tracked_files; i++){
            if(strcmp(commit->tracked_files[i].filename,filename)==0){
                return 1;
            }
        }
        return 0;
    }
}


//check if the file is removed or added
int check_rm_add(char* filename, struct commit* commit){
    char* temp; 
    for(int i=0; i< commit->n_changes; i++){
        temp = (char*)malloc(sizeof(char)*50);
        strcpy(temp,commit->commit_changes[i]);
        temp[strlen(commit->commit_changes[i])-1] = '\0';
        if(strcmp(filename,temp) == 0){  //if found, return its change state, else return 9
            free(temp);
            temp = NULL;
            return (commit->commit_changes[i])[strlen(commit->commit_changes[i])-1] - '0';
        }
        free(temp);
    }
    temp = NULL;
    return 9;
}

char *svc_commit(void *helper, char *message) {
    // TODO: Implement
	if(message == NULL){
		return NULL;
	}
	struct SVC* svc_helper = (struct SVC*)helper;
	struct branch* head = svc_helper->head;
	
	if(head->aim_commit == NULL){
		//head->commits = (struct commit*)malloc(sizeof(struct commit));
		if(svc_helper->stage_info == NULL){
			return NULL;
		}else{
            //check if all changes in stage area is remove
			for(int i=0; i<svc_helper->n_stage_item; i++){
                //if stage info tells that there is at least one item: change state == "add" && file still exists
                //SVC will create a new commit
				if(svc_helper->stage_info[i].change_state == 0 && (hash_file(helper, svc_helper->stage_info[i].file_name)!=-2)){
					//printf("Come Here!\n");
                    // head->commits = (struct commit*)malloc(sizeof(struct commit));
                    // head->curr_index = 0;
                    // head->commits[0].n_tracked_files = 0;
                    // head->commits[0].tracked_files = NULL;
                    // head->commits[0].commit_changes = NULL;
                    // head->commits[0].modify_records = NULL;
                    // head->commits[0].n_modify = 0;
                    // head->commits[0].n_changes = 0;
                    
                    head->aim_commit = (struct commit*)malloc(sizeof(struct commit));
                    head->aim_commit->n_tracked_files = 0;
                    head->aim_commit->tracked_files = NULL;
                    head->aim_commit->commit_changes = NULL;
                    head->aim_commit->modify_records = NULL;
                    head->aim_commit->n_modify = 0;
                    head->aim_commit->n_changes = 0;
                    
                    // link the current commit to its parent commit (head->parent_commit for a new created branch)
                    if(head->parent_commit == NULL){
                        head->aim_commit->parent = NULL;
                    }else{
                        head->aim_commit->parent = head->parent_commit;
                    }
                    
                    //head->commits[0].parent = NULL;
                    head->aim_commit->curr_branch = head;
                    head->aim_commit->commit_id = (char*)malloc(sizeof(char)*7);
                    head->aim_commit->commit_id[6] = '\0';
                    break;
                }
			}
            
            // check if 
            if(head->aim_commit == NULL){
				//printf("Not here!");
                return NULL;
            }else{
				//printf("Oh here!\n");
                struct commit* curr_commit = head->aim_commit;
                for(int i=0; i<svc_helper->n_stage_item; i++){
                    if(svc_helper->stage_info[i].change_state == 0 && (hash_file(helper, svc_helper->stage_info[i].file_name)!=-2)){
                        curr_commit->tracked_files = (struct file_item*)realloc(curr_commit->tracked_files,sizeof(struct file_item)*(curr_commit->n_tracked_files+1));
                        curr_commit->n_tracked_files = curr_commit->n_tracked_files + 1;
                        store_content(helper, svc_helper->stage_info[i].file_name, &(curr_commit->tracked_files[curr_commit->n_tracked_files-1]));
                        //printf("The filename is : %s\n", curr_commit->tracked_files[curr_commit->n_tracked_files-1].filename);
                            
                        curr_commit->n_changes = curr_commit->n_changes + 1;
                        curr_commit->commit_changes = (char**)realloc(curr_commit->commit_changes, sizeof(char*)*curr_commit->n_changes);
                        curr_commit->commit_changes[curr_commit->n_changes-1] = (char*)malloc(sizeof(char)*(strlen(svc_helper->stage_info[i].file_name)+2));
                        strcpy(curr_commit->commit_changes[curr_commit->n_changes-1],svc_helper->stage_info[i].file_name);
                        curr_commit->commit_changes[curr_commit->n_changes-1][strlen(svc_helper->stage_info[i].file_name)] = svc_helper->stage_info[i].change_state + '0';
                        curr_commit->commit_changes[curr_commit->n_changes-1][strlen(svc_helper->stage_info[i].file_name)+1] = svc_helper->stage_info[i].change_state + '\0';
                    }
                }
				
				//print_tracked_files(curr_commit);
                curr_commit->commit_message = (char*)malloc(sizeof(char)*(strlen(message)+1));
                strcpy(curr_commit->commit_message, message);
                curr_commit->commit_message[strlen(message)] = '\0';
                string_order(curr_commit->commit_changes,curr_commit->n_changes);
                store_commit_id(curr_commit);
                //printf("Oh here!\n");
                free_stage(svc_helper);
				//printf("%s\n",curr_commit->commit_id);
                
                svc_helper->n_commits_pool = svc_helper->n_commits_pool + 1;
                svc_helper->commit_pool = (struct commit**)realloc(svc_helper->commit_pool, sizeof(struct commit*)*svc_helper->n_commits_pool);
                svc_helper->commit_pool[svc_helper->n_commits_pool-1] = curr_commit;
                
                return curr_commit->commit_id;
            }
		}
		
	}else{
        // head->curr_index++;
        // head->commits = (struct commit*)realloc(head->commits,sizeof(struct commit)*(head->curr_index+1));
        // head->commits[head->curr_index].n_tracked_files = 0;
        // head->commits[head->curr_index].tracked_files = NULL;
        // head->commits[head->curr_index].commit_changes = NULL;
        // head->commits[head->curr_index].n_changes = 0;
        // head->commits[head->curr_index].modify_records = NULL;
        // head->commits[head->curr_index].n_modify = 0;
        // head->commits[head->curr_index].parent = &(head->commits[head->curr_index-1]);
        // head->commits[head->curr_index].curr_branch = head;
        // head->commits[head->curr_index].commit_id = (char*)malloc(sizeof(char)*6);
        
        struct commit* last_commit = head->aim_commit;
        head->aim_commit = (struct commit*)malloc(sizeof(struct commit));
        head->aim_commit->n_tracked_files = 0;
        head->aim_commit->tracked_files = NULL;
        head->aim_commit->commit_changes = NULL;
        head->aim_commit->n_changes = 0;
        head->aim_commit->modify_records = NULL;
        head->aim_commit->n_modify = 0;
        head->aim_commit->parent = last_commit;
        head->aim_commit->curr_branch = head;
        head->aim_commit->commit_id = (char*)malloc(sizeof(char)*7);
        head->aim_commit->commit_id[6] = '\0';
        struct commit* curr_commit = head->aim_commit;
        
        for(int i=0; i<svc_helper->n_stage_item; i++){
            //the add change is commited only when "add && untracked && undeleted"
            //the rm change is commited only when "tracked && (removed && undeleted)"
            
            if(svc_helper->stage_info[i].change_state == 0 && (hash_file(helper, svc_helper->stage_info[i].file_name)!=-2) && (check_tracked(svc_helper->stage_info[i].file_name, last_commit) == 0)){
                curr_commit->n_changes = curr_commit->n_changes + 1;
                curr_commit->commit_changes = (char**)realloc(curr_commit->commit_changes, sizeof(char*)*curr_commit->n_changes);
                curr_commit->commit_changes[curr_commit->n_changes-1] = (char*)malloc(sizeof(char)*(strlen(svc_helper->stage_info[i].file_name)+2));
                strcpy(curr_commit->commit_changes[curr_commit->n_changes-1],svc_helper->stage_info[i].file_name);
                curr_commit->commit_changes[curr_commit->n_changes-1][strlen(svc_helper->stage_info[i].file_name)] = svc_helper->stage_info[i].change_state + '0';
                curr_commit->commit_changes[curr_commit->n_changes-1][strlen(svc_helper->stage_info[i].file_name)+1] = '\0';
            }else if(check_tracked(svc_helper->stage_info[i].file_name, last_commit) == 1 && (svc_helper->stage_info[i].change_state == 1) && (hash_file(helper, svc_helper->stage_info[i].file_name)!=-2)){
                curr_commit->n_changes = curr_commit->n_changes + 1;
                curr_commit->commit_changes = (char**)realloc(curr_commit->commit_changes, sizeof(char*)*curr_commit->n_changes);
                curr_commit->commit_changes[curr_commit->n_changes-1] = (char*)malloc(sizeof(char)*(strlen(svc_helper->stage_info[i].file_name)+2));
                strcpy(curr_commit->commit_changes[curr_commit->n_changes-1],svc_helper->stage_info[i].file_name);
                curr_commit->commit_changes[curr_commit->n_changes-1][strlen(svc_helper->stage_info[i].file_name)] = svc_helper->stage_info[i].change_state + '0';
                curr_commit->commit_changes[curr_commit->n_changes-1][strlen(svc_helper->stage_info[i].file_name)+1] = '\0';
            }
        
        }
        
        //the delete change is commited when "tracked && deleted"
        
        for(int i=0; i<last_commit->n_tracked_files; i++){
            if(hash_file(helper,last_commit->tracked_files[i].filename) == -2){
                curr_commit->n_changes = curr_commit->n_changes + 1;
                curr_commit->commit_changes = (char**)realloc(curr_commit->commit_changes, sizeof(char*)*curr_commit->n_changes);
                curr_commit->commit_changes[curr_commit->n_changes-1] = (char*)malloc(sizeof(char)*(strlen(last_commit->tracked_files[i].filename)+2));
                strcpy(curr_commit->commit_changes[curr_commit->n_changes-1],last_commit->tracked_files[i].filename);
                curr_commit->commit_changes[curr_commit->n_changes-1][strlen(last_commit->tracked_files[i].filename)] = '1';
                curr_commit->commit_changes[curr_commit->n_changes-1][strlen(last_commit->tracked_files[i].filename)+1] = '\0';
            }
        }
        //the modification is commited when "tracked && not(added, rmed, deleted)&& hash value changed"
        int check_changed; 
        for(int i=0; i<last_commit->n_tracked_files; i++){
            check_changed = check_rm_add(last_commit->tracked_files[i].filename,curr_commit);
            if(check_changed == 9 && (hash_file(helper, last_commit->tracked_files[i].filename) != last_commit->tracked_files[i].hash_value)){
                curr_commit->n_changes = curr_commit->n_changes + 1;
                curr_commit->commit_changes = (char**)realloc(curr_commit->commit_changes, sizeof(char*)*curr_commit->n_changes);
                curr_commit->commit_changes[curr_commit->n_changes-1] = (char*)malloc(sizeof(char)*(strlen(last_commit->tracked_files[i].filename)+2));
                //strcpy(curr_commit->commit_changes[curr_commit->n_changes-1],last_commit->tracked_files[i].filename);
                
                for(int q=0; q<strlen(last_commit->tracked_files[i].filename); q++){
                    curr_commit->commit_changes[curr_commit->n_changes-1][q] = last_commit->tracked_files[i].filename[q];
                }
                curr_commit->commit_changes[curr_commit->n_changes-1][strlen(last_commit->tracked_files[i].filename)] = '2';
                curr_commit->commit_changes[curr_commit->n_changes-1][strlen(last_commit->tracked_files[i].filename)+1] = '\0';
                // printf("++++++++++++++++++++\n");
                // printf("%s\n", last_commit->tracked_files[i].filename);
                // printf("%s\n", curr_commit->commit_changes[curr_commit->n_changes-1]);
                // printf("++++++++++++++++++++\n");
                //record the modification messages for print_commit()
                curr_commit->n_modify = curr_commit->n_modify + 1;
                curr_commit->modify_records = (struct modify_record*)realloc(curr_commit->modify_records, sizeof(struct modify_record)*curr_commit->n_modify);
                curr_commit->modify_records[curr_commit->n_modify-1].filename = (char*)malloc(sizeof(char)*(strlen(last_commit->tracked_files[i].filename)+1));
                strcpy(curr_commit->modify_records[curr_commit->n_modify-1].filename, last_commit->tracked_files[i].filename);
                curr_commit->modify_records[curr_commit->n_modify-1].filename[strlen(last_commit->tracked_files[i].filename)] = '\0';
                curr_commit->modify_records[curr_commit->n_modify-1].pre_hash = last_commit->tracked_files[i].hash_value;
                curr_commit->modify_records[curr_commit->n_modify-1].new_hash = hash_file(helper, last_commit->tracked_files[i].filename);
            }
        }
        
        //if there is no changes
        if(curr_commit->commit_changes == NULL){
            free(curr_commit->commit_id);
            free(curr_commit);
            curr_commit = NULL;
            head->aim_commit = last_commit;
            return NULL;
        }
        
        //update tracked files in this commit;
        //added && untracked
        for(int i=0; i<svc_helper->n_stage_item; i++){
            if(check_rm_add(svc_helper->stage_info[i].file_name,curr_commit) == 0){
                curr_commit->tracked_files = (struct file_item*)realloc(curr_commit->tracked_files,sizeof(struct file_item)*(curr_commit->n_tracked_files+1));
                curr_commit->n_tracked_files = curr_commit->n_tracked_files + 1;
                store_content(helper, svc_helper->stage_info[i].file_name, &(curr_commit->tracked_files[curr_commit->n_tracked_files-1]));
            }
        }
        //tracked && unremoved
        for(int i=0; i<last_commit->n_tracked_files; i++){
            if(check_rm_add(last_commit->tracked_files[i].filename,curr_commit) != 0 && check_rm_add(last_commit->tracked_files[i].filename,curr_commit) != 1){
                curr_commit->tracked_files = (struct file_item*)realloc(curr_commit->tracked_files,sizeof(struct file_item)*(curr_commit->n_tracked_files+1));
                curr_commit->n_tracked_files = curr_commit->n_tracked_files + 1;
                store_content(helper, last_commit->tracked_files[i].filename, &(curr_commit->tracked_files[curr_commit->n_tracked_files-1]));
            }
        }
        
        curr_commit->commit_message = (char*)malloc(sizeof(char)*(strlen(message)+1));
        strcpy(curr_commit->commit_message, message);
        curr_commit->commit_message[strlen(message)] = '\0';
        string_order(curr_commit->commit_changes,curr_commit->n_changes);
        
        store_commit_id(curr_commit);
        svc_helper->n_commits_pool = svc_helper->n_commits_pool + 1;
        svc_helper->commit_pool = (struct commit**)realloc(svc_helper->commit_pool, sizeof(struct commit*)*svc_helper->n_commits_pool);
        svc_helper->commit_pool[svc_helper->n_commits_pool-1] = curr_commit;
        
        free_stage(svc_helper);
        
        return curr_commit->commit_id;
        
    }
    
}

void *get_commit(void *helper, char *commit_id) {
    // TODO: Implement
    
    if(commit_id == NULL){
        return NULL;
    }
    struct SVC* svc_helper = (struct SVC*)helper;
    for(int i=0; i<svc_helper->n_commits_pool; i++){
        if(strcmp(svc_helper->commit_pool[i]->commit_id,commit_id) == 0){
            //printf("Found this commit %s\n",commit_id);
            return svc_helper->commit_pool[i];
        }
    }
    //printf("Not found this commit %s\n",commit_id);
    return NULL;
}

char **get_prev_commits(void *helper, void *commit, int *n_prev) {
    // TODO: Implement
    int count = 0;
    if(n_prev == NULL){
        return NULL;
    }else if(commit == NULL){
        *n_prev = 0;
        return NULL;
    }
    struct commit* curr_commit = (struct commit*) commit;
    if(curr_commit->parent == NULL){
        *n_prev = 0;
        return NULL;
    }else{
        char** id_list = NULL;
        curr_commit = curr_commit->parent;
        while(curr_commit != NULL){
            count++;
            id_list = (char**)realloc(id_list, sizeof(char*)*(count+1));
            //id_list[count-1] = (char*)malloc(sizeof(char)*7);
            //strcpy(id_list[count-1], curr_commit->commit_id);
			id_list[count-1] = curr_commit->commit_id;
            //id_list[count-1][6] = '\0';
            curr_commit = curr_commit->parent;
        }
        *n_prev = count;
        return id_list;
        
    }
    return NULL;
}

void print_commit(void *helper, char *commit_id) {
    // TODO: Implement
	struct SVC* svc = (struct SVC*)helper;
    struct commit* curr_commit = (struct commit*)get_commit(helper,commit_id);
    if(curr_commit == NULL){
        printf("Invalid commit id\n");
    }else{
		curr_commit->curr_branch = svc->head;
        printf("%s [%s]: %s\n", curr_commit->commit_id, curr_commit->curr_branch->branch_id, curr_commit->commit_message);
        char symbol;
        char* temp;
        for(int i=0; i<curr_commit->n_changes; i++){
            //printf("this change is: %s\n", curr_commit->commit_changes[i]);
            temp = (char*)malloc(sizeof(char)*(strlen(curr_commit->commit_changes[i])+1));
			//temp = curr_commit->commit_changes[i];
            strcpy(temp, curr_commit->commit_changes[i]);
            temp[strlen(curr_commit->commit_changes[i])-1] = '\0';
            if(curr_commit->commit_changes[i][strlen(curr_commit->commit_changes[i])-1] == '0'){
                symbol = '+';
                printf("    %c %s\n", symbol,temp);
            }else if(curr_commit->commit_changes[i][strlen(curr_commit->commit_changes[i])-1] == '1'){
                symbol = '-';
                printf("    %c %s\n", symbol,temp);
            }else if(curr_commit->commit_changes[i][strlen(curr_commit->commit_changes[i])-1] == '2'){
                symbol = '/';
                for(int j=0; j<curr_commit->n_modify;j++){
                    if(strcmp(temp,curr_commit->modify_records[i].filename) == 0){
                        printf("    %c %s [%d --> %d]\n", symbol,temp,curr_commit->modify_records[i].pre_hash, curr_commit->modify_records[i].new_hash);
                    }
                }
            }
            free(temp);
            temp = NULL;
        }
        printf("\n");
		printf("    Tracked files (%d):\n",curr_commit->n_tracked_files);
        for(int i=0; i<curr_commit->n_tracked_files; i++){
			
            printf("    [%10d] %s\n", curr_commit->tracked_files[i].hash_value,curr_commit->tracked_files[i].filename);
        }
		//printf("master\n");
    }
}

int check_in_branch(void *helper, char *branch_name){
    struct SVC* svc_helper = (struct SVC*)helper;
    for(int i=0; i<svc_helper->n_branches; i++){
        if(strcmp(svc_helper->branches[i].branch_id, branch_name) == 0){
            return 1;
        }
    }
    return 0;
}

int check_valid(char* name){  // 0:valid, 1: invalid
    for(int i=0; i<strlen(name); i++){
        if((name[i] >= 'A' && name[i]<= 'Z') || (name[i] >= 'a' && name[i]<= 'z') || (name[i] >= '0' && name[i]<= '9') || name[i] == '_' || name[i] == '/' || name[i] == '-'){
            continue;
        }else{
            return 1;
        }
    }
    return 0;
}

int check_uncommited(void* helper){ //0:no uncommited changes  1:have commited changes
    struct SVC* svc_helper = (struct SVC*)helper;
	struct branch* head = svc_helper->head;
    //struct commit* curr_commit = &(head->commits[head->curr_index]);
    
    
    if(head->aim_commit == NULL){
		//head->commits = (struct commit*)malloc(sizeof(struct commit));
		if(svc_helper->stage_info == NULL){
			return 0;
		}else{
            //check if all changes in stage area is remove
			for(int i=0; i<svc_helper->n_stage_item; i++){
                //if stage info tells that there is at least one item: change state == "add" && file still exists
                //SVC will create a new commit
				if(svc_helper->stage_info[i].change_state == 0 && (hash_file(helper, svc_helper->stage_info[i].file_name)!=-2)){
					//printf("Come Here!\n");
                    return 1;
                }
			}
            return 0;
        }
    }else{
        struct commit* last_commit = head->aim_commit;
        for(int i=0; i<svc_helper->n_stage_item; i++){
            //the add change is commited only when "add && untracked && undeleted"
            //the rm change is commited only when "tracked && (removed && undeleted)"

            if(svc_helper->stage_info[i].change_state == 0 && (hash_file(helper, svc_helper->stage_info[i].file_name)!=-2) && (check_tracked(svc_helper->stage_info[i].file_name, last_commit) == 0)){
                return 1;
            }else if(check_tracked(svc_helper->stage_info[i].file_name, last_commit) == 1 && (svc_helper->stage_info[i].change_state == 1) && (hash_file(helper, svc_helper->stage_info[i].file_name)!=-2)){
                return 1;
            }

        }
        
        //the delete change is commited when "tracked && deleted"
        for(int i=0; i<last_commit->n_tracked_files; i++){
            if(hash_file(helper,last_commit->tracked_files[i].filename) == -2){
                return 1;
            }
        }
        
        //the modification is commited when "tracked && not(added, rmed, deleted)&& hash value changed"
        //int check_changed; 
        for(int i=0; i<last_commit->n_tracked_files; i++){
            if((hash_file(helper, last_commit->tracked_files[i].filename) != last_commit->tracked_files[i].hash_value)){
                return 1;
            }
        }
        
        return 0;
    }
}
int svc_branch(void *helper, char *branch_name) {
    // TODO: Implement
    struct SVC* svc_helper = (struct SVC*)helper;
    struct branch* head = svc_helper->head;
	
    //struct commit* curr_commit = head->aim_commit;
    if(branch_name == NULL){
        return -1;
    }else if(check_valid(branch_name) == 1){
        return -1;
    }else if(check_in_branch(helper, branch_name) == 1){
        return -2;
    }else if(check_uncommited(helper) == 1){
        return -3;
    }
    
    int curr_index = 0;
    for(int i=0; i<svc_helper->n_branches; i++){
        if(strcmp(head->branch_id, svc_helper->branches[i].branch_id) == 0){
            curr_index = i;
        }
    }
    svc_helper->n_branches = svc_helper->n_branches+1;
    svc_helper->branches = (struct branch*)realloc(svc_helper->branches, sizeof(struct branch)*svc_helper->n_branches);
    svc_helper->head = &(svc_helper->branches[curr_index]);
    svc_helper->branches[svc_helper->n_branches-1].branch_id = (char*)malloc(sizeof(char)*(strlen(branch_name)+1));
    strcpy(svc_helper->branches[svc_helper->n_branches-1].branch_id, branch_name);
	//svc_helper->branches[svc_helper->n_branches-1].branch_id = (char*)realloc(svc_helper->branches[svc_helper->n_branches-1].branch_id,sizeof(char)*(strlen(branch_name)+1));
    svc_helper->branches[svc_helper->n_branches-1].branch_id[strlen(branch_name)] = '\0';
    svc_helper->branches[svc_helper->n_branches-1].aim_commit = NULL;
	//printf("%p\n",head->aim_commit);
    //svc_helper->branches[svc_helper->n_branches-1].parent_commit = svc_helper->branches[svc_helper->n_branches-2].aim_commit;
	svc_helper->branches[svc_helper->n_branches-1].parent_commit = svc_helper->head->aim_commit;
    
    return 0;
}

int svc_checkout(void *helper, char *branch_name) {
    // TODO: Implement
    if(branch_name == NULL){
        return -1;
    }else if(check_uncommited(helper) == 1){
        return -2;
    }
    struct SVC* svc_helper = (struct SVC*)helper;
    for(int i=0; i<svc_helper->n_branches; i++){
        if(strcmp(svc_helper->branches[i].branch_id, branch_name) == 0){
            svc_helper->head = &(svc_helper->branches[i]);
            return 0;
        }
    }
    return -1;
}

char **list_branches(void *helper, int *n_branches) {
    // TODO: Implement
    if(n_branches == NULL){
        return NULL;
    }
    
    struct SVC* svc_helper = (struct SVC*)helper;
    *n_branches = svc_helper->n_branches;
    char** b_list = (char**)malloc(sizeof(char*)*svc_helper->n_branches);
    for(int i=0; i<svc_helper->n_branches; i++){
		b_list[i] = svc_helper->branches[i].branch_id;
        // b_list[i] = (char*)malloc(sizeof(char)*(strlen(svc_helper->branches[i].branch_id)+1));
        // strcpy(b_list[i], svc_helper->branches[i].branch_id);
        // b_list[i][strlen(svc_helper->branches[i].branch_id)] = '\0';
    }
	printf("master\n");
    return b_list;
}

//check if the file is being tracked by the last commit, if yes, return the hash value(for first remove's return value),
//if no, return 0;
int check_in_track(char* filename, struct file_item* tracked_files, int n_tracked_files){
    for(int i=0; i<n_tracked_files; i++){
        if(strcmp(filename, tracked_files[i].filename) == 0){
            //return 1;
            return tracked_files[i].hash_value;
        }
    }
    return 0;
}

//add the file into stage area.
int add_to_stage(char* filename, struct SVC* svc_helper, int hash_value){
    //first check if the file is already in stage area
    //struct stage_item* stage_info = svc_helper->stage_info;
    for(int i=0; i<svc_helper->n_stage_item;i++){
        if(strcmp(filename,svc_helper->stage_info[i].file_name) == 0){
            if(svc_helper->stage_info[i].change_state == 0){
                return 1;
            }else if(svc_helper->stage_info[i].change_state == 1){
                svc_helper->stage_info[i].change_state = 0;
                svc_helper->stage_info[i].hash_value = hash_value;
                return 0;
            }
        }
    }
	//printf("first go here!\n");
    //if no, add a new information to stage area
    svc_helper->n_stage_item = svc_helper->n_stage_item + 1;
    svc_helper->stage_info = (struct stage_item*)realloc(svc_helper->stage_info,sizeof(struct stage_item)*(svc_helper->n_stage_item));
    svc_helper->stage_info[svc_helper->n_stage_item-1].file_name = (char*)malloc(sizeof(char)*(strlen(filename)+1));
    strcpy(svc_helper->stage_info[svc_helper->n_stage_item-1].file_name,filename);
    svc_helper->stage_info[svc_helper->n_stage_item-1].file_name[strlen(filename)] = '\0';
    svc_helper->stage_info[svc_helper->n_stage_item-1].change_state = 0;
    svc_helper->stage_info[svc_helper->n_stage_item-1].hash_value = hash_value;
    return 0;
}
int svc_add(void *helper, char *file_name) {
    int hash_value = hash_file(helper,file_name);
    // check if valid
    if(hash_value==-1){
        return -1;
    }
    else if(hash_value==-2){
        return -3;
    }
    
    struct SVC* svc_helper = (struct SVC*)helper;
    struct branch* head = svc_helper->head;
    
    if(head->aim_commit == NULL){  //if there is no existed commit, it must not be tracked, add the file directly into stages
        int check_add = add_to_stage(file_name,svc_helper, hash_value);
        if(check_add == 0){
            return hash_value;
        }else{
            return -2;
        }
    }else{  //if there is existed commits
        
        struct commit* curr_commit = head->aim_commit;
        
        int check_tracked = check_in_track(file_name, curr_commit->tracked_files, curr_commit->n_tracked_files);
        
        if(check_tracked == 0){  //if not in last commits' tracked_files, add the file directly into stages
            int check_add = add_to_stage(file_name,svc_helper, hash_value);
            if(check_add == 0){
                return hash_value;
            }else{
                return -2;
            }
        }else{  //if already in last commits' tracked_files 
            for(int i=0; i<svc_helper->n_stage_item; i++){  //and is recorded as "removed" in stage area, and add it again in stage
                if((strcmp(file_name,(svc_helper->stage_info)[i].file_name) == 0) && (svc_helper->stage_info)[i].change_state == 1){
                    (svc_helper->stage_info)[i].change_state = 0;
                    return hash_value;
                }
            }
            return -2;//already in last commits' tracked_files and is not removed
        }
        
    }
    
}
//if the file can be found in the track of last commit track_files, execute below function to find related hashvalue
int rm_from_stage_in_track(char* filename, struct SVC* svc_helper){
    //int hash
    //struct stage_item* stage_info = svc_helper->stage_info;
    for(int i=0; i<svc_helper->n_stage_item;i++){
        if(strcmp(filename,svc_helper->stage_info[i].file_name) == 0){
            if(svc_helper->stage_info[i].change_state == 0){
                svc_helper->stage_info[i].change_state = 1;
                int last_hash = svc_helper->stage_info[i].hash_value;
                svc_helper->stage_info[i].hash_value = -999;
                return last_hash;
            }else if(svc_helper->stage_info[i].change_state == 1){
                
                return 1;
            }
        }
    }
    svc_helper->n_stage_item++;
    svc_helper->stage_info = (struct stage_item*)realloc(svc_helper->stage_info,sizeof(struct stage_item)*(svc_helper->n_stage_item));
    svc_helper->stage_info[svc_helper->n_stage_item-1].file_name = (char*)malloc(sizeof(char)*(strlen(filename)+1));
    strcpy(svc_helper->stage_info[svc_helper->n_stage_item-1].file_name,filename);
    svc_helper->stage_info[svc_helper->n_stage_item-1].file_name[strlen(filename)] = '\0';
    svc_helper->stage_info[svc_helper->n_stage_item-1].change_state = 1;
    svc_helper->stage_info[svc_helper->n_stage_item-1].hash_value = -999;
    return 2;
}
//if the file is not in commit tracked_files, execute below function to find the last known hashvalue
int rm_from_stage_not_in_track(char* filename, struct SVC* svc_helper){
    //int hash
    //struct stage_item* stage_info = svc_helper->stage_info;
    for(int i=0; i<svc_helper->n_stage_item;i++){
        if(strcmp(filename,svc_helper->stage_info[i].file_name) == 0){
            if(svc_helper->stage_info[i].change_state == 0){
                svc_helper->stage_info[i].change_state = 1;
                int last_hash = svc_helper->stage_info[i].hash_value;
                svc_helper->stage_info[i].hash_value = -999;
                return last_hash;
            }else if(svc_helper->stage_info[i].change_state == 1){
                
                return 1;
            }
        }
    }
    
    return 1;
}
int svc_rm(void *helper, char *file_name) {
    // check if valid
    if(file_name == NULL){
        return -1;
    }
    struct SVC* svc_helper = (struct SVC*)helper;
    struct branch* head = svc_helper->head;
    if(head->aim_commit == NULL){  //if there is no existed commit, it must not be tracked
        int check_rm = rm_from_stage_not_in_track(file_name, svc_helper);
        if(check_rm == 1){
            return -2;
        }else{
            return check_rm;
        }
    }else{  //if there are existed commits
		//printf("Come here!\n");
        struct commit* curr_commit = head->aim_commit;
        //printf("%s\n",curr_commit->commit_id);
		//print_tracked_files(curr_commit);
        int check_tracked = check_in_track(file_name, curr_commit->tracked_files, curr_commit->n_tracked_files);//check if is tracked in last commit
        //printf("Come here!\n");
		if(check_tracked == 0){  //not in last commit's track_file
            int check_rm = rm_from_stage_not_in_track(file_name, svc_helper);
            if(check_rm == 1){  //already removed or not in stage, do nothing
                return -2;
            }else{  //return the last known hash value(adding)
                return check_rm;
            }
        }else{  //in last commit's track file
            //printf("Come here!\n");
            int check_rm = rm_from_stage_in_track(file_name,svc_helper);
            if(check_rm == 1){  //already removed
                return -2;
            }else if(check_rm == 2){  //not in stage but is tracked, return the last known hash value(committing) 
                return check_tracked;
            }else{  //return the last known hash value(adding)
                return check_rm;
            }
            
        }
    }
    
}

int svc_reset(void *helper, char *commit_id) {
    // TODO: Implement
    if(commit_id == NULL){
        return -1;
    }
    
    struct SVC* svc_helper = (struct SVC*)helper;
    struct branch* head = svc_helper->head;
    struct commit* curr_commit = head->aim_commit;
    
    while(curr_commit != NULL){
        if(strcmp(curr_commit->commit_id, commit_id) == 0){
            head->aim_commit = curr_commit;
            free_stage(svc_helper);
            return 0;
        }else{
            curr_commit = curr_commit->parent;
            
        }
    }
    return -2;
}

void update_file(char* file1, char* file2){
    FILE* fp1;
    FILE* fp2;
    char c;
    fp1=fopen(file1,"r"); 
    fp2=fopen(file2,"w"); 
    while ((c=fgetc(fp1))!=EOF){
        fputc(c,fp2);
    }
    fclose(fp1); 
    fclose(fp2);
}
char *svc_merge(void *helper, char *branch_name, struct resolution *resolutions, int n_resolutions){
    // TODO: Implement
        struct SVC* svc_helper = (struct SVC*)helper;
        if(branch_name == NULL){
            printf("Invalid branch name\n");
            return NULL;
        }else if(strcmp(branch_name, svc_helper->head->branch_id) == 0){
            printf("Cannot merge a branch with itself\n");
            return NULL;
        }else if(check_in_branch(helper, branch_name) == 0){
            printf("Branch not found\n");
            return NULL;
        }else if(check_uncommited(helper) == 1){
            printf("Changes must be committed\n");
            return NULL;
        }else{
            int check = -1;
            for(int i=0; i<svc_helper->n_branches; i++){
                if(strcmp(branch_name,svc_helper->branches[i].branch_id) == 0){
                    //struct branch* other_branch = &(svc_helper->branches[i]);
                    check = i;
                    //check = 1;
                    break;
                }
            }
            if(check >= 0){
                struct branch* other_branch = &(svc_helper->branches[check]);
                struct commit* last_commit1 = svc_helper->head->aim_commit;
                struct commit* last_commit2 = other_branch->aim_commit;
                struct commit* curr_commit = (struct commit*)malloc(sizeof(struct commit));

                curr_commit->n_tracked_files = 0;
                curr_commit->tracked_files = NULL;
                curr_commit->commit_changes = NULL;
                curr_commit->n_changes = 0;
                curr_commit->modify_records = NULL;
                curr_commit->n_modify = 0;
                curr_commit->parent = NULL;
                curr_commit->curr_branch = svc_helper->head;
                curr_commit->commit_id = (char*)malloc(sizeof(char)*7);
                curr_commit->commit_id[6] = '\0';

                //first if tracked by active branch and untracked by merging branch 
                for(int i=0; i<last_commit1->n_tracked_files; i++){
                    if(check_tracked(last_commit1->tracked_files[i].filename, last_commit2) == 0){
                        curr_commit->tracked_files = (struct file_item*)realloc(curr_commit->tracked_files,sizeof(struct file_item)*(curr_commit->n_tracked_files+1));
                        curr_commit->n_tracked_files = curr_commit->n_tracked_files + 1;
                        store_content(helper, last_commit1->tracked_files[i].filename, &(curr_commit->tracked_files[curr_commit->n_tracked_files-1]));
                    }
                }

                //second if tracked by merging branch and untracked by active branch
                for(int i=0; i<last_commit2->n_tracked_files; i++){
                    if(check_tracked(last_commit2->tracked_files[i].filename, last_commit1) == 0){

                        curr_commit->n_changes = curr_commit->n_changes + 1;
                        curr_commit->commit_changes = (char**)realloc(curr_commit->commit_changes, sizeof(char*)*curr_commit->n_changes);
                        curr_commit->commit_changes[curr_commit->n_changes-1] = (char*)malloc(sizeof(char)*(strlen(last_commit2->tracked_files[i].filename)+2));
                        strcpy(curr_commit->commit_changes[curr_commit->n_changes-1],last_commit2->tracked_files[i].filename);
                        curr_commit->commit_changes[curr_commit->n_changes-1][strlen(last_commit2->tracked_files[i].filename)] = '0';
                        curr_commit->commit_changes[curr_commit->n_changes-1][strlen(last_commit2->tracked_files[i].filename)+1] = '\0';

                        curr_commit->tracked_files = (struct file_item*)realloc(curr_commit->tracked_files,sizeof(struct file_item)*(curr_commit->n_tracked_files+1));
                        curr_commit->n_tracked_files = curr_commit->n_tracked_files + 1;
                        store_content(helper, last_commit2->tracked_files[i].filename, &(curr_commit->tracked_files[curr_commit->n_tracked_files-1]));
                    }
                }

                for(int i=0; i<n_resolutions; i++){

                    //if resolution file_path is NULL, consider as rm
                    if(resolutions[i].resolved_file == NULL && (check_tracked(resolutions[i].file_name, last_commit1) == 1)){  
                        curr_commit->n_changes = curr_commit->n_changes + 1;
                        curr_commit->commit_changes = (char**)realloc(curr_commit->commit_changes, sizeof(char*)*curr_commit->n_changes);
                        curr_commit->commit_changes[curr_commit->n_changes-1] = (char*)malloc(sizeof(char)*(strlen(resolutions[i].file_name)+2));
                        strcpy(curr_commit->commit_changes[curr_commit->n_changes-1],resolutions[i].file_name);
                        curr_commit->commit_changes[curr_commit->n_changes-1][strlen(resolutions[i].file_name)] = '1';
                        curr_commit->commit_changes[curr_commit->n_changes-1][strlen(resolutions[i].file_name)+1] = '\0';
                    }else{
                        //write file1(resolution) to file2(target file)
                        int pre_hash = hash_file(helper, resolutions[i].file_name);

                        update_file(resolutions[i].resolved_file, resolutions[i].file_name);
                        curr_commit->tracked_files = (struct file_item*)realloc(curr_commit->tracked_files,sizeof(struct file_item)*(curr_commit->n_tracked_files+1));
                        curr_commit->n_tracked_files = curr_commit->n_tracked_files + 1;
                        store_content(helper, resolutions[i].file_name, &(curr_commit->tracked_files[curr_commit->n_tracked_files-1]));

                        curr_commit->n_changes = curr_commit->n_changes + 1;
                        curr_commit->commit_changes = (char**)realloc(curr_commit->commit_changes, sizeof(char*)*curr_commit->n_changes);
                        curr_commit->commit_changes[curr_commit->n_changes-1] = (char*)malloc(sizeof(char)*(strlen(resolutions[i].file_name)+2));
                        strcpy(curr_commit->commit_changes[curr_commit->n_changes-1],resolutions[i].file_name);
                        curr_commit->commit_changes[curr_commit->n_changes-1][strlen(resolutions[i].file_name)] = '2';
                        curr_commit->commit_changes[curr_commit->n_changes-1][strlen(resolutions[i].file_name)+1] = '\0';

                        curr_commit->n_modify = curr_commit->n_modify + 1;
                        curr_commit->modify_records = (struct modify_record*)realloc(curr_commit->modify_records, sizeof(struct modify_record)*curr_commit->n_modify);
                        curr_commit->modify_records[curr_commit->n_modify-1].filename = (char*)malloc(sizeof(char)*(strlen(resolutions[i].file_name)+1));
                        strcpy(curr_commit->modify_records[curr_commit->n_modify-1].filename, resolutions[i].file_name);
                        curr_commit->modify_records[curr_commit->n_modify-1].filename[strlen(resolutions[i].file_name)] = '\0';
                        curr_commit->modify_records[curr_commit->n_modify-1].pre_hash = pre_hash;
                        curr_commit->modify_records[curr_commit->n_modify-1].new_hash = hash_file(helper, resolutions[i].file_name);
                    }
                }
                char* message1 = "Merged branch ";
                curr_commit->commit_message = (char*)malloc(sizeof(char)*(strlen(message1)+strlen(branch_name)+1));
                strcpy(curr_commit->commit_message, message1);
                strcat(curr_commit->commit_message, branch_name);
                curr_commit->commit_message[strlen(message1)+strlen(branch_name)] = '\0';

                string_order(curr_commit->commit_changes,curr_commit->n_changes);
                store_commit_id(curr_commit);

                svc_helper->n_commits_pool = svc_helper->n_commits_pool + 1;
                svc_helper->commit_pool = (struct commit**)realloc(svc_helper->commit_pool, sizeof(struct commit*)*svc_helper->n_commits_pool);
                svc_helper->commit_pool[svc_helper->n_commits_pool-1] = curr_commit;

                curr_commit->parent = last_commit1;
                svc_helper->head->aim_commit = curr_commit;

                printf("Merge successful\n");
                return curr_commit->commit_id;
            }else{
                printf("Branch not found\n");
                return NULL;
            }
        }   
}




// void main(){
// 	struct SVC* helper = svc_init();
// 	assert(hash_file(helper, "hello.py") == 2027);
// 	assert(hash_file(helper, "fake.c") == -2);
// 	assert(svc_commit(helper, "No changes") == NULL);
// 	assert(svc_add(helper, "hello.py") == 2027);
// 	print_stage(helper);
// 	assert(svc_add(helper, "Tests/test1.in") == 564);
// 	print_stage(helper);
// 	assert(strcmp(svc_commit(helper, "Initial commit"),"74cde7") == 0);
// 	print_stage(helper);
// 	// assert(svc_add(helper, "Tests/test1.in") == -2);
// 	// print_stage(helper);
// 	assert(svc_rm(helper, "Tests/test1.in") == 564);
// 	print_stage(helper);
//     change_file("hello.py");
//     printf("create a new branch: %d\n", svc_branch(helper, "sadasda"));
// 	char* id_2 = svc_commit(helper, "Second commit");
//     print_changes(helper->head->aim_commit);
//     print_tracked_files(helper->head->aim_commit);
//     get_commit(helper,"74cde7");
//     get_commit(helper,"64cde7");
//     print_commit(helper, id_2);
//     assert(hash_file(helper, "hello.py") == 2562);
    
//     char* a = "master";
//     char* b = (char*)malloc(sizeof(char)*7);
//     printf("same check is: %d\n",strcmp(a,b));
//     free(b);
//     cleanup(helper);
// }
void main(void){
	struct SVC* svc = svc_init();
	
	int a = svc_add(svc,"hello.py");
	printf("%d\n",a);
	int b = svc_rm(svc,"hello.py");
	printf("%d\n",b);
	svc_add(svc,"hello.py");
	svc_add(svc,"a.txt");
	svc_commit(svc,"please");
	svc_add(svc,"b.c");
	svc_add(svc,"c.c");
	svc_commit(svc,"hello");
	svc_rm(svc,"b.c");
	svc_commit(svc,"hi");
}
