
# CSTC - the Cst-Compiler

## Installation

- have a linux distro (I am relatively sure that windows should not work)
- install cmake, make, clang++ and clang if you have not already
- clone this repo
- build the submodule segvcatch in the lib folder (there should be build script inside the segvcatch folder)
- create an enviroment variable called `CSTC_STD` pointing to a path where your stdlib files are going (usually `~/opt/cstc/stdlib` is a good location)
- reload your environment variables
- create a file at this location: `lang.cst`. This is your autoload module.
- in your repo, run `cmake .` then `make`
- should be able to run the executable `./bin/cstc`. try using `--help` 




