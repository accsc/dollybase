* --- INKEY() stress tests ---
* Tests 1-2 are non-interactive (auto-run)
* Tests 3-5 require user interaction

* Test 1: INKEY(0) — non-blocking, should return 0 when no key waiting
k = INKEY(0)
? "Test 1 (INKEY(0) no key): k=", k

* Test 2: INKEY(0) again to confirm non-blocking
k = INKEY(0)
? "Test 2 (INKEY(0) again): k=", k

* Test 3: INKEY() — blocks until a key is pressed
? "Test 3: Press a key (e.g. 'A' = 65)..."
k = INKEY()
? "Test 3: got key code", k

* Test 4: INKEY(0.5) — blocks up to 0.5 seconds
? "Test 4: Press a key within 0.5 seconds..."
k = INKEY(0.5)
? "Test 4: got key code", k

* Test 5: INKEY(0.1) in a loop — press keys, Enter (10) to stop
? "Test 5: Press keys, Enter to stop..."
DO WHILE .T.
    k = INKEY(0.1)
    IF k > 0
        ? "Key:", k
        IF k = 10
            EXIT
        ENDIF
    ENDIF
ENDDO

? "All INKEY tests complete."
