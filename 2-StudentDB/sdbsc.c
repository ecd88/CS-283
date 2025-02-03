#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#include "db.h"
#include "sdbsc.h"

// Opens the database file with read/write access. If `should_truncate` is true, the file is cleared before opening.
int open_db(char *dbFile, bool should_truncate) {
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    int flags = O_RDWR | O_CREAT;
    if (should_truncate)
        flags |= O_TRUNC;
    int fd = open(dbFile, flags, mode);
    if (fd == -1) {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }
    return fd;
}

// Retrieves a student record based on the given ID. Returns an error code if the student does not exist.
int get_student(int fd, int id, student_t *s) {
    off_t offset = id * STUDENT_RECORD_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf(M_ERR_DB_READ);
        return ERR_DB_FILE;
    }
    if (read(fd, s, STUDENT_RECORD_SIZE) != STUDENT_RECORD_SIZE) {
        return SRCH_NOT_FOUND;
    }
    if (s->id == 0) {
        return SRCH_NOT_FOUND;
    }
    return NO_ERROR;
}

// Adds a new student to the database if the ID is not already in use.
int add_student(int fd, int id, char *fname, char *lname, int gpa) {
    student_t s;
    if (get_student(fd, id, &s) == NO_ERROR) {
        printf(M_ERR_DB_ADD_DUP, id);
        return ERR_DB_OP;
    }
    memset(&s, 0, sizeof(student_t));
    s.id = id;
    strncpy(s.fname, fname, sizeof(s.fname) - 1);
    strncpy(s.lname, lname, sizeof(s.lname) - 1);
    s.gpa = gpa;
    off_t offset = id * STUDENT_RECORD_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    if (write(fd, &s, STUDENT_RECORD_SIZE) != STUDENT_RECORD_SIZE) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    printf(M_STD_ADDED, id);
    return NO_ERROR;
}

// Marks a student record as deleted by writing an empty student record at the appropriate file position.
int del_student(int fd, int id) {
    student_t s;
    if (get_student(fd, id, &s) != NO_ERROR) {
        printf(M_STD_NOT_FND_MSG, id);
        return ERR_DB_OP;
    }
    off_t offset = id * STUDENT_RECORD_SIZE;
    if (lseek(fd, offset, SEEK_SET) == -1) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    if (write(fd, &EMPTY_STUDENT_RECORD, STUDENT_RECORD_SIZE) != STUDENT_RECORD_SIZE) {
        printf(M_ERR_DB_WRITE);
        return ERR_DB_FILE;
    }
    printf(M_STD_DEL_MSG, id);
    return NO_ERROR;
}

// Counts the number of active student records in the database and prints the result.
int count_db_records(int fd) {
    student_t s;
    int count = 0;
    lseek(fd, 0, SEEK_SET);
    while (read(fd, &s, STUDENT_RECORD_SIZE) == STUDENT_RECORD_SIZE) {
        if (s.id != 0) {
            count++;
        }
    }
    printf(count == 0 ? M_DB_EMPTY : M_DB_RECORD_CNT, count);
    return count;
}

// Prints all student records that are currently in the database.
int print_db(int fd) {
    student_t s;
    int found = 0;
    lseek(fd, 0, SEEK_SET);
    while (read(fd, &s, STUDENT_RECORD_SIZE) == STUDENT_RECORD_SIZE) {
        if (s.id != 0) {
            if (!found) {
                printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST_NAME", "LAST_NAME", "GPA");
            }
            found = 1;
            printf(STUDENT_PRINT_FMT_STRING, s.id, s.fname, s.lname, s.gpa / 100.0);
        }
    }
    if (!found) {
        printf(M_DB_EMPTY);
    }
    return NO_ERROR;
}

// Prints a single student record in the expected format.
void print_student(student_t *s) {
    if (!s || s->id == 0) {
        printf(M_ERR_STD_PRINT);
        return;
    }
    printf(STUDENT_PRINT_HDR_STRING, "ID", "FIRST NAME", "LAST NAME", "GPA");
    printf(STUDENT_PRINT_FMT_STRING, s->id, s->fname, s->lname, s->gpa / 100.0);
}

// Compresses the database by removing deleted student records.
// Copies only active records to a new file and replaces the original database.
int compress_db(int fd)
{
    int new_fd = open(TMP_DB_FILE, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (new_fd == -1) {
        printf(M_ERR_DB_OPEN);
        return ERR_DB_FILE;
    }
    student_t s;
    while (read(fd, &s, STUDENT_RECORD_SIZE) == STUDENT_RECORD_SIZE) {
        if (s.id != 0) {
            if (write(new_fd, &s, STUDENT_RECORD_SIZE) != STUDENT_RECORD_SIZE) {
                printf(M_ERR_DB_WRITE);
                close(new_fd);
                return ERR_DB_FILE;
            }
        }
    }
    close(fd);
    if (rename(TMP_DB_FILE, DB_FILE) != 0) {
        printf(M_ERR_DB_CREATE);
        return ERR_DB_FILE;
    }
    printf(M_DB_COMPRESSED_OK);
    return new_fd;
}

// Handles command-line arguments and executes the corresponding database operation based on the input.
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <operation> [args]\n", argv[0]);
        return EXIT_FAIL_ARGS;
    }
    char *operation = argv[1];
    int fd = open_db(DB_FILE, false);
    if (fd < 0) {
        return EXIT_FAIL_DB;
    }
    
    if (strcmp(operation, "-a") == 0) {
        if (argc != 6) {
            printf("Usage: %s -a <id> <fname> <lname> <gpa>\n", argv[0]);
            return EXIT_FAIL_ARGS;
        }
        int id = atoi(argv[2]);
        int gpa = atoi(argv[5]);
        int result = add_student(fd, id, argv[3], argv[4], gpa);
        if (result == ERR_DB_OP) return EXIT_FAIL_DB;
    } else if (strcmp(operation, "-f") == 0) {
        if (argc != 3) return EXIT_FAIL_ARGS;
        int id = atoi(argv[2]);
        student_t s;
        if (get_student(fd, id, &s) == NO_ERROR) {
            print_student(&s);
        } else {
            printf(M_STD_NOT_FND_MSG, id);
            return EXIT_FAIL_DB;
        }
    } else if (strcmp(operation, "-d") == 0) {
        if (argc != 3) return EXIT_FAIL_ARGS;
        int id = atoi(argv[2]);
        int result = del_student(fd, id);
        if (result == ERR_DB_OP) return EXIT_FAIL_DB;
    } else if (strcmp(operation, "-c") == 0) {
        count_db_records(fd);
    } else if (strcmp(operation, "-p") == 0) {
        print_db(fd);
    } else if (strcmp(operation, "-x") == 0) {
        close(fd);
        fd = open_db(DB_FILE, false);
        if (fd < 0) {
            return EXIT_FAIL_DB;
        }
        int result = compress_db(fd);
        if (result < 0) return EXIT_FAIL_DB;
    } else {
        printf("Unknown command\n");
        return EXIT_FAIL_ARGS;
    }
    close(fd);
    return EXIT_OK;
}
