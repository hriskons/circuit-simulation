/*
 * Small code for reading the netlist file line by line,
 * extracting string tokens from each line,
 * and recognizing the type of element described on each line.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"
#include "plot.h"


#define	RESISTANCE_NUM_PARSE_ELEMENTS  4
#define	CAPACITY_NUM_PARSE_ELEMENTS    4
#define INDUCTANCE_NUM_PARSE_ELEMENTS  4
#define MOSFET_NUM_PARSE_ELEMENTS      6 


static int get_node_from_line( LIST* list,char* line , NODE* node , int* type);

/*
 * Parse netlist
 */
int parse_netlist(char* filename , LIST* list){

	char line[ MAX_LINE_SIZE + 1];
	NODE element_node;
	int element_type;
	int line_number;
	int res;

	if( !filename || !list  )
		return 0;

	FILE* file;
	file = fopen(filename,"r");
	if( !file )
		return 0;

	line_number = 1 ;
	/*Read until EOF */
	while( !feof(file)){

		/* Get a single line */
		if( fgets( line , MAX_LINE_SIZE , file) != NULL ) {

			/* check for comment,else process */
			if( line[0] != '*'){
				res = get_node_from_line( list, line , &element_node , &element_type);
				if( res == 1 ){

					/* add node read and store at list */
					//printf("NODE READ: %s , %d , %d , %g \n",element_node.resistance.name , element_node.resistance.node1 , element_node.resistance.node2 , element_node.resistance.value);
					res = add_node_to_list(list , &element_node , element_type);
					if( !res ){
						printf("NO MEMORY\n");
						return 0;
					}
				}
				else if( res == 0 ){

					/* Error while parsing line */
					fclose(file);
					printf("Error while parsing.Line %d : %s\n",line_number , line);
					return 0;
				}
			}

		}
		line_number++;
	}

  if( list->has_reference == 1 ){
	 return 1;
  }
  else{
    printf("No reference node (ground) specified\n");
    return 0;
  }
}


/* 
 * Proccess a single line
 *
 * 1) Identify circuit element
 * 2) Check for errors
 * 3) Build circuit node
 *
 * Retuns: 1 when a node was identified correctly.Variables -node-  and -type- contain a circuit node
 *         0 when a parsing error occurs.Variables -node- and -type- values are not predicted
 */

static int get_node_from_line( LIST* list,char* line , NODE* node , int* type){

	char c;
	char* token;
  int flag;
  static int node_count = 1;


	if( line == NULL || node == NULL  || type == NULL )
		return 0;

  //printf("\nParsing line: %s\n",line);
	c = line[0];


	switch(c){
		case 'R':
		case 'r':{
			/* read name */
			token = strtok(line," ");
			if( token == NULL ){

				return 0 ;
			}
			strcpy( node->resistance.name , token);

			/* 
       *Read <+> node
       */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->resistance.node1 = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->resistance.node1 = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){

          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->resistance.node1 = n;
        }
      }
			

      /* 
       * Read <-> node
       */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}


      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->resistance.node2 = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->resistance.node2 = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){

          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->resistance.node2 = n;
        }
      }      	

      //node->resistance.node2 = atoi(token);

			/* read value node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}
    			
      node->resistance.value = atof(token);

			/* NO MORE TOKENS.IF FOUND RETURN ERROR */
			token = strtok(NULL," \n");
			if( token == NULL ){
				*type = NODE_RESISTANCE_TYPE;
				return 1;
			}
			else{
				/* tokens were found.print for debugging...*/
				printf("LINE: %s , garbage token : %s\n" , line , token);
				return 0;
			}

		}
		case 'C':
		case 'c':{
			/* read name */
			token = strtok(line," ");
			if( token == NULL ){
				return 0 ;
			}
			strcpy( node->capacity.name , token);			

			/* read <+> node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->capacity.node1 = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->capacity.node1 = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){
          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->capacity.node1 = n;
        }
      } 

//			node->capacity.node1 = atoi(token);
			
			/* read <-> node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->capacity.node2 = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->capacity.node2 = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){
          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->capacity.node2 = n;
        }
      } 

			/* read value node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}
			node->capacity.value = atof(token);

			/* NO MORE TOKENS.IF FOUND RETURN ERROR */
			token = strtok(NULL," \n");
			if( token == NULL ){
				*type = NODE_CAPACITY_TYPE;
				return 1;
			}
			else{
				/* tokens were found.print for debugging...*/
				printf("LINE: %s , garbage token : %s\n" , line , token);
				return 0;
			}
		}
		case 'L':
		case 'l':{

			/* read name */
			token = strtok(line," ");
			if( token == NULL ){
				return 0 ;
			}
			strcpy( node->inductance.name , token);
			
			/* read <+> node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}
      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->inductance.node1 = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->inductance.node1 = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){
          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->inductance.node1 = n;
        }
      } 


//			node->inductance.node1 = atoi(token);
			
			/* read <-> node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->inductance.node2 = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->inductance.node2 = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){
          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->inductance.node2 = n;
        }
      } 


			//node->inductance.node2 = atoi(token);

			/* read value node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}
			node->inductance.value = atof(token);

			/* NO MORE TOKENS.IF FOUND RETURN ERROR */
			token = strtok(NULL," \n");
			if( token == NULL ){
				*type = NODE_INDUCTANCE_TYPE;
				return 1;
			}
			else{
				/* tokens were found.print for debugging...*/
				printf("LINE: %s , garbage token: %s\n" , line , token);
				return 0;
			}			
		}

		/*
		 * VOLTAGE SOURCE
		 */
		case 'v':
		case 'V':{

			/* read name */
			token = strtok(line," ");
			if( token == NULL ){
				return 0 ;
			}
			strcpy( node->source_v.name , token);

			/* read <+> node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->source_v.node1 = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->source_v.node1 = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){
          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->source_v.node1 = n;
        }
      } 


			//node->source_v.node1 = atoi(token);

			/* read <-> node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->source_v.node2 = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->source_v.node2 = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){
          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->source_v.node2 = n;
        }
      } 

			//node->source_v.node2 = atoi(token);

			/* read value node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}
			node->source_v.value = atof(token);


			/* NO MORE TOKENS.IF FOUND RETURN ERROR */
			token = strtok(NULL," \n");
			if( token == NULL ){
				*type = NODE_SOURCE_V_TYPE;
				return 1;
			}
			else{
				/* tokens were found.print for debugging...*/
				printf("LINE: %s , garbage token : %s\n" , line , token);
				return 0;
			}

		}

		/*
		 * CURRENT SOURCE
		 */
		case 'i':
		case 'I':{

			/* read name */
			token = strtok(line," ");
			if( token == NULL ){
				return 0 ;
			}
			strcpy( node->source_i.name , token);

			/* read <+> node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->source_i.node1 = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->source_i.node1 = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){

          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->source_i.node1 = n;
        }
      }


			//node->source_i.node1 = atoi(token);

			/* read <-> node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->source_i.node2 = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->source_i.node2 = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){

          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->source_i.node2 = n;
        }
      }

			//node->source_i.node2 = atoi(token);

			/* read value node */
			token = strtok(NULL," ");
			if( token == NULL){
				return 0;
			}
			node->source_i.value = atof(token);


			/* NO MORE TOKENS.IF FOUND RETURN ERROR */
			token = strtok(NULL," \n");
			if( token == NULL ){
				*type = NODE_SOURCE_I_TYPE;
				return 1;
			}
			else{
				/* tokens were found.print for debugging...*/
				printf("LINE: %s  garbage token : %s\n" , line , token);
				return 0;
			}

		}

		/*
		 * MOSFET transistor
		 */
		case 'M':
		case 'm':{

			/* read name */
			token = strtok(line," ");
			if( token == NULL ){
				return 0 ;
			}
			strcpy( node->mosfet.name , token);

			/* read drain */
			token = strtok(NULL," ");
			if( token == NULL )
				return 0;

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->mosfet.drain = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->mosfet.drain = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){

          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->mosfet.drain = n;
        }
      }

			//node->mosfet.drain = atoi(token);

			/* read gate */
			token = strtok(NULL," ");
			if( token == NULL )
				return 0;
			
      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->mosfet.gate = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->mosfet.gate = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){

          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->mosfet.gate = n;
        }
      }      


      //node->mosfet.gate = atoi(token);

			/* read source */
			token = strtok(NULL," ");
			if( token == NULL )
				return 0;

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->mosfet.source = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->mosfet.source = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){

          int n;
          printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->mosfet.source = n;
        }
      }

      //node->mosfet.source = atoi(token);

			/* read body */
			token = strtok(NULL," ");
			if( token == NULL )
				return 0;

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->mosfet.body = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->mosfet.body = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){

          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->mosfet.body = n;
        }
      }

      //node->mosfet.body = atoi(token);

			/* read length */
			token = strtok(NULL," ");
			if( token == NULL )
				return 0;
			node->mosfet.l = atof(token);

			/* read width */
			token = strtok(NULL," ");
			if( token == NULL )
				return 0;
			node->mosfet.w = atof(token);

			/* NO MORE TOKENS.IF FOUND RETURN ERROR */
			token = strtok(NULL," \n");
			if( token == NULL ){
				*type = NODE_MOSFET_TYPE;
				return 1;
			}
			else{
				/* tokens were found.print for debugging...*/
				printf("LINE: %s  garbage token : %s\n" , line , token);
				return 0;
			}
		}

		/*
		 * Bipolar junction transistor
		 */
		case 'Q':
		case 'q':{
			/* read name */
			token = strtok(line," ");
			if( token == NULL ){
				return 0 ;
			}
			strcpy( node->bjt.name , token);

			/* read collector */
			token = strtok(NULL," ");
			if( token == NULL )
				return 0;

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->bjt.collector = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->bjt.collector = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){

          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->bjt.collector = n;
        }
      }

      //node->bjt.collector = atoi(token);

			/* read base */
			token = strtok(NULL," ");
			if( token == NULL )
				return 0;

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->bjt.base = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->bjt.base = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){

          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->bjt.base = n;
        }
      }

      //node->bjt.base  = atoi(token);

			/* read emitter */
			token = strtok(NULL," ");
			if( token == NULL )
				return 0;

      /* check for reference node (ground) */
      if( strcmp(token,"0") == 0 ){
        node->bjt.emitter = 0;
        list->has_reference = 1;
      }
      else{
        /*
         * this is not a reference node.Add string to
         * hash table
         */
        flag = ht_insert_pair(list->hashtable, token , node_count);
        if( flag == 1 ){
          /* successfull insertion */
          node->bjt.emitter = node_count;
          node_count++;    // get ready for the next node
        }
        else if( flag == 0 ){
          /* NULL pointer or out of memory */
          printf("Error at inserting pair to hash table..\n");
          //free_list(list);
          exit(1);
        }
        else if( flag == -1 ){

          int n;
          //printf("Node : \"%s\" already on hash table \n",token);
          ht_get(list->hashtable,token,&n);
          node->bjt.emitter = n;
        }
      }			


      //node->bjt.emitter = atoi(token);

			/* more will be added later */
			//
			//			HERE
			//


			/* NO MORE TOKENS.IF FOUND RETURN ERROR */
			token = strtok(NULL," \n");
			if( token == NULL ){
				*type = NODE_BJT_TYPE;
				return 1;
			}
			else{
				/* tokens were found.print for debugging...*/
				printf("LINE: %s  garbage token : %s\n" , line , token);
				return 0;
			}

		}

		/*
		 * DIODE
		 */


		/*
		 * Commands
		 */
		case '.':{

			char* temp = line;
			token = strtok(temp," ");
				
			if( !token )
				return 0;

			/* solving method */
			if( strcmp(token,".OPTIONS") == 0 || strcmp(token,".options") == 0 ){

				token = strtok(NULL," \n");
				//printf("Token after .options :%s\n",token);
				if( !token ){
					printf("Error while parsing...\n");
					printf("Line : %s\n", line );
					return 0;
				}

				//printf("TOKEN AFTER .OPTIONS :%s\n",token);
				if( strcmp(token,"SPD") == 0 || strcmp(token,"spd") == 0 ){
					token = strtok(NULL," \n");
					if(!token){
						list->solving_method = METHOD_CHOLESKY;
						return 2;
					}

					/*------  .OPTIONS SPD ITER----------------*/
					if( strcmp(token,"ITER") == 0 || strcmp(token,"ITER") == 0){
						list->solving_method = METHOD_CG;
						return 2;
					}
					else if (strcmp(token,"SPARCE") == 0 || strcmp(token,"sparce") == 0){
						/* cholesky solution with sparce arrays   */
					}
				}
				else if( strcmp(token,"ITER") == 0 || strcmp(token,"iter") == 0 ){
					token = strtok(NULL," \n");
					if( !token ){					
						list->solving_method = METHOD_BICG;
						return 2;
					}
						
					
					if( strcmp(token,"SPARCE") == 0 || strcmp(token,"sparce") == 0){
						/*   Bi-CG with sparce arrays  */
					}
					else if( strcmp(token, "SPD") == 0 || strcmp(token, "spd") == 0){
						token = strtok(NULL," \n");
						if( !token ){					
							list->solving_method = METHOD_CG;
							return 2;
						}
						else if ( strcmp(token,"SPARCE") == 0 || strcmp(token,"sparce") == 0){
							/*   CG solving method with sparce arrays   */
						}
					}
					
				}
				else if( strcmp(token,"ITOL") == 0 || strcmp(token,"itol") == 0){
					/* tolerance */
					token = strtok(NULL," \n");
					if ( !token ){
						printf("Error while parsing...\n");
						printf("Line: %s\n",line);
						return 0;
					}


					if( strcmp(token,"=") ==  0 ){
						/* now read the tolerance value */
						token = strtok(NULL," \n");

						if( !token ){
							printf("Error while parsing..\n");
							printf("Line: %s\n",line);
							return 0;

						}
						else{
							double val;
							val = atof(token);
						
							list->itol = val;
							return 2;
						}  
					}
					else{
						printf("Error while parsing...\n");
						printf("Line: %s\n",line);
						return 0;
					}

				}
				else if(strcmp(token,"SPARCE") == 0 || strcmp(token,"sparce") == 0){
					/*LU with sparce arrays*/
				}
				
				else{
					printf("No token after .OPTIONS \n");
					return 0;
				}

				return 2;
			}
			else if( strcmp(token,".DC") == 0 || strcmp(token,".dc") == 0 ){  // check for .DC

/*
				token = strtok(temp," ");
				if( !token ){
					printf("Error while parsing...\n");
					printf("Line : %s\n", line );
					return 0;
				}
*/
				token = strtok(NULL , " ");
				if( !token ){
				//	list->solving_method = METHOD_LU;

					//printf("Error while parsing...\n");
					//printf("Line : %s\n", line );
					return 2;
				}
				list->dc_sweep.name = strdup(token);
				//printf("DC: name = %s \n",list->dc_sweep.name);

				token = strtok(NULL , " ");
				if( !token ){
					printf("Error while parsing...\n");
					printf("Line : %s\n", line );
					return 0;
				}
				list->dc_sweep.start_v = atof( token );

				token = strtok(NULL , " ");
				if( !token ){
					printf("Error while parsing...\n");
					printf("Line : %s\n", line );
					return 0;
				}
				list->dc_sweep.end_v = atof(token);

				token = strtok(NULL , " ");
				if( !token ){
					printf("Error while parsing...\n");
					printf("Line : %s\n", line );
					return 0;
				}
				list->dc_sweep.inc = atof(token);


				// check if node already declared 
				list->dc_sweep.node = list_search_by_name(list , list->dc_sweep.name );
				if( !(list->dc_sweep.node)){
					printf(".DC Error: %s element not found\n",list->dc_sweep.name);
					exit(1);
				}
				list->dc_sweep.oldval = list->dc_sweep.node->node.source_v.value ; 

			}
			else if( strcmp(token,".PLOT") == 0 || strcmp(token,".plot") == 0 ){
  			
  			    int plot_num=0;
				//reading the PLOT command keyword


				plot_init();
    	    	list->plot = PLOT_ON;


        		while(1){
					//reading the source type for plotting
					token = strtok(NULL," (\n");
					if( !token && plot_num == 0 ){
           				printf("Error while parsing...\n");
						printf("Line : %s\n", line );
						return 0;
				  	}
          			else if(!token && plot_num>0)
            			break;
          

					//reading the node name
					token = strtok(NULL,")");
					printf("Going to plot results for node: %s \n", token);
					if( !token ){
						printf("Error while parsing...\n");
						printf("Line : %s\n", line );
						return 0;
					}
          			plot_add_node(token);
          			plot_num++;
        		}
			}
	
		}	
	}


	return 2;
}