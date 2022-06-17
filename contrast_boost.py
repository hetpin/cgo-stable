from astropy.io import fits
import numpy as np
import matplotlib.pyplot as plt
from scipy.misc import imsave

def contrast_boost(img, threshold = 0.01):
    """
    Saturate the threshold-% brightest pixels and apply a log transform
    """
    img = img.copy()
    h, edges = np.histogram(img, bins=10000)
    threshold = img.size * threshold
    i = -1
    total = h[-1]
    while total < threshold:
        i -= 1
        total += h[i]
    img[img >= edges[i]] = edges[i]  
    min_im = np.amin(img)
    img = 1 + (img - min_im) / (np.amax(img) - min_im) * 10
    return np.log(img)

#Read a fits file, return data + size
def get_data(filename, is_crop):
    hdulist = fits.open(filename)
    img_data = hdulist[0].data
    hdulist.close()
    #Custom range for im1.fits
    print(img_data.shape)
    img_data = img_data.astype(np.float32)
    if is_crop:
        img_data = img_data[950:1100, 500:800] #Uncomment this line to have a nice crop
        img_data = img_data[0::6, 0::6] #downsampling for test
    [w, h] = img_data.shape
    plt.figure(1)
    img_data = contrast_boost(img_data, threshold=0.003)
    # plt.imshow(-np.log(img_data+1), cmap='Greys')
    plt.imshow(-img_data, cmap='Greys')
    plt.colorbar()
    plt.show()
    return img_data, w, h

def get_data_arp(filename = "data/arp240_g.fits", is_crop = True):
    hdulist = fits.open(filename)
    img_data = hdulist[0].data
    hdulist.close()
    print(img_data.shape)
    img_data = img_data.astype(np.float32)
    if is_crop:
        img_data = img_data[1100:1400, 600:750] #Uncomment this line to have a nice crop
        img_data = img_data[0::2, 0::2] #downsampling for test
    [w, h] = img_data.shape
    plt.figure(1)
    img_data = contrast_boost(img_data)
    # plt.imshow(-np.log(img_data+1), cmap='Greys')
    plt.imshow(-img_data, cmap='Greys')
    plt.colorbar()
    plt.show()
    return img_data, w, h

def get_data_aku(filename = "/blade/home/nguyentx/Data/AkuMockMulti/color_mock/cluster1_i.fits", is_crop = True):
    hdulist = fits.open(filename)
    img_data = hdulist[0].data
    hdulist.close()
    print(img_data.shape)
    img_data = img_data.astype(np.float32)
    if is_crop:
        img_data = img_data[500:1000, 500:1000] #Uncomment this line to have a nice crop
        # img_data = img_data[0::2, 0::2] #downsampling for test
    [w, h] = img_data.shape
    plt.figure(1)
    img_data = contrast_boost(img_data, threshold = 0.005)
    # plt.imshow(-np.log(img_data+1), cmap='Greys')
    plt.imshow(-img_data, cmap='Greys')
    plt.colorbar()
    plt.show()
    imsave("aku_crop_i.png", img_data)
    return img_data, w, h

if __name__ == '__main__':
    get_data_aku()
    # get_data('im1.fits', True)
    # get_data_arp()
    
