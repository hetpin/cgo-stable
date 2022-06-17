# CGO

# Requirements
- cfitsio
```
https://www.gnu.org/software/gnuastro/manual/html_node/CFITSIO.html
```

- gsl
```
sudo apt-get install libgsl-dev 
```

- blas
```
sudo apt-get install libblas-dev
```

- openmp
```
sudo apt-get install libomp-dev 
```

## Compile
```
./demo_compile.sh
```

## Usage
```
./demo <band_1> <band_2> <band_3> <alpha option, default 10^(-6)>
```

```
# Example
./demo \
    data/alex/cluster1_g_10_10.fits \
    data/alex/cluster1_r_10_10.fits \
    data/alex/cluster1_i_10_10.fits \
    0.000001
```

Output in the same directory as the input.

- Visualzed segmentation maps as .ppm file.
- Segmentation map as a .fits file.
- Ouput .csv with dectected object id, x, y, ra, dec, area.


## Notes
### Multiband images need to be calibrated.
```
python data/reproject.py
```

### Image size < (500*500)
Because component graph construction doesn't scale well.