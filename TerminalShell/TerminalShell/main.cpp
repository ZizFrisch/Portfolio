//
//  main.cpp
//  Shell
//
//  Created by Elisabeth Frischknecht on 2/14/24.
//  This program is meant to emulate a simple terminal shell design.
//  The program makes a child process for each applicable command and then execs it to become a new process, and not a copy of the shell program
//  one built-in command has been created--the cd command. This cannot be implemented as an exec command.
//  Background processes can be run on this shell
//  Multiple commands can be linked together by using piping to connect the inputs and outputs
//
//  IMPORTANT: typing "exit" will close the program

#include <iostream>
#include "shelpers.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

//this is a function that checks to see if any of the background processes have been completed. I
//If they have, it will print out the child process's pid, and remove it from the "background commands" vector.
void checkBackgroundCommands(std::vector<pid_t> backgroundCommands){
    for(int i = 0; i < backgroundCommands.size(); i++){
            int status;
            //WNOHANG: If there are no child processes in a waitable state (i.e., no child processes have terminated or stopped), waitpid returns 0 immediately instead of waiting
            pid_t result = waitpid(i, &status, WNOHANG);
            //If waitpid returns a positive number, the process that been completed and should be removed from the vector
            if (result > 0) {
                std::cout << "Background process " << backgroundCommands[i] << " completed." << std::endl;
                backgroundCommands.erase(backgroundCommands.begin() + i);
            }
        }
}

//closes all the file descriptors for all commands
void cleanUp(std::vector<Command> commands){
    for(Command command : commands){
        if(command.inputFd != 0 ){
            close(command.inputFd);
        }
        if(command.outputFd != 1){
            close(command.outputFd);
        }
    }
}

int main(int argc, const char * argv[]) {
    std::cout << "Welcome to Ziz's Shell!\n";
    std::cout << "zizshell$ ";
    //create a loop that will wait for commands from the user's i/o
    std::string request = "";
    std::vector<pid_t> backgroundCommands = {};
    
    while(getline(std::cin, request)){
        //check if any background commands have completed
        checkBackgroundCommands(backgroundCommands);
        
        //if exit is typed, quit the program
        if (request == "exit"){
            break;
        }
        
        //tokenize the request
        std::vector<std::string> tokens = tokenize(request);

        //get the commands
        std::vector<Command> commands = getCommands(tokens);
        
        //cd is a special case of a built-in command because it can't be called with execvp. before trying to run any commands,
        //I handle the special case.
        //if there was a command with an error, we will have an empty vector here.
        //Check that it's not empty so that a segmentation fault is not caused when calling commands[0]
        if(commands.size() > 0 &&  commands[0].execName == "cd"){
            if(tokens.size() == 1){
                //std::cout << "go home!\n";
                std::string homedir = getenv("HOME");
                
                int status_code = chdir(homedir.c_str());
                
                if(status_code != 0){
                    perror("chdir");
                }
                
            }
            else{
                //std::cout << "go somewhere else!\n";
                std::string directory = tokens[1];
                
                int status_code = chdir(directory.c_str() );
                
                if(status_code != 0){
                    perror("chdir");
                }
            }
            std::cout << "changed directory\n";
            std::cout << "zizshell$ ";
            continue;
        }
        
        //Process all the commands
        for(Command command : commands){
            //create a child process with a fork
            int c_pid = fork();
            
            if (c_pid == -1) {
                //check for error code
                perror("fork");
                cleanUp(commands);
                exit(1);
            }//closes if statement - child error code
            else if(c_pid == 0){
                //child process
                
                // i/o redirection
                if(command.inputFd != 0){
                    dup2(command.inputFd, STDIN_FILENO);
                }
                if(command.outputFd !=1){
                    dup2(command.outputFd, STDOUT_FILENO);
                }

                
                //execvp on the child process will change it from being a copy of the "shell" to being its own system call
                int status_code = execvp(command.execName.c_str(), const_cast<char* const*>(command.argv.data()));
                
                //if execvp fails, print an error message
                if(status_code != 0){
                    perror("execvp");
                    cleanUp(commands);
                    exit(1);
                }
        
                exit(0); //terminate the child process
            }//closes else if (child process)
            else{
                //parent process
                if(command.background){
                    //add to the background commands, and don't wait for it to be done
                    backgroundCommands.push_back(c_pid);
                    std::cout << "Started background process " << c_pid << std::endl;
                }
                else{
                    //parent process
                    int status;
                    waitpid(c_pid, &status, 0);
                    
                    //after done waiting for the child, close any file descriptors for that command
                    if(command.inputFd != 0 ){
                        close(command.inputFd);
                    }
                    if(command.outputFd != 1){
                        close(command.outputFd);
                    }
                }
            }//closes else (parent process)
        
        }//closes command execution
        std::cout << "zizshell$ ";
    }//closes while loop
    
    return 0;
}
