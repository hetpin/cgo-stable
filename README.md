# CGO
CGO is a novel multiband object detection method relying on the component-graphs and statistical hypothesis tests. 

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


## Citation

Please cite the related publications if you use the code:

## CGO Article
```
@article{nguyen2021object,
  title={Object Detection with Component-Graphs in Multi-band Images: Application to Source Detection in Astronomical Images},
  author={Nguyen, Thanh Xuan and Chierchia, Giovanni and Razim, Oleksandra and Peletier, Reynier F and Najman, Laurent and Talbot, Hugues and Perret, Benjamin},
  journal={IEEE Access},
  volume={9},
  pages={156482--156491},
  year={2021},
  publisher={IEEE}
}
```

### CGO Paper
```
@inproceedings{nguyen2020cgo,
  title={Cgo: Multiband astronomical source detection with component-graphs},
  author={Nguyen, Thanh Xuan and Chierchia, Giovanni and Najman, Laurent and Venhola, Aku and Haigh, Caroline and Peletier, Reynier and Wilkinson, Michael HF and Talbot, Hugues and Perret, Benjamin},
  booktitle={2020 IEEE International Conference on Image Processing (ICIP)},
  pages={16--20},
  year={2020},
  organization={IEEE}
}
```
