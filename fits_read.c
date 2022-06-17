//#include <stdint.h>
#include "fitsio.h"

#include "mto/main.h"

#include <omp.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int fits_read_size(const char *file_path, PIXEL_TYPE **image,
                   SHORT_TYPE *width, SHORT_TYPE *height)
{
  fitsfile *fptr;
  int status = 0, bitpix, naxis;
  long naxes[2], fpixel[2], nelements;

  if (fits_open_file(&fptr, file_path, READONLY, &status) ||
      fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status))
  {
    fits_report_error(stderr, status);
    return FALSE;
  }
  *width = naxes[0];
  *height = naxes[1];

  free(*image);
  return TRUE;
}

int fits_read(const char *file_path, PIXEL_TYPE **image,
              SHORT_TYPE *width, SHORT_TYPE *height)
{
  fitsfile *fptr;
  int status = 0, bitpix, naxis;
  long naxes[2], fpixel[2], nelements;

  if (fits_open_file(&fptr, file_path, READONLY, &status) ||
      fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status))
  {
    fits_report_error(stderr, status);
    return FALSE;
  }

  nelements = naxes[0] * naxes[1];
  *image = malloc(nelements * sizeof(PIXEL_TYPE));
  fpixel[0] = fpixel[1] = 1;

  *width = naxes[0];
  *height = naxes[1];


  if (fits_read_pix(fptr, FITS_TYPE, fpixel, nelements, NULL, *image,
                    NULL, &status) || fits_close_file(fptr, &status))
  {
    fits_report_error(stderr, status);
    return FALSE;
  }

  return TRUE;
}

int fits_read_crop(const char *file_path, PIXEL_TYPE **image, PIXEL_TYPE **image_crop, SHORT_TYPE *width, SHORT_TYPE *height
                   , SHORT_TYPE *width_crop, SHORT_TYPE *height_crop) {
  printf("fits_read_crop: reading %s\n", file_path );
  fitsfile *fptr;
  int status = 0, bitpix, naxis;
  long naxes[2], fpixel[2], nelements;

  if (fits_open_file(&fptr, file_path, READONLY, &status) ||
      fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status))
  {
    fits_report_error(stderr, status);
    return FALSE;
  }

  nelements = naxes[0] * naxes[1];

  *image = malloc(nelements * sizeof(PIXEL_TYPE));

  fpixel[0] = fpixel[1] = 1;

  *width = naxes[0];
  *height = naxes[1];

  if (fits_read_pix(fptr, FITS_TYPE, fpixel, nelements, NULL, *image,
                    NULL, &status) || fits_close_file(fptr, &status)) {
    fits_report_error(stderr, status);
    return FALSE;
  }

  // int w = 300;// 300;
  // int h = 150;//150;

  // int w = 150;// 300;
  // int h = 100;//150;

  // int w = 16;// 300;
  // int h = 16;//150;

  // int w = 110;// 300;
  // int h = 90;//150;

  // int w = 250;// 300;
  // int h = 250;//150;
  // int w = 40;//110;// 300;
  // int h = 30;//90;//150;
  // int w = 300;//scale = 3
  // int h = 300;//scale = 3

  int w = 250;//scale = 3
  int h = 250;//scale = 3

  int scale = 2;//3;
  *width_crop = w;
  *height_crop = h;

  *image_crop = malloc(w * h * sizeof(PIXEL_TYPE));

  // int start_i = 950;
  // int start_j = 500;

  // int start_i = 270;
  // int start_j = 180;

  // int start_i = 100;
  // int start_j = 150;

  // int start_i = 0;
  // int start_j = 100;

  // int start_i = 0;
  // int start_j = 0;
  // int start_i = 270;
  // int start_j = 180;

  // int start_i = 100; //50;//Doc truc Y
  // int start_j = 200; //150;  //Ngang truc X

  int start_i = 0; //50;//Doc truc Y
  int start_j = 0; //150;  //Ngang truc X

  // #pragma omp parallel for private(i)
  for (int i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++)
    {
      (*image_crop)[i * w + j] = (*image)[naxes[0] * (i * scale + start_i) + (j * scale + start_j)] ;
    }
  }

  free(*image);
  return TRUE;
}

int fits_read_crop_startij(const char *file_path, PIXEL_TYPE **image, PIXEL_TYPE **image_crop, SHORT_TYPE *width, SHORT_TYPE *height
                           , SHORT_TYPE *width_crop, SHORT_TYPE *height_crop, SHORT_TYPE start_i, SHORT_TYPE start_j) {
  printf("fits_read_crop_startij: reading %s\n", file_path );
  fitsfile *fptr;
  int status = 0, bitpix, naxis;
  long naxes[2], fpixel[2], nelements;

  if (fits_open_file(&fptr, file_path, READONLY, &status) ||
      fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status))
  {
    fits_report_error(stderr, status);
    return FALSE;
  }

  nelements = naxes[0] * naxes[1];

  *image = malloc(nelements * sizeof(PIXEL_TYPE));

  fpixel[0] = fpixel[1] = 1;

  *width = naxes[0];
  *height = naxes[1];

  if (fits_read_pix(fptr, FITS_TYPE, fpixel, nelements, NULL, *image,
                    NULL, &status) || fits_close_file(fptr, &status)) {
    fits_report_error(stderr, status);
    return FALSE;
  }

  int w = *width_crop;// 300;
  int h = *height_crop;//150;
  int scale = 1;

  *image_crop = malloc(w * h * sizeof(PIXEL_TYPE));
  // #pragma omp parallel for private(i)
  for (int i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++)
    {
      // int k = h-i-1;
      (*image_crop)[i * w + j] = (*image)[naxes[0] * (i * scale + start_j) + (j * scale + start_i)] ;
      // (*image_crop)[k*w + j] = (*image)[naxes[0] * (i*scale + start_i) + (j*scale + start_j)] ;
    }
    //Horizon flip (i, j) -> (h-i-1, j)

  }
  free(*image);
  return TRUE;
}
int fits_read_crop_fake_test_2(const char *file_path, PIXEL_TYPE **image, PIXEL_TYPE **image_crop, SHORT_TYPE *width, SHORT_TYPE *height
                               , SHORT_TYPE *width_crop, SHORT_TYPE *height_crop, int fake) {
  printf("fits_read_crop_fake_test_2: reading %s\n", file_path );
  fitsfile *fptr;
  int status = 0, bitpix, naxis;
  long naxes[2], fpixel[2], nelements;

  if (fits_open_file(&fptr, file_path, READONLY, &status) ||
      fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status))
  {
    fits_report_error(stderr, status);
    return FALSE;
  }

  nelements = naxes[0] * naxes[1];

  *image = malloc(nelements * sizeof(PIXEL_TYPE));

  fpixel[0] = fpixel[1] = 1;

  *width = naxes[0];
  *height = naxes[1];

  if (fits_read_pix(fptr, FITS_TYPE, fpixel, nelements, NULL, *image,
                    NULL, &status) || fits_close_file(fptr, &status)) {
    fits_report_error(stderr, status);
    return FALSE;
  }

  int w = 50;//50;//300;
  int h = 25;//;//150;
  int scale = 6;//1;
  *width_crop = w;
  *height_crop = h;

  *image_crop = malloc(w * h * sizeof(PIXEL_TYPE));
  int start_i = 950;
  int start_j = 500;
  if (fake == 0) {
    // #pragma omp parallel for private(i)
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        if (i > 10 && i < 20 && j > 10 && j < 20) {
          (*image_crop)[i * w + j] = 100;
        } else {
          (*image_crop)[i * w + j] = 10;
        }
        // printf("%0.3f ", (*image_crop)[i*w + j] );
      }
      // printf("\n");
    }
    printf("static fake = %d\n", fake );
    getchar();
  }
  if (fake == 1) {
    // #pragma omp parallel for private(i)
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        if (i > 5 && i < 23 && j < 30 && j > 15) {
          (*image_crop)[i * w + j] = 300;
        } else {
          (*image_crop)[i * w + j] = 20;
        }
      }
    }
    printf("static fake = %d\n", fake );
  }
  if (fake == 2) {
    // #pragma omp parallel for private(i)
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        if (i > 0 && i < 15 && j < 25 && j > 5) {
          (*image_crop)[i * w + j] = 100;
        } else {
          (*image_crop)[i * w + j] = 20;
        }
      }
    }
    printf("static fake = %d\n", fake );
    getchar();
  }
  return TRUE;
}
int fits_read_crop_fake_test_1(const char *file_path, PIXEL_TYPE **image, PIXEL_TYPE **image_crop, SHORT_TYPE *width, SHORT_TYPE *height
                               , SHORT_TYPE *width_crop, SHORT_TYPE *height_crop, int fake) {
  printf("fits_read_crop_fake_test_1: reading %s\n", file_path );
  fitsfile *fptr;
  int status = 0, bitpix, naxis;
  long naxes[2], fpixel[2], nelements;

  if (fits_open_file(&fptr, file_path, READONLY, &status) ||
      fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status))
  {
    fits_report_error(stderr, status);
    return FALSE;
  }

  nelements = naxes[0] * naxes[1];

  *image = malloc(nelements * sizeof(PIXEL_TYPE));

  fpixel[0] = fpixel[1] = 1;

  *width = naxes[0];
  *height = naxes[1];

  if (fits_read_pix(fptr, FITS_TYPE, fpixel, nelements, NULL, *image,
                    NULL, &status) || fits_close_file(fptr, &status)) {
    fits_report_error(stderr, status);
    return FALSE;
  }

  int w = 50;//50;//300;
  int h = 25;//;//150;
  int scale = 6;//1;
  *width_crop = w;
  *height_crop = h;

  *image_crop = malloc(w * h * sizeof(PIXEL_TYPE));
  int start_i = 950;
  int start_j = 500;
  if (fake == 0) {
    // #pragma omp parallel for private(i)
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        if (i > 10 && i < 20 && j > 10 && j < 20) {
          (*image_crop)[i * w + j] = 100;
        } else {
          (*image_crop)[i * w + j] = 10;
        }
        // printf("%0.3f ", (*image_crop)[i*w + j] );
      }
      // printf("\n");
    }
    printf("static fake = %d\n", fake );
  }
  if (fake == 1) {
    // #pragma omp parallel for private(i)
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        if (i > 5 && i < 23 && j < 30 && j > 15) {
          (*image_crop)[i * w + j] = 300;
        } else {
          (*image_crop)[i * w + j] = 20;
        }
      }
    }
    printf("static fake = %d\n", fake );
  }
  if (fake == 2) {
    // #pragma omp parallel for private(i)
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        if (i > 5 && i < 23 && j < 30 && j > 15) {
          (*image_crop)[i * w + j] = 300;
        } else {
          (*image_crop)[i * w + j] = 20;
        }
      }
    }
    printf("static fake = %d\n", fake );
  }
  return TRUE;
}

int fits_read_crop_fake_test_1dot5(const char *file_path, PIXEL_TYPE **image, PIXEL_TYPE **image_crop,
                                   SHORT_TYPE *width, SHORT_TYPE *height, SHORT_TYPE *width_crop, SHORT_TYPE *height_crop, int fake) {
  printf("fits_read_crop_fake_test_1dot5: reading %s\n", file_path );
  fitsfile *fptr;
  int status = 0, bitpix, naxis;
  long naxes[2], fpixel[2], nelements;

  if (fits_open_file(&fptr, file_path, READONLY, &status) ||
      fits_get_img_param(fptr, 2, &bitpix, &naxis, naxes, &status))
  {
    fits_report_error(stderr, status);
    return FALSE;
  }

  nelements = naxes[0] * naxes[1];

  *image = malloc(nelements * sizeof(PIXEL_TYPE));

  fpixel[0] = fpixel[1] = 1;

  *width = naxes[0];
  *height = naxes[1];

  if (fits_read_pix(fptr, FITS_TYPE, fpixel, nelements, NULL, *image,
                    NULL, &status) || fits_close_file(fptr, &status)) {
    fits_report_error(stderr, status);
    return FALSE;
  }

  int w = 50;//50;//300;
  int h = 25;//;//150;
  int scale = 6;//1;
  *width_crop = w;
  *height_crop = h;

  *image_crop = malloc(w * h * sizeof(PIXEL_TYPE));
  int start_i = 950;
  int start_j = 500;
  if (fake == 0) {
    // #pragma omp parallel for private(i)
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        (*image_crop)[i * w + j] = 0;
        if (i >= 5 && i <= 20 && j > 5 && j < 25) {
          (*image_crop)[i * w + j] = 100;
        }
        if (i > 10 && i < 15 && j > 10 && j < 20) {
          (*image_crop)[i * w + j] = 200;
        }
        // printf("%0.3f ", (*image_crop)[i*w + j] );
      }
      // printf("\n");
    }
    printf("static fake = %d\n", fake );
  }
  if (fake == 1) {
    // #pragma omp parallel for private(i)
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        (*image_crop)[i * w + j] = 0;
        if (i > 5 && i < 23 && j < 30 && j > 15) {
          (*image_crop)[i * w + j] = 300;
        }
        if (i > 10 && i < 15 && j < 25 && j > 17) {
          (*image_crop)[i * w + j] = 400;
        }
      }
    }
    printf("static fake = %d\n", fake );
  }
  if (fake == 2) {
    // #pragma omp parallel for private(i)
    for (int i = 0; i < h; i++)
    {
      for (int j = 0; j < w; j++)
      {
        (*image_crop)[i * w + j] = 0;
        if (i > 5 && i < 23 && j < 30 && j > 15) {
          (*image_crop)[i * w + j] = 300;
        }
        if (i > 10 && i < 15 && j < 25 && j > 17) {
          (*image_crop)[i * w + j] = 400;
        }
      }
    }
    printf("static fake = %d\n", fake );
  }
  return TRUE;
}
