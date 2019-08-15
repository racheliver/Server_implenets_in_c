
            /************************************************
			*    user name:rachelive                        *
            *    Name & ID:Racheli  Verechzon , 305710071   *
            *  ======================================       *
            *   Ex3 - Server implenets in c Programming     *
            *               26/01/2019                      *
            *  ======================================       *
            *************************************************/
			*** Please open this document using Notepad++ ***
			*************************************************

			
**********Description of program files*************


#server.c	    - c file.
#threadpool.c	- c file.
#README 	    - this file.



********** How to Compile this program*************



for compiling the project do:

gcc threadpool.c server.c  -o ex3 -lpthread
ex3 is the executable program.



***********How to run this program***********
 
 in the following format:
./server   <port> <pool-size> <max-number-of-request>                  


			
***********what this program do************************

This program is a HTTP server. It can handle HTTP requests of type GET with HTTP method 1.1 or 1.0 , and send back to the client a corresponding responde. 
The server uses the threadpool in order to handle as maximum as 200 requests simultaniosly.
The file server.c is the main file of the program. There are 15 functions, all comments are inside of the server.c file.


**********Details of the program's objectives******


when the server is running , you can browse from you browser in the following way:
write localhost:<port-number> 
This will bring you to the root directory of the server(showed in table) or if index.html is found - it will open it.
any other pass can be filled in after <port-number> and "/".
The server can handle with files with spaces, and traverse through the folders tree.
Beware that you have to have execution permitions for folders and read permitions for files in order to access them.

 
