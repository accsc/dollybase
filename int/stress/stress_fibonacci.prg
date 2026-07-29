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
i = 0
DO WHILE i <= 10
  fact = 1
  j = 2
  DO WHILE j <= i
    fact = fact * j
    j = j + 1
  ENDDO
  ? i, "! =", fact
  i = i + 1
ENDDO

* Prime check up to 50
? ""
? "Primes up to 50:"
n = 2
DO WHILE n <= 50
  is_prime = .T.
  d = 2
  DO WHILE d <= n - 1
    IF n % d = 0
      is_prime = .F.
      EXIT
    ENDIF
    d = d + 1
  ENDDO
  IF is_prime
    ? n
  ENDIF
  n = n + 1
ENDDO
