#include "minesweeper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//transfer 1 dimension to n dimensions
// N, a=N//BCDE,b=(N%BCDE)//CDE,c=((N%BCDE)%CDE)//DE,d=(((N%BCDE)%CDE)%DE)//E,e=(((N%BCDE)%CDE)%DE)%E
int* trans_1_n(int index, int* dim_CDE, int dim){
    int* list_coord = (int*)malloc(sizeof(int)*dim);
    
    if(index == 0){
        for(int i=0;i<dim;i++){
            list_coord[i] = 0;
        }
        return list_coord;
    }
    for(int i = dim-1; i>=0; i--){
        //int j = dim - 1 -i;
        int curr_cde = index;
        
        for(int j = 0; j < i; j++){
            curr_cde = index%dim_CDE[j];
        }
        curr_cde = curr_cde/dim_CDE[i];
        list_coord[i] = curr_cde;
    }
    return list_coord;
}

//get [BCDE,CDE,DE,E,1]  //[0] (0)
int* trans_CDE(int dim, int* dim_sizes){
    int* dim_count_CDE = (int*)malloc(sizeof(int)*dim);
	
    for(int i = 0;i<dim-1; i++){
        int count_CDE = 1;
        for(int j = i+1;j<dim;j++){
            count_CDE = count_CDE * dim_sizes[j];
        }
		dim_count_CDE[i] = count_CDE;
    }
	dim_count_CDE[dim-1] = 1;
    return dim_count_CDE;
}

//transfer n dimensions to 1 dimension
//if [a,b,c,d,e], -> e*1+d*E+c*D*E+b*C*D*E+a*BCDE [BCDE,CDE,DE,E,1]
int trans_n_1(int* dim_CDE, int dim, int* list_coord){
    int index = 0;
    for(int i =0; i<dim; i++){
        index = index + list_coord[i]*dim_CDE[i];
    }
    return index;
}

void init_game(struct cell * game, int dim, int * dim_sizes, int num_mines, int ** mined_cells){
	//printf("%d**",dim_sizes[0]);
    int num_cells = 1;
	for(int i = 0; i<dim;i++){
		num_cells = num_cells*dim_sizes[i];
	}
    
    int* dim_count_CDE = trans_CDE(dim, dim_sizes) ;
	//printf("%d\n",dim_count_CDE[0]);
    
	int* mined_cells_coord = (int*)malloc(sizeof(int)*num_mines);
	//int mined_cells_coord[num_mines];
	for(int i = 0; i<num_mines;i++){     //num_mines 
		mined_cells_coord[i] = trans_n_1(dim_count_CDE,dim,mined_cells[i]);
	}
	   //{3, 4, 5, 6, 7} [[2,3,4],[3,4,5],[4,5,6],[5,6,7],[6,7,-1]]  
	int** coord;  // all cells coord in dim dimensions
	coord = (int**)malloc(sizeof(int*)*num_cells);
    
    //int cell_neighbours[num_cells][dim];
    int* adj_dim_sizes = (int*)malloc(sizeof(int)*dim);
    
	int* curr_adjacent;
	int* adj_dim_CDE;
    int** all_adjacents;
    int* cell_self;
    int* check_0 = (int*)malloc(sizeof(int)*dim);;
    //int cell_neighbours[dim][3];//all possible neighbours
    int** cell_neighbours = (int**)malloc(sizeof(int*)*dim);
        for(int m=0;m<dim;m++){
            cell_neighbours[m] = (int*)malloc(sizeof(int)*3);
        }
    
    //the first for loop is to add all adjacents and set all 'selected' and 'mined' = 0; initialize 'hint' = 0
    for(int i = 0; i<num_cells; i++){ //num_cells
        game[i].mined = 0;
		game[i].selected = 0;
	    //printf("%d\n",game[i].selected);
        game[i].hint = 0;
        
        
        int adjacent_count = 1;
		//memcpy(b,a,sizeof(int)*k);
		//memcpy(coord[i],(*trans_1_n)(i, dim_count_CDE,dim),sizeof(int)*dim);
        coord[i] = trans_1_n(i, dim_count_CDE,dim);
        for(int j=0; j<dim; j++){
            game[i].coords[j] = coord[i][j];
        }
        
        for(int j=0;j<dim;j++){
            if(coord[i][j]==0 && dim_sizes[j] == 1){
                check_0[j] = 0;
                adjacent_count = adjacent_count*1;
                adj_dim_sizes[j] = 1;
                cell_neighbours[j][0] = 0;
                cell_neighbours[j][1] = -1;
                cell_neighbours[j][2] = -1;
                
            }else if(coord[i][j]==0 && dim_sizes[j] > 1){
                adjacent_count = adjacent_count*2;
                adj_dim_sizes[j] = 2;
                cell_neighbours[j][0] = 1;
                cell_neighbours[j][1] = 0;
                cell_neighbours[j][2] = -1;
            }else if(coord[i][j]==dim_sizes[j]-1){
                adjacent_count = adjacent_count*2;
                adj_dim_sizes[j] = 2;
                cell_neighbours[j][0] = dim_sizes[j]-2;
                cell_neighbours[j][1] = dim_sizes[j]-1;
                cell_neighbours[j][2] = -1;
            }
            else{
                adjacent_count = adjacent_count*3;
                adj_dim_sizes[j] = 3;
                cell_neighbours[j][0] = coord[i][j]-1;
                cell_neighbours[j][1] = coord[i][j];
                cell_neighbours[j][2] = coord[i][j]+1;
            }
                
        }

        game[i].num_adjacent = adjacent_count-1;
        //printf("%d     ",game[i].num_adjacent);
        //int all_adjacents[game[i].num_adjacent][dim]; //all neighbours's coord in dim dimensions
        all_adjacents = (int**)malloc(sizeof(int*)*game[i].num_adjacent);
        for(int q=0;q<game[i].num_adjacent;q++){
            all_adjacents[q] = (int*)malloc(sizeof(int)*dim);
        }
        
        adj_dim_CDE = trans_CDE(dim, adj_dim_sizes);//adjacent cells size was reduced to 3^dim,
      
        cell_self = (int*)malloc(sizeof(int)*dim); // the ith cell itself coord
        for(int k = 0; k<dim;k++){
            if(check_0[k] == 0){
                cell_self[k] = 0;
            }else{
                cell_self[k] = 1;//cell_neighbours[k][1];
            }
            
        }
		
        int index_self = trans_n_1(adj_dim_CDE,dim,cell_self);
        //printf("self coord of game%d is: %d\n",i,index_self);
		
        // add all its neighbours to all_adjacents
        //printf("adjacent_count is: %d\n",adjacent_count);
        for(int k = 0; k<adjacent_count;k++){
           // printf("When k is: %d\n", k);
            curr_adjacent = trans_1_n(k,adj_dim_CDE,dim);
            
            if(k < index_self){
                //printf("the adj index is: ");
                for(int l = 0; l<dim;l++){
                    all_adjacents[k][l]=cell_neighbours[l][curr_adjacent[l]];
                    //printf("%d  ",all_adjacents[k][l]);
                }
            }else if(k > index_self){
                //printf("the adj index is: ");
                for(int l = 0; l<dim;l++){
                    all_adjacents[k-1][l]=cell_neighbours[l][curr_adjacent[l]];
                    //printf("%d  ",all_adjacents[k-1][l]);
                    
                }
                //printf("\n");
            }
            free(curr_adjacent);
        }
        
        int adj_real_index = 0;
        for(int k=0;k<adjacent_count-1;k++){
            adj_real_index = trans_n_1(dim_count_CDE,dim,all_adjacents[k]);
            game[i].adjacent[k] = &game[adj_real_index]; 
			
			//printf("%d\n",game[i].adjacent[k]->mined);
        }
    
        //free all malloc in this for loop
        for(int j=0;j<game[i].num_adjacent;j++){
            free(all_adjacents[j]);
        }
        free(all_adjacents);
        
        free(adj_dim_CDE);
        free(cell_self);
        
        
    }
    
	int mined_index = 0;
	for(int i = 0; i<num_mines; i++){
	    mined_index = trans_n_1(dim_count_CDE,dim,mined_cells[i]);
	    game[mined_index].mined = 1;
        //printf("index of mined cell is: %d\n",mined_index);
	}

 	int mined_count = 0;
	
	for(int i = 0; i<num_cells; i++){
		mined_count = 0;
        //printf("game%d : %d\n",i,game[i].num_adjacent);
		
		for(int j = 0;j<game[i].num_adjacent;j++){
			if(game[i].adjacent[j]->mined == 1){
				mined_count++;
			}
		}
		game[i].hint = mined_count;
		//printf("game%d : %d\n",i,mined_count);
	}
	//printf("OMG!");
	
	// free part
    free(dim_count_CDE);    //dim_count_CDE
    free(mined_cells_coord);
  
    for(int i=0;i<num_cells;i++){
        free(coord[i]);
    }
    free(coord);
    free(adj_dim_sizes);
    
    for(int i=0;i<dim;i++){
        free(cell_neighbours[i]);
    }
    free(cell_neighbours);
    free(check_0);
    
	return;
}

void select_recursion(struct cell * game, int* coords, int dim, int* dim_sizes){
    int* dim_count_CDE = trans_CDE(dim, dim_sizes);
    int cell_index = trans_n_1(dim_count_CDE, dim, coords);
    
    if(game[cell_index].hint == 0){
        for(int i=0; i<game[cell_index].num_adjacent; i++){
            if(game[cell_index].adjacent[i]->selected == 0){
                game[cell_index].adjacent[i]->selected = 1;
                select_recursion(game, game[cell_index].adjacent[i]->coords,dim,dim_sizes);
            }
        }
    }
        
    free(dim_count_CDE);
    return;
}

int check_win(struct cell * game,int dim, int * dim_sizes){
    
    int num_cells = 1;
	for(int i = 0; i<dim;i++){
		num_cells = num_cells*dim_sizes[i];
	}
    
    for(int i=0;i<num_cells;i++){
        if((game[i].selected == 1 && game[i].mined == 0)||(game[i].selected == 0 && game[i].mined == 1)){
            continue;
        }else{
            return 0;
        }
    }
    return 1;
    
}
int select_cell(struct cell * game, int dim, int * dim_sizes, int * coords) {
    // calculate dim_count_CDE first
    
    int* dim_count_CDE = trans_CDE(dim, dim_sizes) ;
    
    int real_index = trans_n_1(dim_count_CDE,dim,coords);
    free(dim_count_CDE);
    //First, check if valid
    for(int i=0;i<dim;i++){
        if(coords[i]<0||coords[i]>=dim_sizes[i]){
            
            return 0;
        }
    }
    
    //Second, check if selected already
    if(game[real_index].selected != 0){
        
        
        return 0;
    }
    //Third, check if the cell contains a mine, if so return 1
    else if(game[real_index].mined == 1){
        game[real_index].selected = 1;
        
        
        return 1;
    }
    //Forth, check if hint != 0
    else if(game[real_index].hint != 0){
        game[real_index].selected = 1;
        if(check_win(game, dim, dim_sizes) == 1){
            
            
            return 2;
        }
        return 0;
    }else if(game[real_index].hint == 0){
        game[real_index].selected = 1;
        for(int i=0;i<game[real_index].num_adjacent;i++){
            if(game[real_index].adjacent[i]->selected == 0){
                game[real_index].adjacent[i]->selected = 1;
                select_recursion(game, game[real_index].adjacent[i]->coords,dim,dim_sizes);
            }
        }
        if(check_win(game, dim, dim_sizes) == 1){
            
            
            return 2;
        }
        
        return 0;
    }
    
    return 0;
    
}


	

