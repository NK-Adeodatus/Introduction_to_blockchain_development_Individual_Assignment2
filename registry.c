#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registry.h"

/**
 * trim_newline - Removes the trailing newline character from a string
 * @str: The string to be modified
 *
 * Return: void
 */
void trim_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) {
        str[len-1] = '\0';
    }
}

/**
 * load_books - Loads books from a CSV file into memory
 * @filename: The path to the books registry file
 * @out_books: Double pointer to store the dynamically allocated array of books
 * @out_count: Pointer to store the number of books loaded
 *
 * Return: 1 on success, 0 on failure
 */
int load_books(const char *filename, Book **out_books, int *out_count) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;

    int count = 0;
    char line[200];
    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) > 1) count++;
    }

    if (count == 0) {
        fclose(file);
        return 0;
    }

    Book *books = malloc(count * sizeof(Book));
    rewind(file);

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        trim_newline(line);
        if (strlen(line) <= 1) continue;

        char *id = strtok(line, ",");
        char *title = strtok(NULL, ",");
        char *author = strtok(NULL, ",");

        if (id && title && author) {
            strncpy(books[i].book_id, id, sizeof(books[i].book_id) - 1);
            strncpy(books[i].title, title, sizeof(books[i].title) - 1);
            strncpy(books[i].author, author, sizeof(books[i].author) - 1);
            books[i].book_id[sizeof(books[i].book_id) - 1] = '\0';
            books[i].title[sizeof(books[i].title) - 1] = '\0';
            books[i].author[sizeof(books[i].author) - 1] = '\0';
            i++;
        }
    }

    fclose(file);
    *out_books = books;
    *out_count = i;
    return 1;
}

/**
 * load_members - Loads members from a CSV file into memory
 * @filename: The path to the members registry file
 * @out_members: Double pointer to store the dynamically allocated members array
 * @out_count: Pointer to store the number of members loaded
 *
 * Return: 1 on success, 0 on failure
 */
int load_members(const char *filename, Member **out_members, int *out_count) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;

    int count = 0;
    char line[200];
    while (fgets(line, sizeof(line), file)) {
        if (strlen(line) > 1) count++;
    }

    if (count == 0) {
        fclose(file);
        return 0;
    }

    Member *members = malloc(count * sizeof(Member));
    rewind(file);

    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        trim_newline(line);
        if (strlen(line) <= 1) continue;

        char *id = strtok(line, ",");
        char *name = strtok(NULL, ",");
        char *course = strtok(NULL, ",");

        if (id[0] == '-' && id[1] == ' ') id += 2;

        if (id && name && course) {
            strncpy(members[i].member_id, id, sizeof(members[i].member_id) - 1);
            strncpy(members[i].full_name, name, sizeof(members[i].full_name) - 1);
            strncpy(members[i].course_code, course, sizeof(members[i].course_code) - 1);
            members[i].member_id[sizeof(members[i].member_id) - 1] = '\0';
            members[i].full_name[sizeof(members[i].full_name) - 1] = '\0';
            members[i].course_code[sizeof(members[i].course_code) - 1] = '\0';
            i++;
        }
    }

    fclose(file);
    *out_members = members;
    *out_count = i;
    return 1;
}

/**
 * find_book - Searches the memory array for a book by ID
 * @books: The array of books to search
 * @num_books: The total number of books in the array
 * @book_id: The ID string to look for
 *
 * Return: Pointer to the matching Book, or NULL if not found
 */
Book* find_book(Book *books, int num_books, const char *book_id) {
    for (int i = 0; i < num_books; i++) {
        if (strcmp(books[i].book_id, book_id) == 0) {
            return &books[i];
        }
    }
    return NULL;
}

/**
 * find_member - Searches the memory array for a member by ID
 * @members: The array of members to search
 * @num_members: The total number of members in the array
 * @member_id: The ID string to look for
 *
 * Return: Pointer to the matching Member, or NULL if not found
 */
Member* find_member(Member *members, int num_members, const char *member_id) {
    for (int i = 0; i < num_members; i++) {
        if (strcmp(members[i].member_id, member_id) == 0) {
            return &members[i];
        }
    }
    return NULL;
}