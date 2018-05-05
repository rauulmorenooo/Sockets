/***************************************************************************
 *            fwServer.c
 *
 *  Copyright  2016  mc
 *  <mc@<host>>
 ****************************************************************************/

#include "fwServer.h"

/**
 * Returns the port specified as an application parameter or the default port
 * if no port has been specified.
 * @param argc the number of the application arguments.
 * @param an array with all the application arguments.
 * @return  the port number from the command line or the default port if
 * no port has been specified in the command line. Returns -1 if the application
 * has been called with the wrong parameters.
 */
int getPort(int argc, char* argv[])
{
  int param;
  int port = DEFAULT_PORT;

  optind=1;
  // We process the application execution parameters.
  while((param = getopt(argc, argv, "p:")) != -1){
		switch((char) param){
			case 'p':
			  // We modify the port variable just in case a port is passed as a
			  // parameter
				port = atoi(optarg);
				break;
			default:
				printf("Parametre %c desconegut\n\n", (char) param);
				port = -1;
		}
	}

	return port;
}


 /**
 * Function that sends a HELLO_RP to the  client
 * @param sock the communications socket
 */
void process_HELLO_msg(int sock)
{
  struct hello_rp hello_rp;
  memset(&hello_rp, '\0', sizeof(hello_rp));
  stshort(MSG_HELLO_RP, &hello_rp.opcode);
  strcpy(hello_rp.msg, "Hello World");

  printf("Sending message: %s\n\n", hello_rp.msg);

  send(sock, &hello_rp, sizeof(hello_rp), 0);
}

/**
 * Function that gathers all the rules in the server, appends to a buffer
 * and sends it to the client.
 * @param sock the communications socket.
 * @param struct FORWARD_chain* chain List of all the rules in the server.
 */
void process_LIST(int sock, struct FORWARD_chain *chain)
{
    int offset = 0;
    char buffer[MAX_BUFF_SIZE];
    memset(buffer, 0, sizeof(buffer));
    stshort(MSG_RULES, buffer);
    offset += sizeof(short);

    memcpy(buffer, &chain->num_rules, sizeof(chain->num_rules));
    offset += sizeof(short);

    struct fw_rule* aux = chain->first_rule;

    while(aux != NULL)
    {
        printf("Appending rule: ");
        print(aux->rule);
        memcpy(buffer + offset, &aux->rule, sizeof(aux->rule));
        offset += sizeof(aux->rule);
        aux = aux->next_rule;
    }

    send(sock, buffer, offset, 0);
}

/**
 * Function that adds a rule into the server.
 * @param sock the communications socket.
 * @param struct FORWARD_chain* chain List of all the rules in the server.
 * @param buffer buffer containing the new rule
 */
void process_ADD(int sock, struct FORWARD_chain *chain, char* buffer)
{
    int offset = sizeof(short);
    int done = FALSE;
    char opcode[MAX_BUFF_SIZE];

    if(chain->first_rule == NULL)
    {
        printf("Chain is empty. Adding recived rule\n");
        chain->first_rule = (struct fw_rule*) malloc(sizeof(struct fw_rule));
        memcpy(&chain->first_rule->rule, (buffer + offset), sizeof(buffer) + sizeof(offset));
        chain->first_rule->next_rule = NULL;
        printf("Recived rule: ");
        print(chain->first_rule->rule);
        done = TRUE;
    }

    else
    {
        printf("Chain isn't empty. Adding recived rule at the end of the chain\n");
        struct fw_rule* aux = chain->first_rule;

        while(aux->next_rule != NULL)
            aux = aux->next_rule;

        aux->next_rule = (struct fw_rule*) malloc(sizeof(struct fw_rule));
        memcpy(&aux->next_rule->rule, (buffer + offset), sizeof(buffer) + sizeof(offset));
        aux->next_rule->next_rule = NULL;
        printf("Recived rule: ");
        print(aux->next_rule->rule);
        done = TRUE;
    }

    chain->num_rules++;

    if(done == TRUE)
        stshort(MSG_OK, &opcode);

    else
        stshort(MSG_ERR, &opcode);

    send(sock, opcode, sizeof(opcode), 0);
}

/**
 * Function that changes a rule of the server.
 * @param sock the communications socket.
 * @param struct FORWARD_chain* chain List of all the rules in the server.
 * @param buffer buffer containing the new rule and the index of the rule
 *               to be changed.
 */
void process_CHANGE(int sock, struct FORWARD_chain* chain, char* buffer)
{
    char opcode[MAX_BUFF_SIZE];
    int index = 0, offset = sizeof(short);
    memcpy(&index, buffer + offset, sizeof(index));
    offset += sizeof(index);
    rule rrule;
    memset(&rrule, 0, sizeof(rrule));
    memcpy(&rrule, buffer + offset, sizeof(rrule));
    printf("Changing rule at index: %d\n", index);

    if (index <= chain->num_rules) // The index recived is in the rule list
    {
        int i = 1;
        struct fw_rule* aux = chain->first_rule;

        while (i < index)
        {
            aux = aux->next_rule;
            i++;
        }

        aux->rule = rrule;
        printf("Rule at index %d will become: ", index);
        print(rrule);


        stshort(MSG_OK, &opcode);
        send(sock, opcode, sizeof(opcode), 0);
    }

    else
    {
        printf("ERROR. Recived index is not in the list range.\n");

        stshort(MSG_ERR, &opcode);
        send(sock, opcode, sizeof(opcode), 0);
    }
}

/**
 * Function that deletes a rule from the server.
 * @param sock the communications socket.
 * @param struct FORWARD_chain* chain List of all the rules in the server.
 * @param buffer buffer containing the index of the rule to be deleted.
 */
void process_DELETE(int sock, struct FORWARD_chain* chain, char* buffer)
{
    char opcode[MAX_BUFF_SIZE];
    int index = 0, offset = sizeof(short);
    memcpy(&index, buffer + offset, sizeof(index));

    if(index <= chain->num_rules)
    {
        int i = 1;
        struct fw_rule* previous;
        struct fw_rule* aux = chain->first_rule;

        if (index == 1)
        {
            previous = aux->next_rule;
            free(aux);
            chain->first_rule = previous;
            chain->num_rules--;
        }

        else
        {
            while(i < index)
            {
                previous = aux;
                aux = aux->next_rule;
                i++;
            }

            previous->next_rule = aux->next_rule;
            free(aux);
            chain->num_rules--;
        }

        stshort(MSG_OK, &opcode);
        send(sock, opcode, sizeof(opcode), 0);
    }

    else
    {
        stshort(MSG_ERR, &opcode);
        send(sock, opcode, sizeof(opcode), 0);
    }
}

/**
 * Function that deletes all rules in the server.
 * @param sock the communications socket.
 * @param struct FORWARD_chain* chain List of all the rules in the server.
 */
void process_FLUSH(int sock, struct FORWARD_chain* chain)
{
    char opcode[MAX_BUFF_SIZE];

    struct fw_rule* aux = chain->first_rule;

    if(chain->first_rule == NULL)
    {
        stshort(MSG_ERR, &opcode);
    }

    else
    {
        while(chain->first_rule != NULL)
        {
            chain->first_rule = aux->next_rule;
            free(aux);
        }

        stshort(MSG_OK, &opcode);
    }

    send(sock, opcode, sizeof(opcode), 0);
}

void process_FINISH_msg(int sock)
{
    close(sock);
}

 /**
 * Receives and process the request from a client.
 * @param the socket connected to the client.
 * @param chain the chain with the filter rules.
 * @return 1 if the user has exit the client application therefore the
 * connection whith the client has to be closed. 0 if the user is still
 * interacting with the client application.
 */
int process_msg(int sock, struct FORWARD_chain *chain)
{
  unsigned short op_code;
  int finish = 0;

  char buffer[MAX_BUFF_SIZE];
  recv(sock, buffer, sizeof(buffer), 0);

  op_code = ldshort(buffer);
  printf("Opcode recived is: %d\n\n", op_code);

  switch(op_code)
  {
    case MSG_HELLO:
      process_HELLO_msg(sock);
      break;
    case MSG_LIST:
      process_LIST(sock, chain);
      break;
    case MSG_ADD:
      process_ADD(sock, chain, buffer);
      break;
    case MSG_CHANGE:
      process_CHANGE(sock, chain, buffer);
      break;
    case MSG_DELETE:
      process_DELETE(sock, chain, buffer);
      break;
    case MSG_FLUSH:
      process_FLUSH(sock, chain);
      break;
    case MSG_FINISH:
      process_FINISH_msg(sock);
      finish = 1;
      break;
    default:
      perror("Message code does not exist.\n");
  }

  return finish;
}

 int main(int argc, char *argv[]){

  int port = getPort(argc, argv);
  int finish=0;
  struct FORWARD_chain chain;

  chain.num_rules=0;
  chain.first_rule=NULL;

  printf("Connected to port: %d\n", port);

  int s_serv = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); //domain, type, protocol
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  bind(s_serv, (struct sockaddr*)&addr, sizeof(addr));

  int backlog = 10;
  listen(s_serv, backlog);

  struct sockaddr client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  while(1){

      //int s2 = accept(s_serv, (struct sockaddr*)&addr, &addr_len);(
      int s2 = accept(s_serv, (struct sockaddr*)&client_addr, &client_addr_len);
      close(s_serv);

      do{
         finish = process_msg(s2, &chain);
      }while(!finish);

      printf("Closing Server...\n");
      exit(0);
  }

  return 0;
}
