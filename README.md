# shell-burgess7
shell-burgess7 created by GitHub Classroom

In general, I attempted to group together similar requirements among those given in the assignment documentation.
To this end, I separated the assignment into the following tasks:

1. Determine the number of arguments run with the program, and create routines to deal with this input accordingly
2. Determine how to loop the routines described above so that the shell runs for as long as the user or file requires.
3. Ensure that the command execution routines are flexible, i.e. can take theoretically unlimited arguments, can deal with
different standard /bin/ commands.
4. Ensure that the program exits with the correct return code in each particular case
5. Hunt for bugs, memory leaks, and any other quirks that would make the shell fail

The solutions to the first and second tasks were relatively straightforward; I realized that I could simply write conditional
statements based on the provided argc variable, allowing for different routines for different initial inputs. Once I implemented
this control structure, it was just a matter of looping these routines based on their potential break points (end of file, exit
entered by user, etc.)

The third task above required some research, trial, and error to ensure that not only was enough memory allocated for a potentially
unlimited entry, but also that this memory would be reliably freed. I can't really give much of a better description because it 
mostly just involved internal screaming.

Finally, once I had a relatively solid framework for how my shell would operate, I went through each possible scenario listed
in the documentation and ensured that in each case, the proper exit code would be called by the parent process. After some more
polishing and adjustment, I began to test my shell with both command line inputs and sample text files. I am now in the process of 
testing the shell more thoroughly, and hopefully will be succesfful in doing so.
