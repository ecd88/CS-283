#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define BUFFER_SZ 50

// Function Prototypes
void print_buff(char *, int);
int setup_buff(char *, char *, int);
int count_words(char *, int, int);
void reverse_string(char *, int);
void print_words(char *, int);
void free_buffer(char *);
int search_replace(char *, int, char *, char *);

int main(int argc, char *argv[]) {
    char *buffer;
    char *user_input;
    char option;
    int return_code;
    int input_length;

    //TODO: #1 Validate argv[1] before accessing to avoid segmentation faults
    if ((argc < 2) || (*argv[1] != '-')) {
        printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", argv[0]);
        exit(1);
    }

    option = (char) *(argv[1] + 1);

    if (option == 'h') {
        printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", argv[0]);
        exit(0);
    }

    //TODO: #2 Ensure an input string is provided, otherwise display usage and exit
    if (argc < 3) {
        printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", argv[0]);
        exit(1);
    }

    user_input = argv[2];

    //TODO: #3 Allocate memory for the buffer; exit with code 99 if allocation fails
    buffer = (char *)malloc(BUFFER_SZ);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(99);
    }

    input_length = setup_buff(buffer, user_input, BUFFER_SZ);
    if (input_length < 0) {
        printf("Error: Input string exceeds allowed size\n");
        free_buffer(buffer);
        exit(3);
    }

    switch (option) {
        case 'c':
            return_code = count_words(buffer, BUFFER_SZ, input_length);
            printf("Word Count: %d\n", return_code);
            break;

        case 'r':
            reverse_string(buffer, input_length);
            break;

        case 'w':
            print_words(buffer, input_length);
            break;

        case 'x':
            if (argc < 5) {
                printf("Error: -x requires two additional arguments\n");
                free_buffer(buffer);
                exit(3);
            }
            return_code = search_replace(buffer, input_length, argv[3], argv[4]);
            if (return_code < 0) {
                printf("Error: String not found or could not be replaced\n");
                free_buffer(buffer);
                exit(3);
            }
            break;

        default:
            printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", argv[0]);
            free_buffer(buffer);
            exit(1);
    }

    //TODO: #6 Free allocated memory before exiting
    print_buff(buffer, BUFFER_SZ);
    free_buffer(buffer);
    exit(0);
}

//TODO: #4 Process input by removing extra spaces and padding with '.'
int setup_buff(char *buffer, char *user_input, int length) {
    char *source = user_input, *destination = buffer;
    int count = 0, space_flag = 1;

    while (*source) {
        if (*source == ' ') {
            if (!space_flag) {
                if (count >= length) return -1;
                *destination++ = ' ';
                count++;
            }
            space_flag = 1;
        } else {
            if (count >= length) return -1;
            *destination++ = *source;
            count++;
            space_flag = 0;
        }
        source++;
    }

    if (count > 0 && *(destination - 1) == ' ') {
        destination--;
        count--;
    }

    while (count < length) {
        *destination++ = '.';
        count++;
    }
    return count;
}

// Free allocated buffer memory
void free_buffer(char *buffer) {
    free(buffer);
}

// Count words within the buffer
int count_words(char *buffer, int length, int input_length) {
    int count = 0, in_word = 0;
    for (int i = 0; i < input_length; i++) {
        if (buffer[i] == ' ') {
            in_word = 0;
        } else if (!in_word) {
            count++;
            in_word = 1;
        }
    }
    return count;
}

// Reverse characters within the buffer while maintaining padding
void reverse_string(char *buffer, int input_length) {
    char *left = buffer, *right = buffer + BUFFER_SZ - 1;

    while (left < right) {
        char temp = *left;
        *left++ = *right;
        *right-- = temp;
    }
}

// Print words in the buffer along with their length
void print_words(char *buffer, int input_length) {
    printf("Word Print\n");
    printf("----------\n");

    int index = 1;
    char *ptr = buffer, *start;

    // Find the position where actual text ends (before dots start)
    char *actual_end = buffer + input_length;
    while (actual_end > buffer && *(actual_end - 1) == '.') {
        actual_end--;  // Move back to ignore trailing dots
    }

    while (ptr < actual_end) {
        while (*ptr == ' ' && ptr < actual_end) ptr++;

        if (ptr < actual_end) {
            start = ptr;
            while (*ptr != ' ' && ptr < actual_end) ptr++;

            int length = ptr - start;
            printf("%d. ", index++);
            while (start < ptr) putchar(*start++);
            printf(" (%d)\n", length);
        }
    }

    printf("\nNumber of words returned: %d\n", index - 1);
}

// Print the buffer content including padding
void print_buff(char *buffer, int length) {
    printf("Buffer:  [");
    for (int i = 0; i < length; i++) putchar(buffer[i]);
    printf("]\n");
}

// Implement search & replace EC
int search_replace(char *buffer, int input_length, char *find, char *replace) {
    char *match = NULL, *src = buffer;
    int find_len = 0, replace_len = 0, i;

    // Calculate lengths of `find` and `replace`
    while (find[find_len]) find_len++;
    while (replace[replace_len]) replace_len++;

    // Locate the first occurrence of `find`
    while (src < buffer + input_length) {
        char *temp = src, *f = find;
        while (*temp == *f && *f && temp < buffer + input_length) {
            temp++;
            f++;
        }
        if (!*f) {  // Found match
            match = src;
            break;
        }
        src++;
    }

    // If match is not found, return an error
    if (!match) {
        return -1;
    }

    int remaining_len = input_length - (match - buffer) - find_len;
    int new_length = (match - buffer) + replace_len + remaining_len;

    // Prevent buffer overflow
    if (new_length > BUFFER_SZ) {
        new_length = BUFFER_SZ;
        remaining_len = BUFFER_SZ - ((match - buffer) + replace_len);
    }

    // Handle shifting if `replace` is larger than `find`
    if (replace_len != find_len) {
        char *shift_src = match + find_len;
        char *shift_dest = match + replace_len;

        if (replace_len > find_len) {
            for (i = input_length; i >= (match - buffer) + find_len; i--) {
                if (i + (replace_len - find_len) < BUFFER_SZ) {
                    buffer[i + (replace_len - find_len)] = buffer[i];
                }
            }
        } else {
            for (i = 0; i < remaining_len; i++) {
                shift_dest[i] = shift_src[i];
            }
        }
    }

    // Copy `replace` into the buffer at `match`
    for (i = 0; i < replace_len; i++) {
        match[i] = replace[i];
    }

    // Fill remaining buffer space with dots
    for (i = new_length; i < BUFFER_SZ; i++) {
        buffer[i] = '.';
    }

    return 0;
}