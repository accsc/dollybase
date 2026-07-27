* stress_fibonacci.prg — Fibonacci via DO WHILE
a = 0
b = 1
? "Fibonacci first 20:"
n = 0
DO WHILE n < 20
  ? a
  tmp = a + b
  a = b
  b = tmp
  n = n + 1
ENDDO

* Factorial
? ""
? "Factorials:"
FOR i = 0 TO 10
  fact = 1
  FOR j = 2 TO i
    fact = fact * j
  ENDFOR
  ? i, "! =", fact
ENDFOR

* Prime check up to 50
? ""
? "Primes up to 50:"
FOR n = 2 TO 50
  is_prime = .T.
  FOR d = 2 TO n - 1
    IF n % d = 0
      is_prime = .F.
      EXIT
    ENDIF
  ENDFOR
  IF is_prime
    ? n
  ENDIF
ENDFOR
