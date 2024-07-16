# mikal-eval
A simple lisp evaluator for fun

## How to Build & Run
Clone the repo, then

`make run`

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

## "Deferred operations"
  1. TCO is kinda urgent, gc buffer will be quickly overflowed without TCO
  2. Error handling & report
  3. Type system is not fully closed
  4. Proper testbench and tests
  5. Comments on some confusing functions

## How to run first naive test
  After compilation, use `cat tests/fib-test | ./run` to run fibonacci test

## Supported basic operations:
  -  `+ - * /`
  -  `lambda`
  -  `cons car cdr`
  -  `define set let`
  -  `= < >`
  -  `if`
