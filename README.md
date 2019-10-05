## Compiler Construction Final Product
This is a C compiler written in C using Flex and Bison.
The following files were provided by Dr. Stephen Fenner
* backend-x86.c/h
* bucket.c/h
* defs.c/h
* gram.y (Only the grammar, all rules were written by me)
* main.c
* message.c/h
* scan.l
* symtab.c/h
* type.c/h
* utls.c/h

The following files were written by me:
* eval.c/h
* gram.y (only the rules)
* myarray.c/h
* README (you're reading it!)
* tree.c/h

## DISCLAIMER
I do not endorse the use of this code to be used by students who are currently in Compiler Construction. Do the right thing and do it on your own. You'll thank yourself in the future, trust me. You also add the risk of multiple students trying to use my code and get caught by MOSS. Don't do it.
https://www.youtube.com/watch?v=hMloyp6NI4E


Added Files:
* tree.c/h - All tree data structures are defined here with their functions. Creates expression trees and declaration trees.
* myarray.c/h - Array data structures defined here. One is a dynamically resizing array for storing multiple declarations in a line. The other is a stack to define what the current loop/switch block we are in.
* eval.c/h - Used for evaluating the tree data structures in tree.c/h. Evaluates expression trees (with constant folding) and declaration trees, installing them at the end.
