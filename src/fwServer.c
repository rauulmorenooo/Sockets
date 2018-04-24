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

void process_ADD(int sock, struct FORWARD_chain *chain)
{
    char recived_rule[MAX_BUFF_SIZE];
    recv(sock, &recived_rule, MAX_BUFF_SIZE, 0);
    printf("RECIVED OK.\nRecived rule is: %s\n", recived_rule);

    // Rule saving as a char array (optional)
    /*FILE *fd = fopen("rules/rulelist.txt", "r+");
    fprintf(fd, "%s\n", recived_rule);
    fclose(fd);*/


}

void process_LIST(int sock, struct FORWARD_chain *chain){

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
      process_LIST(sock, &chain);
      break;
    case MSG_ADD:
      process_ADD(sock, &chain);
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

      //close(s2);
      printf("Closing Server...\n");
      exit(0);
  }

  return 0;
}
