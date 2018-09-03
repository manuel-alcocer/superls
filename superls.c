/* SUPERLS
 *
 * Version: 0.2
 *
 * Programa por: Manuel Alcocer Jiménez <manuel@alcocer.net>
 *
 * Este programa lista los ficheros de un directorio uno a uno,
 * en lugar de crear un listado y después mostrarlo como hace el clásico 'ls'.
 * (Útil para cuando te quedas sin inodos por millones de ficheros)
 *
 * Funcionamiento:
 * ===============
 *
 *  $ superls [opciones] [directorio]
 *
 * Argumentos
 * ==========
 *
 *  [opciones]:
 *      -p <patrón>         patrón a analizar. Si no se pone esta opción lista
 *      (--pattern)         todos los ficheros. (patrón del tipo: * ? !)
 *
 *      -E                  el patrón es una expresión regular extendida
 *      (--eregexp)         (PREDETERMINADO)
 *
 *      -e                  el patrón es una expresión regular básica
 *      (--regexp)
 *
 *      -d                  borra los ficheros pidiendo confirmación 1 a 1
 *      (--delete)          (en la confirmación están las opciones:
 *                           yes,no,all,quit)
 *
 *      -f                  no pide confirmación cuando se está borrando. OJITO!
 *      (--force)
 *
 *      -l <límite>         'límite' es un entero. En el caso de un listado, es
 *      (--limit)           el número máximo de ficheros que se mostrarán. En el
 *                          caso de usar --fill, es el número de ficheros a crear.
 *                          Si el límite es 0 o no se especifica, el límite es el
 *                          máximo entero sin signo especificado en limits.h
 *
 *      -F[prefix]          FILL. Llena el directorio pasado como argumento de ficheros
 *                          Opcionalmente se puede poner un prefijo para los
 *                          ficheros que se van a crear. La opción --limit, limita
 *                          la creación de ficheros al número establecido.
 *                          El prefijo debe ir pegado a -F
 *  [directorio]:
 *      El nombre del directorio, puede ser absoluto o relativo,
 *      si no se especifica, es './'
 *
 */

#define _GNU_SOURCE

#define MAX_REGEXP 4096

const char DEFAULT_PREFIX[] = "tmp_file_";

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

struct options {
    unsigned int limit;     /* 0: no limit */
    int regexp;             /* 0: wildcard , 1: basic regexp, 2: extended regexp */
    int delete;
    int force;
    char prefix[NAME_MAX];
    char directory[PATH_MAX - NAME_MAX];
    char pattern[MAX_REGEXP];
};

void show_help(){
    printf("Ayuda!\n");
}

int read_options(int argc, char **argv, struct options *opts){
    int c;
    int option_index = 0;

    struct option long_options[] = {
        { "pattern",    required_argument,  0,  'p' },
        { "regexp",     no_argument,        0,  'e' },
        { "eregexp",    no_argument,        0,  'E' },
        { "delete",     no_argument,        0,  'd' },
        { "force",      no_argument,        0,  'f' },
        { "limit",      required_argument,  0,  'l' },
        { "fill",       optional_argument,  0,  'F' },
        { "help",       no_argument,        0,  'h' },
        { 0, 0, 0 ,0 }
    };

    while ((c = getopt_long(argc, argv, "p:eEdfhl:F::", long_options, &option_index)) != -1){
        switch(c){
            case 'd':
                opts->delete = 1;
                break;
            case 'e':
                opts->regexp = 1;
                break;
            case 'E':
                opts->regexp = 2;
                break;
            case 'f':
                opts->force = 1;
                break;
            case 'l':
                opts->limit = atoi(optarg);
                break;
            case 'F':
                if (optarg)
                    strcpy(opts->prefix, optarg);
                else
                    strcpy(opts->prefix, DEFAULT_PREFIX);
                break;
            case 'h':
                show_help();
                exit(0);
            case 'p':
                strcpy(opts->pattern, optarg);
                break;
            case '?':
                show_help();
                exit(1);
        }
    }

    // Para terminar establece el directorio
    if (argv[optind] != NULL)
        strcpy(opts->directory, argv[optind]);
    else
        strcpy(opts->directory, getcwd(NULL, 0));

    return c;
}

char * gen_filename(const char *directory, const char *prefix, unsigned int pos){
    static char filename[NAME_MAX];
    char *retname;

    retname = filename;
    sprintf(filename, "%s/%s%u", directory, prefix, pos);

    return retname;
}

int fill_directory(struct options *opts){
    FILE *dp;
    unsigned int limit;
    unsigned int i = 0;

    if (opts->limit == 0)
        limit = UINT_MAX;
    else
        limit = opts->limit;
    while ((dp = fopen(gen_filename(opts->directory, opts->prefix, i), "a")) != NULL && i++ < limit)
        fclose(dp);

    return 0;
}

int main(int argc, char **argv){
    struct options opts;
    struct options *popts = &opts;

    /*  DEFAULT VALUES */
    popts->delete       = 0;
    popts->force        = 0;
    popts->regexp       = 0;
    popts->limit        = 0;
    popts->pattern[0]   = '\0';
    popts->prefix[0]    = '\0';
    /*  END DEFAULT VALUES */

    read_options(argc, argv, popts);

    if (popts->prefix[0] != '\0')
        fill_directory(popts);

    return 0;
}

