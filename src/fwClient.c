/***************************************************************************
 *            fwClient.h
 *
 *  Copyright  2016  mc
 *  <mcarmen@<host>>
 ****************************************************************************/
#include "fwClient.h"

/**
 * Function that sets the field addr->sin_addr.s_addr from a host name
 * address.
 * @param addr struct where to set the address.
 * @param host the host name to be converted
 * @return -1 if there has been a problem during the conversion process.
 */
int setaddrbyname(struct sockaddr_in *addr, char *host)
{
  struct addrinfo hints, *res;
	int status;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(host, NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return -1;
  }

  addr->sin_addr.s_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;

  freeaddrinfo(res);

  return 0;
}


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
	while((param = getopt(argc, argv, "h:p:")) != -1){
		switch((char) param){
		  case 'h': break;
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
 * Returns the host name where the server is running.
 * @param argc the number of the application arguments.
 * @param an array with all the application arguments.
 * @Return Returns the host name where the server is running.<br />
 * Returns null if the application has been called with the wrong parameters.
 */
 char * getHost(int argc, char* argv[]){
  char * hostName = NULL;
  int param;

  optind=1;
    // We process the application execution parameters.
	while((param = getopt(argc, argv, "h:p:")) != -1){
		switch((char) param){
			case 'p': break;
			case 'h':
        hostName = (char*) malloc(sizeof(char)*strlen(optarg)+1);
				// Un cop creat l'espai, podem copiar la cadena
				strcpy(hostName, optarg);
				break;
			default:
				printf("Parametre %c desconegut\n\n", (char) param);
				hostName = NULL;
		}
	}

	printf("in getHost host: %s\n", hostName); //!!!!!!!!!!!!!!
	return hostName;
 }



/**
 * Shows the menu options.
 */
void print_menu()
{
		// Mostrem un menu perque l'usuari pugui triar quina opcio fer

		printf("\nAplicació de gestió del firewall\n");
		printf("  0. Hello\n");
		printf("  1. Llistar les regles filtrat\n");
		printf("  2. Afegir una regla de filtrat\n");
		printf("  3. Modificar una regla de filtrat\n");
		printf("  4. Eliminar una regla de filtrat\n");
		printf("  5. Eliminar totes les regles de filtrat.\n");
		printf("  6. Sortir\n\n");
		printf("Escull una opcio: ");
}


/**
 * Sends a HELLO message and prints the server response.
 * @param sock socket used for the communication.
 */
void process_hello_operation(int sock)
{
  struct hello_rp hello_rp;
  memset(&hello_rp, '\0', sizeof(hello_rp));
  stshort(MSG_HELLO, &hello_rp.opcode);
  send(sock, &hello_rp, sizeof(hello_rp), 0);

  int recived_bytes = recv(sock, &hello_rp, sizeof(hello_rp), 0);
  printf("Recived bytes: %d, with the message: %s\n", recived_bytes, hello_rp.msg);
}

void process_add_rule(int sock)
{
    rule srule;
    memset(&srule, '\0', sizeof(srule));

    char rule[MAX_BUFF_SIZE];
    char* token;
    /*
         * We need a way to accept blank spaces to be accepted
         * in the client inputted rule.
         *
         * If we use a regex it will accept every character until there is a
         * character implying there is a new line (\n).
         *
         * Regex: [^\t\n] -> scanf("%[^\t\n]s", )
         *
         * A space is needed at the start of scanf function in order to not be
         * skipped.
        */
    printf("Introduce the rule with format 'src|dst adress netmask [sport|dport] port:\n'");
    scanf(" %[^\t\n]s", rule);

    // SRC|DST
    token = strtok(rule, " ");
    if(strcmp(token, "src") == 0)
        srule.src_dst_addr = SRC;
    else
        if(strcmp(token, "dst") == 0)
            srule.src_dst_addr = DST;

    //ADRESS
    token = strtok(NULL, " ");
    inet_aton(ip_addr, &srule.addr);

    //MASK
    token = strtok(NULL, " ");
    srule.mask = atoi(token);

    //SPORT|dport
    token = strtok(NULL, " ");
    if(strcmp(token, "dport") == 0)
        srule.src_dst_port = DST;
    else
        if(strcmp(token, "dport" == 0) || strcmp(token, "0") == 0)
            srule.src_dst_port = SRC;

    //PORT
    token = (NULL, " ");
    srule.port = atoi(token);

    char buffer[MAX_BUFF_SIZE];
    stshort(MSG_ADD, &buffer);

    *((rule*)(buffer + sizeof(short))) = srule;

    send(sock, &buffer, (sizeof(srule) + sizeof(short)), 0);
}

void process_list_rule(int sock){

}

/**
 * Closes the socket connected to the server and finishes the program.
 * @param sock socket used for the communication.
 */
void process_exit_operation(int sock)
{
  int op_code;
  stshort(MSG_FINISH, &op_code);
  send(sock, &op_code, sizeof(op_code), 0);

  printf("Closing Client...\n");
  exit(0);
}

/**
 * Function that process the menu option set by the user by calling
 * the function related to the menu option.
 * @param s The communications socket
 * @param option the menu option specified by the user.
 */
void process_menu_option(int s, int option)
{
  switch(option){
    // Opció HELLO
    case MENU_OP_HELLO:
      process_hello_operation(s);
      break;
    case MENU_OP_LIST_RULES:
      process_list_rule(s);
      break;
    case MENU_OP_ADD_RULE:
      process_add_rule(s);
      break;
    case MENU_OP_CHANGE_RULE:
      break;
    case MENU_OP_DEL_RULE:
      break;
    case MENU_OP_FLUSH:
      break;
    case MENU_OP_EXIT:
      process_exit_operation(s);
      break;
    default:
      printf("Invalid menu option\n");
  }
}

int main(int argc, char *argv[]){
  int s;
  unsigned short port;
  char *hostName;
  int menu_option = 0;

  port = getPort(argc, argv);
  hostName = getHost(argc, argv);

  //Checking that the host name has been set.Otherwise the application is stopped.
	if(hostName == NULL){
		perror("No s'ha especificat el nom del servidor\n\n");
		return -1;
	}

    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in source;
    source.sin_family = AF_INET;
    source.sin_port = htons(port);
    setaddrbyname(&source, hostName);
    //source.sin_addr.s_addr = INADDR_ANY;

    connect(s,(struct sockaddr*)&source, sizeof(source));

    printf("Connected to port: %d, with host: %s\n", port, hostName);

  do{
      print_menu();
		  // getting the user input.
		  scanf("%d",&menu_option);
		  printf("\n\n");
		  process_menu_option(s, menu_option);

	  }while(menu_option != MENU_OP_EXIT); //end while(opcio)

  return 0;
}
