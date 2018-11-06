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
 *                          NOTA: Admite wildcards extendidos de KSH
 *
 *      -E                  el patrón es una expresión regular extendida
 *      (--eregexp)         (PREDETERMINADO)
 *
 *      -e                  el patrón es una expresión regular básica
 *      (--regexp)
 *
 *      -d                  borra los ficheros pidiendo confirmación 1 a 1
 *      (--delete)          (en la confirmación están las opciones:
 *                          yes,no,all,quit)
 *                          Adicionalmente se puede anteponer un entero a la acción
 *                          a realizar.
 *
 *      -f                  no pide confirmación cuando se está borrando. ¡OJITO!
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
 *      si no se especifica es CWD pero en ruta absoluta, i.e.: /home/foo/test_superls
 *
 *  Ejemplos de uso:
 *  ================
 *
 *  1. Creación de ficheros
 *      a) 1522 ficheros con prefijo personalizado:
 *
 *          $ ./superls -Fmis_archivos_ -l 1522
 *
 *      b) Crear ficheros hasta que 'pete' el sistema:
 *
 *          $ ./superls -F
 *
 *  2. Listados de ficheros:
 *      a) Usando wildcards:
 *
 *          $ ./superls -p 'a*'
 *
 *      b) Listado de máximo los 1000 primeros usando wildcards KSH:
 *
 *          $ ./superls -p '+(a*|b*)' -l1000
 *
 *      c) Usando Regexp:
 *
 *          $ ./superls -p '^a\[1\].*' -e
 *
 *      d) Usando Regexp Extendidas:
 *
 *          $ ./superls -p '^a[1][^0].*' -E
 *
 */

#define _GNU_SOURCE

#define MAX_REGEXP 4096

#define DEFAULT_PREFIX "tmp_file_"

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <regex.h>

enum RegexTypes { WILDC, BREG, EREG };   /* 0: wildcard , 1: basic regexp, 2: extended regexp */

typedef struct __options {
    unsigned int limit;                 /* 0: no limit */
    enum RegexTypes regexp;
    int delete;
    int force;
    char prefix[NAME_MAX];
    char directory[PATH_MAX - NAME_MAX + 1];
    char pattern[MAX_REGEXP];
    regex_t regcomp;
} options;
typedef struct __options *OPTIONS;

void show_help(){
    printf("Ayuda!\n");
}

int show_compiling_error(int compile_result){
    puts("Error while compiling regex");
}

int compile_pattern(OPTIONS opts){
    int flags = REG_NOSUB, compile_result;

    if (opts->regexp == EREG)
        flags |= REG_EXTENDED;

    compile_result = regcomp(&opts->regcomp, opts->pattern, flags);

    if (compile_result)
        show_compiling_error(compile_result);
    return 0;
}

OPTIONS opts_alloc(void){
    return (OPTIONS) malloc(sizeof(options));
}

OPTIONS read_options(int argc, char **argv, OPTIONS opts){
    int c, i, option_index = 0;

    if (!opts)
        opts = opts_alloc();

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

    /*  DEFAULT VALUES */
    opts->delete       = 0;
    opts->force        = 0;
    opts->regexp       = WILDC;
    opts->limit        = UINT_MAX;
    opts->pattern[0]   = '\0';
    opts->prefix[0]    = '\0';
    /*  END DEFAULT VALUES */

    while ((c = getopt_long(argc, argv, "p:eEdfl:F::h", long_options, &option_index)) != -1){
        switch(c){
            case 'd':
                opts->delete = 1;
                break;
            case 'e':
                opts->regexp = BREG;
                break;
            case 'E':
                opts->regexp = EREG;
                break;
            case 'f':
                opts->force = 1;
                break;
            case 'l':
                i = 0;
                while (optarg[i++] != '\0')
                    if (optarg[i-1] < '0' || optarg[i-1] > '9') {
                        show_help();
                        exit(1);
                    }
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

    if (opts->regexp > 0)
        compile_pattern(opts);

    // Para terminar establece el directorio
    if (argv[optind] != NULL)
        strcpy(opts->directory, argv[optind]);
    else
        strcpy(opts->directory, getcwd(NULL, 0));

    return (OPTIONS) opts;
}

char * gen_filename(char *filename, const char *directory, const char *prefix, unsigned int pos){
    if (!filename)
        filename = malloc(sizeof(char) * (PATH_MAX + 1));

    sprintf(filename, "%s/%s%u", directory, prefix, pos);

    return filename;
}

int fill_directory(OPTIONS opts){
    FILE *dp;
    unsigned int limit, i = 0;
    char *filename = NULL;

    while ((dp = fopen((filename = gen_filename(filename, opts->directory, opts->prefix, i)), "a")) != NULL && ++i < opts->limit)
        fclose(dp);
    free(filename);

    return 0;
}

int check_dirname(const char *dirname){
    struct stat st;

    if (!stat(dirname, &st) && S_ISDIR((&st)->st_mode))
        return 1;

    return 0;
}

int check_pattern(const char *d_name, OPTIONS opts){
    int flags = 0;
    flags |= FNM_FILE_NAME|FNM_PERIOD|FNM_EXTMATCH;

    if (!opts->regexp) {
        if (fnmatch(opts->pattern, d_name, flags) == 0)
            return 1;
    } else if (regexec(&opts->regcomp, d_name, 0, NULL, 0) == 0)
        return 1;

    return 0;
}

int superls_delentry(const char *filename){
    printf("Borrando elemento: %s\n", filename);
    unlink(filename);
    return 0;
}

int superls_readdir(OPTIONS opts){
    struct dirent *ep;
    DIR *dp;
    unsigned int i = 0;
    char filename[NAME_MAX];

    if (check_dirname(opts->directory)){
        dp = opendir(opts->directory);
        while ((ep = readdir(dp)) && i < opts->limit){
            if (!opts->pattern[0] || check_pattern(ep->d_name, opts)){
                strcpy(filename,ep->d_name);
                if (opts->delete && strcmp(filename, ".") != 0 && strcmp(filename,"..") != 0){
                    superls_delentry(filename);
                } else if (!opts->delete) {
                    puts(filename);
                }
                i++;
            }
        }
        if (dp)
            closedir(dp);
    }
}

int main(int argc, char **argv){
    OPTIONS opts;

    opts = read_options(argc, argv, NULL);

    if (opts->prefix[0])
        fill_directory(opts);
    else
        superls_readdir(opts);

    return 0;
}

