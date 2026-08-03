* test_save_simple.prg — Simple SAVE/RESTORE test
x = 42
y = "hello"
z = .T.

SAVE TO test_simple

? "Before restore: x="
? x
? "y="
? y
? "z="
? z

* Now clear and restore
CLEAR MEMORY

? "After clear: TYPE(x)="
? TYPE("x")

RESTORE FROM test_simple

? "After restore: x="
? x
? "y="
? y
? "z="
? z

? "Done"
