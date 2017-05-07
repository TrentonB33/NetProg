Network Programming: Project 4
Team members: Trenton Baldrey, Aaron Brooks

Notes to the grader:

First of all, this code does not fully function. We will discuss what does work in this README.
What does work:
	1. A temp directory of with .4220_fiel_list.txt file is created
	2. The client sends an initial content request to the server
	3. The server responds by sending a write request and sends the contents of the 
	   .4220_fiel_list.txt to the client
	4. The client creates it's own .4220_fiel_list.txt file for comparison
	5. The client has code that allows for the comparison on the hashes of it's own files
	   And the files on the server
	6. The client has code that allows for the sending of a query to the server to determine the 
	   newest version of a file
	7. Both the server and client should have code to both send and recieve files
	
What does not work:
	1. The server does not have code that allows for the parsing of query packets
	   to determine if the server or client file is newer
	2. The file we are using to generate hashes is not perfectly MD5, but it's pretty close
	   In that it is able to generate consistent hashes, the original message is just read
	   slightly wrong.
	3. While we are confident that the query communcation code works, we haven't tested it.
	4. There is no formal code for the generation of gets and puts from the client to the
	   server.
	5. The temporary folder set up by the server is not deleted.
	
	
	Finally, there are a lot of debug statements.
	
	
	
We built our program over the previous code that we developed for the TFTP server, so the file sizes
Are limited to 32MB.  For the new functions, such as content and query, we added opcodes 6 and 7 respectively,
As well as custom packets to send between the client and server.
We have commented the code such that the read should be able to determine what
Each section of the program accomplishes. The Code should compile without warnings.
	