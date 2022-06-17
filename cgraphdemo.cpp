//Copyright (C) 2014, Benoît Naegel <b.naegel@unistra.fr>
//This program is free software: you can use, modify and/or
//redistribute it under the terms of the GNU General Public
//License as published by the Free Software Foundation, either
//version 3 of the License, or (at your option) any later
//version. You should have received a copy of this license along
//this program. If not, see <http://www.gnu.org/licenses/>.


#include <iostream>       // std::cout
#include <string>         // std::string
#include <cstddef>         // std::size_t
#include <algorithm>
#include <ctime>
#include <time.h>
#include <omp.h>
#include "Tile.h"
#include "cgraph.h"
#include "cgraphwatcher.h"
#include "colorordering.h"
extern "C" {
#include "fits_read.h"
#include "mto/background.h"
#include "mto/filter.h"
}

using namespace std;
// for LibTIM classes
using namespace LibTIM;

class graphWatcher : public CGraphWatcher {
public :
    void progressUpdate(int ncur, int nfinal) {
        // if(ncur%10000==0) {
        //     std::cout << ncur << " / " << nfinal << "\r";
        // }
    }

    void progressUpdate() {
        curProgress++;
        if (curProgress % 10000 == 0) {
            std::cout << this->curProgress << " / " << this->finalProgress << "\n";

        }
    }
    graphWatcher(int finalProgress) : CGraphWatcher(finalProgress) {}
};

std::vector<float> shared_mean = std::vector<float>(3, 0);
std::vector<float> shared_gain_vec = std::vector<float>(3, 0);
std::vector<float> shared_sigma2_bg_vec = std::vector<float>(3, 0);
std::vector<float> shared_soft_bias_vec = std::vector<float> (3, 0);
int c = 3;

void read_fits(const char *file_path, image &img, SHORT_TYPE &width, SHORT_TYPE &height, SHORT_TYPE &height_crop, SHORT_TYPE &width_crop, FLOAT_TYPE &gain, FLOAT_TYPE &bg_variance
               , FLOAT_TYPE bg_mean, FLOAT_TYPE &soft_bias, int fake, int is_crop, SHORT_TYPE start_i = -1, SHORT_TYPE start_j = -1) {
    //Try to read fits file
    PIXEL_TYPE *data;
    PIXEL_TYPE *data_crop;

    if (start_i >= 0) {
        std::cout << "Cropping: start i = " << start_i << " start_j = " << start_j << endl;
        if (!fits_read_crop_startij(file_path, &data, &data_crop, &width, &height, &width_crop, &height_crop, start_i, start_j)) {
            exit(1);
        }
    } else {
        if (is_crop) {
            if (!fits_read_crop(file_path, &data, &data_crop, &width, &height, &width_crop, &height_crop)) {
                exit(1);
            }
        } else {
            if (!fits_read(file_path, &data, &height, &width)) {
                exit(1);
            }
        }
        // // if (!fits_read(file_path, &data, &height, &width)){//fake_3,4 can be test here
        // if (!fits_read_crop(file_path, &data, &data_crop, &width, &height, &width_crop, &height_crop)){
        //     exit(1);
        // }
    }
    if (is_crop) {
        image_set(&img, data_crop, height_crop, width_crop);
    } else {
        image_set(&img, data, width, height);
    }
    // printf("Height = %d. Width = %d. Size = %d. \n", img.height, img.width, img.size);
    if (start_i >= 0) {
        //TODO: bg_truncate using shared bg
        bg_subtract(&img, shared_mean[fake], 0);
        bg_truncate(&img, 0);
        return; //skip bg info cal
    }

    //Default variables
    soft_bias = 0;
    gain = -1;
    int significance_test = 4;
    int verbosity_level = 0;
    FLOAT_TYPE move_factor = 0.5;
    FLOAT_TYPE min_distance = 0.0;
    FLOAT_TYPE img_min = 0;
    FLOAT_TYPE img_max = 0;

    bg_info(&img, &bg_mean, &bg_variance, verbosity_level);

    for (int i = 0; i < img.size; ++i) {
        if (img.data[i] < img_min) {
            img_min = img.data[i];
        }
        if (img.data[i] > img_max) {
            img_max = img.data[i];
        }
    }
    // Estimate gain from mean background, bias, and background variance
    if (gain < 0) {
        if (img_min < 0) {
            soft_bias = img_min;
        }
        gain = (bg_mean - soft_bias) / bg_variance;
        // gain = 1;

        if (bg_variance == 0) { //Totally flat case in fake_test_1_2 and fake_4
            gain = 1; bg_variance = 0.001 * (img_max - img_min);
        }
    }

    // Subtract background from image
    bg_subtract(&img, bg_mean, verbosity_level);
    bg_truncate(&img, verbosity_level);

    printf("> Estimates\n"
           "\n"
           "> img_min            : %.6E\n"
           "> img_max            : %.6E\n"
           "> soft_bias          : %.6E\n"
           "> Background mean    : %.6E\n"
           "> Background variance: %.6E\n"
           "> Gain               : %.6E electrons / ADU\n\n",
           img_min, img_max, soft_bias, bg_mean, bg_variance, gain);

    return ;
}

void read_fits_only(const char *file_path, image &img, SHORT_TYPE &width, SHORT_TYPE &height) {
    //Try to read fits file
    PIXEL_TYPE *data;
    if (!fits_read(file_path, &data, &height, &width)) {
        exit(1);
    }
    image_set(&img, data, width, height);
    printf("Height = %d. Width = %d. Size = %d. \n", img.height, img.width, img.size);
}

//Save original image as file in log scale
void export_one_band(image img, char* out_path) {
    //Cal max-min
    float img_min = img.data[0];
    float img_max = img.data[0];
    printf("Cal min of img = ");
    for (int i = 0; i < img.size; ++i) {
        if (img.data[i] < img_min) {
            img_min = img.data[i];
        }
        if (img.data[i] > img_max) {
            img_max = img.data[i];
        }
    }
    float scale = img_max - img_min;
    // printf("%e %e \n", img_min, img_max);
    // getchar();

    //Scale by max-min and log
    FILE *f = fopen(out_path, "wb");
    fprintf(f, "P6\n%i %i 255 \n", img.width, img.height);
    printf("%d %d \n", img.width, img.height);
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            float f_value = (img.data)[y * (img.width) + x];
            int value = (int)(log(f_value / scale));
            // printf("%e %i \n", f_value,  value);
            fputc(value, f);   // 0 .. 255
            fputc(value, f); // 0 .. 255
            fputc(value, f);  // 0 .. 255
        }
    }
    fclose(f);
}

void split_filename(const std::string& str, std::string& dir, std::string& file) {
    std::size_t found = str.find_last_of("/\\");
    dir = str.substr(0, found);
    file = str.substr(found + 1);
}
void get_out_name(const char* in, std::string& out) {
    std::string strin(in);
    std::string dir = "";
    std::string file = "";
    std::string nothing = "";
    split_filename(strin, dir, file);
    file = file.replace(file.begin(), file.begin(), "cgo_" );
    file = file.replace(file.end() - 5, file.end(), ".ppm" );
    out = dir + "/" + file;
}
void get_out_name_fits(const char* in, std::string& out) {
    std::string strin(in);
    std::string dir = "";
    std::string file = "";
    std::string nothing = "";
    split_filename(strin, dir, file);
    file = file.replace(file.begin(), file.begin(), "cgo_" );
    file = file.replace(file.end() - 5, file.end(), ".fits" );
    out = "!" + dir + "/" + file;
}
void get_out_name_csv(const char* in, std::string& out) {
    std::string strin(in);
    std::string dir = "";
    std::string file = "";
    std::string nothing = "";
    split_filename(strin, dir, file);
    file = file.replace(file.begin(), file.begin(), "cgo_" );
    file = file.replace(file.end() - 5, file.end(), ".csv" );
    out = dir + "/" + file;
}

void test_multi_bands(char* paths_fake_6[3], double alpha = pow(10, -6)) {
    std::vector<float> gain_vec = std::vector<float>(c, 0);
    std::vector<float> sigma2_bg_vec = std::vector<float>(c, 0);
    std::vector<float> soft_bias_vec = std::vector<float> (c, 0);
    SHORT_TYPE width;
    SHORT_TYPE height;
    SHORT_TYPE width_crop;
    SHORT_TYPE height_crop;
    std::vector<image> imgs;
    for (int i = 0; i < c; i++) {
        FLOAT_TYPE soft_bias = 0;
        FLOAT_TYPE bg_mean;
        FLOAT_TYPE bg_variance;
        FLOAT_TYPE gain = -1;
        image img;
        int is_crop = 0;
        // is_crop = 1;
        read_fits(paths_fake_6[i], img, height, width, height_crop, width_crop, gain, bg_variance, bg_mean, soft_bias, i, is_crop);

        if (!is_crop) {
            height_crop = height; width_crop = width; // ONLY FOR  fake_3 and CROP TEST ON /im1.fits/ path_arps.fits  ------------------------------
        }

        gain_vec[i] = gain;
        sigma2_bg_vec[i] = bg_variance;
        soft_bias_vec[i] = soft_bias;

        //Saved output image
        char str[100];
        sprintf(str, "read_input_%d.ppm", i);
        export_one_band(img, str);
        printf(" saved %s\n", str);

        imgs.push_back(img);
    }

    Image <RGB> imSrcF;//F for Float
    Image<RGB>::load_fits(imgs, imSrcF, height_crop, width_crop, c);
    FlatSE connexity;
    connexity.make2DN4();
    CGraph *cgraph = new CGraph(imSrcF, connexity, alpha = alpha);
    cgraph->set_bg_info(gain_vec, sigma2_bg_vec, soft_bias_vec);
    graphWatcher *myWatcher = new graphWatcher(imSrcF.getBufSize());
    ColorMarginalOrdering  *order = new ColorMarginalOrdering();

    clock_t tStart = clock();
    cgraph->computeGraph(order, myWatcher);
    printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
    // cgraph->areaFiltering(-1);
    // cgraph->writeDot("cgraph.dot");
    // std::cout<<"cgraph.dot writen \n";
    // return;

    // cgraph->cal_attribute_isect(order);
    // cgraph->cal_attribute_sn1(order);
    // cgraph->cal_attribute_sn2(order);

    // cgraph->cal_attribute_sn_combined_band(order);
    // cgraph->cal_attribute_sn_either_sn(order);
    // cgraph->cal_attribute_sn_both_sn(order);
    cgraph->cal_attribute_sn_all(order);

    // cgraph->cal_attribute_sn1_parallel(order);
    // cgraph->simplify_graph(order);

    std::vector<CGraph::Node *> objs = cgraph->find_object_merger(order);
    // std::vector<CGraph::Node *> objs = cgraph->find_object(order);
    // std::vector<CGraph::Node *> objs = cgraph->find_objectNEW(order, gain_vec, sigma2_bg_vec, 0.5);

    //Recheck SN on detected objects
    for (int i = 0; i < objs.size(); ++i) {
        cgraph->cal_sn_one_node(objs[i], order);
        if (objs[i]->object_id == -1) {
            objs.erase(objs.begin() + i);
            i = i - 1;
        }
    }

    // Move up
    cgraph->move_down_adaptive(objs, order, gain_vec, sigma2_bg_vec);
    float lambda = 0.5;//0.5
    objs = cgraph->move_down(objs, order, gain_vec, sigma2_bg_vec, lambda);
    std::cout << "After move_down " << objs.size() << endl;
    // getchar();

    Image<RGB> all_obj = cgraph->visual_all_object(objs);
    std::cout << "visual_all_object \n";
    std::string out = "";
    get_out_name(paths_fake_6[0], out);
    const char *cstr = out.c_str();
    all_obj.save(cstr);
    std::cout << "Saved " << cstr << endl;

    // cgraph->writeDotForHeatmap("heatmap/multi_cgraph_heatmap.dot");
    // return;


    Image<RGB> all_obj_ids = cgraph->visual_all_object_by_objid(objs);
    get_out_name_fits(paths_fake_6[0], out);
    const char *cstr2 = out.c_str();
    cgraph->export_fits(all_obj_ids, cstr2);//"!all_obj_.fits");
    std::cout << "Saved " << cstr2 << endl;

    get_out_name_csv(paths_fake_6[0], out);
    const char *cstr_csv = out.c_str();
    cgraph->writeObjectToCSV(cstr_csv, objs);

    // //Cout get_main_leave
    // for (int i = 0; i < objs.size(); i++) {
    //     objs[i]->get_main_leave(true);
    //     // getchar();
    // }

    // //Save objects individually
    // std::cout << "Saving single objects \n";
    // for (int i = 0; i < objs.size(); i++) {
    //     Image<RGB> one_obj = cgraph->visual_one_object(objs[i]);
    //     char str[100];
    //     sprintf(str, "refined_%d_%d.ppm", objs[i]->object_id, objs[i]->index);
    //     one_obj.save(str);
    //     std::cout << "saved " << str << endl;
    //     // objs[i]->print_all();
    //     // getchar();
    // }

    delete cgraph;
    delete order;
    for (int i = 0; i < c; ++i) {
        image_free(&imgs[i]);
    }
    return;

    // //Debugging
    // std::cout <<"Saving all nodes separately \n";
    // for (int i = 0; i < cgraph->graph.size(); i++){
    //     Image<RGB> one_obj = cgraph->visual_one_object(cgraph->graph[i]);
    //     char str[100];
    //     sprintf(str, "Node_%d_%d.ppm", i, cgraph->graph[i]->object_id);
    //     one_obj.save(str);
    // }

    //Export dot graph
    //Visualize main branch of objs.
    for (int i = 1; i < objs.size(); i++) {
        CGraph::Node* curNode = objs[i];
        int obj_id = curNode->object_id;
        int obj_id_index = 0;
        while (curNode->get_main_branch_area() != NULL) {
            //export as img
            Image<RGB> one_obj = cgraph->visual_one_object(curNode);
            char str[100];
            sprintf(str, "OBJ_%d_%d.ppm", obj_id, obj_id_index);
            obj_id_index++;
            std::cout << str << endl;
            one_obj.save(str);

            //moving node
            curNode = curNode->get_main_branch_area();
            // curNode->object_id = objs[i]->object_id;
        }
    }

    for (int i = 1; i < objs.size(); i++) {
        CGraph::Node* curNode = objs[i];
        int obj_id = curNode->object_id;
        int obj_id_index = 0;
        while (curNode->main_branch != NULL) {
            //export as img
            Image<RGB> one_obj = cgraph->visual_one_object(curNode);
            char str[100];
            sprintf(str, "OBJ_MB_%d_%d.ppm", obj_id, obj_id_index);
            obj_id_index++;
            std::cout << str << endl;
            one_obj.save(str);

            //moving node
            curNode = curNode->main_branch;
            // curNode->object_id = objs[i]->object_id;
        }
    }


    cgraph->writeDot("cgraph.dot");
    std::cout << "cgraph.dot writen \n";
    cgraph->writeDotFrom("cgraph_from.dot", 1205);
    std::cout << "cgraph_from.dot writen \n";
    cgraph->writeDotSN("cgraph_sn.dot");
    std::cout << "cgraph_sn.dot writen \n";

    //Free MEM
    delete cgraph;
    delete order;
    for (int i = 0; i < c; ++i) {
        image_free(&imgs[i]);
    }
    return;
}

void test_multi_band_compute_full(char* paths_fake_6[3], double alpha = pow(10, -6)) {
    // double alpha = pow(10, -5);// 6 5 4 3
    std::vector<float> gain_vec = std::vector<float>(c, 0);
    std::vector<float> sigma2_bg_vec = std::vector<float>(c, 0);
    std::vector<float> soft_bias_vec = std::vector<float> (c, 0);
    SHORT_TYPE width;
    SHORT_TYPE height;
    SHORT_TYPE width_crop;
    SHORT_TYPE height_crop;
    // char* paths_fake_6[3] = {"data/fake_6_2/band_1_fake_test_6_single_flux_0.3_noise_0.002.fits", "data/fake_6_2/band_1_fake_test_6_single_flux_0.3_noise_0.002.fits", "data/fake_6_2/band_2_fake_test_6_single_flux_0.3_noise_0.0022.fits"};
    std::vector<image> imgs;
    for (int i = 0; i < c; i++) {
        FLOAT_TYPE soft_bias = 0;
        FLOAT_TYPE bg_mean;
        FLOAT_TYPE bg_variance;
        FLOAT_TYPE gain = -1;
        image img;
        int is_crop;
        is_crop = 1;
        // is_crop = 0;
        read_fits(paths_fake_6[i], img, height, width, height_crop, width_crop, gain, bg_variance, bg_mean, soft_bias, i, is_crop);

        if (!is_crop) {
            height_crop = height; width_crop = width; // ONLY FOR  fake_3 and CROP TEST ON /im1.fits/ path_arps.fits  ------------------------------
        }

        gain_vec[i] = gain;
        sigma2_bg_vec[i] = bg_variance;
        soft_bias_vec[i] = soft_bias;

        //Saved output image
        char str[100];
        sprintf(str, "read_input_%d.ppm", i);
        export_one_band(img, str);
        printf(" saved %s\n", str);

        imgs.push_back(img);
    }

    Image <RGB> imSrcF;//F for Float
    Image<RGB>::load_fits(imgs, imSrcF, height_crop, width_crop, c);
    FlatSE connexity;
    connexity.make2DN4();
    CGraph *cgraph = new CGraph(imSrcF, connexity, alpha = alpha);
    cgraph->set_bg_info(gain_vec, sigma2_bg_vec, soft_bias_vec);
    graphWatcher *myWatcher = new graphWatcher(imSrcF.getBufSize());
    ColorMarginalOrdering  *order = new ColorMarginalOrdering();

    clock_t tStart = clock();
    cgraph->computeGraphFull(order, myWatcher);
    printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
    delete cgraph;
    return;

    // cgraph->areaFiltering(-1);
    // cgraph->writeDot("cgraph.dot");
    // std::cout<<"cgraph.dot writen \n";
    // return;

    // cgraph->cal_attribute_isect(order);
    cgraph->cal_attribute_sn1(order);
    // cgraph->simplify_graph(order);

    std::vector<CGraph::Node *> objs = cgraph->find_object_merger(order);
    // std::vector<CGraph::Node *> objs = cgraph->find_object(order);
    // std::vector<CGraph::Node *> objs = cgraph->find_objectNEW(order, gain_vec, sigma2_bg_vec, 0.5);
    // std::cout <<"Detected "<< objs.size() << " objects" << endl;
    // for (int i = 0; i < objs.size(); i++){
    //     std::cout << objs[i]->index << " mleave " << objs[i]->get_main_leave()<<endl;
    // }

    //Move up
    // float lambda = 0.5;//0.5
    // objs = cgraph->move_down(objs, order, gain_vec, sigma2_bg_vec, lambda);

    Image<RGB> all_obj = cgraph->visual_all_object(objs);

    std::string out = "";
    get_out_name(paths_fake_6[0], out);
    const char *cstr = out.c_str();
    all_obj.save(cstr);
    std::cout << "Saved " << cstr << endl;
    // all_obj.save("all_obj.ppm");

    Image<RGB> all_obj_ids = cgraph->visual_all_object_by_objid(objs);
    get_out_name_fits(paths_fake_6[0], out);
    const char *cstr2 = out.c_str();
    cgraph->export_fits(all_obj_ids, cstr2);//"!all_obj_.fits");
    std::cout << "Saved " << cstr2 << endl;

    //Save objects individually
    std::cout << "Saving single objects \n";
    for (int i = 0; i < objs.size(); i++) {
        Image<RGB> one_obj = cgraph->visual_one_object(objs[i]);
        char str[100];
        sprintf(str, "refined_%d_%d.ppm", objs[i]->object_id, objs[i]->index);
        one_obj.save(str);
        std::cout << "saved " << str << endl;
        objs[i]->print_all();
        // getchar();
    }

    // //Debugging
    // std::cout <<"Saving all nodes separately \n";
    // for (int i = 0; i < cgraph->graph.size(); i++){
    //     Image<RGB> one_obj = cgraph->visual_one_object(cgraph->graph[i]);
    //     char str[100];
    //     sprintf(str, "Node_%d_%d.ppm", i, cgraph->graph[i]->object_id);
    //     one_obj.save(str);
    // }

    //Export dot graph
    //Visualize main branch of objs.
    // for (int i = 0; i < objs.size(); i++){
    //     CGraph::Node* curNode = objs[i];
    //     while(curNode->get_main_branch_area() != NULL){
    //         curNode = curNode->get_main_branch_area();
    //         curNode->object_id = objs[i]->object_id;
    //     }
    // }

    cgraph->writeDot("cgraph.dot");
    std::cout << "cgraph.dot writen \n";
    cgraph->writeDotFrom("cgraph_from.dot", 1205);
    std::cout << "cgraph_from.dot writen \n";
    cgraph->writeDotSN("cgraph_sn.dot");
    std::cout << "cgraph_sn.dot writen \n";

    //Free MEM
    delete cgraph;
    return;
}

std::set<int> query_gt_pixels(const char *file_path) {
    std::set<int> set_gt;
    PIXEL_TYPE *data;
    SHORT_TYPE height;
    SHORT_TYPE width;
    if (!fits_read(file_path, &data, &height, &width)) {
        exit(1);
    }
    for (int i = 0; i < height * width; i++) {
        if (data[i] != -1) {
            set_gt.insert(i);
        }
    }
    // printf("Height = %d. Width = %d. gt_size = %d. \n", height, width, int(set_gt.size()));
    return set_gt;
}

void test_boundary(char* paths[3], char *path_gt, double alpha = pow(10, -6)) {

    //Graph for CG
    std::vector<float> gain_vec = std::vector<float>(c, 0);
    std::vector<float> sigma2_bg_vec = std::vector<float>(c, 0);
    std::vector<float> soft_bias_vec = std::vector<float> (c, 0);
    SHORT_TYPE width;
    SHORT_TYPE height;
    SHORT_TYPE width_crop;
    SHORT_TYPE height_crop;
    std::vector<image> imgs;
    for (int i = 0; i < c; i++) {
        FLOAT_TYPE soft_bias = 0;
        FLOAT_TYPE bg_mean;
        FLOAT_TYPE bg_variance;
        FLOAT_TYPE gain = -1;
        image img;
        int is_crop;
        // is_crop = 1;
        is_crop = 0;
        read_fits(paths[i], img, height, width, height_crop, width_crop, gain, bg_variance, bg_mean, soft_bias, i, is_crop);

        if (!is_crop) {
            height_crop = height; width_crop = width; // ONLY FOR  fake_3 and CROP TEST ON /im1.fots/ path_arps.fits  ------------------------------
        }

        gain_vec[i] = gain;
        sigma2_bg_vec[i] = bg_variance;
        soft_bias_vec[i] = soft_bias;
        imgs.push_back(img);
    }
    Image <RGB> imSrcF;//F for Float
    Image<RGB>::load_fits(imgs, imSrcF, height_crop, width_crop, c);
    FlatSE connexity;
    connexity.make2DN4();
    CGraph *cgraph = new CGraph(imSrcF, connexity, alpha = alpha);
    cgraph->set_bg_info(gain_vec, sigma2_bg_vec, soft_bias_vec);
    graphWatcher *myWatcher = new graphWatcher(imSrcF.getBufSize());
    ColorMarginalOrdering  *order = new ColorMarginalOrdering();
    cgraph->computeGraph(order, myWatcher);

    //Graph for ct
    paths[0] = "data/sim2/snr_-0.93/band_2_fake_test_6_snr_-0.93_i_0_ori.fits";
    paths[1] = "data/sim2/snr_-0.93/band_2_fake_test_6_snr_-0.93_i_0_ori.fits";
    paths[2] = "data/sim2/snr_-0.93/band_2_fake_test_6_snr_-0.93_i_0_ori.fits";

    std::vector<image> ct_imgs;
    for (int i = 0; i < c; i++) {
        FLOAT_TYPE soft_bias = 0;
        FLOAT_TYPE bg_mean;
        FLOAT_TYPE bg_variance;
        FLOAT_TYPE gain = -1;
        image img;
        int is_crop;
        is_crop = 0;
        read_fits(paths[i], img, height, width, height_crop, width_crop, gain, bg_variance, bg_mean, soft_bias, i, is_crop);
        if (!is_crop) {
            height_crop = height; width_crop = width; // ONLY FOR  fake_3 and CROP TEST ON /im1.fots/ path_arps.fits  ------------------------------
        }
        gain_vec[i] = gain;
        sigma2_bg_vec[i] = bg_variance;
        soft_bias_vec[i] = soft_bias;
        ct_imgs.push_back(img);
    }
    Image <RGB> ct_imSrcF;//F for Float
    Image<RGB>::load_fits(ct_imgs, ct_imSrcF, height_crop, width_crop, c);
    FlatSE ct_connexity;
    ct_connexity.make2DN4();
    CGraph *ct_cgraph = new CGraph(ct_imSrcF, ct_connexity, alpha = alpha);
    ct_cgraph->set_bg_info(gain_vec, sigma2_bg_vec, soft_bias_vec);
    ct_cgraph->computeGraph(order, myWatcher);

    //Compare
    //Find top similar node score
    std::set<int> set_pixels_gt = query_gt_pixels(path_gt);
    // double top_sim_score = cgraph->top_similar_node(set_pixels_gt, false);
    // top_sim_score = ct_cgraph->top_similar_node(set_pixels_gt, false);
    // std::cout << int(1000*top_sim_score) << endl;
    cgraph->compare_two_graph(ct_cgraph);

    //Free MEM
    delete cgraph;
    delete ct_cgraph;

    return;
}

Image<RGB> test_multi_bands_tiles(char* paths[3], SHORT_TYPE &start_i, SHORT_TYPE &start_j, SHORT_TYPE &width_crop, SHORT_TYPE &height_crop, int index_i = -1, int index_j = -1, int is_crop = 1) {
    std::vector<float> gain_vec = shared_gain_vec;
    std::vector<float> sigma2_bg_vec = shared_sigma2_bg_vec;
    std::vector<float> soft_bias_vec = shared_soft_bias_vec;
    SHORT_TYPE width;
    SHORT_TYPE height;
    std::vector<image> imgs;
    std::cout << "before reading fits" << endl;
    #pragma omp critical
    {
        for (int i = 0; i < c; i++) {
            FLOAT_TYPE soft_bias = 0;
            FLOAT_TYPE bg_mean;
            FLOAT_TYPE bg_variance;
            FLOAT_TYPE gain = -1;
            image img;
            read_fits(paths[i], img, height, width, height_crop, width_crop, gain, bg_variance, bg_mean, soft_bias, i, is_crop, start_i, start_j);
            // Filter image
            // image mask;
            // mask = gaussian_filter(3, 3, FWHM_TO_SIGMA(2), 2);
            // image filtered;
            // filtered = filter(&img, &mask, 2);
            // image_free(&mask);
            // std::cout << "Done filter " << endl;
            if (i == 0) {
                char str[100];
                sprintf(str, "all_export_%d_%d.ppm", start_i, start_j);
                export_one_band(img, str);
                // sprintf(str, "all_export_%d_%d_filtered.ppm", start_i, start_j);
                // export_one_band(filtered, str);
            }
            imgs.push_back(img);
        }
    }

    Image <RGB> imSrcF;//F for Float
    Image<RGB>::load_fits(imgs, imSrcF, height_crop, width_crop, c);
    FlatSE connexity;
    connexity.make2DN4();
    CGraph *cgraph = new CGraph(imSrcF, connexity);
    cgraph->set_bg_info(gain_vec, sigma2_bg_vec, soft_bias_vec);
    graphWatcher *myWatcher = new graphWatcher(imSrcF.getBufSize());
    ColorMarginalOrdering  *order = new ColorMarginalOrdering();

    cgraph->computeGraph(order, myWatcher);
    cgraph->cal_attribute_sn1(order);
    // std::vector<CGraph::Node *> objs = cgraph->find_object(order);
    std::vector<CGraph::Node *> objs = cgraph->find_object_merger(order);

    //Save original segmentation as ppm and fits files
    std::cout << "found " << objs.size() << endl;
    Image<RGB> all_obj = cgraph->visual_all_object(objs);
    char str[100];
    if (index_i >= 0) {
        sprintf(str, "Tile_%d_%d.ppm", index_i, index_j);
    } else {
        sprintf(str, "Tile_%d_%d_%d.ppm", start_i, start_j, int(objs.size()));
    }
    all_obj.save(str);
    Image<RGB> all_obj_ids = cgraph->visual_all_object_by_objid(objs);
    sprintf(str, "!all_obj_%d_%d.fits", start_i, start_j);
    cgraph->export_fits(all_obj_ids, str);

    //Move up and save segmentation as files
    float lambda = 0.5;//0.5 meaningless since we use adaptive lambda inside move_down()
    objs = cgraph->move_down(objs, order, gain_vec, sigma2_bg_vec, lambda);

    all_obj = cgraph->visual_all_object(objs);
    if (index_i >= 0) {
        sprintf(str, "moved_Tile_%d_%d.ppm", index_i, index_j);
    } else {
        sprintf(str, "moved_Tile_%d_%d_%d.ppm", start_i, start_j, int(objs.size()));
    }
    all_obj.save(str);
    all_obj_ids = cgraph->visual_all_object_by_objid(objs);
    sprintf(str, "!moved_all_obj_%d_%d.fits", start_i, start_j);
    cgraph->export_fits(all_obj_ids, str);

    //Free MEM and return
    delete cgraph;
    delete order;
    for (int i = 0; i < c; ++i) {
        image_free(&imgs[i]);
    }
    return all_obj_ids;
}

//Update gain accoding to img_min
void estimate_gain(image &img, FLOAT_TYPE &gain, FLOAT_TYPE &soft_bias, FLOAT_TYPE &bg_mean, FLOAT_TYPE &bg_variance) {
    FLOAT_TYPE img_min = 0;
    for (int i = 0; i < img.size; i++) {
        if (img.data[i] < img_min) {
            img_min = img.data[i];
        }
    }
    if (img_min < 0) {
        soft_bias = img_min;
    }
    gain = (bg_mean - soft_bias) / bg_variance;
    return;
}

//Calculate shared BG values for all patchs
void cal_shared_bg(char *paths[3]) {
    SHORT_TYPE width;
    SHORT_TYPE height;
    SHORT_TYPE width_crop;
    SHORT_TYPE height_crop;
    for (int i = 0; i < c; i++) {
        FLOAT_TYPE soft_bias = 0;
        FLOAT_TYPE bg_mean;
        FLOAT_TYPE bg_variance;
        FLOAT_TYPE gain = -1;
        image img;
        int is_crop = 0;
        read_fits(paths[i], img, height, width, height_crop, width_crop, gain, bg_variance, bg_mean, soft_bias, i, is_crop);
        image_free(&img);
        if (!is_crop) {
            height_crop = height; width_crop = width;
        }
        shared_mean[i] = bg_mean;
        //Estimate gain following mto update
        // estimate_gain(img, gain, soft_bias, bg_mean, bg_variance);

        shared_gain_vec[i] = gain;
        shared_sigma2_bg_vec[i] = bg_variance;
        shared_soft_bias_vec[i] = soft_bias;
        std::cout << "Reading " << paths[i] << " size " << height << " " << width << endl;
        printf("> Estimates\n"
               "\n"
               "> Background mean    : %.6E\n"
               "> Background variance: %.6E\n"
               "> Gain               : %.6E electrons / ADU\n\n",
               bg_mean, bg_variance, gain);
    }
    // getchar();
}

void read_tile(char* path, Tile &tile) {
    std::cout << "reading " << path << endl;
    SHORT_TYPE width;
    SHORT_TYPE height;
    image img;
    int is_crop = 0;
    read_fits_only(path, img, height, width);
    // read_fits(path, img, height, width, height_crop, width_crop, gain, bg_variance, bg_mean, soft_bias, 0, is_crop);
    Image <RGB> imSrcF;//F for Float
    Image<RGB>::load_fits(img, imSrcF, height, width, c);
    image_free(&img);
    tile = Tile(imSrcF, tile.start_i, tile.start_j, tile.overlap);
    //MTO output bg as -1, this function turn this negative to zero
    tile.turn_negative_to_zero();
}

void scan_tile_all() {
    //Init slicing parameters
    SHORT_TYPE overlap = 250; //overlap %
    SHORT_TYPE window_h = 500; //300 - 900
    SHORT_TYPE window_w = 500; // 900 1100
    SHORT_TYPE step_w = window_w - overlap;
    SHORT_TYPE step_h = window_h - overlap;
    SHORT_TYPE full_w, full_h;

    //Read size of full fits
    PIXEL_TYPE *data;
    if (!fits_read_size("/Volumes/DATA/BladeESIEE/cluster1_g.fits", &data, &full_w, &full_h)) {
        exit(1);
    }
    data = NULL;

    //Process tile by tile
    SHORT_TYPE no_tile_h = (full_h - window_h) / step_h + 1;
    SHORT_TYPE no_tile_w = (full_w - window_w) / step_w + 1;

    int size_vec = (no_tile_w) * (no_tile_h);
    std::vector<Tile> tile_vec = std::vector<Tile> (size_vec);
    std::vector<int> processed = std::vector<int> (size_vec, 0);

    //READ TILE_i FROM FILES
    for (int i = 15; i < 23; ++i) {
        for (int j = 15; j < 24; ++j) {
            // for (int i = 15; i < 17; ++i){
            //     for (int j = 15; j < 17; ++j){
            //Set up correct index and names
            SHORT_TYPE start_i = i * step_w;
            SHORT_TYPE start_j = j * step_h;
            char fits_name[100];
            sprintf(fits_name, "data/tiles/moved_all_obj_%d_%d.fits", start_i, start_j); //FOR CGO
            // sprintf(fits_name, "/Volumes/DATA/BladeESIEE/mto_cluster_i_overlap/moved_mto_cluster1_i_%d_%d.fits", start_i, start_j);    //FOR MTO
            // sprintf(fits_name, "data/tiles/all_obj_%d_%d.fits", start_i, start_j);

            //Read file
            Tile tile_i = Tile(0, 0, 0, start_i, start_j, overlap);
            read_tile(fits_name, tile_i);
            tile_i.export_fits();

            //Save to a vector
            tile_vec[j * (no_tile_w) + i] = tile_i;
            processed[j * (no_tile_w) + i] = 1;
            // std::cout << start_i << " " << start_j << " done \n";
            // std::cout << "-------------------------------------\n";
            // getchar();
        }
    }

    //MERGE
    Tile tile_full(full_w, full_h);
    for (int i = 0; i < processed.size(); i++) {
        if (processed[i]) {
            std::cout << processed[i] <<  "---------MERGING TILE " << i << endl;
            char fits_name[100];
            sprintf(fits_name, "tile_vec_i_%d.ppm", i);
            tile_vec[i].visualize(fits_name);
            tile_full.merge_tile(tile_vec[i]);
            // getchar();
        }
    }
    tile_full.reverse();
    tile_full.export_fits(1, false);
    tile_full.visualize("Tile_full_colors.ppm");
}

//Merge a list of tiles -> full_tile
void merge_tile_all(SHORT_TYPE window_w, SHORT_TYPE window_h, SHORT_TYPE overlap, SHORT_TYPE full_w, SHORT_TYPE full_h, std::vector<Tile> &tile_vec, std::vector<int> &processed) {
    SHORT_TYPE step_w = window_w - overlap;
    SHORT_TYPE step_h = window_h - overlap;

    SHORT_TYPE no_tile_h = (full_h - window_h) / step_h + 1;
    SHORT_TYPE no_tile_w = (full_w - window_w) / step_w + 1;

    Tile tile_full(full_w, full_h);
    int size_vec = (no_tile_w) * (no_tile_h);

    // for (int i = 23; i < 24; i++) { //1500 -> 1800
    //     for (int j = 20; j < 21; j++) { // 1500 -> 1800
    //         int id = omp_get_thread_num();
    //         int total = omp_get_num_threads();
    //         printf("Greetings from process %d out of %d \n", id, total);
    //         std::cout << i << " " << j << endl;
    //         SHORT_TYPE start_i = i * step_w;
    //         SHORT_TYPE start_j = j * step_h;
    //         Image<RGB> all_obj_ids = test_multi_bands_tiles(paths, start_i, start_j, window_w, window_h, i, j);
    //         Tile tile_i(all_obj_ids, start_i, start_j, overlap); //push into a set
    //         tile_i.export_fits();
    //         tile_vec[j * (no_tile_w) + i] = tile_i;
    //         processed[j * (no_tile_w) + i] = 1;
    //         std::cout << start_i << " " << start_j << " done \n";
    //         std::cout << "-------------------------------------\n";
    //         // getchar();
    //     }

    //Merge
    for (int i = 0; i < processed.size(); i++) {
        if (processed[i]) {
            std::cout << processed[i] <<  "---------MERGING TILE " << i << endl;
            tile_full.merge_tile(tile_vec[i]);
        }
    }
    tile_full.reverse();
    tile_full.export_fits(1, false);
    tile_full.visualize("Tile_full_colors.ppm");
}

//Multi-tile parallel processing
void multi_tiles_process(char *paths[3]) {

    //1. Init division parameters
    SHORT_TYPE overlap = 100; //overlap %
    SHORT_TYPE window_h = 200; //300 - 900
    SHORT_TYPE window_w = 200; // 900 1100
    SHORT_TYPE step_w = window_w - overlap;
    SHORT_TYPE step_h = window_h - overlap;
    SHORT_TYPE full_w, full_h;

    //2. Read size of full fits
    PIXEL_TYPE *data;
    if (!fits_read_size(paths[0], &data, &full_w, &full_h)) {
        exit(1);
    }
    //3. Cal shared bg for later Graph computation
    cal_shared_bg(paths);

    //4. Process tile by tile
    SHORT_TYPE no_tile_h = (full_h - window_h) / step_h + 1;
    SHORT_TYPE no_tile_w = (full_w - window_w) / step_w + 1;
    std::cout << "full_w=" << full_w << " full_h=" << full_h << endl;
    std::cout << no_tile_w << " " << no_tile_h << endl;

    Tile tile_full(full_w, full_h);
    int size_vec = (no_tile_w) * (no_tile_h);
    std::vector<Tile> tile_vec = std::vector<Tile> (size_vec);
    std::vector<int> processed = std::vector<int> (size_vec, 0);
    int num_thread = omp_get_num_threads();
    std::cout << "num_thread = " << num_thread << " max_num_thread = " << omp_get_max_threads() << endl;

    #pragma omp parallel num_threads(1) shared(tile_vec, processed)
    {
        // //Test omp working
        // int id = omp_get_thread_num();
        // int total = omp_get_num_threads();
        // for (int i = 0; i < 100; ++i){
        //     printf("Greetings from process %d out of %d, i = %d \n", id, total, i);
        // }
        // std::cout << "-------------------------------------\n";

        // for (int i = 0; i < no_tile_w; i++){
        //     for (int j = 0; j < no_tile_h; j++){
        #pragma omp for collapse(2)
        for (int i = 30; i < 31; i++) { //3000 -> 3300
            for (int j = 30; j < 31; j++) { //
                int id = omp_get_thread_num();
                int total = omp_get_num_threads();
                printf("Greetings from process %d out of %d \n", id, total);
                std::cout << i << " " << j << endl;
                SHORT_TYPE start_i = i * step_w;
                SHORT_TYPE start_j = j * step_h;
                Image<RGB> all_obj_ids = test_multi_bands_tiles(paths, start_i, start_j, window_w, window_h, i, j);
                Tile tile_i(all_obj_ids, start_i, start_j, overlap); //push into a set
                tile_i.export_fits();
                tile_vec[j * (no_tile_w) + i] = tile_i;
                processed[j * (no_tile_w) + i] = 1;
                std::cout << start_i << " " << start_j << " done \n";
                std::cout << "-------------------------------------\n";
                // getchar();
            }
        }
    }
    // return;
    // SHORT_TYPE start_i = 200;
    // SHORT_TYPE start_j = 800;
    // Image<RGB> all_obj_ids = test_multi_bands_tiles(paths, start_i, start_j, window_w, window_h);
    // Tile tile_i(all_obj_ids, start_i, start_j, overlap); //push into a set
    // tile_i.export_fits();
    // tile_vec.push_back(tile_i);

    // start_i = 600;
    // start_j = 800;
    // Image<RGB> all_obj_ids_2 = test_multi_bands_tiles(paths, start_i, start_j, window_w, window_h);
    // Tile tile_i_2(all_obj_ids_2, start_i, start_j, overlap); //push into a set
    // tile_i_2.export_fits();
    // tile_vec.push_back(tile_i_2);

    // start_i = 1000;
    // start_j = 800;
    // Image<RGB> all_obj_ids_3 = test_multi_bands_tiles(paths, start_i, start_j, window_w, window_h);
    // Tile tile_i_3(all_obj_ids_3, start_i, start_j, overlap); //push into a set
    // tile_i_3.export_fits();
    // tile_vec.push_back(tile_i_3);


    // start_i = 600;
    // start_j = 1000;
    // Image<RGB> all_obj_ids_4 = test_multi_bands_tiles(paths, start_i, start_j, window_w, window_h);
    // Tile tile_i_4(all_obj_ids_4, start_i, start_j, overlap); //push into a set
    // tile_i_4.export_fits();
    // tile_vec.push_back(tile_i_4);

    //Merge
    for (int i = 0; i < processed.size(); i++) {
        if (processed[i]) {
            std::cout << processed[i] <<  "---------MERGING TILE " << i << endl;
            tile_full.merge_tile(tile_vec[i]);
        }
    }
    tile_full.reverse();
    tile_full.export_fits(1, false);
    tile_full.visualize("Tile_full_colors.ppm");

}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout << "Usage: " << argv[0] << " <band_1.fits> <band_2> <band_3> <alpha option> <gt_path option>\n";
        exit(1);
    }
    char* paths[3];
    for (int i = 0; i < 3; i++) {
        paths[i] = argv[i + 1];
        // std::cout << paths[i] << endl;
    }
    if (argc == 6) { //gt_path is the 5th arg
        test_boundary(paths, argv[5]);
        // multi_tiles_process(paths);
        exit(1);
    }
    if (argc == 5) { //alpha is the 4th arg
        double alpha = atof(argv[4]);
        test_multi_bands(paths, alpha);
        // multi_tiles_process(paths);
        exit(1);
    }
    if (argc == 4) {
        test_multi_bands(paths);

        // test_multi_band_compute_full(paths);
        // multi_tiles_process(paths);

        // scan_tile_all();
    }
    exit(1);
}