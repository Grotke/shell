# shell
Unix command line shell.

![Screenshot of my shell](http://www.josephcmontgomery.com/uploads/4/5/8/3/45834621/4158283_orig.png)
## Description and Background
Shell is an UNIX shell interface that accepts user commands and then executes each command in a separate process.

Shell was created to solve a programming project in the Operating Systems Concepts book.

It has a lot of globals, which is something I wouldn't do now. But it's split into different functions fairly well.

## Building and Use Instructions
`make` should build it. `./shell` should execute it. 

`history` will show the last 10 commands executed. The number of commands remembered can be changed in the code.

`!!` will execute the previous command if it exists. 

`!n` will execute command n, e.g. `!10` will execute the 10th command if it exists and is within the history window.

When you're done, type `exit` to leave the shell. 
