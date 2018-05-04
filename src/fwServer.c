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
  strcpy(hello_rp.msg, "Hello World");

  printf("Sending message: %s\n\n", hello_rp.msg);

  send(sock, &hello_rp, sizeof(hello_rp), 0);
}

void process_ADD(int sock, struct FORWARD_chain *chain, char* buffer)
{
    int offset = sizeof(short);
    char src_dst_addr[MAX_BUFF_SIZE], src_dst_port[MAX_BUFF_SIZE];

    if(chain->first_rule == NULL) //Chain is empty
    {
        printf("Chain is empty. Adding recived rule\n");
        chain->first_rule = (struct fw_rule*) malloc(sizeof(struct fw_rule));
        memcpy(&chain->first_rule->rule, (buffer + offset), sizeof(buffer) + sizeof(offset));
        chain->first_rule->next_rule = NULL;
        printf("Recived rule: ");
        print(chain->first_rule->rule);
        printf("\n");
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
        printf("\n");
    }

    chain->num_rules++;
}

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
        printf(" to the buffer\n");
        memcpy(buffer + offset, &aux->rule, sizeof(aux->rule));
        offset += sizeof(aux->rule);
        aux = aux->next_rule;
    }

    send(sock, buffer, offset, 0);
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
    case MSG_CHANGE:
      break;
    case MSG_DELETE:
      break;
    case MSG_FLUSH:
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
