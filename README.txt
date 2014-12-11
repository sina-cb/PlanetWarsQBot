The files in this package are part of a starter package from the Google AI
Challenge. The Google AI Challenge is an Artificial Intelligence programming
contest. You can get more information by visiting www.ai-contest.com.

The entire contents of this starter package are released under the Apache
license as is all code related to the Google AI Challenge. See
code.google.com/p/ai-contest/ for more details.

There are a bunch of tutorials on the ai-contest.com website that tell you
what to do with the contents of this starter package. For the impatient, here
is a brief summary.
  * In the root directory, there are a bunch of code files. These are a simple
    working contest entry that employs a basic strategy. These are meant to be
    used as a starting point for you to start writing your own entry.
    Alternatively, you can just package up the starter package as-is and submit
    it on the website.
  * The tools directory contains a game engine and visualizer. This is meant
    to be used to test your bot. See the relevant tutorials on the website for
    information about how to use the tools.
  * The example_bots directory contains some sample bots for you to test your
    own bot against.

####################################################################################

You can make the project just using the make command. It will compile the project and create the QBot.bin binary file.
After compiling the project, you should run the project using the Game Engine provided by Google. To do so, you can use the 
play_against_others.sh shell scripts. Running this shell script will automatically run the game engine and two bots with in a 
specific map. You can change these parameters in the script and it is pretty straight forward.

To train the bot on a particular map, you can use the no_replay_play_against_others.sh shell script which basically runs the game
N times and waits after each run so that the engine can have some time to save the Q-values. This script also is dependent on a
map file and it learns the Q-values for that particular map.

