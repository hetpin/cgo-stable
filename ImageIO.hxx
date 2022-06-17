//Copyright (C) 2012, Benoît Naegel <b.naegel@unistra.fr>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#include <omp.h>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;
namespace LibTIM {

inline std::string  GImageIO_NextLine(std::ifstream &file)
{
  std::string str;
  file >> str;
  char buf[256];
  char ch;
  while (str[0] == '#') {file.getline(buf, 256, '\n'); file >> str;}

//For the separator after the ASCII string ('\n' or ' ')
  file.get(ch);
  return str;
}

inline void GImageIO_ReadPPMHeader(std::ifstream &file, std::string &format,
                                   unsigned int &width, unsigned int &height, unsigned int &colormax )
{
  std::string str;

  str = GImageIO_NextLine(file);

  format = str;

  str = GImageIO_NextLine(file);
  std::stringstream str_stream(str);
  str_stream >> width;

  str = GImageIO_NextLine(file);
  str_stream.clear();
  str_stream.str(str);
  str_stream >> height;


  str = GImageIO_NextLine(file);
  str_stream.clear();
  str_stream.str(str);
  str_stream >> colormax;


  // std::cout << format << "\n" << width << " " << height << "\n" << colormax << "\n";
}

//TODO: load should return a value!!!!

template <>
inline int Image<U8>::load(const char*filename, Image <U8> &im)
{
  std::ifstream file(filename, std::ios_base::in  | std::ios_base::binary);
  if (!file)
  {
    std::cerr << "Image file I/O error\n";
    return 0;
  }
  std::string format;
  unsigned int width, height, colormax;

  GImageIO_ReadPPMHeader(file, format, width, height, colormax);

  if (format != "P5" || colormax >= 256)
  {
    std::cerr << "Error: either type mismatch image type or image is in ASCII .ppm format\n";
    exit(1);
  }
  else {
    if (im.data != 0) delete[] im.data;

    im.size[0] = width;
    im.size[1] = height;
    im.size[2] = 1;
    im.dataSize = width * height * 1;

    for (int i = 0; i < 3; i++)
    {
      im.spacing[i] = 1.0;
    }
    im.data = new U8 [im.dataSize];
    file.read(reinterpret_cast<char *> (im.data), im.dataSize);
  }
  file.close();
  return 1;
}

template <>
inline int Image<U16>::load(const char*filename, Image <U16> &im)
{
  std::ifstream file(filename, std::ios_base::in  | std::ios_base::binary);
  if (!file)
  {
    std::cerr << "Image file I/O error\n";
    return 0;
  }
  std::string format;
  unsigned int width, height, colormax;

  GImageIO_ReadPPMHeader(file, format, width, height, colormax);

  if (format != "P5")
  {
    std::cerr << "Error: either type mismatch image type or image is in ASCII .ppm format\n";
    exit(1);
  }
  else {
    if (im.data != (U16*)(0)) delete[] im.data;

    im.size[0] = width;
    im.size[1] = height;
    im.size[2] = 1;
    im.dataSize = width * height * 1;
    for (int i = 0; i < 3; i++)
    {
      im.spacing[i] = 1.0;
    }
    im.data = new U16 [im.dataSize];
    file.read(reinterpret_cast<char *> (im.data), im.dataSize);
  }
  file.close();
  return 1;
}

template <>
inline int Image<RGB>::load(const char*filename, Image <RGB> &im)
{
  std::ifstream file(filename, std::ios_base::in  | std::ios_base::binary);
  if (!file)
  {
    std::cerr << "Image file I/O error\n";
    return 0;
  }
  std::string format;
  unsigned int width, height, colormax;

  GImageIO_ReadPPMHeader(file, format, width, height, colormax);

  if (format != "P6" || colormax >= 256)
  {
    std::cerr << "Error: either type mismatch image type or image is in ASCII .ppm format\n";
    exit(1);
  }
  else {
    if (im.data != (RGB*)(0)) delete[] im.data;

    im.size[0] = width;
    im.size[1] = height;
    im.size[2] = 1;
    im.dataSize = width * height;
    for (int i = 0; i < 3; i++)
    {
      im.spacing[i] = 1.0;
    }
    im.data = new RGB [im.dataSize];
    Table<U8, 3> *data_int = new Table<U8, 3>[im.dataSize];

//      file.read(reinterpret_cast<char *> (im.data),im.dataSize*3);
    file.read(reinterpret_cast<char *> (data_int), im.dataSize * 3);
    //copy data_int to data_float (im.data)
    for (int i = 0; i < im.dataSize; i++) {
      std::cout << "----------" << endl;
      for (int j = 0; j < 3; j++) {
        im.data[i][j] = (int) data_int[i][j] + 0.1;
      }
      data_int[i].print_table();
      im.data[i].print_table();
    }
    getchar();

  }
  file.close();
  return 1;
}


template <>
inline int Image <RGB>::load_fits(image &img, Image <RGB> &im, int h, int w, int c) {
  if (im.data != 0) delete[] im.data;
  im.size[0] = w;
  im.size[1] = h;
  im.size[2] = 1;
  im.dataSize = w * h;

  for (int i = 0; i < c; i++) {
    im.spacing[i] = 1.0;
  }
  im.data = new RGB [im.dataSize];


  // #pragma omp parallel for private(i)
  for (int i = 0; i < im.dataSize; i++) {
    float value = img.data[i];
    im.data[i] = RGB(value, value, value);
    // im.data[i].print_table();
  }
  return 1;
}

template <>
inline int Image <RGB>::load_fits(std::vector<image> &imgs, Image <RGB> &im, int h, int w, int c) {
  if (im.data != 0) delete[] im.data;
  im.size[0] = w;
  im.size[1] = h;
  im.size[2] = 1;
  im.dataSize = w * h;
  im.data = new RGB [im.dataSize];
  for (int i = 0; i < c; i++) {
    im.spacing[i] = 1.0;
  }

  // #pragma omp parallel for private(i)
  // #pragma omp parallel for
  for (int i = 0; i < im.dataSize; i++) {
    RGB values = RGB(0, 0, 0);
    for (int k = 0; k < imgs.size(); k++) {
      values[k] = imgs[k].data[i];
    }
    // values.print_table();
    im.data[i] = values;
  }

  return 1;

}

template <>
inline int Image <RGB>::load_fits_quantize(std::vector<image> &imgs, Image <RGB> &im, int h, int w, int c, float &scale) {
  float max = 0;
  float max_quantize = 65535;//255;//
  if (im.data != 0) delete[] im.data;
  im.size[0] = w;
  im.size[1] = h;
  im.size[2] = 1;
  im.dataSize = w * h;
  im.data = new RGB [im.dataSize];
  for (int i = 0; i < c; i++) {
    im.spacing[i] = 1.0;
  }
  std::cout << im.size[0] << " " << im.size[1] << " " << im.size[2] << " " <<  im.dataSize << endl;
  getchar();
  //Fing max
  for (int i = 0; i < im.dataSize; i++) {
    for (int k = 0; k < imgs.size(); k++) {
      if (imgs[k].data[i] > max) {
        max = imgs[k].data[i];
      }
    }
  }
  //Cal quantization scale, save to update corresponding mean, var, bias, etc
  scale = max_quantize / max; //Since min_quantize = 0 and min = 0;

  //Copy img to c channels of im
  srand(1);//fix the seed
  for (int i = 0; i < im.dataSize; i++) {
    RGB values = RGB(0, 0, 0);
    for (int k = 0; k < imgs.size(); k++) {
      values[k] = (int)((imgs[k].data[i] / max) * max_quantize) ; //+ (rand()%10)/10000.0;
    }
    im.data[i] = values;
    im.data[i].print_table();
  }
  getchar();
  return 1;
}

///Pgm writer
template <>
inline int Image <U8>::save( const char *filename) {
  std::ofstream file(filename, std::ios_base::trunc  | std::ios_base::binary);
  if (!file)
  {
    std::cerr << "Image file I/O error\n";
    return 0;
  }

  std::string str;
  std::string format;

  int width = getSizeX();
  int height = getSizeY();

  int buf_size = width * height;

  file << "P5\n#CREATOR: GImage \n" << width << " " << height << "\n" << "255" ;
  file << "\n";

  file.write(reinterpret_cast<char *> (this->data), buf_size);

  file << "\n";

  file.close();
}

///Pgm writer
template <>
inline int Image <U16>::save( const char *filename) {
  std::ofstream file(filename, std::ios_base::trunc  | std::ios_base::binary);
  if (!file)
  {
    std::cerr << "Image file I/O error\n";
    return 0;
  }
  std::string str;
  std::string format;

  int width = getSizeX();
  int height = getSizeY();

  int buf_size = width * height * sizeof(U16);

  /* file << "P5\n#CREATOR: GImage \n" << width << " " << height << "\n" << "65535\n" ;
  */
  file << "P5\n#CREATOR: GImage \n" << width << " " << height << "\n" << 65535 << "\n" ;

  file.write(reinterpret_cast<char *> (this->data), buf_size);

  file << "\n";

  file.close();
}

template <>
inline int Image <RGB>::save( const char *filename) {
  std::ofstream file(filename, std::ios_base::trunc  | std::ios_base::binary);
  if (!file)
  {
    std::cerr << "Image file I/O error\n";
    return 0;
  }

  std::string str;
  std::string format;

  int width = getSizeX();
  int height = getSizeY();

  int buf_size = width * height * 3;

  file << "P6\n#CREATOR: GImage \n" << width << " " << height << "\n" << "255\n" ;

  U8 *buf = new U8[width * height * 3];

  for (int i = 0; i < width * height; i++)
  {
    buf[i * 3] = (*this)(i)[0];
    buf[i * 3 + 1] = (*this)(i)[1];
    buf[i * 3 + 2] = (*this)(i)[2];
  }

  file.write(reinterpret_cast<char *> (buf), buf_size);

  file << "\n";

  file.close();

  delete[] buf;

  return 1;
}

/*
template <>
    inline int Image <RGB>::save( const char *filename) {
  std::ofstream file(filename,std::ios_base::trunc  | std::ios_base::binary);
  if(!file)
  {
    std::cerr << "Image file I/O error\n";
    return 0;
  }

  std::string str;
  std::string format;

  int width=getSizeX();
  int height=getSizeY();

  int buf_size = width*height*3;

  file << "P6\n#CREATOR: GImage \n" << width << " " << height << "\n" << "255\n" ;

  U8 *buf=new U8[width*height*3];

  for(int i=0; i<width*height; i++)
  {
    buf[i*3]=(*this)(i)[0];
    buf[i*3+1]=(*this)(i)[1];
    buf[i*3+2]=(*this)(i)[2];
  }

  file.write(reinterpret_cast<char *> (buf),buf_size);

  file << "\n";

  file.close();

  delete[] buf;

  return 1;
    }*/

}
