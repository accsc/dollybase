* stress_index.prg — Test INDEX ON creation + SEEK

USE books
? "Records:", RECN()

* Create index on TITULO field
INDEX ON TITULO TO idx_title
? "Index created on TITULO"

* Seek for a known title
SEEK "TEORIA Z"
? "SEEK TEORIA Z - FOUND():", FOUND()
? "RECNO after seek:", RECNO()

SEEK "LAWRENCE DE ARABIA"
? "SEEK LAWRENCE - FOUND():", FOUND()
? "RECNO after seek:", RECNO()

SEEK "EL VILLANO"
? "SEEK EL VILLANO - FOUND():", FOUND()
? "RECNO after seek:", RECNO()

* Seek for something that doesn't exist
SEEK "ZZZZZZZZZZ"
? "SEEK ZZZZZZZZZZ - FOUND():", FOUND()

* Seek for first record alphabetically
SEEK "1080 RECETAS"
? "SEEK 1080 RECETAS - FOUND():", FOUND()
? "RECNO after seek:", RECNO()

* Create index on EDITORIAL field
INDEX ON EDITORIAL TO idx_editorial
? "Index created on EDITORIAL"

SEEK "CLUB INTERNACIONAL"
? "SEEK CLUB INTERNACIONAL - FOUND():", FOUND()
? "RECNO after seek:", RECNO()

SEEK "ARGOS VERGARA"
? "SEEK ARGOS VERGARA - FOUND():", FOUND()
? "RECNO after seek:", RECNO()

* Seek for non-existent editorial
SEEK "ZETA EDITORIAL"
? "SEEK ZETA - FOUND():", FOUND()

* Cleanup
CLOSE DATABASES
? "Done"
