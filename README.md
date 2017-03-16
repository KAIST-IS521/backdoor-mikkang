# Acitivity3: Backdoor

## Files and What I did

* **./interpreter/**: source code for Mini Language interpreter
    * I implemented the interpreter based on the given skeleton code
* **./login/login.mini**: login program written in Mini Language
    * `I forked from KAIST-IS521/backdoor-blukat29 repository`
    * I just changed minor fix like removing whitespace
    * I learned how to implement program in Mini language from his code
* **./test/test.mini**: test program written in Mini Language
    * I implemented test.mini
* **./test/test.md**: description of test.mini
* **./backdoor/**: source code for Mini Language interpreter with backdoor
    * I copied my ./interpreter/ code
    * I inserted the backdoor. It only works for my login program
* **./compiler/compiler.ml**: compiler for Mini Language
    * I just used it
* **./README.md**: simple lists what I did for each file and what I learned

## What I learned

* I learned how to implement simple interpreter and how to insert backdoor in the interpreter.
* So, if some guys create both program and running environment that I use, I might be in danger.
