#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    long long errors404;
    long long total_bytes;
} Stats;

void parse_log_line(char *line, Stats *stats) {
    char *quote_ptr = strstr(line, "\" ");
    if (quote_ptr) {
        int status_code;
        long long bytes_sent;
        if (sscanf(quote_ptr + 2, "%d %lld", &status_code, &bytes_sent) == 2) {
            if (status_code == 404) {
                stats->errors404++;
            } else if (status_code == 200) {
                stats->total_bytes += bytes_sent;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <caminho_do_arquivo_de_log>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    Stats stats = {0, 0};
    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, fp) != -1) {
        parse_log_line(line, &stats);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_spent = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("--- Versão Sequencial ---\n");
    printf("Total de erros 404: %lld\n", stats.errors404);
    printf("Total de bytes transferidos (código 200): %lld\n", stats.total_bytes);
    printf("Tempo de execução: %.4f segundos\n", time_spent);

    fclose(fp);
    if (line) {
        free(line);
    }

    return 0;
}