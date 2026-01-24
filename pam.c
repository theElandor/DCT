#include "pam.h"
#include <stdlib.h>
#include <string.h>

char BUFFER[20];

void pam_free(Pam *pam){
  free(pam->data);
}
void pam_read_entry(FILE* in_file, u32 *var){
  // reads a entry of the PAM file, like WIDTH or HEIGHT
  fscanf(in_file, "%s %d", BUFFER, var);
}

void pam_print_info(const Pam* p){
  // Print header information of the PAM file
  printf("WIDTH: %u\n", p->w);
  printf("HEIGHT: %u\n", p->h);
  printf("DEPTH: %u\n", p->d);
  printf("MAXVAL: %u\n", p->maxval);
  printf("TUPLTYPE %s\n", p->type);
}

void pam_allocate_rgb(Pam *p){
  // Allocates the space for the image
  p->data = (unsigned char *)malloc(p->h * p->w * 3);
}
void pam_allocate_grayscale(Pam *p){
  // Allocates the space for the image
  p->data = (unsigned char *)malloc(p->h * p->w);
}

void pam_write(const Pam *p, const char *filename){
  // writes PAM object to file.
  FILE *out_file = fopen(filename, "wb");
  if(!out_file){
    printf("Error creating output file.\n");
    return;
  }
  fprintf(out_file, "P7\nWIDTH %d\nHEIGHT %d\nDEPTH %d\nMAXVAL %d\n", 
      p->w, p->h, p->d, p->maxval);
  fprintf(out_file, "TUPLTYPE %s\nENDHDR\n", p->type);
  
  fwrite(p->data,p->h*p->w*p->d ,1 ,out_file);
  fclose(out_file);
}

void pam_rgb_to_grayscale(const Pam *p, char *filename)
{
    // saves target pam struct as grayscale pam
    Pam grayscale;
    grayscale.w = p->w;
    grayscale.h = p->h;
    grayscale.maxval = p->maxval;
    grayscale.d = 1;
    strncpy(grayscale.type, "GRAYSCALE", sizeof(grayscale.type) - 1);
    grayscale.type[sizeof(grayscale.type) - 1] = '\0';
    pam_allocate_grayscale(&grayscale);
    size_t r = 0;
    for (size_t i = 0; i < p->h; i++) {
        for (size_t j = 0; j < p->w; j++) {
            size_t idx = (i * p->w + j) * p->d;
            u8 R = p->data[idx];
            u8 G = p->data[idx + 1];
            u8 B = p->data[idx + 2];
            u8 Gray = (R + G + B) / 3;
            grayscale.data[r++] = Gray;
        }
    }
    pam_write(&grayscale, filename);
    pam_free(&grayscale);
}

void pam_fill(Pam *p, FILE* f){
  // reads the whole PAM file.
  unsigned rows = p->h;
  unsigned cols = p->w;
  fread(p->data, 1, rows*cols*p->d, f);
}
