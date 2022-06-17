#include <math.h>
#include <omp.h>
#include <string.h>

#include "main.h"
#include "background.h"

#include <gsl/gsl_cdf.h>

static void bg_error(const char* message)
{
  error("bg_error: %s\n", message);
}

// Taken from
// http://en.wikipedia.org/wiki/D%27Agostino%27s_K-squared_test

static int bg_k_squared_test(int n, FLOAT_TYPE skewness,
  FLOAT_TYPE kurtosis, FLOAT_TYPE significance_level)
{   
  // H_0: skewness and kurtosis are from a normal distribution.
  
  FLOAT_TYPE mu2_g1 = 6.0 * (n - 2.0) / ((n + 1.0) * (n + 3.0));
    
  FLOAT_TYPE gamma2_g1 = 36.0 * (n - 7.0) * (n * n + 2.0 * n - 5.0);
  gamma2_g1 /= (n - 2.0) * (n + 5.0) * (n + 7.0) * (n + 9.0);
    
  FLOAT_TYPE mu1_g2 = -6.0 / (n + 1.0);
    
  FLOAT_TYPE mu2_g2 = 24.0 * n * (n - 2.0) * (n - 3.0);
  mu2_g2 /= (n + 1.0) * (n + 1.0) * (n + 3.0) * (n + 5.0);
    
  FLOAT_TYPE gamma1_g2_a = 6.0 * (n * n - 5.0 * n + 2.0);
  gamma1_g2_a /= (n + 7.0) * (n + 9.0);
    
  FLOAT_TYPE gamma1_g2_b = 6.0 * (n + 3.0) * (n + 5.0);
  gamma1_g2_b /= n * (n - 2.0) * (n - 3.0);
    
  FLOAT_TYPE gamma1_g2 = gamma1_g2_a * sqrt(gamma1_g2_b);
    
  FLOAT_TYPE W_squared = sqrt(2.0 * gamma2_g1 + 4.0) - 1.0;
  FLOAT_TYPE delta = 1.0 / sqrt(log(sqrt(W_squared)));
  FLOAT_TYPE alpha_squared = 2.0 / (W_squared - 1.0);

  FLOAT_TYPE A = 6.0 + (8.0 / gamma1_g2) * (2.0 / gamma1_g2 +
    sqrt(1.0 + 4.0 / (gamma1_g2 * gamma1_g2)));
        
  FLOAT_TYPE Z_1 = delta *
    log(skewness / (sqrt(alpha_squared) * sqrt(mu2_g1)) +
      sqrt(skewness * skewness / (alpha_squared * mu2_g1) + 1.0));
        
  FLOAT_TYPE Z_2 = sqrt(9.0 * A / 2.0);
  Z_2 *= 1.0 - 2.0 / (9.0 * A) - pow((1.0 - 2.0 / A) /
    (1.0 + (kurtosis - mu1_g2) / sqrt(mu2_g2) * sqrt(2.0 / (A - 4.0))),
    1.0 / 3.0);
        
  FLOAT_TYPE K_squared = Z_1 * Z_1 + Z_2 * Z_2;
   
  // K_squared has approx. chi^2 distribution with 2 degrees of freedom.
   
  // The inverse of the chi^2 CDF with 2 degrees of freedom is
  // -2.0 * log(1 - x)
   
  //printf("%f %f\n", K_squared, -2.0 * log(significance_level));
  
  if (K_squared < -2.0 * log(significance_level))
    return BG_ACCEPT_TILE;   

  // K_squared could be NaN which will lead to rejection of the tile.   
    
  return BG_REJECT_TILE;
}

// Taken from
// http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance

static void bg_tile_skewness_and_kurtosis(
  PIXEL_TYPE *img_data,
  int width,
  int pos_y,
  int pos_x,
  int tile_width,
  int tile_height,
  FLOAT_TYPE *skewness,
  FLOAT_TYPE *kurtosis)
{
  int n = 0;
  FLOAT_TYPE mean = 0;
  FLOAT_TYPE M2 = 0;
  FLOAT_TYPE M3 = 0;
  FLOAT_TYPE M4 = 0;
  
  int y;
  for (y = pos_y; y != pos_y + tile_height; ++y)
  {
    int offset = y * width + pos_x;
    PIXEL_TYPE* img_ptr = img_data + offset;
    
    while (img_ptr != img_data + offset + tile_width)
    {
      ++n;
      
      FLOAT_TYPE delta = *img_ptr - mean;
      FLOAT_TYPE delta_over_n = delta / n;
      FLOAT_TYPE don_squared = delta_over_n * delta_over_n;
      FLOAT_TYPE expr = delta * delta_over_n * (n - 1);
      mean += delta_over_n;
      M4 += expr * don_squared * (n * n - 3 * n + 3) +
        6 * don_squared * M2 - 4 * delta_over_n * M3;
      M3 += expr * delta_over_n * (n - 2) - 3 * delta_over_n * M2;
      M2 += expr;
      
      ++img_ptr;
    }
  }
 
  //*variance = M2 / (n - 1.0);
  *skewness = (sqrt(n) * M3) / (M2 * sqrt(M2));
  *kurtosis = (n * M4) / (M2 * M2) - 3.0;        
}

static int is_tile_totally_flat(
  PIXEL_TYPE *img_data,
  int width,
  int pos_y,
  int pos_x,
  int tile_width,
  int tile_height){
  int n = 0;
  int y;
  PIXEL_TYPE* img_first_item = img_data + pos_y * width + pos_x;
  for (y = pos_y; y != pos_y + tile_height; ++y){
    int offset = y * width + pos_x;
    PIXEL_TYPE* img_ptr = img_data + offset;    
    while (img_ptr != img_data + offset + tile_width){
      if (*img_first_item != *img_ptr){
        return 0;
      }
      ++img_ptr;
    }
  }
  printf("Found a totally flat at %d %d size %d %d \n", pos_x, pos_y, tile_width, tile_height);
  return 1;
}

static void bg_tile_mean_and_variance(
  PIXEL_TYPE *img_data,
  int width,
  int pos_y,
  int pos_x,
  int tile_width,
  int tile_height,
  FLOAT_TYPE *mean,
  FLOAT_TYPE *variance)
{
  int n = 0;
  FLOAT_TYPE mu = 0;
  FLOAT_TYPE M2 = 0;
    
  int y;
  for (y = pos_y; y != pos_y + tile_height; ++y)
  {
    int offset = y * width + pos_x;
    PIXEL_TYPE* img_ptr = img_data + offset;
    
    while (img_ptr != img_data + offset + tile_width)
    {
      ++n;
      FLOAT_TYPE delta = *img_ptr - mu;
      mu += delta / n;
      M2 += delta * (*img_ptr - mu);
      ++img_ptr;
    }
  }
    
  *mean = mu;
  *variance = M2 / (n - 1);
}

static FLOAT_TYPE bg_combined_variance(FLOAT_TYPE var_A,
  FLOAT_TYPE var_B, int n_A, int n_B)
{
    return ((n_A - 1.0) * var_A + (n_B - 1.0) * var_B) /
      (n_A + n_B - 2.0);
}

static int bg_equal_means(FLOAT_TYPE mean_A, FLOAT_TYPE mean_B,
  int n_A, int n_B, FLOAT_TYPE combined_var,
  FLOAT_TYPE significance_level)
{
  FLOAT_TYPE t = (mean_A - mean_B) / 
    sqrt(combined_var / n_A + combined_var / n_B);
        
  FLOAT_TYPE boundary =
    gsl_cdf_tdist_Pinv(0.5 * significance_level, n_A + n_B - 2);

    //math_t_cdf_inv(0.5 * significance_level, n_A + n_B - 2);
    
  return t > boundary && t < -boundary;
}

static int bg_tile_check_means(
  PIXEL_TYPE *img_data,
  int width,
  int pos_y,
  int pos_x,
  int tile_width,
  int tile_height,
  FLOAT_TYPE significance_level)
{
  // H_0:
  // equal means in the top and bottom half of the tile 
  //   AND
  // in the left and right half.
  // 
 
  FLOAT_TYPE top_mean;
  FLOAT_TYPE bot_mean;

  FLOAT_TYPE top_var;
  FLOAT_TYPE bot_var;
    
  bg_tile_mean_and_variance(img_data, width, pos_y, pos_x,
    tile_width, tile_height / 2, &top_mean, &top_var);
        
  bg_tile_mean_and_variance(img_data, width, pos_y + tile_height / 2,
    pos_x, tile_width, tile_height / 2, &bot_mean, &bot_var);
      
  FLOAT_TYPE local_sig_level = 1 - pow(1 - significance_level, 0.5);
  int n = tile_width * tile_height;
    
  if (bg_equal_means(top_mean, bot_mean, n / 2, n / 2,
    bg_combined_variance(top_var, bot_var, n / 2, n / 2),
    local_sig_level) == FALSE)
  {
    return BG_REJECT_TILE;
  }

  FLOAT_TYPE left_mean;
  FLOAT_TYPE right_mean;

  FLOAT_TYPE left_var;
  FLOAT_TYPE right_var;
    
  bg_tile_mean_and_variance(img_data, width, pos_y, pos_x,
    tile_width / 2, tile_height, &left_mean, &left_var);
        
  bg_tile_mean_and_variance(img_data, width, pos_y, pos_x +
    tile_width / 2, tile_width / 2, tile_height, &right_mean,
    &right_var);        

    
  if(bg_equal_means(left_mean, right_mean, n / 2, n / 2,
    bg_combined_variance(left_var, right_var, n / 2, n / 2),
    local_sig_level) == FALSE)
  {
    return BG_REJECT_TILE;
  }
        
  return BG_ACCEPT_TILE;
}

static int bg_check_tile_is_flat(
  PIXEL_TYPE *img_data,
  int width,
  int pos_y,
  int pos_x,
  int tile_width,
  int tile_height,
  FLOAT_TYPE significance_level){
  //This segment to check if a tile is totally flat (In test case)

  if (is_tile_totally_flat(img_data, width, pos_y, pos_x, tile_width, tile_height)){
    return BG_ACCEPT_TILE;
  }


  // Normality and equal means tests.
  FLOAT_TYPE local_sig_level = 1 - pow(1 - significance_level, 0.5);
  
  FLOAT_TYPE skewness;
  FLOAT_TYPE kurtosis;
  
  bg_tile_skewness_and_kurtosis(img_data, width, pos_y, pos_x,
    tile_width, tile_height, &skewness, &kurtosis);    
      
  if(bg_k_squared_test(tile_width * tile_height, skewness, kurtosis,
    local_sig_level) == BG_REJECT_TILE)
  {
    return BG_REJECT_TILE;
  }

  if (bg_tile_check_means(img_data, width, pos_y, pos_x, tile_width,
    tile_height, local_sig_level) == BG_REJECT_TILE)
  {
    return BG_REJECT_TILE;
  }
  
  return BG_ACCEPT_TILE;
}

static int bg_available_tiles(
  image* img,
  int tile_width,
  int tile_height,
  FLOAT_TYPE significance_level)
{
  int num_tiles_x = img->width / tile_width;
  int num_tiles_y = img->height / tile_height;
  int num_tiles = num_tiles_x * num_tiles_y;
    
  int found_usable = 0;
  
  // Share f_u between threads
  #pragma omp parallel shared(found_usable)
  {  
    int i = omp_get_thread_num();
    int nthreads = omp_get_num_threads();
    int piece = num_tiles / nthreads;
    int rest = num_tiles % nthreads;
    int start;
    int end;
    
    // If thread number < tile remainder
    // Start = thread no. * (1 + tiles per thread)
    // End = start + (1 + tiles per thread)
    if (i < rest)
    {
      start = i * (piece + 1);
      end = start + piece + 1;
    } 
    else
    {
      start = rest * (piece + 1) + (i - rest) * piece;
      end = start + piece;
    }
        
    int x;
    for (x = start; x != end; ++x)
    {
      int pos_y = (x / num_tiles_x) * tile_height;
      int pos_x = (x % num_tiles_x) * tile_width;
      
      if (bg_check_tile_is_flat(img->data, img->width, pos_y, pos_x,
        tile_width, tile_height, significance_level) == BG_ACCEPT_TILE)
      {
        found_usable = 1;
      }
      
      if (found_usable)
      {
        break;
      }
    }
  }  
  // printf("Checking tiles size %dx%d found %d flats\n", tile_width, tile_height, found_usable);  
  return found_usable;
}

static void bg_estimate_mean_and_variance(
  image* img,
  int tile_width,
  int tile_height,
  int num_usable,
  int *usable_y,
  int *usable_x,
  FLOAT_TYPE *mean,
  FLOAT_TYPE *variance)
{
  int n = 0;
  *mean = 0;
  FLOAT_TYPE M2 = 0;
    
  int idx;
  for (idx = 0; idx != num_usable; ++idx)
  {
    int i;
    int j;
    for (i = usable_y[idx]; i != usable_y[idx] + tile_height; ++i)
    {
      for (j = usable_x[idx]; j != usable_x[idx] + tile_width; ++j)
      {
        ++n;
        FLOAT_TYPE delta = img->data[j + i * img->width] - *mean;
        *mean += delta / n;
        M2 += delta * (img->data[j + i * img->width] - *mean);
      }        
    }
  }
    
  *variance = M2 / (n - 1.0);
}

static int bg_collect_info(
  image* img,
  int tile_width,
  int tile_height,
  FLOAT_TYPE *mean,
  FLOAT_TYPE *variance,
  FLOAT_TYPE significance_level,
  int verbosity_level)
{
  int num_tiles_x = img->width / tile_width;
  int num_tiles_y = img->height / tile_height;
  int num_tiles = num_tiles_x * num_tiles_y;
  int* usable = safe_malloc(num_tiles * sizeof(int));
  memset(usable, 0, num_tiles * sizeof(int));
    
  int y;
  int x;
  #pragma omp parallel for private(x, y)
  for (y = 0; y < num_tiles_y; ++y)
  {
    for (x = 0; x != num_tiles_x; ++x)
    {
      if (bg_check_tile_is_flat(img->data, img->width, y * tile_height,
        x * tile_width, tile_width, tile_height, significance_level) ==
        BG_ACCEPT_TILE)
      {
        // printf("num_tiles_x = %d img->width=%d tile_width=%d\n",num_tiles_x,img->width ,tile_width );
        usable[y * num_tiles_x + x] = 1;
      }         
    }
  }

  int num_usable = 0;
  int* usable_x = safe_malloc(num_tiles * sizeof(int));
  int* usable_y = safe_malloc(num_tiles * sizeof(int));
  
  for (y = 0; y != num_tiles_y; ++y)
  {
    for (x = 0; x != num_tiles_x; ++x)
    {
      if (usable[y * num_tiles_x + x])
      {
        usable_x[num_usable] = x * tile_width;
        usable_y[num_usable++] = y * tile_height;
      }
    }
  }
  
  if (verbosity_level)
  {
    printf("Number of usable tiles: %d.\n\n", num_usable);
  }
    
  bg_estimate_mean_and_variance(img, tile_width, tile_height,
    num_usable, usable_y, usable_x, mean, variance);
      
  free(usable);
  free(usable_x);
  free(usable_y);
           
  return num_usable;
}

static int bg_find_usable_tile_size(
  image* img,
  int *tile_width,
  int *tile_height,
  FLOAT_TYPE significance_level){        
  int current_size = BG_TILE_SIZE_START;
    
  if (bg_available_tiles(img, current_size, current_size,
    significance_level) == TRUE)
  {
    current_size *= 2;
    
    while (current_size <= BG_TILE_SIZE_MAX &&
      bg_available_tiles(img, current_size, current_size,
        significance_level) == TRUE)
    {
      current_size *= 2;
    }
    
    current_size /= 2;
  }
  else
  {
    current_size /= 2;

    if (current_size <  BG_TILE_SIZE_MIN)
    {
      return FALSE;
    }
    
    while (bg_available_tiles(img, current_size, current_size,
      significance_level) == FALSE)
    {
      current_size /= 2;
      
      if (current_size <  BG_TILE_SIZE_MIN)
      {
        return FALSE;
      }
    }    
  }
  
  *tile_width = current_size;
  *tile_height = current_size;
  
  return TRUE;
}

void bg_info(
  image* img,
  FLOAT_TYPE *mean,
  FLOAT_TYPE *variance,
  int verbosity_level){
  int tile_width = BG_TILE_SIZE_START;
  int tile_height = BG_TILE_SIZE_START;
  
  if (bg_find_usable_tile_size(img, &tile_width, &tile_height,
    BG_REJECTION_RATE) == FALSE)
  {
    // mean = 0;
    // variance = 1;
    // return;
    bg_error("could not find usable tiles.");
  }
  
  if (verbosity_level)
  {
    printf("Using a tile size of %dx%d in the background estimation.\n",
      tile_height, tile_width);
  }
    
  bg_collect_info(img, tile_width, tile_height, mean,
    variance, BG_REJECTION_RATE, verbosity_level); 
}

void bg_subtract(image* img, FLOAT_TYPE mean, int verbosity_level)
{
  int i;
  #pragma omp parallel for private(i)
  for (i = 0; i < img->size; ++i)
  {
    img->data[i] -= mean;
  }
}

void bg_truncate(image *img, int verbosity_level)
{
  int i;
  #pragma omp parallel for private(i)
  for (i = 0; i < img->size; ++i)
  {
    if (img->data[i] < 0.0)
      img->data[i] = 0;
  }
}
