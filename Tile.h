//Tile cclass cpp
#include "Image.h"
#include "fitsio.h"
#include <algorithm>
using namespace LibTIM;
// using namespace std;


class Tile {
public:

	//Variables
	int last_index = 0;
	float sim_threshold = 0.3;
	int h;
	int w;
	int start_i;//col//w
	int start_j;//row//h
	int overlap;
	int no_colors;
	Image <int> img;
	std::set<int> set_l_b;
	std::map<int, int> id_to_area = std::map<int, int>();

	//Default constructor
	Tile() {
		//It's neccessary when initialize a fixed size vector of Tile.
	}

	Tile(int w, int h, int value = 0, int start_i = 0, int start_j = 0, int overlap = 0) {
		// std::cout << "Constructor" << endl;
		this->img = Image <int>(w, h, 1);
		this->w = img.getSizeX();
		this->h = img.getSizeY();
		this->start_i = start_i;
		this->start_j = start_j;
		this->overlap = overlap;
		this->img.fill(value);
		// this->print_tile();
		this->print_info();
	}

	Tile(Image<RGB> rgb, int start_i = 0, int start_j = 0, int overlap = 0) {
		this->w = rgb.getSizeX();
		this->h = rgb.getSizeY();
		this->start_i = start_i;
		this->start_j = start_j;
		this->overlap = overlap;
		this->img = Image <int>(w, h, 1);
		//Copy data
		this->no_colors = 0;
		for (int i = 0; i < w; i++) {
			for (int j = 0; j < h; j++) {
				img(i, j) = rgb(i, j)[0];
				if (img(i, j) > no_colors){
					this->no_colors = img(i,j);
				}
			}
			// std::cout << endl;
		}
		this->set_l_b = this->gen_lelf_bottom();
		this->id_to_area = this->gen_id_to_area();
		// this->print_tile();
		this->print_info();
	}

	~Tile() {
		// std::cout << "Calling deconstructor of Tile start_i " << start_i << " start_j " << start_j << endl;
		// delete img;
	}

    //MTO output bg as -1, this function turn this negative to zero
	void turn_negative_to_zero(){
		for (int i = 0; i < this->w; i++) {
			for (int j = 0; j < this->h; j++) {
				if (img(i, j) < 0){
					img(i, j) = 0;
				}
			}
		}		
	}

	void fill_img(int value) {
		this->img.fill(value);
	}

	void reverse() {
		for (int i = 0; i < w; i++) {
			for (int j = 0; j < h; j++) {
				img(i, j) = - img(i, j);
			}
		}
	}

	void print_info() {
		std::cout << "TILE info: w = " << w << " h = " << h  << " start_i = " << start_i << " start_j=" << start_j << " overlap=" << overlap << endl;
		std::cout << "img.getSizeX = " << img.getSizeX() << " img.getSizeY = " << img.getSizeY() << endl;
	}

	void print_tile() {
		for (int i = 0; i < w; i++) {
			for (int j = 0; j < h; j++) {
				std::cout << img(i, j) << " ";
			}
			std::cout << endl;
		}
		this->print_info();
	}

	//Extract all object that touch left_bottom border
	std::set<int> gen_lelf_bottom() {
		std::set<int> set_l_b;
		//Bottom border check
		for (int i = 0; i < this->w; i++) {
			if (this->img(i, 0) != 0) {
				set_l_b.insert(this->img(i, 0));
			}
		}

		//Left border check
		for (int i = 0; i < this->h; i++) {
			if (this->img(0, i) != 0) {
				set_l_b.insert(this->img(0, i));
			}
		}

		//Cout test
		std::cout << "left_bottom border set: \n";
		for (int i : set_l_b) {
			std::cout << i << "; ";
		}
		std::cout << endl;

		return set_l_b;
	}

	std::map<int, int> gen_id_to_area() {
		std::map<int, int> maps;
		for (int i = 0; i < this->w; ++i) {
			for (int j = 0; j < this->h; ++j) {
				int id = this->img(i, j);
				//Skip bg pixels
				if (id == 0) {
					continue;
				}
				//Counting
				if (maps.count(id) == 0) {
					maps[id] = 1;
				} else {
					maps[id] += 1;
				}

			}
		}
		maps[0] = 0;
		// for (auto elem: maps){
		// 	std::cout << elem.first << " " << elem.second << "\n";
		// }
		// std::cout << "End get id to area \n";
		// getchar();
		return maps;
	}

	int export_fits(int scale = 1, bool crop = false) {
		char fits_name[100];
		sprintf(fits_name, "!Tile_%d_%d_%d_%d.fits", this->start_i, this->start_j, w, h);
		// std::cout << "Start export_fits " << fits_name << " w= " << w << " h= "<<h << endl;
		int s_h, s_w;
		if (crop) {
			int crop_w = 1000;
			int crop_h = 4000;
			s_h = crop_h / scale;
			s_w = crop_w / scale;
		} else {
			s_h = this->h / scale;
			s_w = this->w / scale;
		}
		fitsfile *fptr;       /* pointer to the FITS file; defined in fitsio.h */
		int status, ii, jj;
		long  fpixel = 1, naxis = 2, nelements;
		long naxes[2] = { s_w, s_h };
		// short array[s_h][s_w]; //Limited by stacked size
		short *new_array = new short[s_h * s_w]; //Dynamic allocation to avoid segmentation fault stacked size limit
		status = 0;         /* initialize status before calling fitsio routines */
		fits_create_file(&fptr, fits_name, &status);   /* create new file */
		/* Create the primary array image (16-bit short integer pixels */
		fits_create_img(fptr, SHORT_IMG, naxis, naxes, &status);
		for (jj = 0; jj < naxes[1]; jj++) { //Iterate over s_h
			for (ii = 0; ii < naxes[0]; ii++) { //Iter over s_w
				// array[jj][ii] = this->img(ii*scale, jj*scale);
				new_array[jj * s_w + ii] = this->img(ii * scale, jj * scale);
			}
		}
		nelements = naxes[0] * naxes[1];          /* number of pixels to write */
		/* Write the array of integers to the image */
		// fits_write_img(fptr, TSHORT, fpixel, nelements, array[0], &status);
		fits_write_img(fptr, TSHORT, fpixel, nelements, new_array, &status);
		fits_close_file(fptr, &status);            /* close the file */
		fits_report_error(stderr, status);  /* print out any error messages */
		delete[] new_array;
		std::cout << "export_fits " << fits_name << endl;
		return ( status );
	}
	void visualize(char* filename) {
		//1. Init colors
		std::vector<RGB> rgbs;
		if (this->last_index != 0){//Only for full tile
			this->no_colors = abs(this->last_index);
		}
		srand(1);//fix the seed
		for (int i = 0; i < this->no_colors; i++) {
			rgbs.push_back(RGB(rand() % 255, rand() % 255, rand() % 255));
		}
		rgbs[0] = RGB(0, 0, 0);


		//2. Visualize
		Image<RGB> imRes = Image <RGB>(this->w, this->h, 3);
		imRes.fill(0);
		//replace id by a random color
		for (int i = 0; i < this->w; i++) {
			for (int j = 0; j < this->h; j++) {
				imRes(i, j) = rgbs[this->img(i, j)];
			}
		}
		imRes.save(filename);
		std::cout << "visualize full done \n" ;
	}

	//Get id_index of i,j in full TILE img.
	int get_1d_index(int i, int j, int absolute_w) {
		return absolute_w * j + i;
	}

	//Expanding id ONLY in OVERLAP region, return set of pixels in FULL TILE 1d_index
	set<int> expand_region_id_full(int id, Tile tile) {
		//TODO: Using queue will be more efficient
		set<int> pixels;

		//FULL: tile.start_i -> tile.start_i + overlap
		for (int i = tile.start_i; i < (tile.start_i + tile.overlap); i++) {
			for (int j = tile.start_j; j < (tile.start_j + tile.h); j++) {
				if (this->img(i, j) == id) {
					pixels.insert(this->get_1d_index(i, j, this->w));
				}
			}
		}

		//FULL: tile.start_j -> tile.start_j + overlap
		for (int i = tile.start_i; i < (tile.start_i + tile.w); i++) {
			for (int j = tile.start_j; j < (tile.start_j + tile.overlap); j++) {
				if (this->img(i, j) == id) {
					pixels.insert(this->get_1d_index(i, j, this->w));
				}
			}
		}

		return pixels;
	}

	//Expanding id ONLY in OVERLAP region, return set of pixels in FULL TILE 1d_index
	set<int> expand_region_id_tile(int id, int absolute_w) {
		//TODO: Using queue will be more efficient
		set<int> pixels;

		//Tile: 0 -> (this->overlap) vertical
		for (int i = 0; i < this->overlap; i++) {
			for (int j = 0; j < this->h; j++) {
				if (this->img(i, j) == id) {
					pixels.insert(this->get_1d_index(i + this->start_i, j + this->start_j, absolute_w));
				}
			}
		}

		//Tile: 0 -> (this->overlap) horizontal
		for (int i = 0; i < this->w ; i++) {
			for (int j = 0; j < this->overlap; j++) {
				if (this->img(i, j) == id) {
					pixels.insert(this->get_1d_index(i + this->start_i, j + this->start_j, absolute_w));
				}
			}
		}

		return pixels;
	}

	int next_index() {
		this->last_index -= 1;
		return this->last_index;
	}

	void copy_n_reindex_old(Tile &tile, std::map<int, int> map = std::map<int, int>()) {

		//Copy tile into FullTile using map({tile_id:full_id})
		//1. In nonoverlaping, just copy
		//2. In overlaping,


		//map from tile ids to THIS decreasing id (from last_index)
		int start_point_i = 0;
		int start_point_j = 0;
		if (tile.start_i != 0) {
			start_point_i = tile.overlap;
		}
		if (start_point_i != 0) {
			start_point_j = tile.overlap;
		}
		for (int i = start_point_i; i < tile.w; i++) {
			for (int j = start_point_j; j < tile.h; j++) {
				//bg
				if (tile.img(i, j) == 0) {
					continue;
				}
				//add mapping for new object in tile
				if (!map.count(tile.img(i, j))) {
					map[tile.img(i, j)] = this->next_index();
				}
				//Reindex
				this->img(tile.start_i + i, tile.start_j + j) = map[tile.img(i, j)];
			}
		}
		std::cout << "mapping: \n";
		for (auto elem : map) {
			std::cout << elem.first << " " << elem.second << "\n";
		}
		return;
	}

	void copy_n_reindex(Tile &tile, std::map<int, int> map = std::map<int, int>()) {
		//0. Update latest areas of objects brutelly, [Lazy here]
		this->id_to_area.clear();
		this->id_to_area = this->gen_id_to_area();

		//Copy tile into FullTile using map({tile_id:full_id})
		//1. In nonoverlaping, just copy
		//2. In overlaping: merge

		//map from tile ids to THIS decreasing id (from last_index)
		for (int i = 0; i < tile.w; i++) {
			for (int j = 0; j < tile.h; j++) {
				//bg -> skip
				if (tile.img(i, j) == 0) {
					continue;
				} else {
					//Smaller objects infront of larger objects
					//We can query area of tile[id]: area_tile
					int area_tile = tile.id_to_area[tile.img(i,j)];
					//We need area of this->[id]: area_full
					int area_full = this->id_to_area[this->img(tile.start_i + i, tile.start_j + j)];
					//If T > F: F in front of T
					//Else: F behind T

					if (map.count(tile.img(i, j))) {
						//If smaller objects in THIS exist, smaller objects in front -> no copy those pixels.
						//img(i,j) in map -> just copy
						if (area_full == 0 or area_tile <= area_full){
							this->img(tile.start_i + i, tile.start_j + j) = map[tile.img(i, j)];
						}
					} else {
						//img(i,j) (not in map and not touch border) ->add to map -> then copy
						// if (!tile.set_l_b.count(tile.img(i, j))) {
						if (area_full == 0 or area_tile <= area_full){
							map[tile.img(i, j)] = this->next_index();
							this->img(tile.start_i + i, tile.start_j + j) = map[tile.img(i, j)];
						}
						// }
					}

				}
			}
		}
		std::cout << "mapping: \n";
		for (auto elem : map) {
			std::cout << elem.first << " " << elem.second << "\n";
		}
		return;
	}

	float compare_two_sets(std::set<int> set_a, std::set<int> set_b) {
		// for (int i : set_a)
		// {
		// 	std::cout<< i << "; ";
		// }
		// for (int i : set_b)
		// {
		// 	std::cout<< i << "; ";
		// }
		set<int> shared;
		set_intersection(set_a.begin(), set_a.end(), set_b.begin(), set_b.end(),
		                 std::inserter(shared, shared.begin()));
		// (intersect/union)
		// std::cout << "shared.size() = " << shared.size() << endl;
		return (1.0 * shared.size()) / (set_a.size() + set_b.size() - shared.size());

	}

	void merge_tile(Tile &tile) {
		std::cout << "merge_tile between:" << endl;
		this->print_info();
		tile.print_info();

		//0. Merge 1st tile -> merge into empty THIS
		if (tile.start_i == 0 && tile.start_j == 0) {
			this->copy_n_reindex(tile);
			return;
		}

		// 1. Intersection = start_i: start_i + overlap and tile.start_j:tile.start_j + tile.overlap
		// Find object in THIS that touch intersection -> set_1
		std::vector<set<int>> full_border_ids_vec; //Vector of (Set of pixels valued id)
		std::vector<int> full_border_id_vec; //Vector of (id)
		set<int> full_border_ids; //Set of (id)

		//1.1 if tile.start_i != 0: Check vertical border at col for FULL
		if (tile.start_i != 0) {
			// col: tile.start_i -> (tile.start_i + tile.overlap)
			// for (int col = tile.start_i + tile.overlap - 1; col < tile.start_i + tile.overlap; col++) {
			for (int col = tile.start_i ; col < tile.start_i + tile.overlap; col++) {
				for (int j = tile.start_j; j < tile.start_j + tile.h; j++) {
					int full_id = this->img(col, j);
					if (full_id != 0) {
						if (full_border_ids.insert(full_id).second) {
							std::cout << "THIS vertical: full_id at " << col << " " << j << " = " << full_id << endl;
							//insert successfully -> expanding region on THIS
							set<int> pixels = this->expand_region_id_full(full_id, tile);
							full_border_ids_vec.push_back(pixels);
							full_border_id_vec.push_back(full_id);
							std::cout << " full_border_ids: " << full_id  << " expanding size = " << pixels.size() << endl;
							// getchar();
						}
					}
				}
			}
		}

		//1.2 if tile.start_j != 0: Check horizontal boder at row for FULL
		if (tile.start_j != 0) {
			// row: tile.start_j -> tile.start_j + tile.overlap;
			// for (int row = tile.start_j + tile.overlap - 1; row < (tile.start_j + tile.overlap); row++) {
			for (int row = tile.start_j; row < (tile.start_j + tile.overlap); row++) {
				for (int i = tile.start_i; i < tile.start_i + tile.h; i++) {
					int full_id = this->img(i, row);
					// std::cout << "THIS horizontal: full_id at " << i << " "<< row << " = " << full_id << endl;
					if (full_id != 0) {
						if (full_border_ids.insert(full_id).second) {
							std::cout << "THIS horizontal: full_id at " << i << " " << row << " = " << full_id << endl;
							//insert successfully -> expanding region on THIS
							set<int> pixels = this->expand_region_id_full(full_id, tile);
							full_border_ids_vec.push_back(pixels);
							full_border_id_vec.push_back(full_id);
							std::cout << " full_border_ids: " << full_id  << " expanding size = " << pixels.size() << endl;
							// getchar();
						}
					}
				}
			}
		}

		if (full_border_ids.size() == 0) { //Nothing to merge
			this->copy_n_reindex(tile);
			return;
		}

		// 2. Find object in tile that touch intersection -> set_2
		set<int> tile_i_border_ids;
		std::vector<set<int>> tile_i_border_ids_vec;
		std::vector<int> tile_i_border_id_vec;

		//2.1 if tile.start_i != 0: Check vertical border at col for TILE
		if (tile.start_i != 0) {
			//col: 0 -> tile.overlap
			// for (int col = tile.overlap - 1; col < tile.overlap; col++) {
			for (int col = 0; col < tile.overlap; col++) {
				for (int j = 0; j < tile.h; j++) {
					int tile_id = tile.img(col, j);
					if (tile.img(col, j) != 0) {
						if (tile_i_border_ids.insert(tile_id).second) {
							std::cout << "TILE vertical: tile_id at " << col << " " << j << " = " << tile_id << endl;
							set<int> pixels = tile.expand_region_id_tile(tile_id, this->w);
							tile_i_border_ids_vec.push_back(pixels);
							tile_i_border_id_vec.push_back(tile_id);
							std::cout << "tile_i_border_ids: " << tile_id << " expanding size = " << pixels.size() << endl;
							// getchar();
						}
					}
				}
			}
		}

		//2.2 if tile.start_j != 0: Check horizontal boder at row for TILE
		if (tile.start_j != 0) {
			// int row = tile.overlap - 1;
			//row: 0 -> tile.overlap
			// for (int row = tile.overlap - 1; row < tile.overlap; row++) {
			for (int row = 0; row < tile.overlap; row++) {
				for (int i = 0; i < tile.h; i++) {
					int tile_id = tile.img(i, row);
					if (tile.img(i, row) != 0) {
						if (tile_i_border_ids.insert(tile_id).second) {
							std::cout << "TILE horizontal: tile_id at " << i << " " << row << " = " << tile_id << endl;
							set<int> pixels = tile.expand_region_id_tile(tile_id, this->w);
							tile_i_border_ids_vec.push_back(pixels);
							tile_i_border_id_vec.push_back(tile_id);
							std::cout << "tile_i_border_ids: " << tile_id << " expanding size = " << pixels.size() << endl;
							// getchar();
						}
					}
				}
			}
		}

		if (tile_i_border_ids.size() == 0) { //Nothing to merge
			this->copy_n_reindex(tile);
			return;
		}

		// //3. Merge two vectors of border ids
		//Sort by set.size()
		// std::sort(full_border_ids_vec.begin(), full_border_ids_vec.end(), [this] (std::set<int> set_a, std::set<int> set_b) { return ((set_a.size())>(set_b.size())); });
		// std::sort(tile_i_border_ids_vec.begin(), tile_i_border_ids_vec.end(), [this] (std::set<int> set_a, std::set<int> set_b) { return ((set_a.size())>(set_b.size())); });

		// // THIS as referece, look at TILE_i
		// std::cout <<"Cal score betwwen: size "<< full_border_ids_vec.size() << " and size " << tile_i_border_ids_vec.size() << endl;
		//       std::map<int, int> map;
		//       std::vector<bool> merger = std::vector<bool>(tile_i_border_ids_vec.size(), false);
		//       for (int i = 0; i < full_boreddr_ids_vec.size(); i++){
		//       	set<int> set_a = full_border_ids_vec[i];
		//       	float max_score = 0;
		//       	int max_j = -1;
		//       	for (int j = 0; j < tile_i_border_ids_vec.size(); j++){
		//       		if (merger[j]){
		//       			continue;
		//       		}
		//       		set<int> set_b = tile_i_border_ids_vec[j];
		//       		float score = compare_two_sets(set_a, set_b);
		//       		std::cout << "score = " <<score << endl;
		//       		// getchar();
		//       		if (score > max_score){
		//       			max_score = score;
		//       			max_j = j;
		//       		}
		//       	}
		//     		//Merge if > threshold
		//       	if (max_score > 0.5){// threshold = 0.5 temporarily
		//       		merger[max_j] = true;
		//       		map[tile_i_border_id_vec[max_j]] = full_border_id_vec[i];
		//       		std::cout << "Merging "<< tile_i_border_id_vec[max_j] << " and " <<full_border_id_vec[i]<< " score = "<<max_score << endl;
		//       		getchar();
		//       	}
		//       }

		// // TILE_i as referece, look at THIS
		// std::cout <<"Cal score betwwen: size "<< full_border_ids_vec.size() << " and size " << tile_i_border_ids_vec.size() << endl;
		std::map<int, int> map;
		std::vector<bool> merger = std::vector<bool> (full_border_ids_vec.size(), false);
		for (int i = 0; i < tile_i_border_ids_vec.size(); i++) {
			set<int> set_a = tile_i_border_ids_vec[i];
			float max_score = 0;
			int max_j = -1;
			std::cout << "score: ";
			for (int j = 0; j < full_border_ids_vec.size(); j++) {
				if (merger[j]) {
					continue;
				}
				set<int> set_b = full_border_ids_vec[j];
				float score = compare_two_sets(set_a, set_b);
				std::cout << score << " ";
				// getchar();
				if (score > max_score) {
					max_score = score;
					max_j = j;
				}
			}
			//Merge if > threshold
			if (max_score > this->sim_threshold) { // threshold = 0 temporarily
				//TODO: no. of pixel shared with border line as another criterion


				merger[max_j] = true;
				map[tile_i_border_id_vec[i]] = full_border_id_vec[max_j];
				std::cout << "\n merge betwwen " << tile_i_border_id_vec[i] << " and " << full_border_id_vec[max_j] << " score = " << max_score << endl;
			}
		}
		// getchar();

		//copy and reindex with std::map
		this->copy_n_reindex(tile, map);

		// 3. Copy objects in tile that don't touch intersection -> THIS
		// 4. Merge(list_1, list_2, threshold intersect/union metrics) -> list_3
		// 5. Copy list_3 to THIS
		// 6. Free tile object
	}
};