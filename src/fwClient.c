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

  recv(sock, &hello_rp, sizeof(hello_rp), 0);
  printf("Recived: %s\n", hello_rp.msg);
}

/**
 * Sends a LIST Request to recive all rules at the server and print them.
 * @param sock socket used for the communication.
 */
void process_list_rule(int sock)
{
    int op_code;
    stshort(MSG_LIST, &op_code);
    send(sock, &op_code, sizeof(op_code), 0);

    unsigned short opcode = 0;
    int offset = 0;
    int num_rules = 0;
    rule aux;
    char buffer[MAX_BUFF_SIZE];
    memset(buffer, 0, sizeof(buffer));

    recv(sock, buffer, sizeof(buffer), 0);

    memcpy(&opcode, buffer, sizeof(opcode));
    offset += sizeof(unsigned short);
    memcpy(&num_rules, buffer + offset, sizeof(num_rules));
    printf("There are %d rules at the server.\n", num_rules);
    offset += sizeof(int);

    if(num_rules > 0)
    {
        printf("|-----------------------------------------------|\n");
        printf("|\t\tFORWARD RULE LIST\t\t|\n");
        printf("|-----------------------------------------------|\n");

        int i = 0;
        while(i < num_rules)
        {
            memset(&aux, 0, sizeof(aux));
            memcpy(&aux, (buffer + offset), sizeof(aux) + sizeof(offset));
            printf("   %d. ", i+1);
            print(aux);
            offset += sizeof(rule);
            i++;
        }

        printf("|-----------------------------------------------|\n\n");
    }

    else
    {
        printf("ERROR. There are no rules to be listed.\n\n");
    }
}

/**
 * Sends an ADD Request with a rule to be added into the rule list at server.
 * @param sock socket used for communication.
 */
void process_add_rule(int sock)
{
    char buffer[MAX_BUFF_SIZE], recivedcode[MAX_BUFF_SIZE];
    int offset = 0;
    unsigned short opcode;

    memset(buffer, 0, sizeof(buffer));

    char src_dst_address[MAX_BUFF_SIZE], ip_address[MAX_BUFF_SIZE],
    src_dst_port[MAX_BUFF_SIZE];
    int netmask, port;

    printf("Introduce the rule in the format (src|dst address netmask [sport|dport] port): \n");
    scanf("%s %s %d %s %d", &src_dst_address, &ip_address, &netmask, &src_dst_port, &port);

    stshort(MSG_ADD, buffer);
    offset += sizeof(unsigned short);

    rule srule = setRule(src_dst_address, ip_address, netmask, src_dst_port, port);

    memcpy(buffer + offset, &srule, sizeof(srule));
    offset += (sizeof(srule) + 1);

    if(send(sock, buffer, offset, 0) > 0)
    {
        recv(sock, recivedcode, sizeof(recivedcode), 0);
        opcode = ldshort(recivedcode);
        if(opcode == MSG_OK)
            printf("Rule added correctly.\n");

        else if(opcode == MSG_ERR)
            printf("Cannot add rule.\n");
    }
}

/**
 * Sends a CHANGE Request with the index and the new rule to be changed at the server.
 * @param sock socket used for the communication.
 */
void process_change_rule(int sock)
{
    char buffer[MAX_BUFF_SIZE], recivedcode[MAX_BUFF_SIZE];
    unsigned short opcode;
    memset(buffer, 0, sizeof(buffer));
    stshort(MSG_CHANGE, buffer);

    int offset = sizeof(unsigned short);

    char src_dst_address[MAX_BUFF_SIZE], ip_address[MAX_BUFF_SIZE],
    src_dst_port[MAX_BUFF_SIZE];
    int netmask, port, index;

    printf("Introduce the index and new rule you want to modify, with format (src|dst address netmask [sport|dport] port): \n");
    scanf("%d %s %s %d %s %d", &index, &src_dst_address, &ip_address, &netmask, &src_dst_port, &port);
    printf("Rule %d will become %s %s %d %s %d\n", index, src_dst_address, ip_address, netmask, src_dst_port, port);

    memcpy(buffer + offset, &index, sizeof(index));
    offset += sizeof(int);

    rule srule = setRule(src_dst_address, ip_address, netmask, src_dst_port, port);
    memcpy(buffer + offset, &srule, sizeof(srule));
    offset += (sizeof(srule) + 1);

    send(sock, buffer, offset, 0);

    recv(sock, recivedcode, sizeof(recivedcode), 0);
    opcode = ldshort(recivedcode);

    if(opcode == MSG_OK)
        printf("Rule changed correctly.\n");

    else if(opcode == MSG_ERR)
        printf("Cannot add rule.\n");
}

/**
 * Sends a DELETE Request with the index of the rule to be deleted at the server.
 * @param sock socket used for the communication.
 */
void process_DELETE_RULE(int sock)
{
    unsigned short opcode;
    int offset = 0, index;
    char buffer[MAX_BUFF_SIZE], recivedcode[MAX_BUFF_SIZE];
    memset(buffer, 0, sizeof(buffer));
    memset(recivedcode, 0, sizeof(recivedcode));
    stshort(MSG_DELETE, &buffer);
    offset += sizeof(unsigned short);

    printf("Introduce the rule index to be deleted: \n");
    scanf("%d", &index);
    memcpy(buffer + offset, &index, sizeof(index));

    send(sock, buffer, sizeof(buffer), 0);

    recv(sock, recivedcode, sizeof(recivedcode), 0);
    opcode = ldshort(recivedcode);

    if(opcode == MSG_OK)
        printf("Rule deleted.\n");

    else
        if(opcode == MSG_ERR)
            printf("Cannot delete rule.\n");
}

/**
 * Sends a FLUSH Request to delete all rules at the server.
 * @param sock socket used for the communication.
 */
void process_FLUSH(int sock)
{
    unsigned short opcode;
    char buffer[MAX_BUFF_SIZE], recivedcode[MAX_BUFF_SIZE];
    memset(buffer, 0, sizeof(buffer));
    memset(recivedcode, 0, sizeof(recivedcode));
    stshort(MSG_FLUSH, &buffer);

    send(sock, buffer, sizeof(buffer), 0);

    recv(sock, recivedcode, sizeof(recivedcode), 0);
    opcode = ldshort(recivedcode);

    if(opcode == MSG_OK)
        printf("All rules have been deleted.\n");

    else
        if(opcode == MSG_ERR)
            printf("ERROR. There was a problem deleting rules.\n");
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
      process_change_rule(s);
      break;
    case MENU_OP_DEL_RULE:
      process_DELETE_RULE(s);
      break;
    case MENU_OP_FLUSH:
      process_FLUSH(s);
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

    connect(s,(struct sockaddr*)&source, sizeof(source));

    printf("Connected to port: %d, with host: %s\n", port, hostName);

  do{
      print_menu();
		  // getting the user input.
		  scanf(" %d",&menu_option);
		  printf("\n\n");
		  process_menu_option(s, menu_option);

	  }while(menu_option != MENU_OP_EXIT); //end while(opcio)

  return 0;
}
