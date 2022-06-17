#include "mto/main.h"
int fits_read(const char *file_path, PIXEL_TYPE **image, SHORT_TYPE *width, SHORT_TYPE *height);
int fits_read_size(const char *file_path, PIXEL_TYPE **image, SHORT_TYPE *width, SHORT_TYPE *height);
int fits_read_crop(const char *file_path, PIXEL_TYPE **image, PIXEL_TYPE **image_crop,
                   SHORT_TYPE *width, SHORT_TYPE *height, SHORT_TYPE *width_crop, SHORT_TYPE *height_crop);
int fits_read_crop_startij(const char *file_path, PIXEL_TYPE **image, PIXEL_TYPE **image_crop,
                           SHORT_TYPE *width, SHORT_TYPE *height, SHORT_TYPE *width_crop, SHORT_TYPE *height_crop, SHORT_TYPE start_i, SHORT_TYPE start_j);
int fits_read_crop_fake_test_1(const char *file_path, PIXEL_TYPE **image, PIXEL_TYPE **image_crop,
                               SHORT_TYPE *width, SHORT_TYPE *height, SHORT_TYPE *width_crop, SHORT_TYPE *height_crop, int channel);
int fits_read_crop_fake_test_1dot5(const char *file_path, PIXEL_TYPE **image, PIXEL_TYPE **image_crop,
                                   SHORT_TYPE *width, SHORT_TYPE *height, SHORT_TYPE *width_crop, SHORT_TYPE *height_crop, int channel);
int fits_read_crop_fake_test_2(const char *file_path, PIXEL_TYPE **image, PIXEL_TYPE **image_crop,
                               SHORT_TYPE *width, SHORT_TYPE *height, SHORT_TYPE *width_crop, SHORT_TYPE *height_crop, int channel);

