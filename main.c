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
};

struct word_stat **wordStat = NULL;
unsigned int wordCount = 0;

struct word_stat *stat_create(char *word) {
    struct word_stat *result = calloc(1, sizeof(struct word_stat));
    result->word = strdup(word);
    return result;
}

struct word_stat *stat_exists(char *word) {
    for(size_t i = 0;i<wordCount;i++) {
        if(!strcmp(wordStat[i]->word, word)) {
            return wordStat[i];
        }
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
    if(ws != NULL) {
        ws->count++;
    } else {
        wordStat = realloc(wordStat, (wordCount + 1) * sizeof(struct word_stat**));
        wordStat[wordCount] = stat_create(word);
        wordStat[wordCount]->count = 1;
        wordCount++;
    }
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
        swapped = false;
        for(size_t i = 0;i<wordCount-1;i++) {
            if(wordStat[i]->count < wordStat[i+1]->count) {
                struct word_stat *tmp = wordStat[i];
                wordStat[i] = wordStat[i+1];
                wordStat[i+1] = tmp;
                swapped = true;
                break;
            }
        }
    }
    while(swapped);

    for( size_t i = 0;i<wordCount;i++ ) {
        printf("%25.25s %d\n", wordStat[i]->word, wordStat[i]->count);
        free(wordStat[i]->word);
        free(wordStat[i]);
    }
    return 0;
}
