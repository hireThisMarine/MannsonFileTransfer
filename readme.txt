/*! ============================================================================
  @file readme.txt

  @author Maximilian Manndorff
  @author Justin Dodson
  @par Email maximilian.manndorff@@digipen.edu
  @par Login maximilian.manndorff
  @par Email justin.dodson@@digipen.edu
  @par Login justin.dodson
  @par Course CS206
  @par Project cs260su10a03_maximilian.manndorff_justin.dodson
  @date Tuesday, 13 July 2010
============================================================================= */

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ! Team Project: Maximilian Manndorff & Justin Dodson                        !
// ! Only Maximilian Manndorff submitted on distance.digipen.edu               !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

////////////////////////////////////////////////////////////////////////////////
// Compiling: 
/*

To compile the client launch build_client.bat
To compile the server launch build_server.bat

Note that the Microsoft Command Line Compiler must be installed and set up in
order to compile using the batch files. Alternatively a new Visual Studio
Win32 Console Application solution / project can be created and compiled with
no need for special configuration.  
  
*/

////////////////////////////////////////////////////////////////////////////////
// Setup & Configuration: 
/*

In order for the client to run correctly a config.cfg file has to be present
in the same directory the client executable is located. A sample config.cfg
file has been provided with this submission package. Please note that the
configured directories

          directoryToStoreReceivedFiles
          directoryToShareFilesFrom

have to be present for the client to start. As well the client will not start if
a connection to the configured server fails. Should the configured local UDP port
not be available the client will shut down, however, should it be blocked by a 
firewall or router no transfers will be possible.

In order for the server to run no special instructions have to be followed. Simply
start the server executable and enter the port the server should be listening on.

*/

////////////////////////////////////////////////////////////////////////////////
// Usage: 
/*

If the Setup & Configuration steps were followed correctly the client should
load up without warnings and connect to the server directly. A list of available
commands will be printed to the console window.

Note that the client only scans the shared folder once during start up.

*/


////////////////////////////////////////////////////////////////////////////////
// Additional Information: 
/*

  All extra credit options attempted.

*/