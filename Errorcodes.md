

# Errorcodes

## Errorcodes of cstc

### 1 [No File Specified]

A valid main file could not be parsed. This could be due to a missing file parameter or a multiple of them since `cstc` takes exactly one of them

### 2 [Compiler Error Occured]

A compiler error occured. See the stderr output for details

### 3 [File not found]

A file (ex. main File) was not found. See the stderr output for details

### 4 [Multiple main Files]

This could be due to a missing file parameter or a multiple of them since `cstc` takes exactly one of them

### 5 [File could not be opened]


## Compiler-Errorcodes

| Number | Name | Meaning |
|:-------|:-----|:--------|
|E2      | Module not found    | A module of this name wasn't found |
|E17     | Type mismatch       | The found type does not match the expected type |
|E18     | Unknown operator    | The desired Operator is not implemented for given types |
|E19     | Unknown Type        | A Type of this name was not found. Make sure you have imported the module and you are using the right namespace|
|E20     | Unknown Variable    | A variable of this name was not found in this scope |
|E21     | Unknown Funtcion    | A Funtcion of this name was not found in this scope |
|E25     | Variable already defined | A variable of this name is already defined in this scope |
|E26     | Unsupported name    | A Variable was named after a Type/scope. This is not supported |
|E30     | Expected Symbol     | The compiler expected a variable or namespace |
|E31     | Expected Expression | The compiler expected a valid epression |
|E45     | Sign mismatch       | The found type does imply a sign that does not fit into the expected type |
|E50     | Identifier expected | An identifier token was expected |
|E51     | Block open expected | A '{' was expected |
|E52     | Block close expected | A '}' was expected |
|E60     | Namespace not allowed | A Block of type Namespace was not allowed here |
|E75     | Unexpected import   | An 'import' token was detected inside a statement |
|E76     | Unexpected Token     | An unexpected Token was encoutered |


## Compiler-Warncodes

| Number | Name | Meaning |
|:-------|:-----|:--------|
|W2      | Missing Doccomment  | All functions, namespaces and Types require Doccomments. If they aren't provided, this Warning triggers |
|W3      | Unused Import       | A module was imported but never used |
|W4      | Duplicated Import   | A module was already imported |
|W5      | Unused variable     | A parameter or variable was declared but not used. This could also be due to shadowing of this variable later on. If this was intended, prefix the variable/parameter with `_` |
|W6      | Unjoined Thread     | A Thread was never joined. If this was intended, prefix the thread variable with `_` |
|W10     | snake_case expected | parameters and variables should be snake_case. This warning triggers if they aren't |
|W11     | CamelCase expected  | types and enums should be CamelCase. This warning triggers if they aren't |
|W12     | pascalCase expected | functions and methods should be pascalCase. This warning triggers if they aren't |
|W13     | Unreachable code    | Code is not reachable. This could be because of a return that always triggers before |
|W14     | Line too long       | Triggers if the line length is bigger than 120 |
|W15     | `std::threads` not imported | Triggers if you use the `locked` keyword without `std::threads` imported |
|W16     | Wrong casing        | The compiler expects a different casing on this symbol |
|E17     | Integer to big      | Trying to fit an integer too big into a variable with too few bits. this will lead to information loss. |
