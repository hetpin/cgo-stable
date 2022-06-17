import numpy as np
import io_compare as c_io
from scipy import stats
import sys


class Evaluator:
    """A class to hold the ground truth segment properties"""
    def __init__(self, in_img, gt_file, opt_method=0):
        self.method = opt_method
        self.input_file = in_img
        self.original_img = c_io.read_fits_image(in_img)

        img_shape = self.original_img.shape
    
        # Open the ground truth file
        self.target_map_2d = c_io.read_fits_image(gt_file)
        self.target_map = self.target_map_2d.ravel() #Flatten gt
    
        # Sort the target ID map for faster pixel retrieval
        sorted_ids = self.target_map.argsort() #Get indexs of for sorted gt
        id_set = np.unique(self.target_map) # get unique id of objects as a set
        id_set.sort()
        
        # Get the locations in sorted_ids of the matching pixels: left to right
        right_indices = np.searchsorted(self.target_map, id_set, side='right', sorter=sorted_ids)
        left_indices = np.searchsorted(self.target_map, id_set, side='left', sorter=sorted_ids)

        # Create an id-max_index dictionary: To save indices of brightest pixel of objects
        self.id_to_max = {}
        
        # Create an id - area dictionary (for merging error comparisons)
        self.id_to_area = {}

        # Iterate over object IDs
        for n in range(len(id_set)):
            
            # Find the location of the brightest pixel in each object
            pixel_indices = np.unravel_index(sorted_ids[left_indices[n]:right_indices[n]], img_shape)
            
            m = np.argmax(self.original_img[pixel_indices])
            max_pixel_index = (pixel_indices[0][m], pixel_indices[1][m])
            
            # Save the location and area in dictionaries
            self.id_to_max[id_set[n]] = max_pixel_index
            self.id_to_area[id_set[n]] = right_indices[n] - left_indices[n]


    def is_any_det(self, target_map, target_id, det_map):
        "Check two regions overlap"
        target_region = det_map[np.where(target_map == target_id)]
        # print(target_region)
        #Check if non_zero id exist
        return target_region.any()
        # return np.count_nonzero(target_region)

    def match_to_bp_list(self, detection_map):
        """Match at most one detection to each target object"""
        
        # Reverse to ensure 1:1 mapping
        det_to_target = {}
        target_ids = list(self.id_to_max.keys()) #Exactly id_set
        
        # Find the id of the background detection (0 or -1)
        det_min = detection_map.min()
        det_to_target[det_min] = -1 # Bg detection match bg gt 
        target_ids.remove(-1) # Remove bg gt

        #hetpin: This can be change to adapt single object simulation by checking detection_maps contain at least one gt pixel
        if self.is_any_det(self.target_map_2d, 1, detection_map):
            det_to_target[1] = 1
        return det_to_target
        
        # Map each id to the detection containing it
        for t_id in target_ids:
            max_loc = self.id_to_max[t_id]
            d_id = detection_map[max_loc]
            
            if d_id == det_min:
                continue
                    
            # Assign detections covering multiple maxima to the object with the largest maximum
            if d_id in det_to_target: # ~ d_id in det_to_target.keys()
                #There're multiple targets to current det_id: Choose brighter target, then of course darker target never match any other det 
                old_max_val = self.original_img[self.id_to_max[det_to_target[d_id]]]
                new_max_val = self.original_img[max_loc]
                
                if old_max_val < new_max_val:
                    det_to_target[d_id] = t_id
                
            else:
                #No duplication det to current target
                det_to_target[d_id] = t_id
                    
        return det_to_target #1:1 mapping from dets to targets
            

    def get_basic_stats(self, detection_map, det_to_target): 
        """Calculate statistics for F-score"""
                
        tp = len(det_to_target) - 1 # dont count bg target
        fp = len(set(detection_map.ravel())) - 1 - tp
        fn = len(self.id_to_max) - 1 - tp
    
        # print("OUTSTAT True positive:", tp)
        # print("OUTSTAT False negative:", fn)
        # print("OUTSTAT False positive:", fp)

        try:
            r = tp / (tp + fn)
        except:
            r = 0
            
        try:
            p = tp / (tp + fp)
        except:
            p = 0

        # print("OUTSTAT Recall:", r)
        # print("OUTSTAT Precision:", p)

        try:
            f_score = 2*((p*r)/(p+r))
        except:
            f_score = 0

        # print("OUTSTAT F score:", f_score)

        return f_score, tp, fp, fn
        
        
    
    def get_merging_scores(self, detection_map):
        """Calculate overmerging and undermerging scores."""
        
        t_map = self.target_map
        d_map = detection_map.ravel()
        
        # Sort the detection ID map for faster pixel retrieval
        sorted_ids = d_map.argsort()
        id_set = np.unique(d_map)
        id_set.sort()
        
        # Get the locations in sorted_ids of the matching pixels
        right_indices = np.searchsorted(d_map, id_set, side='right', sorter=sorted_ids)
        left_indices = np.searchsorted(d_map, id_set, side='left', sorter=sorted_ids)
        
        # Calculate under-merging score
        um_score = 0.0
        om_score = 0.0
        
        d_id_to_area = {}

        # UNDER-MERGING - match to target map
        for n in range(len(id_set)):
            # Find the target labels of the pixels with detection label n
            target_vals = t_map[sorted_ids[left_indices[n]:right_indices[n]]]
            
            d_id_to_area[id_set[n]] = right_indices[n] - left_indices[n]
            
            # Find the number of pixels with each ID in the target map
            t_ids, t_counts = np.unique(target_vals, return_counts=True)

            # Find the area of the object with the most overlap 
            target_area = self.id_to_area[t_ids[np.argmax(t_counts)]]
            
            # Find overlap area
            correct_area = np.max(t_counts)
     
            um_score += (np.double(target_area - correct_area) * correct_area) / target_area

            
        # Sort the target ID map for faster pixel retrieval
        sorted_ids = t_map.argsort()
        id_set = np.unique(t_map)
        id_set.sort()
        
        # Get the locations in sorted_ids of the matching pixels
        right_indices = np.searchsorted(t_map, id_set, side='right', sorter=sorted_ids)
        left_indices = np.searchsorted(t_map, id_set, side='left', sorter=sorted_ids)
        
        # OVER-MERGING (modified) - match to detection map
        for n in range(len(id_set)):
            # Find the target labels of the pixels with detection label n
            detection_vals = d_map[sorted_ids[left_indices[n]:right_indices[n]]]
            
            # Find the number of pixels with each ID in the target map
            d_ids, d_counts = np.unique(detection_vals, return_counts=True)

            # Find the area of the object with the most overlap 
            detection_area = d_id_to_area[d_ids[np.argmax(d_counts)]]
            
            # Find overlap area
            correct_area = np.max(d_counts)
     
            om_score += (np.double(detection_area - correct_area) * correct_area) / detection_area

        # Calculate over-merging score        
        print("OUTSTAT Undermerging score:", um_score/ detection_map.size)
        
        # Calculate over-merging score        
        print("OUTSTAT Overmerging score:", om_score/ detection_map.size)
        
        total_score = np.sqrt((om_score **2) + (um_score ** 2)) / detection_map.size
        print("OUTSTAT Total area score:", total_score)
        
        return um_score/detection_map.size, om_score/detection_map.size, total_score

    
    def bg_distribution_test(self, detection_map):
        """Calculate the properties of the background pixels of the image."""
        bg_id = detection_map.min()
        bg_pixels = self.original_img[np.where(detection_map == bg_id)].ravel()
           
        s, p = stats.skewtest(bg_pixels)
        k, p = stats.kurtosistest(bg_pixels)
        bg_mean = bg_pixels.mean()
        
        print("OUTSTAT Background skew score:", s)
        print("OUTSTAT Background kurtosis score:", k)
        print("OUTSTAT Background mean score:", bg_mean)
        
        return s, k, bg_mean

        
    def get_p_score(self, detection_map, opt_on=2):
        """Calculate all the scores for an image, and return them (+ one to optimise on)"""
            
        # Match detected ids to target ids
        det_to_target = self.match_to_bp_list(detection_map)

        # F score
        f_score, tp, fp, fn = self.get_basic_stats(detection_map, det_to_target)
        print(tp, fp, fn)

        return 0

        # Area score
        um, om, area_score = self.get_merging_scores(detection_map)
        
        # Background skew score
        s, k, bg_mean = self.bg_distribution_test(detection_map)
        
        # Combined scores
        combined_one = np.sqrt(((1-f_score) **2) + (area_score **2))
        combined_two = 1 - np.cbrt((1-om) * (1-um) * (f_score))
        
        # print("OUTSTAT Combined one:", combined_one)
        # print("OUTSTAT Combined two:", combined_two)
        
        eval_stats = [tp, fp, fn, f_score, um, om, area_score, s, k, bg_mean, combined_one, combined_two] 

        # Optimise on f_score
        if opt_on == 0:
            return 1 - f_score, eval_stats

        # Optimise on area
        elif opt_on == 1:
            return area_score, eval_stats
        
        # Optimise on something
        elif opt_on == 2:    
            return combined_one, eval_stats
            
        # Optimise on something else
        elif opt_on == 3:
            return combined_two, eval_stats
            
            
    def eval_file(self, fn, opt_method=0, index=0):
        """Evaluate an image file."""
            
        # Open file
        eval_img = c_io.read_fits_image(fn, index)

        # Evaluate output fits file
        return self.get_p_score(eval_img, opt_method)

   
if __name__ == "__main__":
    eve = Evaluator(sys.argv[1], sys.argv[2])
    out = eve.eval_file(sys.argv[3])
    
    
