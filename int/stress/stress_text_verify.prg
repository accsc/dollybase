* Verification test — uses IF to assert expected values
* If any assertion fails, prints FAIL:<test_name>

FAILS = 0

* --- ASC() ---
IF ASC("J") <> 74
  FAILS = FAILS + 1
ENDIF
IF ASC("j") <> 106
  FAILS = FAILS + 1
ENDIF
IF ASC("A") <> 65
  FAILS = FAILS + 1
ENDIF
IF ASC("a") <> 97
  FAILS = FAILS + 1
ENDIF
IF ASC("0") <> 48
  FAILS = FAILS + 1
ENDIF
IF ASC(" ") <> 32
  FAILS = FAILS + 1
ENDIF
IF ASC("JOOO") <> 74
  FAILS = FAILS + 1
ENDIF
W = "Hello"
IF ASC(W) <> 72
  FAILS = FAILS + 1
ENDIF

* --- CHR() ---
IF CHR(74) <> "J"
  FAILS = FAILS + 1
ENDIF
IF CHR(65) <> "A"
  FAILS = FAILS + 1
ENDIF
IF CHR(97) <> "a"
  FAILS = FAILS + 1
ENDIF
IF CHR(48) <> "0"
  FAILS = FAILS + 1
ENDIF
IF CHR(32) <> " "
  FAILS = FAILS + 1
ENDIF
C = 74
IF CHR(C) <> "J"
  FAILS = FAILS + 1
ENDIF

* --- ASC/CHR round-trip ---
IF ASC(CHR(65)) <> 65
  FAILS = FAILS + 1
ENDIF
IF CHR(ASC("A")) <> "A"
  FAILS = FAILS + 1
ENDIF

* --- AT() ---
IF AT("LAW", "LAWRENCE OF ARABIA") <> 1
  FAILS = FAILS + 1
ENDIF
IF AT("XYZ", "LAWRENCE OF ARABIA") <> 0
  FAILS = FAILS + 1
ENDIF
IF AT("law", "Lawrence") <> 1
  FAILS = FAILS + 1
ENDIF
IF AT("OO", "JOOO") <> 2
  FAILS = FAILS + 1
ENDIF
IF AT("O", "JOOO") <> 2
  FAILS = FAILS + 1
ENDIF
IF AT("", "ABC") <> 0
  FAILS = FAILS + 1
ENDIF
IF AT("ABC", "") <> 0
  FAILS = FAILS + 1
ENDIF
IF AT("", "") <> 0
  FAILS = FAILS + 1
ENDIF
T = "needle"
H = "haystack with needle"
IF AT(T, H) <> 15
  FAILS = FAILS + 1
ENDIF

* --- $ operator ---
IF .NOT. ("LAW" $ "LAWRENCE OF ARABIA")
  FAILS = FAILS + 1
ENDIF
IF "XYZ" $ "LAWRENCE OF ARABIA"
  FAILS = FAILS + 1
ENDIF
IF .NOT. ("law" $ "Lawrence")
  FAILS = FAILS + 1
ENDIF
IF .NOT. ("OO" $ "JOOO")
  FAILS = FAILS + 1
ENDIF
S1 = "LAW"
S2 = "LAWRENCE OF ARABIA"
IF .NOT. (S1 $ S2)
  FAILS = FAILS + 1
ENDIF
IF .NOT. (S1 $ "LAWRENCE")
  FAILS = FAILS + 1
ENDIF
IF .NOT. ("LAW" $ S2)
  FAILS = FAILS + 1
ENDIF

* --- ISALPHA() ---
IF .NOT. ISALPHA("J")
  FAILS = FAILS + 1
ENDIF
IF .NOT. ISALPHA("j")
  FAILS = FAILS + 1
ENDIF
IF ISALPHA("1")
  FAILS = FAILS + 1
ENDIF
IF ISALPHA("!")
  FAILS = FAILS + 1
ENDIF
IF ISALPHA(" ")
  FAILS = FAILS + 1
ENDIF
IF ISALPHA("")
  FAILS = FAILS + 1
ENDIF
V = "Hello"
IF .NOT. ISALPHA(V)
  FAILS = FAILS + 1
ENDIF
V = "123"
IF ISALPHA(V)
  FAILS = FAILS + 1
ENDIF

* --- ISLOWER() ---
IF .NOT. ISLOWER("j")
  FAILS = FAILS + 1
ENDIF
IF ISLOWER("J")
  FAILS = FAILS + 1
ENDIF
IF ISLOWER("1")
  FAILS = FAILS + 1
ENDIF
IF ISLOWER("!")
  FAILS = FAILS + 1
ENDIF
IF ISLOWER("")
  FAILS = FAILS + 1
ENDIF
V = "hello"
IF .NOT. ISLOWER(V)
  FAILS = FAILS + 1
ENDIF
V = "Hello"
IF ISLOWER(V)
  FAILS = FAILS + 1
ENDIF

* --- ISUPPER() ---
IF .NOT. ISUPPER("J")
  FAILS = FAILS + 1
ENDIF
IF ISUPPER("j")
  FAILS = FAILS + 1
ENDIF
IF ISUPPER("1")
  FAILS = FAILS + 1
ENDIF
IF ISUPPER("!")
  FAILS = FAILS + 1
ENDIF
IF ISUPPER("")
  FAILS = FAILS + 1
ENDIF
V = "HELLO"
IF .NOT. ISUPPER(V)
  FAILS = FAILS + 1
ENDIF
V = "Hello"
IF .NOT. ISUPPER(V)
  FAILS = FAILS + 1
ENDIF

* --- LEFT() ---
IF LEFT("Hello", 3) <> "Hel"
  FAILS = FAILS + 1
ENDIF
IF LEFT("Hello", 10) <> "Hello"
  FAILS = FAILS + 1
ENDIF
IF LEFT("Hello", 0) <> ""
  FAILS = FAILS + 1
ENDIF
IF LEFT("", 3) <> ""
  FAILS = FAILS + 1
ENDIF
IF LEFT("Hello", -1) <> ""
  FAILS = FAILS + 1
ENDIF
S = "Hello"
IF LEFT(S, 2) <> "He"
  FAILS = FAILS + 1
ENDIF
N = 3
IF LEFT(S, N) <> "Hel"
  FAILS = FAILS + 1
ENDIF

* --- RIGHT() ---
IF RIGHT("Hello", 3) <> "llo"
  FAILS = FAILS + 1
ENDIF
IF RIGHT("Hello", 10) <> "Hello"
  FAILS = FAILS + 1
ENDIF
IF RIGHT("Hello", 0) <> ""
  FAILS = FAILS + 1
ENDIF
IF RIGHT("", 3) <> ""
  FAILS = FAILS + 1
ENDIF
IF RIGHT("Hello", -1) <> ""
  FAILS = FAILS + 1
ENDIF
S = "Hello"
IF RIGHT(S, 2) <> "lo"
  FAILS = FAILS + 1
ENDIF
N = 3
IF RIGHT(S, N) <> "llo"
  FAILS = FAILS + 1
ENDIF

* --- LEN() ---
IF LEN("Hello") <> 5
  FAILS = FAILS + 1
ENDIF
IF LEN("") <> 0
  FAILS = FAILS + 1
ENDIF
IF LEN(" ") <> 1
  FAILS = FAILS + 1
ENDIF
S = "Hello"
IF LEN(S) <> 5
  FAILS = FAILS + 1
ENDIF

* --- LOWER() ---
IF LOWER("HELLO") <> "hello"
  FAILS = FAILS + 1
ENDIF
IF LOWER("Hello") <> "hello"
  FAILS = FAILS + 1
ENDIF
IF LOWER("hello") <> "hello"
  FAILS = FAILS + 1
ENDIF
IF LOWER("") <> ""
  FAILS = FAILS + 1
ENDIF
IF LOWER("123!") <> "123!"
  FAILS = FAILS + 1
ENDIF
S = "HELLO"
IF LOWER(S) <> "hello"
  FAILS = FAILS + 1
ENDIF

* --- UPPER() ---
IF UPPER("hello") <> "HELLO"
  FAILS = FAILS + 1
ENDIF
IF UPPER("Hello") <> "HELLO"
  FAILS = FAILS + 1
ENDIF
IF UPPER("HELLO") <> "HELLO"
  FAILS = FAILS + 1
ENDIF
IF UPPER("") <> ""
  FAILS = FAILS + 1
ENDIF
IF UPPER("123!") <> "123!"
  FAILS = FAILS + 1
ENDIF
S = "hello"
IF UPPER(S) <> "HELLO"
  FAILS = FAILS + 1
ENDIF

* --- LTRIM() ---
IF LTRIM("  hi  ") <> "hi  "
  FAILS = FAILS + 1
ENDIF
IF LTRIM("hi") <> "hi"
  FAILS = FAILS + 1
ENDIF
IF LTRIM("   ") <> ""
  FAILS = FAILS + 1
ENDIF
IF LTRIM("") <> ""
  FAILS = FAILS + 1
ENDIF
S = "  Hello  "
IF LTRIM(S) <> "Hello  "
  FAILS = FAILS + 1
ENDIF

* --- RTRIM() ---
IF RTRIM("  hi  ") <> "  hi"
  FAILS = FAILS + 1
ENDIF
IF RTRIM("hi") <> "hi"
  FAILS = FAILS + 1
ENDIF
IF RTRIM("   ") <> ""
  FAILS = FAILS + 1
ENDIF
IF RTRIM("") <> ""
  FAILS = FAILS + 1
ENDIF
S = "  Hello  "
IF RTRIM(S) <> "  Hello"
  FAILS = FAILS + 1
ENDIF

* --- STUFF() ---
IF STUFF("Hello World", 7, 5, "Friend") <> "Hello Friend"
  FAILS = FAILS + 1
ENDIF
IF STUFF("ABCDEFG", 3, 2, "XY") <> "ABXYEFG"
  FAILS = FAILS + 1
ENDIF
IF STUFF("Hello", 1, 5, "Hi") <> "Hi"
  FAILS = FAILS + 1
ENDIF
IF STUFF("Hello", 4, 0, "lo") <> "Hellolo"
  FAILS = FAILS + 1
ENDIF
IF STUFF("Hello", 1, 0, "Pre") <> "PreHello"
  FAILS = FAILS + 1
ENDIF
IF STUFF("", 1, 3, "ABC") <> "ABC"
  FAILS = FAILS + 1
ENDIF
IF STUFF("Hello", 1, 2, "") <> "llo"
  FAILS = FAILS + 1
ENDIF
S = "Hello World"
IF STUFF(S, 7, 5, "Friend") <> "Hello Friend"
  FAILS = FAILS + 1
ENDIF

* --- SUBSTR() ---
IF SUBSTR("Hello", 2, 3) <> "ell"
  FAILS = FAILS + 1
ENDIF
IF SUBSTR("Hello", 1) <> "Hello"
  FAILS = FAILS + 1
ENDIF
IF SUBSTR("Hello", 4) <> "lo"
  FAILS = FAILS + 1
ENDIF
IF SUBSTR("Hello", 10) <> ""
  FAILS = FAILS + 1
ENDIF
IF SUBSTR("Hello", 0, 3) <> "Hel"
  FAILS = FAILS + 1
ENDIF
IF SUBSTR("", 1, 3) <> ""
  FAILS = FAILS + 1
ENDIF
IF SUBSTR("Hello", 1, 100) <> "Hello"
  FAILS = FAILS + 1
ENDIF
S = "Hello"
IF SUBSTR(S, 2, 2) <> "el"
  FAILS = FAILS + 1
ENDIF

* --- REPLICATE() ---
IF REPLICATE("*", 5) <> "*****"
  FAILS = FAILS + 1
ENDIF
IF REPLICATE("A", 3) <> "AAA"
  FAILS = FAILS + 1
ENDIF
IF REPLICATE("X", 0) <> ""
  FAILS = FAILS + 1
ENDIF
IF REPLICATE(" ", 4) <> "    "
  FAILS = FAILS + 1
ENDIF
IF REPLICATE("#", -1) <> ""
  FAILS = FAILS + 1
ENDIF
C = "-"
N = 4
IF REPLICATE(C, N) <> "----"
  FAILS = FAILS + 1
ENDIF

* --- SPACE() ---
IF SPACE(5) <> "     "
  FAILS = FAILS + 1
ENDIF
IF LEN(SPACE(10)) <> 10
  FAILS = FAILS + 1
ENDIF
IF SPACE(0) <> ""
  FAILS = FAILS + 1
ENDIF
IF SPACE(-3) <> ""
  FAILS = FAILS + 1
ENDIF

* --- Combined expressions ---
IF .NOT. (ISALPHA("J") .AND. ISUPPER("J"))
  FAILS = FAILS + 1
ENDIF
IF .NOT. (ISALPHA("j") .AND. ISLOWER("j"))
  FAILS = FAILS + 1
ENDIF
IF .NOT. (.NOT. ISALPHA("1"))
  FAILS = FAILS + 1
ENDIF
IF IIF(ISLOWER("j"), 1, 0) <> 1
  FAILS = FAILS + 1
ENDIF
IF IIF(ISUPPER("J"), 1, 0) <> 1
  FAILS = FAILS + 1
ENDIF
IF ASC(CHR(ASC("A"))) <> 65
  FAILS = FAILS + 1
ENDIF
IF AT(CHR(74), "JO") <> 1
  FAILS = FAILS + 1
ENDIF
IF .NOT. (CHR(74) $ "JO")
  FAILS = FAILS + 1
ENDIF
IF LEFT(RIGHT("HelloWorld", 5), 3) <> "Wor"
  FAILS = FAILS + 1
ENDIF
IF RIGHT(LEFT("HelloWorld", 5), 3) <> "llo"
  FAILS = FAILS + 1
ENDIF

* --- Report ---
IF FAILS = 0
  ? "ALL PASSED"
ELSE
  ? "FAILURES:", FAILS
ENDIF

