#include "filter.h"

#include <assert.h>
#include <math.h>

image gaussian_filter(INT_TYPE height, INT_TYPE width,
  FLOAT_TYPE sigma, int verbosity_level)
{
  assert(width > 0);
  assert(height > 0);
  assert(width % 2 == 1);
  assert(height % 2 == 1);

  int filter_radius_x = width / 2;
  int filter_radius_y = height / 2;
  
  image ret;
  image_set(&ret, NULL, height, width);

  ret.data = malloc(ret.size * sizeof(PIXEL_TYPE));
  
  PIXEL_TYPE* filter_data_ptr = ret.data;
  
  PIXEL_TYPE sum = 0;
      
  INT_TYPE y;
  for (y = -filter_radius_y; y <= filter_radius_y; ++y)
  {
    INT_TYPE x;
    for (x = -filter_radius_x; x <= filter_radius_x; ++x)
    {
      *filter_data_ptr = 1.0 / (2 * M_PI * sigma * sigma) *
        exp(-((x * x + y * y)/(2.0 * sigma * sigma)));
        
      sum += *filter_data_ptr++;        
    }     
  }
  
  FLOAT_TYPE inv_sum = 1 / sum;
  
  if (verbosity_level)
  {
    printf("%dx%d Gaussian filter. Sigma = %f"
      " (equivalent to FWHM = %f).\n", height, width,
      sigma, sigma * 2.354820045030949382);
  }
  
  for (y = 0; y != ret.size; ++y)
  {
    ret.data[y] *= inv_sum;
  }

  if (verbosity_level > 1)
  {
    printf("[\n");
    
    for (y = 0; y != ret.size; ++y)
    {
      if (y % width == 0)
        printf("  ");
      printf("%.6E ", ret.data[y]);
      if ((y + 1) % width == 0)
        printf("\n");
    }
    
    printf("]\n");
  }
  
  if (verbosity_level)
  {
    printf("\n");
  }
    
  return ret;
}

image filter(const image* in, const image* H, int verbosity_level)
{
  assert(H->width > 0);
  assert(H->height > 0);
  assert(H->width % 2 == 1);
  assert(H->height % 2 == 1);
  
  int filter_radius_x = H->width / 2;
  int filter_radius_y = H->height / 2;
  
  // Add zero'd borders to the image, making filtering easier.
  
  INT_TYPE img_width = in->width + filter_radius_x * 2;
  INT_TYPE img_height = in->height + filter_radius_y * 2;
  
  PIXEL_TYPE* img_data = calloc(img_width * img_height,
    sizeof(PIXEL_TYPE));
      
  INT_TYPE y;
  #pragma omp parallel for private(y)
  for (y = filter_radius_y; y < in->height + filter_radius_y; ++y)
  {
    PIXEL_TYPE* in_data_ptr = in->data + (y - filter_radius_y) *
      in->width;

    INT_TYPE x;
    for (x = filter_radius_x; x != in->width + filter_radius_x; ++x)
    {
      img_data[y * img_width + x] = *in_data_ptr++;
    }  
  }

  image out;
  image_set(&out, NULL, in->height, in->width);  
  out.data = malloc(out.size * sizeof(*out.data));
  
  #pragma omp parallel for private(y)
  for (y = filter_radius_y; y < in->height + filter_radius_y; ++y)
  {
    PIXEL_TYPE* out_data_ptr = out.data + (y - filter_radius_y) *
      in->width;

    INT_TYPE x;
    for (x = filter_radius_x; x != in->width + filter_radius_x; ++x)
    {
      FLOAT_TYPE sum = 0;
      
      PIXEL_TYPE* filter_ptr = H->data;
      PIXEL_TYPE* img_data_ptr = img_data +
        (y - filter_radius_y) * img_width +
        x - filter_radius_x;
      
      int i;
      for (i = 0; i != H->height; ++i)      
      {
        int j;
        for (j = 0; j != H->width; ++j)
        {
          sum += *filter_ptr++ * *img_data_ptr++;
        }        

        img_data_ptr += img_width - j;        
      }
      
      *out_data_ptr++ = sum;
    }
  }
  
  free(img_data);
  
  return out;
}
/*
int main(int argc, char** argv)
{
  PIXEL_TYPE *data;
  SHORT_TYPE width;
  SHORT_TYPE height;
  
  if (argc < 3)
    return EXIT_FAILURE;
    
  if (!fits_read(argv[1], &data, &width, &height))
  {
    return EXIT_FAILURE;
  }

  image img;
  image_set(&img, data, height, width);

  if (!fits_read(argv[2], &data, &width, &height))
  {
    return EXIT_FAILURE;
  }

  image img_comparison;
  image_set(&img_comparison, data, height, width);  
  
  image H = gaussian_filter(7, 7, 0.849321800288019*2);
  image filtered = filter(&img, &H);  
  image_free(&H);
  
  PIXEL_TYPE max_abs_diff = 0;
  
  int i;
  for (i = 0; i != img.size; ++i)
  {
    if (fabs(filtered.data[i] - img_comparison.data[i]) > max_abs_diff)
    {
      max_abs_diff = fabs(filtered.data[i] - img_comparison.data[i]);
    }
  }
  
  printf("%E\n", max_abs_diff);
  
  image_free(&img);
  image_free(&img_comparison);
  
  return 0;
}
*/
