/*
 * make_books_memo.c — Build books_memo.dbf + .dbt from books.dbf
 *
 * Copies all fields from books.dbf and adds a COMMENTS memo field
 * with long descriptions (some spanning multiple DBT blocks).
 *
 * Compile:
 *   gcc -w -o make_books_memo make_books_memo.c ../../libdbase_4/.libs/libdbase_0.4_s.a
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../libdbase_4/libdbase.h"

static const char *comments[] = {
    NULL,
    "Una obra maestra del teatro clasico espanol que explora los temas de la hipocresia religiosa y la corrupcion moral en la sociedad del Siglo de Oro. La obra presenta un analisis profundo de la dualidad entre la apariencia y la realidad, y como los personajes navegan entre el mundo terrenal y las aspiraciones espirituales. Un estudio esencial para cualquier estudiante de literatura espanola.",
    "Epopeya historica basada en las campanas del desierto durante la Primera Guerra Mundial. La narrativa sigue las aventuras de T.E. Lawrence y su papel clave en la revuelta arabe contra el Imperio Otomano. Una reflexion sobre el liderazgo, la identidad cultural y los conflictos entre Oriente y Occidente que sigue siendo relevante en el contexto geopolitico moderno.",
    "Comedia clasica del teatro espanol que satiriza las convenciones sociales de la epoca. El villano, lejos de ser un antagonista tradicional, se presenta como un personaje carismatico que desafias las normas establecidas. La obra es un ejemplo brillante del ingenio dramatico del Siglo de Oro y su capacidad para criticar la sociedad a traves del humor y la ironia.",
    "Tragedia clasica que examina la naturaleza de la justicia y la venganza en el contexto de la monarquia espanola. La tension entre el deber real y la moral personal crea un conflicto dramatico intenso que resuena con los temas filosoficos de la epoca. Una pieza fundamental del canon teatral espanol.",
    "Maxima filosofica sobre la jerarquia social y politica que ha sido interpretada de multiples formas a lo largo de los siglos. La frase encapsula la tension entre el poder establecido y la libertad individual, un tema que sigue siendo central en el debate politico contemporaneo sobre la autoridad y la resistencia civil.",
    "Coleccion de ensayos y articulos que abordan temas variados de literatura, politica y sociedad. Cada pieza refleja la perspectiva unica del autor sobre los desafios de su tiempo, ofreciendo una ventana valiosa al pensamiento intelectual de la epoca. Una lectura esencial para comprender el contexto cultural y politico del periodo.",
    "Comentario biblico que explora las capas de significado en el libro mas poetico de la Biblia. El analisis abarca las interpretaciones literales, alegoricas y misticas del texto, revelando como esta obra ha influido en la literatura, la musica y el arte occidental durante milenios. Un estudio profundo de la expresion del amor divino y humano.",
    "El poema heroico mas importante de la literatura medieval espanola, que narra las hazaas de el Cid Campeador. Esta obra fundacional de la literatura castellana es un testimonio unico de los valores caballerescos, el honor y la lealtad que definieron la cultura espanola durante la Reconquista. Su influencia en la identidad nacional espanola es incalculable.",
    "Coleccion de relatos cortos que representan la cumbre de la narrativa breve en lengua espanola. Cada novela es un estudio de caracter magistral que explora las vicisitudes de la vida humana con humor, compasion y una profundidad filosofica extraordinaria. Estas obras establecieron el modelo para la narrativa moderna y siguen siendo leidas y estudiadas en todo el mundo.",
    "Seleccion de fabulas que ilustran lecciones morales a traves de historias breves y ingeniosas. Las fabulas utilizan personajes animales para representar virtudes y vicios humanos, ofreciendo una critica social velada que ha entretenido y educado a generaciones de lectores. Un tesoro de la literatura didactica espanola.",
    "Antologia que recopila las obras mas representativas de la Generacion del 27, uno de los movimientos poeticos mas importantes de la literatura espanola del siglo XX. Poetas como Lorca, Cernuda, Aleixandre y Alberti revolucionaron la poesia espanola con su fusion de tradicion clasica y vanguardia moderna.",
    "Recopilacion poetica que abarca diversas etapas de la produccion literaria del autor. Los poemas exploran temas de amor, muerte, existencialismo y la busqueda de significado en un mundo caotico. La lengua poetica es rica en imagenes y simbolos que invitan a multiples interpretaciones.",
    "Coleccion poetica que demuestra la evolucion estilistica del autor a lo largo de su carrera. Desde los primeros versos influenciados por la tradicion romantica hasta las ultimas composiciones de caracter mas introspectivo y existencial, esta antologia ofrece un panorama completo de una de las voces poeticas mas importantes de la literatura espanola.",
    "Antologia poetica seleccionada que presenta los mejores trabajos del autor en diferentes periodos de su vida creativa. Los temas recurrentes incluyen la nostalgia, el paso del tiempo, la belleza efimera y la reflexion sobre la condicion humana. Cada poema es una joya de expresion lirica.",
    "Novela anonima del siglo XVI que narra las aventuras de un paje que se convierte en estafador para sobrevivir en la sociedad espanola. Considerada la primera novela picaresca, esta obra establecio un genero literario que influiria en la narrativa europea durante siglos. Su critica social mordaz y su humor negro la convierten en una lectura fascinante.",
    "Segunda parte de la trilogia picaresca que sigue las aventuras de Pablos, un estafador que viaja por Espana enganos a todos los que encuentra. La obra es una satira feroz de la sociedad espanola del siglo XVII y una exploracion profunda de la moralidad, la supervivencia y la naturaleza humana en un mundo hostil.",
    "Antologia poetica que recopila las obras mas significativas del autor, abarcando desde sus primeros experimentos poeticos hasta sus ultimas composiciones maduras. La seleccion incluye poemas de amor, reflexiones filosoficas y meditaciones sobre la naturaleza y la existencia humana.",
    "Obra historica monumental que narra la civilizacion egipcia desde sus origenes hasta la conquista romana. A traves de una investigacion exhaustiva de fuentes arqueologicas y documentales, el autor reconstruye el mundo de los faraones, sus creencias religiosas, sus logros artisticos y su influencia duradera en la cultura occidental.",
    "Analisis economico que introduce los fundamentos de la economia espanola, cubriendo temas como el crecimiento economico, el empleo, la inflacion y las politicas fiscales. El libro ofrece una perspectiva clara y accesible sobre los desafios economicos de Espana en el contexto de la Union Europea y la economia global.",
    "Estudio antropologico y historico sobre las brujas y las practicas magicas en la Europa medieval y moderna. El libro explora las creencias populares, los procesos de la Inquisicion y la construccion social del mito de la bruja. Una lectura fascinante sobre como el miedo y la supersticion moldearon la sociedad europea durante siglos.",
    "Primera parte de una enciclopedia culinaria que contiene recetas tradicionales y modernas de la cocina espanola e internacional. Cada receta incluye instrucciones detalladas, listas de ingredientes y consejos del chef. Un recurso indispensable para cualquier amante de la gastronomia.",
    "Segunda parte de la enciclopedia culinaria que continua con recetas avanzadas, postres, bebidas y platos especiales para ocasiones festivas. Incluye secciones sobre tecnicas de cocina, conservacion de alimentos y presentacion de platos. Una guia completa para el cocinero aficionado y profesional.",
    "Referencia esencial para artistas, estudiantes y entusiastas del arte que necesitan comprender la terminologia tecnica del mundo artistico. El diccionario cubre terminos de pintura, escultura, arquitectura, fotografia y artes digitales, con definiciones claras y ejemplos ilustrativos.",
    "Guia practica para interpretar estados financieros y balances de empresas. El libro explica los conceptos basicos de contabilidad, los ratios financieros mas importantes y como utilizar esta informacion para tomar decisiones de inversion informadas. Un recurso valioso para inversores, emprendedores y estudiantes de negocios.",
    "Introduccion completa a los principios fundamentales del marketing moderno, cubriendo estrategias de posicionamiento, segmentacion de mercado, branding y comunicacion comercial. El libro combina teoria academica con casos practicos de empresas reales para ilustrar los conceptos clave del marketing contemporaneo.",
    "Analisis exhaustivo de la economia espanola que diagnostica los problemas estructurales del pais y propone soluciones practicas. El libro abarca temas como la deuda publica, el desempleo juvenil, la productividad y la competitividad internacional, ofreciendo una perspectiva realista sobre los desafios economicos de Espana.",
    "Obra seminal de management que introduce el concepto de Teoria Z, un modelo de gestion que combina las mejores practicas de las empresas japonesas y americanas. El libro explora como la confianza, la implicacion de los empleados y la toma de decisiones consensuada pueden mejorar la productividad y la satisfaccion laboral en las organizaciones modernas.",
    "Memorias de uno de los publicitarios mas influyentes del siglo XX, que comparte sus experiencias, estrategias y filosofias sobre la creatividad, la persuasion y el arte de vender ideas. El libro es tanto una guia practica de publicidad como una reflexion sobre la cultura de consumo y su impacto en la sociedad.",
    "Estudio economico que analiza la situacion de Espana en el contexto europeo y global. El libro examina las fortalezas y debilidades de la economia espanola, las politicas economicas implementadas y sus resultados, asi como las perspectivas de futuro en un mundo economico cada vez mas interconectado y competitivo.",
    "Analisis de la estructura economica de Espana que examina los sectores productivos, la distribucion de la riqueza, las desigualdades regionales y los desafios de la modernizacion economica. El libro ofrece datos estadisticos detallados y analisis cualitativos que proporcionan una comprension profunda de la economia espanola.",
    "Manual sobre la gestion informatica de empresas que cubre los sistemas de informacion empresarial, la automatizacion de procesos, la gestion de bases de datos y la implementacion de tecnologias de informacion para mejorar la eficiencia operativa. Un recurso esencial para directores y gestores que buscan modernizar sus organizaciones.",
    "Tratado clasico sobre la estrategia militar y la filosofia de la guerra que ha influido en pensadores, politicos y militares durante mas de dos siglos. El libro explora la naturaleza de la guerra, la relacion entre politica y fuerza militar, y los principios estrategicos que siguen siendo relevantes en el conflicto moderno.",
    "Obra pionera que aplica los principios de la cibernetica y la teoria de sistemas a la economia, proponiendo un modelo de economia basada en el procesamiento de informacion y los sistemas de retroalimentacion. Una vision innovadora que anticipa muchos de los conceptos de la economia digital y la inteligencia artificial aplicada a los mercados.",
    "Guia practica sobre como organizar y controlar la informacion en las organizaciones modernas. El libro cubre sistemas de archivo, gestion documental, bases de datos y herramientas de productividad que permiten a las empresas manejar grandes volumenes de informacion de manera eficiente y segura.",
    "Novela de fantasia ambientada en la corte real de una nacion ficticia, donde un mago ejerce influencia politica y enfrenta conspiraciones de poder. La historia combina elementos de fantasia, politica y romance en una narrativa cautivadora que explora temas de poder, coraje y sacrificio personal.",
    "Aventura clasica de Julio Verne que sigue a un grupo de naufragos que llegan a una isla desierta y descubren sus secretos. La novela combina ciencia, aventura y suspense en una historia que ha cautivado a lectores de todas las edades durante mas de un siglo. Una obra maestra de la literatura de aventuras cientificas.",
    "Novela infantil de Roald Dahl que narra las aventuras de James Hurst, un nino que es secuestrado por una gigante y debe usar su ingenio para escapar. La historia es una celebracion del coraje infantil y la imaginacion, con un humor caracteristico de Dahl que ha hecho de esta obra un clasico de la literatura infantil mundial.",
};

int main(void)
{
    DATABASEDBF *src = calloc(1, sizeof(DATABASEDBF));
    use("books.dbf", &src);
    if (!src || src->tipo == 0) { fprintf(stderr, "Failed to open books.dbf\n"); free(src); return 1; }
    printf("Source: %d records, %d fields\n", src->recnos, src->camposn);

    DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
    db->camposn = src->camposn + 1;

    for (int i = 1; i <= src->camposn; ++i) {
        for (int c = 0; c <= 11; ++c)
            db->fields.names[c][i] = src->fields.names[c][i];
        db->fields.tipos[i] = src->fields.tipos[i];
        db->fields.longitudes[i] = src->fields.longitudes[i];
        db->fields.decimales[i] = src->fields.decimales[i];
    }

    int memo_field = src->camposn + 1;
    db->fields.names[0][memo_field] = 'C'; db->fields.names[1][memo_field] = 'O';
    db->fields.names[2][memo_field] = 'M'; db->fields.names[3][memo_field] = 'M';
    db->fields.names[4][memo_field] = 'E'; db->fields.names[5][memo_field] = 'N';
    db->fields.names[6][memo_field] = 'T'; db->fields.names[7][memo_field] = 'S';
    db->fields.tipos[memo_field] = 'M';
    db->fields.longitudes[memo_field] = 10;

    unlink("books_memo.dbf"); unlink("books_memo.dbt");

    if (create_database("books_memo.dbf", 27, 7, 26, db, 0) != 0) {
        fprintf(stderr, "create_database failed\n"); free(db); return 1;
    }
    printf("Created books_memo.dbf + .dbt\n");

    DATABASEDBF *tgt = calloc(1, sizeof(DATABASEDBF));
    use("books_memo.dbf", &tgt);
    if (!tgt || tgt->tipo == 0) { fprintf(stderr, "Failed to open books_memo.dbf\n"); free(db); free(tgt); return 1; }
    printf("Target tipo=%d\n", tgt->tipo);

    int memo_block = 0;
    for (int rec = 1; rec <= src->recnos; ++rec) {
        gotos(&src, rec);
        append_blank(&tgt);

        for (int f = 1; f <= src->camposn; ++f) {
            char *val = malloc(257);
            char *p = val;
            get_field(src, f, &p);
            char fname[16];
            for (int c = 0; c < 12; ++c) fname[c] = src->fields.names[c][f];
            fname[12] = '\0';
            replace2(tgt, fname, val);
            free(val);
        }

        int ncomments = sizeof(comments)/sizeof(comments[0]) - 1;  // exclude index 0 (NULL)
        const char *comment = comments[(rec - 1) % ncomments + 1];  // cycle through 1..ncomments
        add_to_dbt("books_memo.dbt", (char *)comment, strlen(comment));
        memo_block++;

        char block_str[16];
        snprintf(block_str, sizeof(block_str), "%d", memo_block);
        replace2(tgt, "COMMENTS", block_str);

        if (rec % 10 == 0) printf("  Copied %d/%d\n", rec, src->recnos);
    }

    printf("Copied %d records\n", src->recnos);

    /* Verify */
    printf("\n--- Verification ---\n");
    for (int rec = 1; rec <= 3; ++rec) {
        gotos(&tgt, rec);
        char *memo = malloc(4096);
        char *p = memo;
        get_field(tgt, memo_field, &p);
        printf("Rec %d: %.80s...\n", rec, memo);
        free(memo);
    }

    free(db);
    free(src);
    free(tgt);
    printf("\nDone.\n");
    return 0;
}
