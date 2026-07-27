* stress_loop.prg — loops, conditionals, and control flow

* FOR loop: sum 1..100
s = 0
FOR i = 1 TO 100
  s = s + i
ENDFOR
? "Sum 1..100 =", s

* FOR with STEP
cnt = 0
FOR i = 0 TO 20 STEP 5
  cnt = cnt + 1
ENDFOR
? "Count 0..20 step 5 =", cnt

* FOR descending
total = 0
FOR i = 10 TO 1 STEP -1
  total = total + i
ENDFOR
? "Sum 10..1 desc =", total

* DO WHILE with EXIT
n = 0
DO WHILE .T.
  n = n + 1
  IF n >= 10
    EXIT
  ENDIF
ENDDO
? "DO WHILE EXIT at n =", n

* DO WHILE with LOOP
sum_even = 0
k = 0
DO WHILE k < 20
  k = k + 1
  IF k % 2 = 0
    sum_even = sum_even + k
  ENDIF
ENDDO
? "Sum even 1..20 =", sum_even

* Nested loops
product = 0
FOR a = 1 TO 3
  FOR b = 1 TO 3
    product = product + 1
  ENDFOR
ENDFOR
? "3x3 nested count =", product

* IF/ELSE/ENDIF chains
x = 15
IF x < 10
  cat = "small"
ELSE
  IF x < 20
    cat = "medium"
  ELSE
    cat = "large"
  ENDIF
ENDIF
? "15 is", cat

x = 25
IF x < 10
  cat = "small"
ELSE
  IF x < 20
    cat = "medium"
  ELSE
    cat = "large"
  ENDIF
ENDIF
? "25 is", cat

* FOR with EXIT inside IF
s = 0
FOR i = 1 TO 100
  IF i > 10
    EXIT
  ENDIF
  s = s + i
ENDFOR
? "Sum 1..10 (EXIT) =", s

* RETURN stops execution
y = 1
RETURN
y = 2
? "This should not print"
