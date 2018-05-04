# FW Server and Client

An university project using TCP sockets on Linux with C.

It's a project to learn the functionalitites of sockets and get some skill with them, so don't expect much about it.

The client sends operation codes to the server to make some actions.

CLIENT | SERVER
------ | ------
0. HELLO  | Sends through the socket a 'Hello World!' message
1. LIST RULES | Sends the rules that are stored in the server
2. ADD RULE | Adds the recived from the client rule to a list of rules on the server
3. MODIFY RULE | Modify the rule with the passed one that has the same ID
4. REMOVE RULE | Removes the rule with the same ID as passed
5. REMOVE ALL RULES | Removes all the rules from the server