# Lit

lit is a lightweight version control system like git (but much simpler).

This project is done as one assignment for the course *703807 Advanced C++ programming* at UIBK. 
A detailed description can be found [here](https://git.uibk.ac.at/c7031162/703807-advanced-cxx/tree/master/assignment1). 


## Known Issues / TODOs / Comments

* merging with conflicts is not implemented
  * merges are assumed to be conflict free
* graph printing fails if there are two many branches
  * there are only 20 columns available for printing
* not every command implements proper error handling - the program may crash on *invalid* inputs
* format of graph printing is a little bit different as in specification;
for example, a graph could look like this:
```
	o  ┐                ⇽ r4: merge r2 into r2
	|  o                r3: add file2
	o  |                r2: add line three
	o  ┘                r1: add line two
	o                   r0: add file1
```