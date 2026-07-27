* stress_string.prg — string functions
s1 = "hello"
s2 = "world"

? "LEN(s1) =", LEN(s1)
? "UPPER(s1) =", UPPER(s1)
? "LOWER(s2) =", LOWER(s2)
? "ALLTRIM('  hi  ') =", ALLTRIM("  hi  ")

? "SUBSTR('abcdef',2,3) =", SUBSTR("abcdef", 2, 3)
? "LEFT('abcdef', 2) =", LEFT("abcdef", 2)
? "RIGHT('abcdef', 3) =", RIGHT("abcdef", 3)

? "AT('l','hello') =", AT("l", "hello")

* String concatenation
greeting = "hello" + " " + "world"
? "concat =", greeting

* IIF
result = IIF(1 > 0, "yes", "no")
? "IIF(1>0) =", result

result2 = IIF(1 < 0, "yes", "no")
? "IIF(1<0) =", result2

* TYPE
? "TYPE('hello') =", TYPE("hello")
? "TYPE(42) =", TYPE(42)
? "TYPE(.T.) =", TYPE(.T.)

* EMPTY
? "EMPTY('') =", EMPTY("")
? "EMPTY('x') =", EMPTY("x")
? "EMPTY(0) =", EMPTY(0)

* VAL
? "VAL('123') =", VAL("123")
? "VAL('45.67') =", VAL("45.67")
? "VAL('abc') =", VAL("abc")

* INT
? "INT(3.9) =", INT(3.9)
? "INT(-2.5) =", INT(-2.5)

* ROUND
? "ROUND(3.14159, 2) =", ROUND(3.14159, 2)
? "ROUND(3.14159, 0) =", ROUND(3.14159, 0)

* ABS
? "ABS(-42) =", ABS(-42)
? "ABS(42) =", ABS(42)

* SQRT
? "SQRT(144) =", SQRT(144)
? "SQRT(2) =", SQRT(2)

* MAX / MIN
? "MAX(3, 7) =", MAX(3, 7)
? "MIN(3, 7) =", MIN(3, 7)

* BETWEEN
? "BETWEEN(5, 1, 10) =", BETWEEN(5, 1, 10)
? "BETWEEN(15, 1, 10) =", BETWEEN(15, 1, 10)

* AT
? "AT('l', 'hello') =", AT("l", "hello")
? "AT('z', 'hello') =", AT("z", "hello")

* IIF
? "IIF(.T., 'yes', 'no') =", IIF(.T., "yes", "no")
? "IIF(.F., 'yes', 'no') =", IIF(.F., "yes", "no")

* TYPE
? "TYPE('hello') =", TYPE("hello")
? "TYPE(42) =", TYPE(42)
? "TYPE(.T.) =", TYPE(.T.)

* DATE functions
? "DATE() =", DATE()
? "DTOC(DATE()) =", DTOC(DATE())
? "DAY(DATE()) =", DAY(DATE())
? "MONTH(DATE()) =", MONTH(DATE())
? "YEAR(DATE()) =", YEAR(DATE())

* CTOD
dt = CTOD("2024-01-15")
? "CTOD result =", dt

* TRIM / LTRIM / RTRIM
? "TRIM('  hi  ') =", TRIM("  hi  ")
? "LTRIM('  hi') =", LTRIM("  hi")
? "RTRIM('hi  ') =", RTRIM("hi  ")

* SPACE
? "LEN(SPACE(10)) =", LEN(SPACE(10))

* CHR / ASC
? "CHR(65) =", CHR(65)
? "CHR(97) =", CHR(97)
? "ASC('A') =", ASC("A")
? "ASC('a') =", ASC("a")

* SIGN
? "SIGN(-5) =", SIGN(-5)
? "SIGN(5) =", SIGN(5)
? "SIGN(0) =", SIGN(0)
