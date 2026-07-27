* stress_date.prg — date functions
d = DATE()
? "DATE() =", d

? "DTOC(DATE()) =", DTOC(d)

dt = CTOD("2024-01-15")
? "CTOD result =", dt

? "DAY(DATE()) =", DAY(d)
? "MONTH(DATE()) =", MONTH(d)
? "YEAR(DATE()) =", YEAR(d)
