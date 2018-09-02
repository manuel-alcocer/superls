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

struct arguments {
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

int read_options(int argc, char **argv, struct arguments *args){
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
                args->delete = 1;
                break;
            case 'e':
                args->regexp = 1;
                break;
            case 'E':
                args->regexp = 2;
                break;
            case 'f':
                args->force = 1;
                break;
            case 'l':
                args->limit = atoi(optarg);
                break;
            case 'F':
                if (optarg)
                    strcpy(args->prefix, optarg);
                else
                    strcpy(args->prefix, DEFAULT_PREFIX);
                break;
            case 'h':
                show_help();
                exit(0);
            case 'p':
                strcpy(args->pattern, optarg);
                break;
            case '?':
                show_help();
                exit(1);
        }
    }

    // Para terminar establece el directorio
    if (argv[optind] != NULL)
        strcpy(args->directory, argv[optind]);
    else
        strcpy(args->directory, getcwd(NULL, 0));

    return c;
}

char * gen_filename(const char *directory, const char *prefix, unsigned int pos){
    static char filename[NAME_MAX];
    char *retname;

    retname = filename;
    sprintf(filename, "%s/%s%u", directory, prefix, pos);

    return retname;
}

int fill_directory(struct arguments *args){
    FILE *dp;
    unsigned int limit;
    unsigned int i = 0;

    if (args->limit == 0)
        limit = UINT_MAX;
    else
        limit = args->limit;
    while ((dp = fopen(gen_filename(args->directory, args->prefix, i), "a")) != NULL && i++ < limit)
        fclose(dp);

    return 0;
}

int main(int argc, char **argv){
    struct arguments args;
    struct arguments *pargs = &args;

    /*  DEFAULT VALUES */
    pargs->delete       = 0;
    pargs->force        = 0;
    pargs->regexp       = 0;
    pargs->limit        = 0;
    pargs->pattern[0]   = '\0';
    pargs->prefix[0]    = '\0';
    /*  END DEFAULT VALUES */

    read_options(argc, argv, pargs);

    if (pargs->prefix[0] != '\0')
        fill_directory(pargs);

    return 0;
}

