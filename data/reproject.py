#Reproject all bands into g-band WCS
import numpy as np
from contrast_boost import contrast_boost
from astropy.io import fits
from astropy.utils.data import get_pkg_data_filename
from reproject import reproject_interp
from astropy.wcs import WCS
import matplotlib.pyplot as plt

def reproject_fits(data_filename = 'im1_g.fits', header_filename = 'im1_r.fits', is_show = False):
        hdu1 = fits.open(header_filename)[0]
        hdu2 = fits.open(data_filename)[0] #maps 2 to 1
        if is_show:
                ax1 = plt.subplot(1,2,1, projection=WCS(hdu1.header))
                ax1.imshow(contrast_boost(hdu1.data, threshold = 0.001), origin='lower')
                ax1.set_title('Data band')
                ax2 = plt.subplot(1,2,2, projection=WCS(hdu2.header))
                ax2.imshow(contrast_boost(hdu2.data, threshold = 0.001), origin='lower')
                ax2.set_title('Header band')
                plt.show()

        array, footprint = reproject_interp(hdu2, hdu1.header)
        #remove nan for notcovering pixels
        array[np.isnan(array)] =0

        if is_show:
                ax1 = plt.subplot(1,2,1, projection=WCS(hdu1.header))
                ax1.imshow(contrast_boost(array, threshold = 0.001))
                ax1.set_title('Reprojected data band')
                ax2 = plt.subplot(1,2,2, projection=WCS(hdu1.header))
                ax2.imshow(footprint)
                ax2.set_title('Data band footprint')
                plt.show()

        fits.writeto('reprojected/reprjted_'+ data_filename, array, hdu1.header, overwrite=True)

if __name__ == '__main__':
        reproject_fits('im1_r.fits', 'im1_g.fits', True)
        # reproject_fits('im1_z.fits', 'im1_g.fits', False)
        # reproject_fits('im1_i.fits', 'im1_g.fits', False)
        reproject_fits('im1_u.fits', 'im1_g.fits', True)