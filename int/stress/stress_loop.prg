* stress_loop.prg — loops, conditionals, and control flow

* DO WHILE loop: sum 1..100
s = 0
i = 1
DO WHILE i <= 100
  s = s + i
  i = i + 1
ENDDO
? "Sum 1..100 =", s

* DO WHILE with STEP equivalent
cnt = 0
i = 0
DO WHILE i <= 20
  cnt = cnt + 1
  i = i + 5
ENDDO
? "Count 0..20 step 5 =", cnt

* DO WHILE descending
total = 0
i = 10
DO WHILE i >= 1
  total = total + i
  i = i - 1
ENDDO
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
a = 1
DO WHILE a <= 3
  b = 1
  DO WHILE b <= 3
    product = product + 1
    b = b + 1
  ENDDO
  a = a + 1
ENDDO
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

* DO WHILE with EXIT inside IF
s = 0
i = 1
DO WHILE i <= 100
  IF i > 10
    EXIT
  ENDIF
  s = s + i
  i = i + 1
ENDDO
? "Sum 1..10 (EXIT) =", s

* RETURN stops execution
y = 1
RETURN
y = 2
? "This should not print"
