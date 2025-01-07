# mikal-eval
A simple scheme evaluator for fun

## How to Build & Run
Clone the repo, then

`make run`
 To build and run the evaluator

## Plan
  - [x] Internal type system(mikal)
  - [x] Frontend parser & AST
  - [x] Basic REPL
  - [x] Environment model
  - [x] `+ - * /` (Maybe `or and not` in future?)
  - [x] Lambda and closure
  - [x] `cons/car/cdr`
  - [x] GC
  - [x] `def/let/set`
  - [ ] quotation(`quote` and `eval`)
  - [ ] TCO
  - [x] `if`
  - [x] Read from file ~~(This should be easy :))~~ Done by using pipe
  - [x] `remainder`
  - [x] `assert`
  - [ ] Comments

## "Deferred operations"
  1. TCO is kinda urgent, gc buffer will be quickly overflowed without TCO
  2. Error handling & report
  3. Type system is not fully closed
  4. Proper testbench and tests
  5. Comments on some confusing functions

## How to run tests
  You can also tests by using `make tests`. 
  
  All the test answers are generated by [CHICKEN Scheme Interpreter](https://wiki.call-cc.org/man/4/Using%20the%20interpreter). You can generate your own test cases if you have that installed

  Instructins to generate new tests:
  1. Implement test program(single file) under `tests/` directory
  2. Run `python testall.py --gen-ans --ans-test` to generate ans files and run test script
  3. You can also use `-csi-test` to directly compare stdout with CSI without generate anything

  

## Supported basic operations:
  -  `+ - * / remainder`
  -  `lambda`
  -  `cons car cdr`
  -  `define set! let`
  -  `= < >`
  -  `if`
  -  `assert`
