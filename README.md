# mikal-eval
A simple lisp evaluator for fun

## How to Build & Run
Clone the repo, then

`make run`

## Plan
  - [x] Internal type system(mikal)
  - [x] Frontent parser & AST
  - [x] Basic REPL
  - [x] Environment model
  - [x] `+ - * /` (Maybe `and or not` in future?)
  - [x] Lambda and closure
  - [x] `cons/car/cdr`
  - [x] GC
  - [ ] `def/let/set`
  - [ ] quotation
  - [ ] TCO
  - [ ] `if`
  - [ ] Read from file (This should be easy :))

## "Deferred operations"
  1. Memory leaks when evaluate lambda expression
  2. Error handling & report
  3. Type system is not fully closed
  4. Proper testbench and tests
  5. Comments on some confusing functions
