* ========================================================================
* PONG.PRG — Classic Pong game in dollybase
* ========================================================================
* Controls:
*   Left paddle:  w / s
*   Right paddle: i / k
*   Q: Quit
* ========================================================================

* --- Game constants ---
STORE 24    TO gRows
STORE 79    TO gCols
STORE 3     TO pLeftRow
STORE 3     TO pRightRow
STORE 5     TO pHeight
STORE 2     TO pLeftCol
STORE 77    TO pRightCol
STORE 0     TO scoreLeft
STORE 0     TO scoreRight
STORE 12    TO ballRow
STORE 40    TO ballCol
STORE 1     TO ballDirR
STORE 1     TO ballDirC
STORE 0     TO running
STORE 0     TO key
STORE 0     TO i
STORE 0     TO tick
STORE 1     TO ballDelay

* --- Initialize ncurses and clear screen ---
SET CURSOR OFF
CLEAR

* --- Draw static borders ---
@ 0,0 SAY REPLICATE("=", gCols)
@ gRows-1,0 SAY REPLICATE("=", gCols)

* --- Title ---
@ 0,30 SAY "PONG - DOLLYBASE  (w/s=i/k move, Q quit)"

* --- Scores (initial) ---
@ 1,30 SAY "SCORE:  0   |   0"

* --- Net (dashed center line) ---
STORE 0 TO i
DO WHILE i < gRows
    @ i, 39 SAY "|"
    i = i + 2
ENDDO

* --- Draw paddles (initial) ---
STORE 0 TO i
DO WHILE i < pHeight
    @ pLeftRow + i, pLeftCol SAY "|"
    @ pRightRow + i, pRightCol SAY "|"
    i = i + 1
ENDDO

* --- Ball (initial) ---
@ ballRow, ballCol SAY "o"

* ========================================================================
* Main game loop — input every tick, ball moves every ballDelay ticks
* ========================================================================
DO WHILE running = 0

    * --- Input with delay ---
    key = INKEY(0.001)

    IF key > 0
        IF key = 113 .OR. key = 81
            running = 1
        ENDIF

        * --- Left paddle: w(119) / s(115) ---
        IF key = 119 .AND. pLeftRow > 1
            @ pLeftRow + pHeight - 1, pLeftCol SAY " "
            pLeftRow = pLeftRow - 1
            @ pLeftRow, pLeftCol SAY "|"
        ENDIF
        IF key = 115 .AND. pLeftRow + pHeight < gRows - 1
            @ pLeftRow, pLeftCol SAY " "
            pLeftRow = pLeftRow + 1
            @ pLeftRow + pHeight - 1, pLeftCol SAY "|"
        ENDIF

        * --- Right paddle: i(105) / k(107) ---
        IF key = 105 .AND. pRightRow > 1
            @ pRightRow + pHeight - 1, pRightCol SAY " "
            pRightRow = pRightRow - 1
            @ pRightRow, pRightCol SAY "|"
        ENDIF
        IF key = 107 .AND. pRightRow + pHeight < gRows - 1
            @ pRightRow, pRightCol SAY " "
            pRightRow = pRightRow + 1
            @ pRightRow + pHeight - 1, pRightCol SAY "|"
        ENDIF
    ENDIF

    * --- Advance tick counter ---
    tick = tick + 1

    * --- Move ball only every ballDelay ticks ---
    IF tick = ballDelay
        tick = 0

        * --- Erase old ball position ---
        @ ballRow, ballCol SAY " "

        * --- Move ball ---
        ballRow = ballRow + ballDirR
        ballCol = ballCol + ballDirC

        * --- Bounce top/bottom walls ---
        IF ballRow <= 1
            ballRow = 2
            ballDirR = 1
        ENDIF
        IF ballRow >= gRows - 2
            ballRow = gRows - 3
            ballDirR = -1
        ENDIF

        * --- Paddle collision: Left ---
        IF ballCol <= pLeftCol + 1
            IF ballRow >= pLeftRow .AND. ballRow <= pLeftRow + pHeight - 1
                ballDirC = 1
                ballCol = pLeftCol + 2
            ENDIF
        ENDIF

        * --- Paddle collision: Right ---
        IF ballCol >= pRightCol - 1
            IF ballRow >= pRightRow .AND. ballRow <= pRightRow + pHeight - 1
                ballDirC = -1
                ballCol = pRightCol - 2
            ENDIF
        ENDIF

        * --- Score detection ---
        IF ballCol < 0
            ballCol = 0
            scoreRight = scoreRight + 1
            @ 1,30 SAY "SCORE:  " + LTRIM(STR(scoreLeft)) + "   |   " + LTRIM(STR(scoreRight))
            ballRow = INT(gRows / 2)
            ballCol = INT(gCols / 2)
            ballDirR = 1
            ballDirC = 1
        ENDIF

        IF ballCol > gCols
            ballCol = gCols
            scoreLeft = scoreLeft + 1
            @ 1,30 SAY "SCORE:  " + LTRIM(STR(scoreLeft)) + "   |   " + LTRIM(STR(scoreRight))
            ballRow = INT(gRows / 2)
            ballCol = INT(gCols / 2)
            ballDirR = -1
            ballDirC = -1
        ENDIF

        * --- Draw new ball ---
        @ ballRow, ballCol SAY "o"
    ENDIF

ENDDO

* ========================================================================
* Cleanup
* ========================================================================
SET CURSOR ON
CLEAR
? "Game Over!"
? "Left: " + LTRIM(STR(scoreLeft)) + "  |  Right: " + LTRIM(STR(scoreRight))
? "Press any key to exit..."
INKEY()
