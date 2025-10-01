#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    long long errors404;
    long long total_bytes;
} Stats;

typedef struct {
    int start_line;
    int end_line;
    char **lines;
} ThreadArgs;

Stats global_stats = {0, 0};
pthread_mutex_t stats_mutex;

void* process_log_chunk(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Stats local_stats = {0, 0};

    for (int i = args->start_line; i < args->end_line; i++) {
        char *line = args->lines[i];
        char *quote_ptr = strstr(line, "\" ");
        if (quote_ptr) {
            int status_code;
            long long bytes_sent;
            if (sscanf(quote_ptr + 2, "%d %lld", &status_code, &bytes_sent) == 2) {
                if (status_code == 404) {
                    local_stats.errors404++;
                } else if (status_code == 200) {
                    local_stats.total_bytes += bytes_sent;
                }
            }
        }
    }

    pthread_mutex_lock(&stats_mutex);
    global_stats.errors404 += local_stats.errors404;
    global_stats.total_bytes += local_stats.total_bytes;
    pthread_mutex_unlock(&stats_mutex);

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <caminho_do_arquivo_de_log> <numero_de_threads>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int num_threads = atoi(argv[2]);

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }
    
    char **lines = NULL;
    char *line = NULL;
    size_t len = 0;
    long num_lines = 0;
    while (getline(&line, &len, fp) != -1) {
        lines = realloc(lines, sizeof(char*) * (num_lines + 1));
        lines[num_lines] = strdup(line);
        num_lines++;
    }
    fclose(fp);
    if(line) free(line);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t threads[num_threads];
    ThreadArgs thread_args[num_threads];
    pthread_mutex_init(&stats_mutex, NULL);

    long lines_per_thread = num_lines / num_threads;
    long remaining_lines = num_lines % num_threads;
    long current_line = 0;

    for (int i = 0; i < num_threads; i++) {
        thread_args[i].lines = lines;
        thread_args[i].start_line = current_line;
        long chunk_size = lines_per_thread + (i < remaining_lines ? 1 : 0);
        thread_args[i].end_line = current_line + chunk_size;
        current_line += chunk_size;
        
        pthread_create(&threads[i], NULL, process_log_chunk, &thread_args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_spent = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("--- Versão Paralela (%d threads) ---\n", num_threads);
    printf("Total de erros 404: %lld\n", global_stats.errors404);
    printf("Total de bytes transferidos (código 200): %lld\n", global_stats.total_bytes);
    printf("Tempo de execução: %.4f segundos\n", time_spent);

    pthread_mutex_destroy(&stats_mutex);
    for (long i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    free(lines);

    return 0;
}