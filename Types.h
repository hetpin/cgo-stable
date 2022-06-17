//Copyright (C) 2012, Benoît Naegel <b.naegel@unistra.fr>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.

#ifndef Types_h
#define Types_h

#include <cmath>
#include <typeinfo>
// #include <iostream>

namespace LibTIM
{
//Machine dependant typedefs

typedef unsigned char U8;
typedef signed char S8;

typedef unsigned short U16;
typedef signed short S16;

typedef unsigned long  U32;
typedef signed long    S32;

typedef float F32;
//typedef double F64;

//Table
template <class T, int N>
struct Table{
	Table(){};
	Table(const Table &v){
		for(int i=0; i<N; i++) el[i]=v.el[i];
		}
	Table(int p) {for(int i=0; i<N; i++) el[i]=p;}
	Table(int *vect) {for(int i=0; i<N; i++) el[i]=vect[i];}
    void print_table(){ for(int i=0; i<N; i++){std::cout<<" "<<el[i];} std::cout << " " <<typeid(el).name()<<" \n";}
    // Table(int r, int g, int b ) {el[0]=r; el[1]=g; el[2]=b; print_table();}
    Table(T r, T g, T b ) {el[0]=r; el[1]=g; el[2]=b; /*print_table();*/}
    bool is_equal(Table &v){for (int i = 0; i < N; i++){if(el[i] != v.el[i]) return false;} return true;}
    void get_supremum(Table &v){for (int i = 0; i < N; i++){if(el[i] < v.el[i]){el[i]=v.el[i];}}}
    void get_infimum(Table &v){for (int i = 0; i < N; i++){if(el[i] > v.el[i]){el[i]=v.el[i];}}}
    T get_sum(){
        T sum = 0;
        for (int i = 0; i < N; i++){
            sum +=el[i];
        }
        return sum;
    }

	T el[N];
    T &operator[](int i)
	{return el[i];}
    T operator[](int i) const {return el[i];}

	};

//Type of RGB point
//typedef Table<U8,3> RGB;
typedef Table<F32,3> RGB;
typedef Table<F32, 3> RGBF;


template <class T, int N>
inline bool operator==(Table<T,N> &t1, Table<T,N> &t2)
{
    for(int i=0; i<N; i++)
        {
        if(t1[i]!=t2[i]) return false;
        }
    return true;
}


template <class T, int N>
inline bool operator!=(Table<T,N> &t1, Table<T,N> &t2)
{

    return !(t1==t2);
}

template <class T, int N>
inline bool operator<=(Table<T,N> &t1, Table<T,N> &t2)
{
    for(int i=0; i<N; i++)
        {
        if(t1[i]>t2[i]) return false;
        }
    return true;
}


template <class T, int N>
inline bool operator<(Table<T,N> &t1, Table<T,N> &t2)
{
    if(t1==t2) return false;
    for(int i=0; i<N; i++)
        {
        if(t1[i]>t2[i]) return false;
        }
    return true;
}


template <class T, int N>
inline bool operator>=(Table<T,N> &t1, Table<T,N> &t2)
{
    for(int i=0; i<N; i++)
        {
        if(t1[i]<t2[i]) return false;
        }
    return true;
}


template <class T, int N>
inline bool operator>(Table<T,N> &t1, Table<T,N> &t2)
{
    if(t1==t2) return false;
    for(int i=0; i<N; i++)
        {
        if(t1[i]<t2[i]) return false;
        }
    return true;
}


template <int N>
inline double operator-(Table<U8,N> &t1, Table<U8,N> &t2)
{
    double res=0.0;
    double sum=0.0;
    for(int i=0; i<N; i++)
        {
        sum+=(t1[i]-t2[i])*(t1[i]-t2[i]);
        }
    return sqrt(sum);
}


//Type of image size
typedef unsigned short TSize;

//Type of point spacing
typedef double TSpacing;

//Type of points coordinates
typedef int TCoord;

//Type of label
typedef unsigned long TLabel;

//Type of offset
typedef  long TOffset;

const float FLOAT_EPSILON=0.0000000001f;
}

#endif
