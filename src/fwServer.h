/***************************************************************************
 *            fwServer.h
 *
 *  Copyright  2016  mc
 *  <mcarmen@<host>>
 ****************************************************************************/
 #include "common.h"

 #define MAX_QUEUED_CON 10 // Max number of connections queued



/**
 * Structures to implement the firewall rules
 * ==========================================
 */

 struct fw_rule
 {
    rule rule; // FORWARD_rule
    struct fw_rule * next_rule;
 };

 struct FORWARD_chain
 {
   int num_rules;
   struct fw_rule * first_rule;
 };


/**
 * Returns the port specified as an application parameter or the default port
 * if no port has been specified.
 * @param argc the number of the application arguments.
 * @param an array with all the application arguments.
 * @return  the port number from the command line or the default port if
 * no port has been specified in the command line. Returns -1 if the application
 * has been called with the wrong parameters.
 */
int getPort(int argc, char* argv[]);

 /**
 * Receives and process the request from a client.
 * @param the socket connected to the client.
 * @param chain the chain with the filter rules.
 * @return 1 if the user has exit the client application therefore the
 * connection whith the client has to be closed. 0 if the user is still
 * interacting with the client application.
 */
int process_msg(int sock, struct FORWARD_chain *chain);

void process_HELLO_msg(int sock);
void process_LIST(int sock, struct FORWARD_chain *chain);
void process_ADD(int sock, struct FORWARD_chain *chain, char* buffer);
void process_CHANGE(int sock, struct FORWARD_chain* chain, char* buffer);
void process_DELETE(int sock, struct FORWARD_chain* chain, char* buffer);
void process_FLUSH(int sock, struct FORWARD_chain* chain);
void process_FINISH_msg(int sock);
