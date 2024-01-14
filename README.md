# YADC - Yet Another Degenerated C
[//]: # (![logo.png]&#40;doc%2Flogo.png&#41;)
<img src="./doc/logo.png" alt="logo.png" width="200"/>

## Description
This application is done as a project for the course "Formal Languages and Compilers" at the University of West Bohemia, Faculty of Applied Sciences

This project is a compiler for our own made up language called YADC

## Usage
The compiler takes one to two arguments

The first argument is the input file and is required

The second argument is the optimizations flag and is optional

The optimizations flag can be either `-o=0` or `-o=1` (default is 1 = optimizations enabled)

The compiler outputs the compiled code to the standard output and generates a file called instructions.txt

Example usage:
    
    ./yadc input.txt -o=1

### Build (Linux)

    mkdir build
    cd build
    cmake ../
    make

## Language description
The language is a simple C-like language with some limitations

Target platform is extended PL/0 instruction set

### Types
- `int`
- `bool`
- `string`
- `float`
- `void` (only for functions)

### Variables
- `int a;`
- `bool b;`
- `string c;`
- `float d;`
- `const int e = 10;`

### Functions
- `void foo() { ... }`
- `int foo(int a, int b) { ... }`

### Operators
- `+` (addition)
- `-` (subtraction)
- `*` (multiplication)
- `/` (division)
- `%` (modulo)
- `=` (assignment)
- `==` (equality)
- `!=` (inequality)
- `>` (greater than)
- `<` (less than)
- `>=` (greater than or equal to)
- `<=` (less than or equal to)
- `&&` (logical and)
- `||` (logical or)
- `!` (logical not)
- `? :` (ternary operator)

### Control flow
- `if (a == b) { ... }`
- `if (a == b) { ... } else { ... }`
- `while (a == b) { ... }`
- `do { ... } while (a == b);`
- `until (a == b) { ... }`
- `do { ... } until (a == b);`
- `for (int i = 0; i < 10; i = i + 1) { ... }`
- `break;`
- `continue;`
- `return;`
- `label: ...`
- `goto label;`

### Comments
- `/* ... */`

### Input/Output
- `int a = read_int();`
- `print_int(a);`
- `float b = read_float();`
- `print_float(b);`
- `string c = read_str();`
- `print_str(c);`

### Pointers and arrays
- `int^ ptr = @a;`
- `int^ arr = new(int, 10);`
- `^(arr + 5) = 10;`
- `int a = ^(arr + 5);`
- `delete(arr);`

### Built-in functions
- `void print_int(int a);`
- `void print_float(float a);`
- `void print_str(string a);`
- `int read_int();`
- `float read_float();`
- `string read_str();`
- `string strcat(string a, string b);`
- `bool strcmp(string a, string b);`
- `int strlen(string a);`

## Example program
```
/*
This example expects a float in the input field
and prints the circumference and area of a circle
with radius specified by the input float value
*/


              const float
           pi = 3.1415; float
        calc_circumference(float
       r){return 2.0*pi*r;} float
      calc_area(float r){return pi
      *r*r;}int main(){ /*This is v
      */float r = read_float();/**/
      float c=calc_circumference(r)
      ;float area = calc_area(r);/*
      */print_float(c) ; print_str(
       "\n");print_float(area); /*
          */return 0;}/*Its rea
              lly working */
```
