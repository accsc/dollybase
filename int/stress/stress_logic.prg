* stress_logic.prg — logical operators and comparisons

* AND / OR / NOT
a = .T. .AND. .F.
? ".T. AND .F. =", a

b = .T. .OR. .F.
? ".T. OR .F. =", b

c = NOT .T.
? "NOT .T. =", c

d = NOT .F.
? "NOT .F. =", d

* Complex boolean
e = (.T. .AND. .F.) .OR. (.T. .AND. .T.)
? "complex bool =", e

* Comparisons
? "5 = 5:", 5 = 5
? "5 == 5:", 5 == 5
? "5 <> 3:", 5 <> 3
? "5 != 3:", 5 != 3
? "5 > 3:", 5 > 3
? "5 < 3:", 5 < 3
? "5 >= 5:", 5 >= 5
? "5 <= 4:", 5 <= 4

* Variable comparisons
x = 10
y = 20
? "x < y:", x < y
? "x = y:", x = y
? "x > y:", x > y

* BETWEEN
? "5 BETWEEN 1,10:", BETWEEN(5, 1, 10)
? "15 BETWEEN 1,10:", BETWEEN(15, 1, 10)

* ROUND
? "ROUND(3.14159, 2) =", ROUND(3.14159, 2)
? "ROUND(3.14159, 0) =", ROUND(3.14159, 0)

* MAX / MIN
? "MAX(3,7) =", MAX(3, 7)
? "MIN(3,7) =", MIN(3, 7)
