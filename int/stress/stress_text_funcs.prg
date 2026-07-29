* ============================================================
* stress_text_funcs.prg — Stress test for text functions
*   ASC, CHR, AT, ISALPHA, ISLOWER, ISUPPER, $ operator
* ============================================================

* --- ASC() tests ---
? "--- ASC() tests ---"
? "ASC(J):", ASC("J")
? "ASC(j):", ASC("j")
? "ASC(A):", ASC("A")
? "ASC(a):", ASC("a")
? "ASC(0):", ASC("0")
? "ASC(space):", ASC(" ")
? "ASC(JOOO):", ASC("JOOO")
W = "Hello"
? "ASC(Hello):", ASC(W)

* --- CHR() tests ---
? "--- CHR() tests ---"
? "CHR(74):", CHR(74)
? "CHR(65):", CHR(65)
? "CHR(97):", CHR(97)
? "CHR(48):", CHR(48)
? "CHR(32):", CHR(32)
C = 74
? "CHR(var 74):", CHR(C)

* --- ASC/CHR round-trip ---
? "--- ASC/CHR round-trip ---"
? "ASC(CHR(65)):", ASC(CHR(65))
? "CHR(ASC(A)):", CHR(ASC("A"))

* --- AT() tests ---
? "--- AT() tests ---"
? "AT(LAW in LAWRENCE):", AT("LAW", "LAWRENCE OF ARABIA")
? "AT(XYZ in LAWRENCE):", AT("XYZ", "LAWRENCE OF ARABIA")
? "AT(law in Lawrence):", AT("law", "Lawrence")
? "AT(OO in JOOO):", AT("OO", "JOOO")
? "AT(O in JOOO):", AT("O", "JOOO")
? "AT(empty in ABC):", AT("", "ABC")
? "AT(ABC in empty):", AT("ABC", "")
? "AT(empty in empty):", AT("", "")
T = "needle"
H = "haystack with needle"
? "AT(var vars):", AT(T, H)

* --- $ operator tests ---
? "--- $ operator tests ---"
? "'LAW'$'LAWRENCE':", "LAW" $ "LAWRENCE OF ARABIA"
? "'XYZ'$'LAWRENCE':", "XYZ" $ "LAWRENCE OF ARABIA"
? "'law'$'Lawrence':", "law" $ "Lawrence"
? "'OO'$'JOOO':", "OO" $ "JOOO"
S1 = "LAW"
S2 = "LAWRENCE OF ARABIA"
? "var$var:", S1 $ S2
? "var$literal:", S1 $ "LAWRENCE"
? "literal$var:", "LAW" $ S2

* --- ISALPHA() tests ---
? "--- ISALPHA() tests ---"
? "ISALPHA(J):", ISALPHA("J")
? "ISALPHA(j):", ISALPHA("j")
? "ISALPHA(1):", ISALPHA("1")
? "ISALPHA(!):", ISALPHA("!")
? "ISALPHA(space):", ISALPHA(" ")
? "ISALPHA(empty):", ISALPHA("")
V = "Hello"
? "ISALPHA(Hello):", ISALPHA(V)
V = "123"
? "ISALPHA(123):", ISALPHA(V)

* --- ISLOWER() tests ---
? "--- ISLOWER() tests ---"
? "ISLOWER(j):", ISLOWER("j")
? "ISLOWER(J):", ISLOWER("J")
? "ISLOWER(1):", ISLOWER("1")
? "ISLOWER(!):", ISLOWER("!")
? "ISLOWER(empty):", ISLOWER("")
V = "hello"
? "ISLOWER(hello):", ISLOWER(V)
V = "Hello"
? "ISLOWER(Hello):", ISLOWER(V)

* --- ISUPPER() tests ---
? "--- ISUPPER() tests ---"
? "ISUPPER(J):", ISUPPER("J")
? "ISUPPER(j):", ISUPPER("j")
? "ISUPPER(1):", ISUPPER("1")
? "ISUPPER(!):", ISUPPER("!")
? "ISUPPER(empty):", ISUPPER("")
V = "HELLO"
? "ISUPPER(HELLO):", ISUPPER(V)
V = "Hello"
? "ISUPPER(Hello):", ISUPPER(V)

* --- LEFT() tests ---
? "--- LEFT() tests ---"
? "LEFT(Hello,3):", LEFT("Hello", 3)
? "LEFT(Hello,10):", LEFT("Hello", 10)
? "LEFT(Hello,0):", LEFT("Hello", 0)
? "LEFT(empty,3):", LEFT("", 3)
S = "Hello"
? "LEFT(var,2):", LEFT(S, 2)
N = 3
? "LEFT(var,var):", LEFT(S, N)

* --- RIGHT() tests ---
? "--- RIGHT() tests ---"
? "RIGHT(Hello,3):", RIGHT("Hello", 3)
? "RIGHT(Hello,10):", RIGHT("Hello", 10)
? "RIGHT(Hello,0):", RIGHT("Hello", 0)
? "RIGHT(empty,3):", RIGHT("", 3)
S = "Hello"
? "RIGHT(var,2):", RIGHT(S, 2)
N = 3
? "RIGHT(var,var):", RIGHT(S, N)

* --- LEN() tests ---
? "--- LEN() tests ---"
? "LEN(Hello):", LEN("Hello")
? "LEN(empty):", LEN("")
? "LEN(space):", LEN(" ")
S = "Hello"
? "LEN(var):", LEN(S)

* --- LOWER() tests ---
? "--- LOWER() tests ---"
? "LOWER(HELLO):", LOWER("HELLO")
? "LOWER(Hello):", LOWER("Hello")
? "LOWER(empty):", LOWER("")
S = "HELLO"
? "LOWER(var):", LOWER(S)

* --- UPPER() tests ---
? "--- UPPER() tests ---"
? "UPPER(hello):", UPPER("hello")
? "UPPER(Hello):", UPPER("Hello")
? "UPPER(empty):", UPPER("")
S = "hello"
? "UPPER(var):", UPPER(S)

* --- LTRIM() tests ---
? "--- LTRIM() tests ---"
? "LTRIM(  hi  ):", LTRIM("  hi  ")
? "LTRIM(empty):", LTRIM("")
S = "  Hello  "
? "LTRIM(var):", LTRIM(S)

* --- RTRIM() tests ---
? "--- RTRIM() tests ---"
? "RTRIM(  hi  ):", RTRIM("  hi  ")
? "RTRIM(empty):", RTRIM("")
S = "  Hello  "
? "RTRIM(var):", RTRIM(S)

* --- STUFF() tests ---
? "--- STUFF() tests ---"
? "STUFF(Hello World,7,5,Friend):", STUFF("Hello World", 7, 5, "Friend")
? "STUFF(ABCDEFG,3,2,XY):", STUFF("ABCDEFG", 3, 2, "XY")
? "STUFF(Hello,1,5,Hi):", STUFF("Hello", 1, 5, "Hi")
? "STUFF(Hello,4,0,lo):", STUFF("Hello", 4, 0, "lo")
S = "Hello World"
? "STUFF(var,7,5,Friend):", STUFF(S, 7, 5, "Friend")

* --- SUBSTR() tests ---
? "--- SUBSTR() tests ---"
? "SUBSTR(Hello,2,3):", SUBSTR("Hello", 2, 3)
? "SUBSTR(Hello,1):", SUBSTR("Hello", 1)
? "SUBSTR(Hello,4):", SUBSTR("Hello", 4)
? "SUBSTR(Hello,10):", SUBSTR("Hello", 10)
S = "Hello"
? "SUBSTR(var,2,2):", SUBSTR(S, 2, 2)

* --- REPLICATE() tests ---
? "--- REPLICATE() tests ---"
? "REPLICATE(*,5):", REPLICATE("*", 5)
? "REPLICATE(A,3):", REPLICATE("A", 3)
? "REPLICATE(X,0):", REPLICATE("X", 0)
C = "-"
N = 4
? "REPLICATE(var,var):", REPLICATE(C, N)

* --- SPACE() tests ---
? "--- SPACE() tests ---"
? "SPACE(5):", SPACE(5)
? "SPACE(0):", SPACE(0)
? "LEN(SPACE(10)):", LEN(SPACE(10))

* --- Combined expression tests ---
? "--- Combined expressions ---"
? "ISALPHA AND ISUPPER:", ISALPHA("J") .AND. ISUPPER("J")
? "ISALPHA AND ISLOWER:", ISALPHA("j") .AND. ISLOWER("j")
? "NOT ISALPHA(1):", .NOT. ISALPHA("1")
? "IIF(ISLOWER(j),low,up):", IIF(ISLOWER("j"), "low", "up")
? "IIF(ISUPPER(J),low,up):", IIF(ISUPPER("J"), "up", "low")
? "ASC(CHR(ASC(A))):", ASC(CHR(ASC("A")))
? "AT(CHR(74),JO):", AT(CHR(74), "JO")
? "CHR(74)$JO:", CHR(74) $ "JO"
? "LEFT(RIGHT(HelloWorld,5),3):", LEFT(RIGHT("HelloWorld", 5), 3)
? "RIGHT(LEFT(HelloWorld,5),3):", RIGHT(LEFT("HelloWorld", 5), 3)

QUIT
