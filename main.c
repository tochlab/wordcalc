#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <ctype.h>

struct word_stat {
    char *word;
    unsigned int count;
    struct word_stat *next;
};

struct word_stat *wordlist_head = NULL;

struct word_stat *stat_create(char *word) {
    struct word_stat *result = calloc(1, sizeof(struct word_stat));
    result->word = strdup(word);
    return result;
}

struct word_stat *stat_exists(char *word) {
    struct word_stat *current_word = wordlist_head;
    while(current_word != NULL) {
        if(!strcmp(current_word->word, word)) {
            return current_word;
        }
        current_word = current_word->next;
    }
    return NULL;
}

void add_to_stat(char *word) {
    size_t i = 0;
    size_t slen = strlen(word);
    for(i = 0;i<slen;i++) {
        word[i] = (char ) tolower(word[i]);
    }
    struct word_stat *ws = stat_exists(word);

    if(ws == NULL) {
        ws = stat_create(word);
        ws->next = wordlist_head;
        wordlist_head = ws;
    }

    ws->count++;
}

bool isdelim(char c) {
    return !(isalpha(c) || (c == '_') || (c == '-'));
}

int main() {
    int fd = open("test.txt", O_RDONLY);
    if( fd < 0 ) {
        return EXIT_FAILURE;
    }
    struct stat stat1;
    int res = fstat(fd, &stat1);
    if( res != 0) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }
    size_t file_length = stat1.st_size;
    char *text_ptr = mmap(NULL, file_length, PROT_READ, MAP_PRIVATE, fd, 0);
    if(text_ptr == MAP_FAILED) {
        close(fd);
        return EXIT_FAILURE;
    }

    size_t start = 0;
    size_t end = 0;
    for(size_t idx = 0;idx<file_length;idx++) {
        if(!isdelim(text_ptr[idx])) continue;

        end = idx;
        if(end - start > 0) {
            char *word = strndup(&text_ptr[start], end - start);
            add_to_stat(word);
            free(word);
        }
        start = idx + 1;
    }

    close(fd);

    bool swapped = false;
    do {
        struct word_stat *current = wordlist_head;
        swapped = false;
        while(current->next != NULL) {
            if(current->next->count > current->count) {
                swapped = true;
                char *word = current->word;
                size_t cnt = current->count;
                current->word = current->next->word;
                current->count = current->next->count;
                current->next->word = word;
                current->next->count = cnt;
                break;
            }
            current = current->next;
        }
    }
    while(swapped);

    struct word_stat *current = wordlist_head;
    while(current->next != NULL) {
        printf("%25.25s %d\n", current->word, current->count);
        current = current->next;
    }

    current = wordlist_head;
    while (current != NULL) {
        struct word_stat *next = current->next;
        free(current->word);
        free(current);
        current = next;
    }

    return 0;
}
