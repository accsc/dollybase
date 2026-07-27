* stress_procedure.prg — Test PROCEDURE / DO / RETURN / PARAMETERS

* Basic procedure call
DO Hello
? "After Hello"

* Procedure with variable side effects
x = 0
DO Increment
? "x after Increment =", x

* Multiple procedure calls
DO Hello
DO Hello
? "Done with doubles"

* Nested procedure calls
DO Outer

* Parameters test
DO AddEm WITH 10, 20
? "10 + 20 =", r

* Parameters with expressions
DO Calc WITH 3 + 4, 5 * 2
? "(3+4) * (5*2) =", res

* Parameters with strings
DO Greet WITH "Alice", "Bob"
? msg

* Nested parameters
DO OuterParam WITH 5

PROCEDURE Hello
? "Hello from procedure"
RETURN

PROCEDURE Increment
x = x + 1
RETURN

PROCEDURE Outer
? "In Outer"
DO Inner
? "Back in Outer"
RETURN

PROCEDURE Inner
? "  In Inner"
RETURN

PROCEDURE AddEm
PARAMETERS a, b
r = a + b
RETURN

PROCEDURE Calc
PARAMETERS x, y
res = x * y
RETURN

PROCEDURE Greet
PARAMETERS name1, name2
msg = "Hi " + name1 + " and " + name2
RETURN

PROCEDURE OuterParam
PARAMETERS p
? "OuterParam got p =", p
DO InnerParam WITH p + 100, p * 10
? "After InnerParam, total =", total
RETURN

PROCEDURE InnerParam
PARAMETERS x, y
? "  InnerParam got x =", x, "y =", y
total = x + y
RETURN
