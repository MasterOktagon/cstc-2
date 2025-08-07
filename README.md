
# CSTC - the Cst-Compiler

C* - or Cst - is a spriritual successor to C++ that forces memory safety and good coding style without compromising speed to much

`main.cst`:
```c++
import std::io : { print, println };

void main() {
    std::io::println("Hello World!");
}
```

```bash
$ cstc main.cst
```

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




