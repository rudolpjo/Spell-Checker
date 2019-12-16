#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * param file
 * return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * param file
 * param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    assert(map != NULL);
    assert(file != NULL);

    char * words = nextWord(file);
    while(words!=NULL){
        hashMapPut(map, words, -1);
        words = nextWord(file);
    }
}

/* Levenshtein calculation, code from: https://www.lemoda.net/c/levenshtein/ */
static int distance (const char * word1,
                     int len1,
                     const char * word2,
                     int len2)
{
    int matrix[len1 + 1][len2 + 1];
    int i;
    for (i = 0; i <= len1; i++) {
        matrix[i][0] = i;
    }
    for (i = 0; i <= len2; i++) {
        matrix[0][i] = i;
    }
    for (i = 1; i <= len1; i++) {
        int j;
        char c1;

        c1 = word1[i-1];
        for (j = 1; j <= len2; j++) {
            char c2;

            c2 = word2[j-1];
            if (c1 == c2) {
                matrix[i][j] = matrix[i-1][j-1];
            }
            else {
                int delete;
                int insert;
                int substitute;
                int minimum;

                delete = matrix[i-1][j] + 1;
                insert = matrix[i][j-1] + 1;
                substitute = matrix[i-1][j-1] + 1;
                minimum = delete;
                if (insert < minimum) {
                    minimum = insert;
                }
                if (substitute < minimum) {
                    minimum = substitute;
                }
                matrix[i][j] = minimum;
            }
        }
    }
    return matrix[len1][len2];
}

/**
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * param argc
 * param argv
 * return
 */


int main(int argc, const char** argv)
{
    HashMap* map = hashMapNew(1000);

    FILE* file = fopen("dictionary.txt", "r");
    if(!file){
        printf("error opening dictionary\n");
    }
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    char inputBuffer[256];
    int quit = 0;
    int set = 0;
    while(!quit){
        
        set = 0;
        printf("Enter a word or \"quit\" to quit: ");
        scanf("%s", inputBuffer);
        
        int i = 0;
        while(inputBuffer[i]){
            inputBuffer[i] = tolower(inputBuffer[i]);
            i++;
        }
    
        /* Iterate through dictionary creating lev numbers using inputBuffer as compare string */
        struct HashLink * cur;
        for(int i = 0; i < map->capacity; i++){
            cur = map->table[i];
            while(cur != NULL){
                cur->value = distance(cur->key, ((int)strlen(cur->key)), inputBuffer, ((int)strlen(inputBuffer)));
                if(strcmp(cur->key, inputBuffer) == 0){
                    /* Case when word is in dictionary */
                    if(strcmp(cur->key, "quit") == 0){
                        printf("Thanks for learning proper spelling! Goodbye.\n");
                        set = 1;
                    } else {
                        printf("Your word: \"%s\" is spelled correctly.\n", inputBuffer);
                        set = 1;
                    }
                }
                cur = cur->next;
            }
        }
        
        /* Case for when word does match any dictionary entries */
        if(set == 0){
            struct misSpell {
                int value;
                char* word;
            };
            struct misSpell suggest[5];
            for(int i = 0; i < 5; i++){
                suggest[i].value = 100;
                suggest[i].word = "";
            }
            
            /* Compare input word to Lev. numbers, storing 5 closest */
            for(int i = 0; i < map->capacity; i++){
                cur = map->table[i];
                while(cur != NULL){
                    for(int i = 0; i < 5; i++){
                        if(cur->value < suggest[i].value){
                            suggest[i].value = cur->value;
                            suggest[i].word = cur->key;
                            i = 5;
                        }
                    }
                    cur = cur->next;
                }
            }
            printf("Your word \"%s\" is misspelled, maybe you meant one of these:\n", inputBuffer);
            for(int i = 0; i < 5; i++){
                printf("%s\n", suggest[i].word);
            }
        }
    
        if (strcmp(inputBuffer, "quit") == 0){
            quit = 1;
        }
    }

    hashMapDelete(map);
    return 0;
}
