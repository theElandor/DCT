#ifndef PAM_H
#define PAM_H

#include <stdio.h>
#include <stdint.h>
typedef uint32_t u32;
typedef uint8_t u8;

#endif

typedef struct Pam{
  // PAM storing RGB image
  u32 w;
  u32 h;
  u32 d;
  u32 maxval;
  char type[20];
  u8 *data; // R*C*3
} Pam;

void pam_free(Pam *pam);
void pam_read_entry(FILE* in_file, u32 *var);
void pam_print_info(const Pam* p);
void pam_allocate_rgb(Pam *p);
void pam_allocate_grayscale(Pam *p);
void pam_write(const Pam *p, const char *filename);
void pam_rgb_to_grayscale(const Pam *p, char *filename);
void pam_fill(Pam *p, FILE* f);

