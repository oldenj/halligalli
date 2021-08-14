**Halligalli**
Requirements:
cmake (>=3.5)
boost libraries (>=1.65)

To build:
```
mkdir build
cd build
cmake ..
make
```
To test (after building):
`./bin/test_catch2`
For debug builds use (enables additional logging):
`cmake -DCMAKE_BUILD_TYPE=Debug ..`


To run:
`./bin/solver <LP-file> <XML-graphs folder>`
For example, solving the test LP:
`./bin/solver src/test/res/example_problem.lp src/test/res/example_problem_graphs/`
There has to be a config.json file in the runtime directory. A sample configuration can be found at the repository root.
If both the LP and graph folder paths are specified in the config, command line arguments will be ignored.
If either path is missing from the config, the command line arguments have to be specified.

To generate documentation (./build/doxygen):
```
mkdir build
cd build
cmake ..
make doc
```

Configuration:
lp: path to the problem file
graphs: path to the folder containing graph xml files for generalized flow column generation
loglevel: possible values are -2 (error), -1 (warning), 0 (info) up to 9 (max)
logfiles: any amount of additional logfiles
threadcount: how many threads the thread pool should use. a zero value means automatic assignment, usually the cpu core count
report\_results: if results should be written as csv
pricing\_strategy: if set to n > 0: only add the n shortest paths. if set to 0: add all paths
unneeded\_constraints\_substrings: specifies which constraints are unneeded for column generation. all constraints of the LP will be checked, if a constraint contains a substring of this array, it will be deleted
