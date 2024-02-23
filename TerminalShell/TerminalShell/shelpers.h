//
//  shelpers.h
//  Shell
//
//  Created by Elisabeth Frischknecht on 2/14/24.
//  CS6013 HW 3 - UNIX SHELL

#pragma once

//////////////////////////////////////////////////////////////////////////////////
//
// Author: Ben Jones (I think) with a lot of clean up by J. Davison de St. Germain
//
// Date:   2019?
//         Jan 2022 - Cleanup
//
// Class: CS 6013 - Systems I
//
//////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////
// A struct to contain a command line program information.
// You'll need to fork/exec for each one of these commands!
// To get started, initially just assume the user tries to only
// execute 1 command.

struct Command {
  std::string execName; // The name of the executable.

  // Remember argv[0] should be the name of the program (same as execName)
  // Also, argv should end with a nullptr!
  std::vector<const char*> argv;
  int                      inputFd, outputFd;
  bool                     background;
};

//////////////////////////////////////////////////////////////////////////////////
//
// getCommands()
//
//    Takes in a list of string tokens and places the results into Command structures.
//
// Read through this function.  You'll need to fill in a few parts to implement
// I/O redirection and (possibly) backgrounded commands.
// Most of the places you need to fill in contain an assert(false), so you'll
// discover them when you try to use more functionality
//
// Returns an empty vector on error.
//
std::vector<Command> getCommands( const std::vector<std::string> & tokens );

//////////////////////////////////////////////////////////////////////////////////
// Helper function for you to use.  (Already implemented for you.)

// Takes in a command line (string) and returns it broken into separate pieces.
std::vector< std::string > tokenize( const std::string & command_line_string );

// Prints out the contents of a Command structure.  Useful for debugging.
std::ostream& operator<<( std::ostream& outs, const Command& c );

