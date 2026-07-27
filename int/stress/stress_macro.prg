* stress_macro.prg — Test macro expansion with &

* Basic macro expansion
i = 2
myMacro = "i + 10"
i = &myMacro
? "Macro basic: i =", i

* Macro with string literal
? "Macro literal:", &"3 * 4"

* Macro with variable reference
x = 100
y = 200
expr = "x + y"
? "Macro vars:", &expr

* Macro with function call (avoid nested quotes)
w = "hello"
funcCall = "LEN(w)"
? "Macro function:", &funcCall

* Macro with nested expression
a = 5
b = 3
nested = "(a + b) * 2"
? "Macro nested:", &nested

* Macro with logical
logExpr = ".T. AND .T."
? "Macro logical:", &logExpr

* Empty macro (should return null/0)
emptyMacro = ""
? "Macro empty:", &emptyMacro

* Macro referencing undefined variable (should return 0)
undefMacro = "nonexistent"
? "Macro undefined:", &undefMacro

? "Done"
