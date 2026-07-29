* stress_database.prg — Test database operations

USE books
? "DBF name:", ALIAS()
? "Records:", RECN()
? "First record:", RECNO()

DISPLAY STRUCTURE

t=TIME()
t2=DATE()

TEXT

    Test of text 

Now is &t &t2


ENDTEXT

WAIT "Press any key..." 

