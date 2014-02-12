

*********************************************************************

README FOR SHELL
*********************************************************************
Team Members:
----------------
Nikhileswar Gosala      A20305497
Geetha Sankineni        A20310882
Sai Harshita Bandlamudi A20304108

Minix Version : 3.2.1 
Compiler : cc 


Loading Program to minix:
------------------------------

1. use the below commands to configure ssh
	pkgin update
	pkgin install openssh
2. Set the password for user by typing passwd
3. Use an ftp client like fileZilla to connect to MINIX using ssh and transfer the file 
4. Go to the OSDI directory by typing "cd .." followed by "cd OSDI"
4. Invoke the shell by using "sh shell.sh" command 
5. On successful login, the custom shell will execute a profile file which will initialize the environment variables like HOME and PATH
      By Default: HOME variable is set to “/root”
                  PATH variable contains “/bin:/usr/bin”.
                  PROMPT variable is set to the current directory and will be changed when we navigate to other directory.
				  
		These values can be changed inside profile.txt
                  

6. The new custom shell can support all command like "ls", "ls -l", "cat filename" (All commands with and without argument)

7. The new custom shell can support redirection and piped operations also.
      
	Syntax
	Redirection   -- "command argument =>filename"
	Pipe command  -- " $wc $ (fgrep -l include MinixShell.c)
    

 
8. To create a new process, we use getpid where a fork() command is executed and  when we enter the  command “getpid”, shell will execute fork() to generate a new child process. 
	
	Eg:
	PROMPT> getpid
	the process id is 23456



9. The shell can be terminated by using “exit” or by pressing ctrl -c. 

