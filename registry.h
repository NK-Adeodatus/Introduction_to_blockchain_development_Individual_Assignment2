#ifndef REGISTRY_H
#define REGISTRY_H

/**
 * struct Book - Represents a library book
 * @book_id: The unique identifier for the book
 * @title: The title of the book
 * @author: The author of the book
 *
 * Description: Stores details about a book loaded from the registry.
 */
typedef struct Book {
    char book_id[20];
    char title[80];
    char author[50];
} Book;

/**
 * struct Member - Represents a library member
 * @member_id: The unique identifier for the member
 * @full_name: The full name of the member
 * @course_code: The course code of the member
 *
 * Description: Stores details about a member loaded from the registry.
 */
typedef struct Member {
    char member_id[20];
    char full_name[50];
    char course_code[10];
} Member;

int load_books(const char *filename, Book **out_books, int *out_count);
int load_members(const char *filename, Member **out_members, int *out_count);

Book *find_book(Book *books, int num_books, const char *book_id);
Member *find_member(Member *members, int num_members, const char *member_id);

#endif /* REGISTRY_H */
