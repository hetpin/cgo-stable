#ifndef CGRAPH_H
#define CGRAPH_H

#include <vector>
#include <map>
#include <omp.h>

#include "fitsio.h"
#include "Image.h"
#include "utils.h"
#include "FlatSE.h"
#include "ragraph.h"
#include "cgraphwatcher.h"
#include "colorordering.h"
#include <math.h>
#include <string.h>
#include <gsl/gsl_cdf.h>
#include <unordered_set>

using namespace LibTIM;
using namespace std;

/** Component-graph storage and computation **/

class CGraph
{
public:
    /** A node of the directed graph */
    struct Node {
        int index;
        int N; //no of channels
        int depth;
        int object_id;//-1 for no object
        int index_explicit_merge_to;//-1 mean no merge any others
        int obj_extd_level;//-1 for non-object node, represents how many parents objs this node bases on.
        RGB color;
        RGB infiColor;
        RGB dispColor;
        Node(int index, RGB color, int area, int N = 3) {
            this->index = index; this->color = color; this->area = area; this->compact = 0; active = true; sn1 = false; main_branch = NULL; object_id = -1;
            this->similarity = 0;
            this->infiColor = color;
            this->index_explicit_merge_to = -1;
            this->obj_extd_level = -1;
            this->intersect = false;
            this->N = N;
            this->depth = 0;
            this->sigx = std::vector<double>(N, 0);
            this->sigx2 = std::vector<double>(N, 0);
            this->powers = std::vector<double>(N, 0);
            this->powers_norm = std::vector<double>(N, 0);
            this->addSigx(color, area);
            this->addSigx2(color, area);
            this->sn1_value = -1;
            this->snes = std::vector<bool> (N, false);
            this->snes_v = std::vector<double> (N, -1.0);
            this->far_infimum = RGB(100000, 100000, 100000);
            this->child_supremum = RGB(-1, -1, -1);
        }
        std::vector<Node *> childs;
        std::vector<Node *> fathers;
        RGB far_infimum;
        RGB child_supremum;
        // list of flat-zones belonging to node and having same value
        std::vector<int > regions;
        std::vector<int> pixels;
        std::set<int> set_childs;
        std::set<int> set_pixels;
        std::set<int> set_closest_obj_anc;
        std::set<int> set_nb_vertex;
        //----------------------------------------------------------------------------
        float compact;
        int area;
        int contrast;
        bool sn1;
        bool intersect;
        float sn1_value;
        bool active;
        std::vector<bool> snes;
        std::vector<double> snes_v;
        std::vector<double> sigx;
        std::vector<double> sigx2;
        //Since one node may have multiple parrents -> multiple powers -> vector of power
        std::vector<double> powers;// = sigx2 -2*parrent*sigx + area*pow(parrent,2)
        std::vector<double> powers_norm;// = sigx2 -2*parrent*sigx + area*pow(parrent,2)
        std::vector<Node*> closest_sig_anc;//Could be multiple -> vector
        std::unordered_set<Node*> set_closest_sig_anc;
        Node* main_branch;
        float similarity;

        ~Node() {
            // std::cout << "Calling deconstructor of Node struct index " << this->index << endl;
        }

        int writeDotForHeatmap_get_mb(){
            if (this->main_branch != NULL){
                return this->main_branch->index;
            }
            return -1;
        }
        //Get minimum snes_v value in [0,1], init by -1
        double get_snes_v_min(){
            double value = 1.0;
            for (int i = 0; i < this->snes_v.size(); ++i){
                if (snes_v[i] != -1 && snes_v[i] < value){
                    value = snes_v[i];
                }
            }
            return value;
            // return 1.0-value;
        }

        //Get intermediate parents for a child Ci of this Node
        //IP(Ci) = {Ci union Cj| (Ci incomparable Cj) and (Ci join/nb Cj)}
        std::vector<Node*> get_iparents(std::vector<Node *> &graph, ColorOrdering *order, RAGraph *rag, Node* child, bool &early_stop, RGB &syn) {
            std::vector<Node*> ip_vec;
            std::set<int>* set_a_nb = child->get_set_nb_vertex(graph, rag);
            for (int i = 0; i < this->childs.size(); i++) {
                Node* curNode = this->childs[i];

                //1. Check incomparable: if comparable -> skip
                if (!order->is_incomparable_not_equal(child->color, curNode->color)) {
                    continue;
                }

                //2. Check join/nb: no join/nb -> skip
                std::set<int>* set_b_nb = curNode->get_set_nb_vertex(graph, rag);
                if (!is_connected(set_a_nb, set_b_nb)) {
                    continue;
                }

                //3. Save intermediate parent as a Node
                // RGB ip_color(0, 0, 0);
                // ip_color.get_supremum(child->color);
                // ip_color.get_supremum(curNode->color);
                RGB ip_color(1000000, 1000000, 1000000);
                ip_color.get_infimum(child->color);
                ip_color.get_infimum(curNode->color);

                syn.get_supremum(ip_color);
                if (order->isequal(child->color, syn)) {
                    // std::cout << "Early stop \n";
                    // getchar();
                    early_stop = true;
                    return ip_vec;
                }

                //Area = union(child, curNode)
                Node *ip_node = new Node( -2, ip_color, 0);
                ip_vec.push_back(ip_node);
            }

            return ip_vec;
        }

        //Return vector of all iparents, not only the closest one
        std::vector<Node*> get_iparents_vec(std::vector<Node *> &graph, ColorOrdering *order, RAGraph *rag, Node* child) {
            std::vector<Node*> ip_vec;
            std::set<int>* set_a_nb = child->get_set_nb_vertex(graph, rag);
            std::set<int> set_b = child->get_set_pixels(rag);
            for (int i = 0; i < this->childs.size(); i++) {
                Node* curNode = this->childs[i];

                //1. Check incomparable: if comparable -> skip
                if (!order->is_incomparable_not_equal(child->color, curNode->color)) {
                    continue;
                }

                //2. Check join/nb: no join/nb -> skip
                std::set<int>* set_b_nb = curNode->get_set_nb_vertex(graph, rag);
                if (!is_connected(set_a_nb, set_b_nb)) {
                    continue;
                }

                //3. Save intermediate parent as a Node
                //Color
                RGB ip_color(1000000, 1000000, 1000000);
                ip_color.get_infimum(child->color);
                ip_color.get_infimum(curNode->color);

                //4. Check condition: not islessequal() any colors in ip_vec
                bool is_add = true;
                for(int k = 0; k < ip_vec.size(); k++){
                    Node* vec_k = ip_vec[k];
                    if ( order->islessequal(ip_color, vec_k->color)){
                        is_add = false;
                        break;
                    }
                }
                
                //5. Satisfied the condition above
                if (is_add){
                    //Area = union(child, curNode)
                    std::set<int> set_a = curNode->get_set_pixels(rag);
                    set<int> shared;
                    set_intersection(set_a.begin(), set_a.end(), set_b.begin(), set_b.end(),
                                     std::inserter(shared, shared.begin()));
                    int ip_area = curNode->area + child->area - shared.size();

                    //Create ip node
                    Node *ip_node = new Node( -2, ip_color, ip_area);
                    ip_node->snes = curNode->snes;
                    ip_node->sigx = curNode->sigx;
                    ip_node->sigx2 = curNode->sigx2;
                    ip_node->set_pixels = set_a;
                    ip_vec.push_back(ip_node);
                    // if (curNode->sn1){
                    //     for (int x = 0; x < child->N; ++x){
                    //         if (child->color[x] == ip_color[x] && curNode->snes[x]){
                    //             std::cout<<"Same at band "<<x <<"; ";
                    //         }
                    //     }
                    //     std::cout <<"SN: ";
                    // }
                    // curNode->print_snes();
                }
            }
            return ip_vec;
        }

        void update_infi_color(RGB value) {
            infiColor.get_infimum(value);
        }

        inline RGB update_far_infimum(RGB &value) {
            // value.print_table();
            // far_infimum.print_table();
            far_infimum.get_infimum(value);
            // far_infimum.print_table();
            // getchar();
        }

        inline RGB update_child_supremum(RGB &value) {
            // value.print_table();
            // child_supremum.print_table();
            child_supremum.get_supremum(value);
            // child_supremum.print_table();
            // getchar();
        }

        bool is_greater_power(Node* other) {
            for (int i = 0; i < this->N; i++) {
                if (this->powers_norm[i] < other->powers_norm[i]) {
                    return false;
                }
            }
            return true;
        }

        //Select main_leave by top child power
        Node* get_main_branch_power() {
            if (this->childs.size() > 0) {
                Node* m_b_power = this->childs[0];
                for (int i = 1; i < childs.size(); i++) {
                    if (childs[i]->is_greater_power(m_b_power)) {
                        m_b_power = childs[i];
                    }
                }
                return m_b_power;
            } else {
                return NULL;
            }
        }

        //Select main_leave by top child sum_color
        Node* get_main_branch_sum_color() {
            if (this->childs.size() > 0) {
                Node* m_b_power = this->childs[0];
                for (int i = 1; i < childs.size(); i++) {
                    if (childs[i]->color.get_sum() > m_b_power->color.get_sum()) {
                        m_b_power = childs[i];
                    }
                }
                return m_b_power;
            } else {
                return NULL;
            }
        }

        //Select main_leave by top child area
        Node* get_main_branch_area() {
            if (this->childs.size() > 0) {
                Node* m_b_area = this->childs[0];
                for (int i = 1; i < childs.size(); i++) {
                    if (childs[i]->area > m_b_area->area) {
                        m_b_area = childs[i];
                    }
                }

                //All child area = 1, choose top sum color
                if (this->childs.size() > 1 && m_b_area->area == 1) {
                    return this->get_main_branch_sum_color();
                }
                return m_b_area;
            } else {
                return NULL;
            }
        }

        //Following main power branch until reach the leave.
        int get_main_leave(bool is_debug = false) {
            if (is_debug){
                std::cout << "Getting get_main_leave for Node " << this->index << endl;
            }
            // Node* curNode = this;
            // while (curNode->get_main_branch_area() != NULL) {
            //     curNode = curNode->get_main_branch_area();
            // }
            // return curNode->index;

            Node* curNode = this;
            Node* tmp = curNode->main_branch;
            if (tmp == NULL){
                tmp = curNode->get_main_branch_power();
            }
            while(tmp != NULL){
                curNode = tmp;
                tmp = curNode->main_branch;
                if (tmp == NULL){
                    tmp = curNode->get_main_branch_power();
                }
            }

            // while(curNode->get_main_branch_power() != NULL){
            //     curNode = curNode->get_main_branch_power();
            // }

            // std::cout << "get_main_leave of "<< curNode->index << endl;
            // while(curNode->get_main_branch_sum_color() != NULL){
            //     curNode = curNode->get_main_branch_sum_color();
            //     // std::cout <<curNode->index << endl;
            // }
            // // curNode->color.print_table();

            if (is_debug){
                std::cout << curNode->index <<  endl;
            }
            return curNode->index;
        }

        //Obtaion childs for all nodes in graph
        std::set<int> cal_childs(std::vector<Node *> graph, RAGraph *rag) {
            std::cout << "cal_childs " << this->index << endl;

            for (int i = 0; i < this->childs.size(); ++i) {
                //Add un-visited child index
                Node* child = this->childs[i];
                if (this->set_childs.count(child->index) > 0) {
                    continue;
                }
                this->set_childs.insert(child->index);

                // Add child's set_childs
                std::set<int> set_childs_i = child->cal_childs(graph, rag);
                this->set_childs.insert(set_childs_i.begin(), set_childs_i.end());
            }
            this->set_nb_vertex = set_nodes_to_set_nb_vertex(graph, rag, this->set_childs);
            return this->set_childs;
        }

        // //Obtaion vertexes for all nodes in graph
        // std::set<int> cal_nb_vertexes(RAGraph *rag){
        //     std::cout << "cal_nb_vertexes " << this->index << endl;

        //     //Add vertex of current nodes
        //     for (int r = 0; r < this->regions.size(); r++){
        //         std::vector <int> allNb = rag->nodes[this->regions[r]]->allNb;//A vector<int> of nb of r
        //         for (int i = 0; i < allNb.size(); ++i){
        //             this->set_nb_vertex.insert(allNb[i]);
        //         }
        //     }

        //     //Looking at children nodes
        //     for (int i = 0; i < this->childs.size(); ++i){
        //         // //Add un-visited child index
        //         // Node* child = this->childs[i];
        //         // if (this->set_nb_vertex .count(child->index) > 0){
        //         //     continue;
        //         // }
        //         // this->set_childs.insert(child->index);

        //         // // Add child's set_childs
        //         // std::set<int> set_childs_i = child->cal_childs(rag);
        //         // this->set_childs.insert(set_childs_i.begin(), set_childs_i.end());

        //         Node* tmp = graph[f];
        //         for (int r = 0; r < tmp->regions.size(); r++){
        //             std::vector <int> allNb = rag->nodes[tmp->regions[r]]->allNb;//A vector<int> of nb of r
        //             for (int i = 0; i < allNb.size(); ++i){
        //                 set_a_nb.insert(allNb[i]);
        //             }
        //         }

        //     }
        //     return this->set_nb_vertex;
        // }

        //Add your index + all your chilren index into set_childs SET
        void query_child(std::set<int> &set_childs, std::set<int> &set_pixels, RAGraph *rag, int threshold) {
            if (this->area > threshold) { //Early stop: Only consider node large than threshold, since bellowing threshold are useless.
                if (set_childs.count(this->index) <= 0) { //Ensure no duplication because of multiple parents node.
                    //Insert this node (index) to set_childs
                    set_childs.insert(this->index);
                    //Insert pixels(w*x+y) of this node to set_pixels
                    int width = rag->get_imsource_width();
                    for (int r = 0; r < this->regions.size(); r++) {
                        std::vector<Point<TCoord>> points = rag->nodes[this->regions[r]]->pixels;
                        for (int p = 0; p < points.size(); p++) {
                            // points[p].print();
                            if (set_pixels.count(points[p].get_index(width)) <= 0) {
                                set_pixels.insert(points[p].get_index(width));
                            }
                        }
                    }
                    //Continue recursively
                    for (int i = 0; i < childs.size(); i++) {
                        childs[i]->query_child(set_childs, set_pixels, rag, threshold);
                    }
                }
            }
        }

        //Return vector of descendant objects
        std::vector<Node*> query_obj_descendants(std::vector<Node *> &graph, RAGraph *rag) {
            std::vector<Node*> obj_des;
            std::set<int> set_a = this->get_set_childs(rag);
            for (auto node : set_a ) {
                // (!non-object) && (!this->object_id)
                if (graph[node]->object_id != -1  && graph[node]->object_id != this->object_id) {
                    obj_des.push_back(graph[node]);
                }
            }
            return obj_des;
        }

        void query_closest_obj_anc(std::set<int> &set_closest_obj_anc, std::set<int> &set_processed) {
            if (set_processed.count(this->index) > 0) { //Already visited
                return;
            } else {
                set_processed.insert(this->index);    //Mark as visited
            }
            for (int i = 0; i < closest_sig_anc.size(); i++) {
                if (closest_sig_anc[i] != NULL) {
                    //sn_anc[i] is an object OR sn_anc[i] is potntial explicit object that has been merge to other larger explicit.
                    if (closest_sig_anc[i]->object_id >= 0 || closest_sig_anc[i]->index_explicit_merge_to > 0) {
                        std::cout << "found " << closest_sig_anc[i]->index << endl;
                        set_closest_obj_anc.insert(closest_sig_anc[i]->index);//Insert and stop this path
                        // getchar();
                    } else {
                        closest_sig_anc[i]->query_closest_obj_anc(set_closest_obj_anc, set_processed); //Recursively backtracking this path
                    }
                }
            }
        }
        std::set<int> get_set_closest_obj_anc() {
            //= Union of get_closest_sn_anc and their anc which are potential objects
            //Object status can be changed rapidly without update, then re-query eveytime this function is called.
            this->set_closest_obj_anc.clear();
            std::set<int> set_processed;
            query_closest_obj_anc(this->set_closest_obj_anc, set_processed);
            std::cout << this->set_closest_obj_anc.size() << endl;
            return this->set_closest_obj_anc;
        }
        std::set<int> get_set_pixels(RAGraph *rag, int threshold = 0) {
            if (!set_pixels.empty()) {
                return set_pixels;
            } else {
                query_child(this->set_childs, set_pixels, rag, threshold);
                return this->set_pixels;
            }
        }
        std::set<int> get_set_childs(RAGraph *rag, int threshold = 0) {
            if (!set_childs.empty()) {
                // std::cout << "get_set_childs not empty \n";
                return set_childs;
            } else {
                // std::cout << "get_set_childs querying \n";
                query_child(this->set_childs, set_pixels, rag, threshold);
                return this->set_childs;
            }
        }

        //Transform set of nodes into set of nb vertex
        std::set<int> set_nodes_to_set_nb_vertex(std::vector<Node *> &graph, RAGraph *rag, std::set<int> &set_a) {
            std::set<int> set_a_nb;
            for (auto f : set_a) {
                Node* tmp = graph[f];
                for (int r = 0; r < tmp->regions.size(); r++) {
                    std::vector <int> allNb = rag->nodes[tmp->regions[r]]->allNb;//A vector<int> of nb of r
                    for (int i = 0; i < allNb.size(); ++i) {
                        set_a_nb.insert(allNb[i]);
                    }
                }
            }
            return set_a_nb;
        }

        bool is_connected(std::set<int>* set_a, std::set<int>* set_b) {
            // std::cout << " - ";
            for (auto item : *set_a) {
                if (set_b->find(item) != set_b->end()) {
                    return true;
                }
            }
            return false;
        }

        //Return set of nb vertex of a current node
        std::set<int>* get_set_nb_vertex(std::vector<Node *> &graph, RAGraph *rag) {
            if (this->set_nb_vertex.empty()) {
                // std::cout << "get_set_nb_vertex querying \n";
                if (this->set_childs.empty()) {
                    // std::cout << "get_set_childs querying \n";
                    this->get_set_childs(rag);
                }
                // std::cout << "get_set_childs not empty \n";
                this->set_nb_vertex = set_nodes_to_set_nb_vertex(graph, rag, this->set_childs);
            }
            // std::cout << "get_set_nb_vertex not empty \n";
            return &(this->set_nb_vertex);
        }

        bool isIntersectSkip() {
            //Check no. of parents
            if (this->fathers.size() > 1) {
                this->intersect = true;
            } else {
                //Check Intersect of the only parent
                if (this->fathers.size() == 1) {
                    this->intersect = this->fathers[0]->intersect;
                }
            }
            return this->intersect;
        }

        //By hetpin
        bool isLeave() {
            for (int c = 0; c < this->childs.size(); c++) {
                if (this->childs[c]->active) {
                    return false;
                }
            }
            return true;
        }

        void addSigx(RGB &color, int pixels) {
            for (int i = 0; i < sigx.size(); i++) {
                sigx[i] += pixels * color[i];
            }
        }

        void addSigx2(RGB &color, int pixels) {
            for (int i = 0; i < sigx.size(); i++) {
                sigx2[i] += pixels * pow(color[i], 2);
            }
        }
        void addSigx(std::vector<double> newsigx) {
            for (int i = 0; i < sigx.size(); i++) {
                sigx[i] += newsigx[i];
            }
        }

        void addSigx2(std::vector<double> newsigx2) {
            for (int i = 0; i < sigx.size(); i++) {
                sigx2[i] += newsigx2[i];
            }
        }

        void addChild(Node *child) {
            this->childs.push_back(child);
            child->fathers.push_back(this);
        }

        void print_closest_sig_anc() {
            if (this->closest_sig_anc.size() > 0) {
                std::cout << "closest_sig_anc: \n";
                for (int i = 0; i < closest_sig_anc.size(); i++) {
                    if (closest_sig_anc[i] != NULL) {
                        std::cout << closest_sig_anc[i]->index << " a=" << closest_sig_anc[i]->area;
                        closest_sig_anc[i]->color.print_table();
                    }
                }
            }
        }
        void print_mbranch() {
            if (this->main_branch != NULL) {
                std::cout << "mbranch of " << index << ": " << main_branch->index << endl;
                main_branch->print_node();
            } else {
                std::cout << "mbranch of " << index << ": is NULL" << endl;
            }
        }

        bool add_closest_sig_anc(Node* node_sn) {
            if (set_closest_sig_anc.count(node_sn) <= 0) {
                set_closest_sig_anc.insert(node_sn);
                closest_sig_anc.push_back(node_sn);
                return true;
            }
            return false;
        }
        RGB synthesize_supremum(std::vector<Node *> fathers) { //Supremum
            RGB syn(0, 0, 0);
            int size = fathers.size();
            for (int i = 0; i < size; i++) {
                // fathers[i]->color.print_table();
                syn.get_supremum(fathers[i]->color);
            }
            return syn;
        }
        RGB synthesize_max_area(std::vector<Node *> fathers) {
            RGB syn(0, 0, 0);
            int flag_area = 0;
            int size = fathers.size();
            for (int i = 0; i < size; i++) {
                if (fathers[i]->area > flag_area) {
                    flag_area = fathers[i]->area;
                    syn = fathers[i]->color;
                }
            }
            return syn;
        }

        RGB synthesize_avg(std::vector<Node *> fathers) {
            RGB syn(0, 0, 0);
            int size = fathers.size();
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < 3; j++) {
                    syn[j] = syn[j] + (fathers[i]->color[j]) / size;
                }
            }
            return syn;
        }
        //Generate power_norm of a Node given a Background Node (color and area)
        std::vector<double> gen_powers_given_bg(std::vector<float> gain_vec, std::vector<float> sigma2_bg_vec, RGB &syn_color, RGB& syn_area) {
            std::vector<double> pes = std::vector<double>(N, 0.0);
            for (int i = 0; i < this->N; i++) {
                float gain = gain_vec[i];
                float sigma2_bg = sigma2_bg_vec[i];
                double f_value = syn_color[i];
                int area = this->area;
                // if (f_value == this->color[i]){
                //     area = syn_area[i];
                // }
                double power = this->sigx2[i] - 2 * f_value * (this->sigx[i]) + area * pow(f_value, 2);
                double power_norm = power / (sigma2_bg + f_value / gain);
                pes[i] = power_norm;
            }
            return pes;
        }

        //Generate power_norm of a Node given a Background Node (color and area)
        std::vector<double> gen_powers_given_bg(std::vector<float> gain_vec, std::vector<float> sigma2_bg_vec, RGB &syn_color, int syn_area) {
            std::vector<double> pes = std::vector<double>(N, 0.0);
            for (int i = 0; i < this->N; i++) {
                float gain = gain_vec[i];
                float sigma2_bg = sigma2_bg_vec[i];
                double f_value = syn_color[i];
                int area = this->area;
                if (f_value == this->color[i]){
                    area = syn_area;
                }
                double power = this->sigx2[i] - 2 * f_value * (this->sigx[i]) + area * pow(f_value, 2);
                double power_norm = power / (sigma2_bg + f_value / gain);
                pes[i] = power_norm;
            }
            return pes;
        }

        //Generate power_norm of a Node given a Background Node.
        std::vector<double> gen_powers_given_bg(std::vector<float> gain_vec, std::vector<float> sigma2_bg_vec, RGB &syn) {
            std::vector<double> pes = std::vector<double>(N, 0.0);
            for (int i = 0; i < this->N; i++) {
                float gain = gain_vec[i];
                float sigma2_bg = sigma2_bg_vec[i];
                double f_value = syn[i];
                double power = this->sigx2[i] - 2 * f_value * (this->sigx[i]) + this->area * pow(f_value, 2);
                double power_norm = power / (sigma2_bg + f_value / gain);
                pes[i] = power_norm;
            }
            return pes;
        }

        void gen_powers(std::vector<float> gain_vec, std::vector<float> sigma2_bg_vec) {
            RGB syn = synthesize_supremum(this->fathers);
            // RGB syn = this->color;
            // RGB syn = this->infiColor;
            // this->infiColor.print_table();
            // RGB syn = synthesize_max_area(this->fathers);
            // RGB syn = synthesize_avg(this->fathers);
            double power_norm_combined = 0;
            for (int i = 0; i < this->N; i++) {
                float gain = gain_vec[i];
                float sigma2_bg = sigma2_bg_vec[i];
                double f_value = syn[i];
                double power = this->sigx2[i] - 2 * f_value * (this->sigx[i]) + this->area * pow(f_value, 2);
                double power_norm = power / (sigma2_bg + f_value / gain);
                this->powers[i] = power;
                this->powers_norm[i] = power_norm;
            }
        }

        //Gen compactness for this node (4pi*area)/(perimetter^2)
        void gen_compactness(RAGraph *rag, ColorOrdering  *order, Image<RGB> &imSource) {
            int perimeter = 1;
            int dx = rag->get_imsource_width();
            int dy = rag->get_imsource_height();
            FlatSE connexity = rag->get_connexity();

            //Get set of pixels
            std::set<int> pixels = this->get_set_pixels(rag);
            std::cout << endl;
            for (int curindex : pixels) {
                Point<TCoord> cur(curindex % dx, curindex / dx);
                // cur.print();
                bool border = false;
                for (int i = 0; i < connexity.getNbPoints(); ++i) {
                    Point<TCoord> nb = cur + connexity.getPoint(i);

                    //at image borders
                    if (nb.x < 0 && nb.x >= dx && nb.y < 0 && nb.y >= dy) {
                        border = true;
                        break;
                    }

                    //Check ordering between this->color and imSource(nb)
                    if (!order->islessequal(this->color, imSource(nb))) {
                        border = true;
                        break;
                    }
                }
                if (border) {
                    // cur.print();
                    perimeter++;
                }
            }
            this->compact = (4*3.14159) * (this->area) / (perimeter * perimeter);
            // std::cout << this->index << " - " << this->compact << " - " << this->area << " - " << perimeter << "; ";
            // getchar();
        }

        void print_childs() {
            if (this->childs.size() > 0) {
                std::cout << "childs of " << this->index << ": \n";
                for (int i = 0; i < childs.size(); i++) {
                    std::cout << this->childs[i]->index << " a=" << this->childs[i]->area << " ";
                    this->childs[i]->color.print_table();
                }
            }
        }

        void print_parents() {
            if (this->fathers.size() > 0) {
                std::cout << "parents of " << this->index << ": \n";
                for (int i = 0; i < fathers.size(); i++) {
                    std::cout << this->fathers[i]->index << " a=" << this->fathers[i]->area << " ";
                    this->fathers[i]->color.print_table();
                }
            }
        }

        void print_node() {
            if (main_branch == NULL || powers_norm.size() == 0) {
                std::cout << "index " << index << ", id " << object_id << " active " << active << " isect " << intersect << " sn1 " << sn1 << " childs " << childs.size() << " par " << fathers.size()
                          << " sigx[0] " << sigx[0] << " (" << color[0] << " " << color[1] << " " << color[2] << ") sn_anc " << closest_sig_anc.size()  <<
                          " obj_id=" << object_id << "mbranch || p_norm null " << "area=" << area << " p0=" << powers_norm[0] << " p1=" << powers_norm[1] << " p2=" << powers_norm[2] << endl;
            } else {
                std::cout << "index " << index << ", id " << object_id << " active " << active << " isect " << intersect << " sn1 " << sn1 << " childs " << childs.size() << " par " << fathers.size()
                          << " sigx[0] " << sigx[0] << " (" << color[0] << " " << color[1] << " " << color[2] << ") sn_anc " << closest_sig_anc.size()
                          << " obj_id=" << object_id << " mbranch " << main_branch->index << " area=" << area << " p0=" << powers_norm[0] << " p1=" << powers_norm[1] << " p2=" << powers_norm[2] << endl;
            }
        }

        void print_snes() {
            std::cout << "snes: ";
            for (int i = 0; i < this->N; i++) {
                std::cout << this->snes[i] << " ";
            }
            for (int i = 0; i < this->N; i++) {
                std::cout << this->snes_v[i] << " ";
            }
            std::cout <<", area="<< this->area;

            std::cout << endl;
        }
        void print_all() {
            this->print_node();
            std::cout << "Node color: ";
            this->color.print_table();
            this->print_snes();
            this->print_closest_sig_anc();
            this->print_childs();
            this->print_parents();
            std::cout << "Syn = ";
            RGB syn = this->synthesize_supremum(this->fathers);
            syn.print_table();
            std::cout << endl;
        }

        RGB get_obj_corlor(int num_obj, std::vector<RGB> rgbs) {
            int value;
            if (object_id > 0) {
                value = (int)(255 * object_id / num_obj);
            } else {
                value = 0;
            }
            return rgbs[object_id - 1];
        }
    };

public :
    RAGraph *rag; /*!< Region adjacency graph on which is computed the component-graph */

    Image<RGB> imSource; /*! Source image */
    FlatSE connexity;   /*!< Underlying connexity (4- or 8-) */

    /** Container of the nodes of the (directed) graph **/
    std::vector<Node *> graph;
    /** Root of the graph **/
    Node *root;
    int num_obj; //Amount of object found
    std::vector<RGB> rgbs;
    int N;
    std::vector<float> gain_vec;
    std::vector<float> sigma2_bg_vec;
    std::vector<float> soft_bias_vec;
    float gain;
    float sigma2_bg;
    float soft_bias;
    double alpha;

public:

    CGraph(Image <RGB> &imSource, FlatSE &connexity, double alpha = pow(10, -6), int N = 3) {
        this->N = N;
        this->alpha = alpha;
        this->sigma2_bg_vec = std::vector<float>(N, 0);
        this->gain_vec = std::vector<float>(N, 0);
        this->soft_bias_vec = std::vector<float> (N, 0);
        this->imSource = imSource;
        this->connexity = connexity;
        this->rag = new RAGraph(imSource, connexity);
        num_obj = 0;
        int no_colors = 10000;
        srand(1);//fix the seed
        for (int i = 0; i < no_colors; i++) {
            rgbs.push_back(RGB(rand() % 255, rand() % 255, rand() % 255));
        }
    }
    CGraph(RAGraph *rag) : rag(rag) {}
    ~CGraph() {
        // std::cout << "Calling deconstructor of CGraph class \n";
        delete rag;
        delete root;
        for (int i = 0; i < graph.size(); ++i) {
            delete graph[i];
        }
    }

    Node *componentGraphNaive(FlatSE &connexity);
    void set_bg_info(std::vector<float> gain, std::vector<float> sigma2_bg, std::vector<float> soft_bias) {
        this->gain_vec = gain;
        this->sigma2_bg_vec = sigma2_bg;
        this->soft_bias_vec = soft_bias;
    }

    /** Component-graph \ddot G **/
    //This function also calculate area and contrast attribute for each nodes
    int computeGraph(ColorOrdering *order, CGraphWatcher *watcher);
    /** Component-graph G **/
    int computeGraphFull(ColorOrdering *order, CGraphWatcher *watcher);

    int computeGraphNEW(ColorOrdering *order, CGraphWatcher *watcher);

    CGraph::Node* findRoot(CGraph::Node* from);
    /** Return synthetic images */
    static Image<RGB> syntheticImage();
    static Image<RGB> syntheticImage2();

    /** Write graph in dot format **/
    void writeObjectToCSV(const char *filename, std::vector<CGraph::Node*> objs);
    int writeDot(const char *filename);
    int writeDotForHeatmap(const char *filename);
    int writeDotSN(const char* filename);
    int writeDotFrom(const char *filename, int start_id);
    int writeDotForPython(const char *filename);
    void areaFiltering(int areaMin);
    void reconstructMin();

    Image<RGB> constructImage(ColorOrdering *order);

    /** Helper functions **/
    vector<CGraph::Node *> computeComponents(ColorOrdering *order, OrderedQueueDouble<RGB> &pq);
    static bool notComparable(Image<RGB> &im, Point<TCoord> &p, Point<TCoord> &q);

    bool isEqual(Node *n, Node *m);
    // Test if set n is included in m
    bool isIncluded(Node *n, Node *m);
    bool isIncludedFast(Node *n, Node *m, vector<bool> &tmp);
    void computeLinks(ColorOrdering *order, vector<Node *> &nodes);
    Node *addRoot(vector<Node *> &nodes);
    vector<Node *> minimalElements(ColorOrdering *order, vector<Node *> &nodes, vector<bool> &tmp);
    void computeTransitiveReduction(ColorOrdering *order, vector<Node *> &nodes);

    int computeGraphInverse(CGraphWatcher *watcher);
    Image<RGB> constructImageInf();
    void areaFiltering(int areaMin, int areaMax);
    bool isLTE(RGB &v, RGB &w);
    Image<RGB> constructImageInverse();

    void contrastFiltering(int contrastMin);
    void contrastFiltering(int contrastMin, int contrastMax);
    void resetFiltering();

    /** Adaptive filtering **/
    void adaptiveAreaFiltering(int p);
    void adaptiveContrastFiltering(int p);

    bool isIncluded(Node *n, Node *m, vector<bool> &tmp);

    //MTO
    //Add sn1 attribute true/false
    void cal_attribute_sn1(ColorOrdering *order);

    void cal_attribute_sn2(ColorOrdering *order);

    void cal_attribute_sn_combined_band(ColorOrdering *order);

    void cal_attribute_sn_both_sn(ColorOrdering *order);

    void cal_attribute_sn_either_sn(ColorOrdering *order);

    void cal_attribute_sn_all(ColorOrdering *order);

    //Check sn for only a node
    void cal_sn_one_node(Node *curNode, ColorOrdering *order);

    //Add sn1 attribute true/false parallel version
    void cal_attribute_sn1_parallel(ColorOrdering *order);

    //Add intersection attribute
    void cal_attribute_isect(ColorOrdering *order);

    void simplify_graph(ColorOrdering *order);

    //Compare ct and cg node by node
    void compare_two_graph(CGraph *ct_cgraph);

    //Get similar score of the most alike gt node
    double top_similar_node(std::set<int> set_pixels_gt, bool is_center_check);

    // #This function update closest_sn_anc attribute top-down from node_id
    // #Be aware, sn_id could be None, since we gonna go top-down from root_id, and root_id is likely to be None
    void update_closest_sn_anc_from_id(Node* node_from, Node* node_sn, ColorOrdering *order, std::vector<int> &processed);
    void update_closest_sn_anc_from_id_new(Node* node_from);

    // # Update main_branch for a new SN node[id]
    // # For the whole tree, must update bottom up
    // # then main_branch(id) could look through all node below node id
    void update_parent_main_branch(Node* node_sn, ColorOrdering *order);

    // # Find object, return list of node id marked as object
    std::vector<CGraph::Node *> find_object(ColorOrdering *order);

    //Find object that update merging object asc order.
    std::vector<CGraph::Node *> find_object_merger(ColorOrdering *order);

    std::vector<CGraph::Node *> find_objectNEW(ColorOrdering *order,
            std::vector<float> gain_vec, std::vector<float> sigma2_bg_vec, float lambda);

    std::vector<CGraph::Node *> gos_find_object(ColorOrdering *order);

    Node* max_area(CGraph::Node* new_node, CGraph::Node* old_node);

    Image<RGB> constructImageToS(ColorOrdering *order);
    void recalculate_power(ColorOrdering *order);
    void cal_depth(ColorOrdering *order);
    void visual_gos(ColorOrdering *order);
    void visual_one_object_recursive(Image<RGB> &imRes, CGraph::Node* node, RGB &color, std::vector<int> &up_processed);
    Image<RGB> visual_all_object_by_objid(std::vector<CGraph::Node*> objs);
    Image<RGB> visual_one_object(CGraph::Node* object);
    Image<RGB> visual_all_object(std::vector<CGraph::Node*> objs);
    Image<RGB> constructImageGoS(ColorOrdering *order, int object_id);
    void trace_sn_values(int index);
    int export_fits(Image<RGB> &all_obj, const char* fits_name );
    std::vector<CGraph::Node *> move_down(std::vector<CGraph::Node *> &objs
                                          , ColorOrdering *order, std::vector<float> gain_vec, std::vector<float> sigma2_bg_vec, float lambda);
    void move_down_adaptive(std::vector<CGraph::Node *> &objs
                                          , ColorOrdering *order, std::vector<float> gain_vec, std::vector<float> sigma2_bg_vec, float step = 0.1);

private:
    void paintNode(Image<RGB> &im, Node *n, RGB &value);
    void paintNodeSup(Image<RGB> &imRes, Node *n, RGB &value);

    vector<Node *> computeComponents(Image<RGB> &im, FlatSE &connexity);
};

#endif // CGRAPH_H