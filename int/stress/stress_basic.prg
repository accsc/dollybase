* stress_basic.prg — basic arithmetic and variable operations
a = 1
b = 2
c = a + b
? "a+b =", c

d = a - b
? "a-b =", d

e = a * b
? "a*b =", e

f = b / a
? "b/a =", f

g = 7 % 3
? "7%3 =", g

h = 2 ^ 10
? "2^10 =", h

i = -5
? "-5 =", i

j = ABS(-42)
? "ABS(-42) =", j

k = SQRT(144)
? "SQRT(144) =", k

m = INT(3.9)
? "INT(3.9) =", m

* Chained arithmetic
x = 1 + 2 * 3 - 4 / 2
? "1+2*3-4/2 =", x

* Parentheses
y = (1 + 2) * (3 - 4 / 2)
? "(1+2)*(3-4/2) =", y

* SET commands (parse without error)
SET TALK ON
SET TALK OFF
SET EXACT ON
SET EXACT OFF
SET SAFETY ON
SET SAFETY OFF
SET CENTURY ON
SET CENTURY OFF
SET CONSOLE ON
SET CONSOLE OFF
SET STEP ON
SET STEP OFF
SET UNIQUE ON
SET UNIQUE OFF
SET MULTILOCKS ON
SET MULTILOCKS OFF
? "SET commands - no errors"

* := assignment
a := 42
? "a := 42 -> a =", a

b := 3.14
? "b := 3.14 -> b =", b
