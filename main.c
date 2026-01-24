#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include "pam.h"
// compile with gcc -Wall -Wextra main.c pam.c
typedef uint32_t u32;
typedef uint8_t u8;
#define PI 3.14159265358979323846

void float_to_u8(const float *src, u8 *dst, size_t count){
  for(size_t i = 0; i < count; i++){
    dst[i] = (u8)src[i];
  }
}
int DCT(Pam *pam, float *frequencies, u32 ps){
  /*
  * Implements forward DCT filling the frequency array.
  * Returns 1 in case of success, 0 in case of errors.
  * Args:
  *   - pam: Pam structure containing the signal
  *   - frequencies: will be filled with the frequencies!
  *   - ps: Patch size
  **/
  if(!frequencies){
    printf("You need to provide a empty frequency array.\n");
    return 1;
  }
  printf("Patch size: %d\n", ps);
  printf("Image size: %d %d\n", pam->w, pam->h);
  u32 rows = pam->h;
  u32 cols = pam->w;
  if (pam->h % ps != 0 || pam->w  % ps != 0){
    printf("Uncompatible patch size and image dimensions.\n");
    return 0;
  }
  u32 grid_rows = rows / ps;
  u32 grid_cols = cols / ps;
  printf("Grid rows: %d\n", grid_rows);
  printf("Grid rows: %d\n", grid_cols);
  for(u32 gr = 0; gr < grid_rows; gr++){
    for(u32 gc = 0; gc < grid_cols; gc++){
      for(u32 i = 0; i < ps; i++){
        for(u32 j = 0; j < ps; j++){
          // We are computing C(i,j)
          float summation = 0;
          for(u32 ii = 0; ii < ps; ii++){
            float partial_summation =0;
            for(u32 jj = 0; jj < ps; jj++){ 

              size_t index = ((size_t)gr * ps + ii)*cols + (gc*ps+jj);
              if (index >= (size_t)rows*cols){
                return 0;
              }
              float value = pam->data[index];
              float mul1 = cos(((2*ii+1)*i*PI)/(2.0*ps));
              float mul2 = cos(((2*jj+1)*j*PI)/(2.0*ps));
              partial_summation += (value * mul1 * mul2);
            }
            summation += partial_summation;
          }
          // normalization factors
          float a_u = 0.0;
          float a_v = 0.0;
          if (i == 0){a_u = sqrt(1.0/ps);}
          else {a_u = sqrt(2.0/ps);}
          if (j == 0){a_v = sqrt(1.0/ps);}
          else {a_v = sqrt(2.0/ps);}
          float result = summation * a_u * a_v;
          size_t index = (gr * ps + i)*cols + (gc * ps + j);
          frequencies[index] = result;
        }
      }
    }
  }
  return 1;
}

int DCT_reverse(Pam *pam, float* frequencies, float *signal, u32 ps){
  /*
  * Implements reversed DCT.
  * Returns 1 in case of success, 0 otherwise.
  * Args:
  *   - pam: Pam struct containing information on rows and columns.
  *       Data array can be a nullptr.
  *   - frequencies: contains the frequencies obtained with the forward DCT.
  *   - signal: will contain the restored signal. Must be allocated by the caller.
  *   - ps: Patch size.
  **/
  if(!frequencies || !signal){
    printf("You need to provide a valid frequency array and an empty signal array.\n");
    return 0;
  } 
  printf("Patch size: %d\n", ps);
  printf("Image size: %d %d\n", pam->w, pam->h);
  u32 rows = pam->h;
  u32 cols = pam->w;
  if (pam->h % ps != 0 || pam->w  % ps != 0){
    printf("Uncompatible patch size and image dimensions.\n");
    return 0;
  }
  u32 grid_rows = rows / ps;
  u32 grid_cols = cols / ps;
  printf("Grid rows: %d\n", grid_rows);
  for(u32 gr = 0; gr < grid_rows; gr++){
    for(u32 gc = 0; gc < grid_cols; gc++){
      for(u32 i = 0; i < ps; i++){
        for(u32 j = 0; j < ps; j++){
          // We are computing f(i,j)
          float summation = 0;
          for(u32 ii = 0; ii < ps; ii++){
            float partial_summation =0;
            for(u32 jj = 0; jj < ps; jj++){ 

              size_t index = ((size_t)gr * ps + ii)*cols + (gc*ps+jj);
              if (index >= (size_t)rows*cols){
                return 0;
              }
              float value = frequencies[index];
              float mul1 = cos(((2*i+1)*ii*PI)/(2.0*ps));
              float mul2 = cos(((2*j+1)*jj*PI)/(2.0*ps));
              float a_u = 0.0;
              float a_v = 0.0;
              if (ii == 0){a_u = sqrt(1.0/ps);}
              else {a_u = sqrt(2.0/ps);}
              if (jj == 0){a_v = sqrt(1.0/ps);}
              else {a_v = sqrt(2.0/ps);}
              partial_summation += (value * mul1 * mul2 * a_v * a_u);
            }
            summation += partial_summation;
          }
          size_t index = (gr * ps + i)*cols + (gc * ps + j);
          signal[index] = summation;
        }
      }
    }
  }
  return 1; 
}

int main(int argc, char **argv){
  FILE *in_file = fopen(argv[1], "rb");
  if (!in_file){
    printf("Error opening file.\n");
    return 1;
  }
  Pam pam;
  char BUFFER[20];
  u32 ps = 8; 
  fscanf(in_file, "%s", BUFFER); // P7
  pam_read_entry(in_file, &pam.w);
  pam_read_entry(in_file, &pam.h);
  pam_read_entry(in_file, &pam.d);
  pam_read_entry(in_file, &pam.maxval);
  fscanf(in_file, "%s %s", BUFFER, pam.type);
  fscanf(in_file, "%s", BUFFER); fgetc(in_file);
  pam_allocate_grayscale(&pam);
  pam_fill(&pam, in_file);
  pam_print_info(&pam);
  pam_write(&pam, "test_out.pam"); // check if PAM has been read correctly.
  float *frequencies = (float *)malloc(pam.w * pam.h * sizeof(float));
  if (!DCT(&pam, frequencies, ps)){
    printf("Something went wrong during DCT.\n");
    return 1;
  }
  printf("Throwing away frequencies...\n");
  u32 grid_rows = pam.h / ps;
  u32 grid_cols = pam.w / ps;
  // ====================================================
  // throw away some frequency bins here to have some fun!
  // In this case, bin (1,0) is zeroed out.
  // ====================================================
  u32 lost_freq_row = 1;
  u32 lost_freq_col = 0;
  for(u32 gr = 0; gr < grid_rows; gr++){
    for(u32 gc = 0; gc < grid_cols; gc++){
        size_t index = (gr * ps + lost_freq_row) * pam.w + (gc * ps + lost_freq_col);
        frequencies[index] = 0.0f;
    }
  } 
  float *signal = (float *)malloc(pam.w * pam.h * sizeof(float));
  if (!DCT_reverse(&pam, frequencies, signal, ps)){
    printf("Something went wrong during reverse DCT.\n");
    return 1;
  } 
  Pam restored_image;
  restored_image.w = pam.w;
  restored_image.h = pam.h;
  restored_image.d = pam.d;
  restored_image.maxval = 255;
  strncpy(restored_image.type, "GRAYSCALE", sizeof(restored_image.type) - 1);
  pam_allocate_grayscale(&restored_image);
  float_to_u8(signal, restored_image.data, restored_image.w*restored_image.h*restored_image.d);
  pam_write(&restored_image, "restored.pam");
  printf("Wrote restored image.\n");
  pam_free(&pam);
  pam_free(&restored_image);
  free(signal);
  return 0;
}
