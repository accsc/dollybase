* stress_nested.prg — deeply nested structures

* 3-level nested IF
x = 7
IF x > 0
  IF x < 10
    IF x % 2 = 0
      ? x, "is even single digit"
    ELSE
      ? x, "is odd single digit"
    ENDIF
  ELSE
    IF x < 100
      ? x, "is two digits"
    ELSE
      ? x, "is large"
    ENDIF
  ENDIF
ELSE
  ? x, "is negative"
ENDIF

* Nested DO WHILE inside DO WHILE
? ""
? "Multiplication table 3x3:"
r = 1
DO WHILE r <= 3
  c = 1
  line = ""
  DO WHILE c <= 3
    v = r * c
    line = line + " " + ALLTRIM(STR(v))
    c = c + 1
  ENDDO
  ? line
  r = r + 1
ENDDO

* DO WHILE inside DO WHILE
? ""
? "Accumulator:"
outer = 0
DO WHILE outer < 3
  inner_sum = 0
  i = 1
  DO WHILE i <= 5
    inner_sum = inner_sum + i
    i = i + 1
  ENDDO
  ? "outer=", outer, " inner_sum=", inner_sum
  outer = outer + 1
ENDDO

* EXIT from deeply nested context
? ""
? "Deep EXIT test:"
total = 0
a = 1
DO WHILE a <= 5
  b = 1
  DO WHILE b <= 5
    total = total + 1
    IF total >= 7
      EXIT
    ENDIF
    b = b + 1
  ENDDO
  IF total >= 7
    EXIT
  ENDIF
  a = a + 1
ENDDO
? "Stopped at total =", total

* 4-level nested IF
? ""
? "4-level nested IF:"
x = 15
IF x > 0
  IF x < 100
    IF x < 50
      IF x < 20
        ? x, "is < 20"
      ELSE
        ? x, "is 20-49"
      ENDIF
    ELSE
      ? x, "is 50-99"
    ENDIF
  ELSE
    ? x, "is >= 100"
  ENDIF
ELSE
  ? x, "is negative"
ENDIF

* Nested DO WHILE inside IF inside DO WHILE
? ""
? "Triple-nested control:"
total = 0
i = 1
DO WHILE i <= 3
  IF i > 0
    j = 1
    DO WHILE j <= 2
      total = total + i * j
      j = j + 1
    ENDDO
  ENDIF
  i = i + 1
ENDDO
? "Triple-nested total =", total

* EXIT from nested DO WHILE
? ""
? "EXIT from nested DO WHILE:"
outer_count = 0
a = 1
DO WHILE a <= 10
  b = 1
  DO WHILE b <= 10
    outer_count = outer_count + 1
    IF outer_count = 5
      EXIT
    ENDIF
    b = b + 1
  ENDDO
  IF outer_count >= 5
    EXIT
  ENDIF
  a = a + 1
ENDDO
? "Exited at count =", outer_count

* LOOP in DO WHILE
? ""
? "LOOP in DO WHILE:"
tsum = 0
n = 0
DO WHILE n < 10
  n = n + 1
  IF n % 3 = 0
    LOOP
  ENDIF
  tsum = tsum + n
ENDDO
? "Sum skipping multiples of 3 =", tsum

* Multiple IF/ELSE chains
? ""
? "IF/ELSE chain:"
x = 42
IF x < 10
  cat = "tiny"
ELSE
  IF x < 20
    cat = "small"
  ELSE
    IF x < 50
      cat = "medium"
    ELSE
      IF x < 100
        cat = "large"
      ELSE
        cat = "huge"
      ENDIF
    ENDIF
  ENDIF
ENDIF
? "42 is", cat
