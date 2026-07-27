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

* Nested DO WHILE inside FOR
? ""
? "Multiplication table 3x3:"
FOR r = 1 TO 3
  c = 1
  line = ""
  DO WHILE c <= 3
    v = r * c
    line = line + " " + ALLTRIM(v)
    c = c + 1
  ENDDO
  ? line
ENDFOR

* FOR inside DO WHILE
? ""
? "Accumulator:"
outer = 0
DO WHILE outer < 3
  inner_sum = 0
  FOR i = 1 TO 5
    inner_sum = inner_sum + i
  ENDFOR
  ? "outer=", outer, " inner_sum=", inner_sum
  outer = outer + 1
ENDDO

* EXIT from deeply nested context
? ""
? "Deep EXIT test:"
total = 0
FOR a = 1 TO 5
  FOR b = 1 TO 5
    total = total + 1
    IF total >= 7
      EXIT
    ENDIF
  ENDFOR
  IF total >= 7
    EXIT
  ENDIF
ENDFOR
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

* Nested DO WHILE inside FOR inside IF
? ""
? "Triple-nested control:"
total = 0
FOR i = 1 TO 3
  IF i > 0
    j = 1
    DO WHILE j <= 2
      total = total + i * j
      j = j + 1
    ENDDO
  ENDIF
ENDFOR
? "Triple-nested total =", total

* EXIT from nested FOR
? ""
? "EXIT from nested FOR:"
outer_count = 0
FOR a = 1 TO 10
  FOR b = 1 TO 10
    outer_count = outer_count + 1
    IF outer_count = 5
      EXIT
    ENDIF
  ENDFOR
  IF outer_count >= 5
    EXIT
  ENDIF
ENDFOR
? "Exited at count =", outer_count

* LOOP in DO WHILE
? ""
? "LOOP in DO WHILE:"
sum = 0
n = 0
DO WHILE n < 10
  n = n + 1
  IF n % 3 = 0
    LOOP
  ENDIF
  sum = sum + n
ENDDO
? "Sum skipping multiples of 3 =", sum

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
