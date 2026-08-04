* gen_helpers.prg - Helper procedures for generate_products.prg
* Load with: SET PROCEDURE TO gen_helpers

PROCEDURE pick_pipe
* Return the nth pipe-delimited item from global string g_pp_str
* Usage: g_pp_str = "a|b|c" then DO pick_pipe WITH 2
* Result in g_pp_result (1-based index)
* NOTE: LOCAL not implemented, so all vars are global with g_pp_ prefix
PARAMETERS g_pp_n
g_pp_saved_n = g_pp_n
g_pp_ppos = 0
g_pp_pstart = 1
g_pp_pi = 0
DO WHILE g_pp_pi < g_pp_saved_n - 1
    g_pp_ppos = AT("|", SUBSTR(g_pp_str, g_pp_pstart))
    IF g_pp_ppos = 0
        EXIT
    ENDIF
    g_pp_pstart = g_pp_pstart + g_pp_ppos
    g_pp_pi = g_pp_pi + 1
ENDDO
g_pp_ppos = AT("|", SUBSTR(g_pp_str, g_pp_pstart))
IF g_pp_ppos = 0
    g_pp_result = SUBSTR(g_pp_str, g_pp_pstart)
ELSE
    g_pp_result = SUBSTR(g_pp_str, g_pp_pstart, g_pp_ppos - 1)
ENDIF
RETURN
