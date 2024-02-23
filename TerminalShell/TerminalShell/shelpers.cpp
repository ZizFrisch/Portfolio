#include "shelpers.h"
#include <cassert>

//  Author: Elisabeth Frischknecht


using namespace std;

////////////////////////////////////////////////////////////////////////
// Example test commands you can try once your shell is up and running:
//
// ls
// ls | nl
// cd [dir]
// cat < shelpers.cpp
// cat < shelpers.cpp | nl
// cat shelpers.cpp | nl
// cat shelpers.cpp | nl | head -50 | tail -10
// cat shelpers.cpp | nl | head -50 | tail -10 > ten_lines.txt 
//
// - The following two commands are equivalent.  [data.txt is sent into nl and the
//   output is saved to numbered_data.txt.]
//
// nl > numbered_data.txt < data.txt
// nl < data.txt > numbered_data.txt 
//
// - Assuming numbered_data.txt has values in it... try running:
//   [Note this probably doesn't work like one might expect...
//    does it behave the same as your normal shell?]
//
// nl < numbered_data.txt > numbered_data.txt
//
// - The following line is an error (input redirection at end of line).
//   It should fail gracefully (ie, 1) without doing anything, 2) cleaning
//   up any file descriptors that were opened, 3) giving an appropriate
//   message to the user).
//
// cat shelpers.cpp | nl | head -50 | tail -10 > ten_lines.txt < abc
// 

////////////////////////////////////////////////////////////////////////
// This routine is used by tokenize().

bool splitOnSymbol( vector<string> & words, int i, char c )
{
   if( words[i].size() < 2 ){
      return false;
   }
   int pos;
   if( (pos = words[i].find(c)) != string::npos ){
      if( pos == 0 ){
         // Starts with symbol.
         words.insert( words.begin() + i + 1, words[i].substr(1, words[i].size() -1) );
         words[i] = words[i].substr( 0, 1 );
      }
      else {
         // Symbol in middle or end.
         words.insert( words.begin() + i + 1, string{c} );
         string after = words[i].substr( pos + 1, words[i].size() - pos - 1 );
         if( !after.empty() ){
            words.insert( words.begin() + i + 2, after );
         }
         words[i] = words[i].substr( 0, pos );
      }
      return true;
   }
   else {
      return false;
   }
}

////////////////////////////////////////////////////////////////////////

vector<string> tokenize( const string& s )
{
   vector<string> ret;
   int pos = 0;
   int space;

   // Split on spaces:

   while( (space = s.find(' ', pos) ) != string::npos ){
      string word = s.substr( pos, space - pos );
      if( !word.empty() ){
         ret.push_back( word );
      }
      pos = space + 1;
   }

   string lastWord = s.substr( pos, s.size() - pos );

   if( !lastWord.empty() ){
      ret.push_back( lastWord );
   }

   for( int i = 0; i < ret.size(); ++i ) {
      for( char c : {'&', '<', '>', '|'} ) {
         if( splitOnSymbol( ret, i, c ) ){
            --i;
            break;
         }
      }
   }
   return ret;
}

////////////////////////////////////////////////////////////////////////
// overload the operator for the Commands class so it can be printed nicely
// useful in debugging
ostream& operator<<( ostream& outs, const Command& c )
{
   outs << c.execName << " [argv: ";
   for( const auto & arg : c.argv ){
      if( arg ) {
         outs << arg << ' ';
      }
      else {
         outs << "NULL ";
      }
   }
   outs << "] -- FD, in: " << c.inputFd << ", out: " << c.outputFd << " "
        << (c.background ? "(background)" : "(foreground)");
   return outs;
}

////////////////////////////////////////////////////////////////////////
//
// getCommands()
//
// Parses a vector of command line tokens and places them into (as appropriate)
// separate Command structures.
//
// Returns an empty vector if the command line (tokens) is invalid.
vector<Command> getCommands( const vector<string> & tokens )
{
   vector<Command> commands( count( tokens.begin(), tokens.end(), "|") + 1 ); // 1 + num |'s commands

   int first = 0;
   int last = find( tokens.begin(), tokens.end(), "|" ) - tokens.begin();

   bool error = false;

   for( int cmdNumber = 0; cmdNumber < commands.size(); ++cmdNumber ){
      const string & token = tokens[ first ];

      if( token == "&" || token == "<" || token == ">" || token == "|" ) {
         error = true;
         break;
      }

      Command & command = commands[ cmdNumber ]; // Get reference to current Command struct.
      command.execName = token;

      // Must _copy_ the token's string (otherwise, if token goes out of scope (anywhere)
      // this pointer would become bad...)

      command.argv.push_back( strdup( token.c_str() ) ); // argv0 == program name

      command.inputFd  = STDIN_FILENO;
      command.outputFd = STDOUT_FILENO;

      command.background = false;
	
      for( int j = first + 1; j < last; ++j ) {

         if( tokens[j] == ">" || tokens[j] == "<" ) {
            // Handle I/O redirection tokens
            //
            // Note, that only the FIRST command can take input redirection
            // (all others get input from a pipe)
            // Only the LAST command can have output redirection!
             
             //the first cmdnumber is the first command, where cmdNumber is the first one
             if(cmdNumber == 0 && tokens[j] == "<"){
                 //O_RDWR allows reading and writing from the file
                 //the token right after the < will hold the file name
                 command.inputFd  = open(tokens[j+1].c_str(), O_RDWR);
                 
                 //error clause
                 if(command.inputFd < 0){
                     perror("open");
                     exit(2);
                 }
             }
             //the last command can have output redirection
             else if(cmdNumber == commands.size() - 1 && tokens[j] == ">"){
                 //O_CREAT: creates a new file if one doesn't exist
                 //O_TRUNC: If the file exists, its size is set to 0 (truncated). else it makes a new file
                 //O_RDWR: allows reading and writing from the file
                 //0666: I ran into permissions issues, means the file will have read and write permissions for all users.
                 //the token right after the > will hold the file name
                 command.outputFd = open(tokens[j+1].c_str(), O_CREAT | O_RDWR | O_TRUNC, 0666);
                 
                 //error clause
                 if(command.outputFd <0){
                     perror("open");
                     exit(3);
                 }
             }
             else{
                 //if we haven't received valid input at this point, it was an error
                 std::cout << "\ni/o redirection error\n";
                 error = true;
                 break;
             }
             //we have already processed the file name, so skip past it by incrementing j
             j++;
         }
         else if( tokens[j] == "&" ){
            //the & indicates that the background should be run in the background. so set that boolean to true.
            command.background = true;
         }
         else {
            // Otherwise this is a normal command line argument! Add to argv.
            command.argv.push_back( tokens[j].c_str() );
         }
      }//closes the for loop (j)

      if( !error ) {

         if( cmdNumber > 0 ){
            // There are multiple commands.  Open a pipe and
            // connect the ends to the fd's for the commands!
             
             //make a file descriptor
             int fds[2];
             
             //make the pipe
             int rc = pipe(fds);
             
             //fds[0] is for reading
             //fds[1] is for writing
             
             //the previous command should write to the pipe this command will read from
             commands[cmdNumber].inputFd = fds[0];
             commands[cmdNumber - 1].outputFd = fds[1];
         }
        
         // Exec wants argv to have a nullptr at the end!
         command.argv.push_back( nullptr );

         // Find the next pipe character
         first = last + 1;

         if( first < tokens.size() ){
            last = find( tokens.begin() + first, tokens.end(), "|" ) - tokens.begin();
         }
      } // end if !error
   } // end for( cmdNumber = 0 to commands.size )

   if( error ){
       cout << "hit the error clause\n";
      // Close any file descriptors you opened in this function and return an empty vector of commands so that we don't iterate over partial invalid commands
       
       for(Command brokenCommand : commands){
           if(brokenCommand.inputFd != 0 ){
               close(brokenCommand.inputFd);
           }
           if(brokenCommand.outputFd != 1){
               close(brokenCommand.outputFd);
           }
       }
       
       //return an empty commands vector?
       vector<Command> emptyVector = {};
       return emptyVector;
       
      // Note, an error can happen while parsing any command. However, the "commands" vector is
      // pre-populated with a set of "empty" commands and filled in as we go.  Because
      // of this, a "command" name can be blank (the default for a command struct that has not
      // yet been filled in).  (Note, it has not been filled in yet because the processing
      // has not gotten to it when the error (in a previous command) occurred.
       
      //assert(false);
   }

   return commands;

} // end getCommands()
