#include <omp.h>

#include "ragraph.h"
#include "cgraph.h"
// #include <gsl/gsl_cdf.h>

using namespace std;

#include <iostream>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <string>
#include <limits>
#include <sstream>
#include <iomanip>
namespace my {

std::string to_string( double d ) {

    std::ostringstream stm ;
    stm << std::setprecision(std::numeric_limits<double>::digits10) << d ;
    return stm.str() ;
}
}


Image<RGB> CGraph::syntheticImage()
{
    int a[] = {0, 0, 0};
    int b[] = {1, 0, 0};
    int c[] = {0, 1, 0};
    int d[] = {1, 1, 0};
    int e[] = {1, 1, 1};
    int f[] = {0, 0, 1};

    RGB A(a);
    RGB B(b);
    RGB C(c);
    RGB D(d);
    RGB E(e);
    RGB F(f);

    TSize size[] = {7, 1, 1};
    TSpacing spacing[] = {1.0, 1.0, 1.0};
//    const RGB data[]={  A,B,C,B,A,
//                        C,C,D,B,A,
//                        B,B,C,A,B,
//                        B,C,A,B,A,
//                        A,C,B,C,A};
    //const RGB data[]={  A,C,C,C,B,E,D,D,C,E,E,D,B,E,B,C,C,C,B,B,A};
    const RGB data[] = { A, E, C, F, B, E, A};

    //    TSize size[]={4,4,1};
    //    TSpacing spacing[]={1.0,1.0,1.0};
    //    const RGB data[]={A,A,B,B,
    //                 B,C,B,B,
    //                 A,A,E,D,
    //                 A,E,E,D};
    Image<RGB> imTmp(size, spacing, data);
    return imTmp;
}


Image<RGB> CGraph::syntheticImage2()
{
    int a[] = {0, 0, 0};
    int b[] = {1, 0, 0};
    int c[] = {0, 1, 0};
    int d[] = {1, 1, 0};
    int e[] = {1, 1, 1};
    int f[] = {0, 0, 1};

    RGB A(a);
    RGB B(b);
    RGB C(c);
    RGB D(d);
    RGB E(e);
    RGB F(f);

    TSize size[] = {21, 1, 1};
    TSpacing spacing[] = {1.0, 1.0, 1.0};
//    const RGB data[]={  A,B,C,B,A,
//                        C,C,D,B,A,
//                        B,B,C,A,B,
//                        B,C,A,B,A,
//                        A,C,B,C,A};
    const RGB data[] = {  A, C, C, C, B, E, D, D, C, E, E, D, B, E, B, C, C, C, B, B, A};
    //const RGB data[]={ A,E,C,F,B,E,A};

    //    TSize size[]={4,4,1};
    //    TSpacing spacing[]={1.0,1.0,1.0};
    //    const RGB data[]={A,A,B,B,
    //                 B,C,B,B,
    //                 A,A,E,D,
    //                 A,E,E,D};
    Image<RGB> imTmp(size, spacing, data);
    return imTmp;
}

void print_queue(std::queue<int> q)
{
    std::cout << "Fifo: " << q.front() << " ";
    while (!q.empty())
    {
        std::cout << q.front() << " ";
        q.pop();
    }
    std::cout << std::endl;
}

/**
*   Computation of component-graph \ddot G
**/
int CGraph::computeGraph(ColorOrdering *order, CGraphWatcher *watcher) {

    OrderedQueueDouble <int> pq;
    vector<bool> processed(rag->nodes.size());
    /** Warning : nodes[0] does not exist !!!! */
    for (int i = 1; i < rag->nodes.size(); i++) {
        RGB value = rag->nodes[i]->color;
        F32 priority = order->getPriority(value);
        pq.put(priority , i);
        processed[i] = false;
    }

    // index from flat-zone number to corresponding node
    vector <Node *> regionToNode(rag->nodes.size());
    regionToNode.assign(regionToNode.size(), 0);

    std::queue<int> fifo;
    std::vector<bool> infifo(regionToNode.size());

    int done = 0;
    float fifocount = 0;

    while (!pq.empty() ) {
        int i = pq.get();
        if (watcher != 0) {
            watcher->progressUpdate();
        }
        if (regionToNode[i] == 0) {
            /** i est une région canonique (i==regionToNode[i]->index- */
            RAGraph::Vertex *curVertex = rag->nodes[i];
            Node *curNode = new Node(curVertex->index, curVertex->color, curVertex->pixels.size());

            curNode->regions.push_back(i);
            regionToNode[i] = curNode;

            /** on lance la propagation */
            infifo.assign(infifo.size(), false);
            fifo.push(i);
            infifo[i] = true;
            done++;
            // std::cout <<"remaining = "<<done << "/"<< rag->nodes.size() << " pq_i = "<< i << " pre fifo = " <<fifocount << endl;
            fifocount = 0;
            while (!fifo.empty()) {
                int curpt = fifo.front();
                fifo.pop();
                fifocount++;
                RAGraph::Vertex *tmpVertex = rag->nodes[curpt];
                // std::cout << fifo.size() << " ";
                // std::cout << tmpVertex->allNb.size() << " \n";
                for (int v = 0; v < tmpVertex->allNb.size(); v++ ) {
                    int nb = tmpVertex->allNb[v];
                    if (!infifo[nb] && order->islessequal(rag->nodes[i]->color, rag->nodes[nb]->color)) {
                        // if (curNode->childs.empty() || (!curNode->childs.empty() && order->islessequal(rag->nodes[nb]->color, curNode->child_supremum))){
                        if (order->isequal(rag->nodes[i]->color , rag->nodes[nb]->color)) {
                            curNode->regions.push_back(nb);
                            regionToNode[nb] = curNode;
                            // std::cout<< "opening " ;
                            // rag->nodes[nb]->color.print_table();
                        } else {
                            Node *tmp = regionToNode[nb];
                            bool isChild = true;
                            if (!tmp->fathers.empty()) {
                                // std::cout << "far is empty \n";
                                if (order->islessequal(curNode->color, tmp->far_infimum)) {
                                    isChild = false;
                                } else {
                                    for (int a = 0; a < tmp->fathers.size(); a++) {
                                        if (order->islessequal( curNode->color , tmp->fathers[a]->color) ) {
                                            isChild = false;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (isChild) {
                                // std::cout << "Found a child of " << curNode->index << " is " << tmp->index <<endl;
                                curNode->childs.push_back(tmp);
                                curNode->update_child_supremum(tmp->color);
                                tmp->fathers.push_back(curNode);
                                tmp->update_far_infimum(curNode->color);
                            }
                        }
                        curNode->area += rag->nodes[nb]->pixels.size();
                        //This SIGX CALCULATION WAS IMPLEMENTED as recalculate_power()
                        //Add sigx and sigx2
                        // curNode->sigx += rag->nodes[nb]->get_sigx();
                        curNode->addSigx(rag->nodes[nb]->color, rag->nodes[nb]->pixels.size());
                        // curNode->sigx2 += rag->nodes[nb]->get_sigx2();
                        curNode->addSigx2(rag->nodes[nb]->color, rag->nodes[nb]->pixels.size());
                        fifo.push(nb);
                        infifo[nb] = true;
                        // }
                    }
                }


            }
            processed[i] = true;
        }
    }

    /** Assign a new index to each node */
    for (int i = 0; i < regionToNode.size(); i++) {
        if (regionToNode[i] != 0 && regionToNode[i]->index == i) {
            graph.push_back(regionToNode[i]);
            graph[graph.size() - 1]->index = graph.size() - 1;
        }

    }

    /** Add fictitious root*/

    RGB value(0, 0, 0);

    root = new Node(-1, value, imSource.getBufSize());

    for (int i = 0; i < graph.size(); i++) {
        if (graph[i]->fathers.size() == 0) {
            root->addChild(graph[i]);
        }
    }

    // recalculate_power(order);
    // cal_depth(order);

    //Cal child for all nodes, propagate from root to leaves.
    // root->cal_childs(graph, rag);

    //Calculate power attribute
    // #pragma omp parallel for private(i)
    for (int i = 0; i < graph.size(); i++) {
        // std::cout << "attributing " << i << "/" << graph.size() << endl;
        graph[i]->gen_powers(gain_vec, sigma2_bg_vec);
        // graph[i]->gen_compactness(rag, order, imSource);
    }

    std::cout << "Done ComputeGraph() \n";
    return 1;
}

void going_up(ColorOrdering *order, std::vector<bool> &infifo, CGraph::Node *curNode, vector <CGraph::Node *> &regionToNode, RAGraph *rag,
              int i, int j) {
    //0. Mark node i as visited
    std::cout << "going_up at i = " << i  << " j = " << j << endl;
    infifo[j] = true;

    //1. Process node j
    //current pointing node
    CGraph::Node *tmp = regionToNode[j];
    if (order->isequal(rag->nodes[i]->color , rag->nodes[j]->color)) {
        curNode->regions.push_back(j);
        regionToNode[j] = curNode;
        std::cout << "Found equal regionToNode[j] " << j << " to " << curNode->index << endl;
        //Transfer childs and parents nodes
        // curNode->print_node();
        // curNode->childs.insert(curNode->childs.begin(), tmp->childs.begin(), tmp->childs.end());
        // curNode->fathers.insert(curNode->fathers.begin(), tmp->fathers.begin(), tmp->fathers.end());
        // curNode->print_node();
        // getchar();
        // return;
    } else {
        bool isChild = true;
        if (!tmp->fathers.empty()) {
            if (order->islessequal(curNode->color, tmp->far_infimum)) {
                isChild = false;
            } else {
                for (int a = 0; a < tmp->fathers.size(); a++) {
                    if (order->islessequal( curNode->color , tmp->fathers[a]->color) ) {
                        isChild = false;
                        break;
                    } else {
                        std::cout << "curNode->color <= tmp->fathers[a]->color \n";
                        curNode->color.print_table();
                        tmp->fathers[a]->color.print_table();
                    }
                }
            }
        }
        if (isChild) {
            curNode->childs.push_back(tmp);
            tmp->fathers.push_back(curNode);
            if (tmp->fathers.size() > 1 ) {
                curNode->color.print_table();
                tmp->color.print_table();
                std::cout << "Multiple fathers for a node \n";
                tmp->print_node();
                tmp->print_closest_sig_anc();
                tmp->print_childs();
                tmp->print_parents();
                getchar();
            }
            tmp->update_far_infimum(curNode->color);
            std::cout << "Found a child at " << tmp->index << endl;
            //TODO update attributes should be here
            return;
        }
    }
    // curNode->area+=rag->nodes[j]->pixels.size();
    // //This SIGX CALCULATION WAS IMPLEMENTED as recalculate_power()
    // //Add sigx and sigx2
    // // curNode->sigx += rag->nodes[nb]->get_sigx();
    // curNode->addSigx(rag->nodes[j]->color, rag->nodes[j]->pixels.size());
    // // curNode->sigx2 += rag->nodes[nb]->get_sigx2();
    // curNode->addSigx2(rag->nodes[j]->color, rag->nodes[j]->pixels.size());

    //2. Recursively process j's parents
    for (int f = 0; f < tmp->fathers.size(); f++) {
        //for all direct regions of node
        for (int r = 0; r < tmp->fathers[f]->regions.size(); r++) {
            int k = tmp->fathers[f]->regions[r];
            if (!infifo[k] && order->islessequal(rag->nodes[i]->color , rag->nodes[k]->color)) {
                going_up(order, infifo, curNode, regionToNode, rag, i, k);
            }
        }
    }
}

void update_far_attribute(std::vector<bool> &visited, CGraph::Node* curNode, int root_index, int value, RGB &color) {
    int size = curNode->fathers.size();
    // std::cout << "curnode "<< curNode->index <<  " area " << curNode->area << " far size " << size << endl;
    for (int i = 0; i < size; i++) {
        CGraph::Node* far = curNode->fathers[i];
        int index = far->index;
        if (index == -1) {
            index = root_index;
        }
        if (!visited[index]) {
            // std::cout << "far index " << far->index << " area " << far->area << endl;
            far->area += value;
            // std::cout << "far index " << far->index << " area " << far->area << endl;
            // //Add sigx and sigx2
            far->addSigx(color, value);
            far->addSigx2(color, value);
        }
    }

    for (int i = 0; i < size; i++) {
        int index = curNode->fathers[i]->index;
        if (index == -1) {
            index = root_index;
        }
        if (!visited[index]) {
            visited[index] = true;
            update_far_attribute(visited, curNode->fathers[i], root_index, value, color);
        }
    }
}

//Find root of connected component containing Node "from"
CGraph::Node* CGraph::findRoot(CGraph::Node* from) {
    // std::cout<<"debug "<< from->index << endl;
    if (!from->fathers.empty()) {
        return findRoot(from->fathers[0]);
    } else {
        // std::cout<<"Final return " << from->index <<endl;
        // getchar();
        return from;
    }
}

// //Merge rNode to curNode
// void CGraph::union(CGraph::Node* curNode, CGraph:Node* rNode){
//     //copy rNode data to curNode

//     if (!from->fathers.empty()){
//         return findRoot(from->fathers[0]);
// } else{
//         // std::cout<<"Final return " << from->index <<endl;
//         // getchar();
//         return from;
//     }
// }

int CGraph::computeGraphNEW(ColorOrdering *order, CGraphWatcher *watcher) {
    OrderedQueueDouble <int> pq;
    vector<bool> processed(rag->nodes.size());
    /** Warning : nodes[0] does not exist !!!! */
    for (int i = 1; i < rag->nodes.size(); i++) {
        RGB value = rag->nodes[i]->color;
        F32 priority = order->getPriority(value);
        if (priority == 0) {
            continue;
        }
        pq.put(priority , i);
        // std::cout << priority << " " << i << endl;
        processed[i] = false;
    }

    // index from flat-zone number to corresponding node
    vector <Node *> regionToNode(rag->nodes.size());
    regionToNode.assign(regionToNode.size(), 0);

    while (!pq.empty() ) {
        int i = pq.get();
        if (watcher != 0) {
            watcher->progressUpdate();
        }
        if (regionToNode[i] == 0) {
            /** i est une région canonique (i==regionToNode[i]->index- */
            RAGraph::Vertex *curVertex = rag->nodes[i];
            Node *curNode = new Node(curVertex->index, curVertex->color, curVertex->pixels.size());
            std::cout << "-----------------Create new node sum = " << curVertex->color.get_sum() << " " << i << endl;
            curNode->regions.push_back(i);
            regionToNode[i] = curNode;

            //Check all nb of current vertex
            //For each nb: findRoot() recursively
            RAGraph::Vertex *tmpVertex = rag->nodes[i];
            std::cout << "Checking nb " << tmpVertex->allNb.size() << endl;
            for (int v = 0; v < tmpVertex->allNb.size(); v++ ) {
                int nb = tmpVertex->allNb[v];
                //Check (cac nb region da xu ly truoc do) va (tat nhien order cua nb region phai be hon curNode)
                if (processed[nb] && order->islessequal(rag->nodes[i]->color, rag->nodes[nb]->color)) {
                    std::cout << "visiting " << nb << endl;
                    //1. Find root of nb
                    std::cout << "currentNode " << curNode->index << " vs. nbNode " <<  regionToNode[nb]->index << endl;
                    Node* rNode = findRoot(regionToNode[nb]);
                    //If root of nb connected component is different from curNode, then:
                    if (rNode->index != curNode->index) {
                        if (order->isequal(rNode->color, curNode->color)) {
                            //Merge rNode and curNode: Which one to keep?

                        }
                        std::cout << "Merging " << rNode->index << " as child of curNode " << curNode->index << endl;
                        //2. Merge rNode of nb as childs of curNode
                        curNode->childs.push_back(rNode);
                        rNode->fathers.push_back(curNode);
                        //2.5. Update infiColor attribute of the current Node
                        curNode->update_infi_color(rNode->color);
                        //3. Update attribute: area, sigx, sigx2
                        curNode->area += rNode->area;// + rag->nodes[nb]->pixels.size();
                        curNode->addSigx(rNode->sigx);
                        curNode->addSigx2(rNode->sigx2);
                    }
                } else {
                    std::cout << "skip " << nb << endl;
                }
            }
            processed[i] = true;
        }
    }

    /** Assign a new index to each node */
    for (int i = 0; i < regionToNode.size(); i++) {
        if (regionToNode[i] != 0 && regionToNode[i]->index == i) {
            std::cout << "i " << i << endl;
            regionToNode[i]->color.print_table();
            graph.push_back(regionToNode[i]);
            graph[graph.size() - 1]->index = graph.size() - 1;
        } else {
            if (regionToNode[i] != 0) {
                std::cout << "fail " << i << " " << regionToNode[i]->index << endl;
                regionToNode[i]->color.print_table();
            } else {
                std::cout << "fail " << i << endl;
            }
            // getchar();
        }
    }

    /** Add fictitious root*/

    RGB value(0, 0, 0);

    root = new Node(-1, value, imSource.getBufSize());
    for (int i = 0; i < graph.size(); i++) {
        if (graph[i]->fathers.size() == 0) {
            root->addChild(graph[i]);
            std::cout << "No root \n";
            graph[i]->print_node();
        }
    }

    // cal_depth(order);

    //Calculate power attribute
    // #pragma omp parallel for private(i)
    for (int i = 0; i < graph.size(); i++) {
        graph[i]->gen_powers(gain_vec, sigma2_bg_vec);
    }

    std::cout << "Done ComputeGraphNEW() \n";
    return 1;
}

void CGraph::recalculate_power(ColorOrdering *order) {
    // std::cout<< "Start recalculate_power" << endl;
    OrderedQueueDouble <Node *> pq;
    for (int i = 0; i < graph.size(); i++) {
        pq.put(order->getPriority(graph[i]->color), graph[i]); //Visit bottom-up priority
    }
    pq.put(1, root);

    while (!pq.empty()) {
        // std::cout<<"-----------------------recalculate_powering "<< endl;
        Node *curNode = pq.get();
        curNode->sigx = std::vector<double>(curNode->N, 0);
        curNode->sigx2 = std::vector<double>(curNode->N, 0);
        //parrent node: Sum all childs attributes
        for (int c = 0; c < curNode->childs.size(); c++) {
            Node *curChild = curNode->childs[c];
            for (int i = 0; i < curNode->N; i++) {
                curNode->sigx[i] += curChild->sigx[i];
                curNode->sigx2[i] += curChild->sigx2[i];
            }
        }

        for (int r = 0; r < curNode->regions.size(); r++) {
            // std::cout << "--Visit region " << r << endl;
            vector<Point<TCoord> > points = rag->nodes[curNode->regions[r]]->pixels;
            for (int p = 0; p < points.size(); p++) {
                for (int i = 0; i < curNode->N; i++) {
                    float value = imSource(points[p])[i];
                    curNode->sigx[i] += value;
                    curNode->sigx2[i] += value * value;
                }
            }
        }
        // getchar();

    }
    // std::cout<< "Stop recalculate_power " << endl;
    // getchar();
}

void CGraph::cal_depth(ColorOrdering *order) {
    OrderedQueueDouble <Node *> pq;
    for (int i = 0; i < graph.size(); i++) {
        pq.put(order->getPriority(graph[i]->color), graph[i]); //Visit bottom-up priority
    }
    pq.put(1, root);

    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->childs.size() == 0) {
            curNode->depth = 1;
        }
        for (int c = 0; c < curNode->childs.size(); c++) {
            Node *curChild = curNode->childs[c];
            if (c == 0) {
                curNode->depth = curChild->depth + 1;
            }
            if (curChild->depth > (curNode->depth - 1 )) {
                curNode->depth = curChild->depth + 1;
            }
        }
        // std::cout<<"depth = " << curNode->depth << endl;
    }
}
bool CGraph::isLTE(RGB &v, RGB &w)
{
    if (v[0] >= w[0] && v[1] >= w[1] && v[2] >= w[2] )
        return true;
    else return false;
}

/**
*   Computation of component-graph \ddot G using inverse order
**/
int CGraph::computeGraphInverse(CGraphWatcher *watcher) {

    OrderedQueueDouble <int> pq;

    vector<bool> processed(rag->nodes.size());

    //std::cout << rag->nodes.size() << "\n";


    /** Warning : nodes[0] does not exist !!!! */
    for (int i = 1; i < rag->nodes.size(); i++) {

        RGB value = rag->nodes[i]->color;

        int R = value[0];
        int G = value[1];
        int B = value[2];
        // put in the prior. queue the flat zone number
        pq.put((R + G + B), i);
        processed[i] = false;
    }

    // index from flat-zone number to corresponding node
    vector <Node *> regionToNode(rag->nodes.size());
    regionToNode.assign(regionToNode.size(), 0);

    std::queue<int> fifo;
    std::vector<bool> infifo(regionToNode.size());

    std::vector<Node *> potentialChilds;

    while (!pq.empty() ) {
        int i = pq.get();

        if (watcher != 0)
        {
            watcher->progressUpdate();
        }
        //std::cout << "Région visitée : " << i << "\n";

        if (regionToNode[i] == 0) {
            /** i est une région canonique (i==regionToNode[i]->index- */
            RAGraph::Vertex *curVertex = rag->nodes[i];
            Node *curNode = new Node(curVertex->index, curVertex->color, curVertex->pixels.size());

            curNode->regions.push_back(i);
            regionToNode[i] = curNode;

            potentialChilds.clear();

            /** on lance la propagation */
            infifo.assign(infifo.size(), false);
            fifo.push(i);
            infifo[i] = true;
            int M = 0;
            while (!fifo.empty()) {
                int curpt = fifo.front();
                M++;
                fifo.pop();
                RAGraph::Vertex *tmpVertex = rag->nodes[curpt];
                for (int v = 0; v < tmpVertex->allNb.size(); v++ ) {
                    int nb = tmpVertex->allNb[v];
                    if (!infifo[nb] && isLTE(rag->nodes[i]->color, rag->nodes[nb]->color))  {
                        if (rag->nodes[i]->color == rag->nodes[nb]->color) {
                            /** ZP qui appartient au même noeud : on fusionne*/
                            curNode->regions.push_back(nb);
                            regionToNode[nb] = curNode;

                            /** Mise à jour de l'aire */

                            //std::cout << "Fusion de " << i << " et " << nb << "\n";
                        }
                        else {
                            /** zp potentiellement descendant du noeud courant : on teste si un père du noeud est comparable avec curNode
                            *  Si c'est le cas, a n'est pas un fils direct de curNode
                            */
                            Node *tmp = regionToNode[nb];
                            bool isChild = true;
                            for (int a = 0; a < tmp->fathers.size(); a++) {
                                if (isLTE(curNode->color , tmp->fathers[a]->color) ) {
                                    isChild = false;
                                    break;
                                }
                            }
                            if (isChild) {
                                curNode->childs.push_back(tmp);
                                tmp->fathers.push_back(curNode);

                            }
                            //potentialChilds.push_back(tmp);
                            //std::cout << "\t Ajoute " << nb << "\n";
                        }
                        curNode->area += rag->nodes[nb]->pixels.size();
                        fifo.push(nb);
                        infifo[nb] = true;
                    }
                }

            }
            processed[i] = true;
        }
    }

    /** Assign a new index to each node */
    for (int i = 0; i < regionToNode.size(); i++) {
        if (regionToNode[i] != 0 && regionToNode[i]->index == i) {
            graph.push_back(regionToNode[i]);
            graph[graph.size() - 1]->index = graph.size() - 1;
        }

    }

    /** Add fictitious root*/

    RGB value(255, 255, 255);

    root = new Node(-1, value, imSource.getBufSize());

    for (int i = 0; i < graph.size(); i++) {

        if (graph[i]->fathers.size() == 0) {
            root->addChild(graph[i]);
        }
    }

    return 1;
}



void CGraph::areaFiltering(int areaMin)
{


    for (int i = 0; i < graph.size(); i++) {
        Node *n = graph[i];
        if (n->area < areaMin) n->active = false;
    }

    if (root->area < areaMin) root->active = false;
}


void CGraph::areaFiltering(int areaMin, int areaMax)
{


    for (int i = 0; i < graph.size(); i++) {
        Node *n = graph[i];
        if (n->area < areaMin || n->area > areaMax) n->active = false;
    }

    if (root->area < areaMin || root->area > areaMax) root->active = false;
}


void CGraph::contrastFiltering(int contrastMin)
{

    for (int i = 0; i < graph.size(); i++) {
        Node *n = graph[i];
        if (n->contrast < contrastMin) n->active = false;
    }

    if (root->contrast < contrastMin) root->active = false;
}


void CGraph::resetFiltering()
{
    /** Initialisation*/
    for (int i = 0; i < graph.size(); i++) {graph[i]->active = true;}
}

/**
    Keep the p % nodes having the biggest area
*/
void CGraph::adaptiveAreaFiltering(int p)
{


    std::map<int, int> histo;
    for (int i = 0; i < graph.size(); i++) {
        Node *n = graph[i];
        histo[-n->area]++;
    }

    histo[-root->area]++;

    int totalNodes = graph.size() + 1;
    int bestNodes = (p * totalNodes) / 100;

    int count = 0;

    int threshold;

    std::map<int, int>::iterator it;
    for (it = histo.begin(); it != histo.end(); ++it) {
        count += it->second;
        if (count > bestNodes )
        {
            threshold = -it->first;
            break;
        }
    }
    //qDebug() << "Area = " << threshold <<"\n";
    areaFiltering(threshold);

}


/**
    Keep the p % nodes having the biggest area
*/
void CGraph::adaptiveContrastFiltering(int p)
{
    std::map<int, int> histo;
    for (int i = 0; i < graph.size(); i++) {
        Node *n = graph[i];
        histo[-n->contrast]++;
    }

    histo[-root->contrast]++;

    int totalNodes = graph.size() + 1;
    int bestNodes = (p * totalNodes) / 100;

    int count = 0;

    int threshold;

    std::map<int, int>::iterator it;
    for (it = histo.begin(); it != histo.end(); ++it) {
        count += it->second;
        if (count > bestNodes )
        {
            threshold = -it->first;
            break;
        }
    }
    //qDebug() << "Area = " << threshold <<"\n";
    contrastFiltering(threshold);

}


void CGraph::contrastFiltering(int contrastMin, int contrastMax)
{


    for (int i = 0; i < graph.size(); i++) {
        Node *n = graph[i];
        if (n->contrast < contrastMin || n->contrast > contrastMax) n->active = false;
    }

    if (root->contrast < contrastMin || root->contrast > contrastMax) root->active = false;
}

//void CGraph::reconstructMin()
//{
//    std::queue<Node *> fifo;

//    vector <bool> active(graph.size());
//    active.assign(active.size(),true);

//    for(int i=0; i<graph.size(); i++) {
//        if(graph[i]->active==false && graph[i]->fathers.size()>1) {
//            fifo.push(graph[i]);
//            active[graph[i]->index]=false;

//            while(!fifo.empty()) {
//                Node *n=fifo.front();
//                fifo.pop();

//                int nactive=0;

//                for(int j=0; j<n->fathers.size(); j++) {
//                    if(n->fathers[j]->active==true) nactive++;
//                }
//                //qDebug() << "Node: " << n->label << " "<<  nactive << "\n";
//                int j=0;
//                while(nactive>1) {
//                    if(n->fathers[j]->active==true) {
//                        n->fathers[j]->active=false;
//                        if(active[n->fathers[j]->index])
//                        {
//                            fifo.push( n->fathers[j]);
//                            active[n->fathers[j]->index]=false;
//                        }
//                        nactive--;
//                    }
//                    j++;
//                }
//            }
//        }
//    }

//    for(int i=0; i<graph.size(); i++) {
//        if(graph[i]->active==false) {
//            Node *n=graph[i];
//            int nactive=0;
//            for(int j=0; j<n->fathers.size(); j++) {
//                if(n->fathers[j]->active==true) nactive++;
//            }
//            assert(nactive==0 || nactive==1);
//        }
//    }

//}


Image<RGB> CGraph::constructImageInf()
{
    std::queue<Node *> fifo;

    vector <bool> active(graph.size());
    active.assign(active.size(), true);

    OrderedQueueDouble <Node *> pq;

    vector<bool> processed(graph.size());

    RGB rgbmin(0, 0, 0);

    /** Warning : nodes[0] does not exist !!!! */
    for (int i = 0; i < graph.size(); i++) {

        graph[i]->dispColor = rgbmin;

        RGB value = graph[i]->color;

        int R = value[0];
        int G = value[1];
        int B = value[2];
        pq.put(-(R + G + B), graph[i]);
        processed[i] = false;
    }

    /** Special case for the root*/
    root->dispColor = rgbmin;

    while (!pq.empty()) {
        Node *n = pq.get();

        if (n->active == false && n->fathers.size() > 1) {

            int nactive = 0;
            RGB value(255, 255, 255);
            for (int j = 0; j < n->fathers.size(); j++) {
                if (n->fathers[j]->active == true)
                {
                    nactive++;
                    value[0] = std::min(value[0], n->fathers[j]->color[0]);
                    value[1] = std::min(value[1], n->fathers[j]->color[1]);
                    value[2] = std::min(value[2], n->fathers[j]->color[2]);
                }
            }
            //qDebug() << "Node: " << n->label << " "<<  nactive << "\n";
            if (nactive > 1) {
                for (int j = 0; j < n->fathers.size(); j++) {

                    if (n->fathers[j]->active == true) {
                        n->fathers[j]->dispColor = value;
                    }
                }
            }
        }
    }

    Image<RGB> imRes = imSource;
    imRes.fill(0);

    for (int i = 0; i < graph.size(); i++) {


        RGB value = graph[i]->color;

        int R = value[0];
        int G = value[1];
        int B = value[2];
        pq.put((R + G + B), graph[i]);
        processed[i] = false;
    }

    /** Special case for the root*/
    pq.put(-1, root);

    while (!pq.empty()) {
        Node *curNode = pq.get();
        //qDebug() << curNode->index << "(" << curNode->color[0] << "," << curNode->color[1] << ","  << curNode->color[2] << ") \n";
        if (curNode->active == true && curNode->dispColor != rgbmin) {
            curNode->dispColor = curNode->color;
        }

        paintNode(imRes, curNode, curNode->dispColor);

        for (int c = 0; c < curNode->childs.size(); c++) {
            Node *curChild = curNode->childs[c];
            if (curChild->active == false) {
//            if(curNode->dispColor>curChild->dispColor)
                curChild->dispColor = curNode->dispColor;
            }

        }
    }

    return imRes;

}

void CGraph::paintNode(Image <RGB> &imRes, Node *n, RGB &value) {
    // std::cout << "Painting--------------------------------------------- "<< n->index << endl;
    for (int r = 0; r < n->regions.size(); r++) {
        // std::cout << "--Visit region " << r << endl;
        vector<Point<TCoord> > points = rag->nodes[n->regions[r]]->pixels;
        for (int p = 0; p < points.size(); p++) {
            imRes(points[p]) = value;
        }
    }
}


void CGraph::paintNodeSup(Image <RGB> &imRes, Node *n, RGB &value)
{
    for (int r = 0; r < n->regions.size(); r++) {
        assert(n->regions[r] != 0);
        vector<Point<TCoord> > points = rag->nodes[n->regions[r]]->pixels;
        for (int p = 0; p < points.size(); p++) {
            RGB imValue = imRes(points[p]);
            imValue[0] = std::max(imValue[0], value[0]);
            imValue[1] = std::max(imValue[1], value[1]);
            imValue[2] = std::max(imValue[2], value[2]);

            imRes(points[p]) = imValue;
        }
    }
}

Image<RGB> CGraph::constructImage(ColorOrdering *order)
{
    Image<RGB> imRes = imSource;
    imRes.fill(0);
    std::queue<Node *> fifo;

    OrderedQueueDouble <Node *> pq;

    vector<bool> processed(graph.size());

    RGB rgbmin(0, 0, 0);

    for (int i = 0; i < graph.size(); i++) {

        RGB value = graph[i]->color;

        pq.put(-order->getPriority(value), graph[i]);
        processed[i] = false;
    }

    /** Special case for the root*/
    pq.put(-1, root);


    while (!pq.empty()) {
        Node *curNode = pq.get();
        //qDebug() << curNode->index << "(" << curNode->color[0] << "," << curNode->color[1] << ","  << curNode->color[2] << ") \n";
        if (curNode->active == true ) {
            curNode->dispColor = curNode->color;
        }

        paintNode(imRes, curNode, curNode->dispColor);

        for (int c = 0; c < curNode->childs.size(); c++) {
            Node *curChild = curNode->childs[c];
            if (curChild->active == false) {
//            if(curNode->dispColor>curChild->dispColor)
                curChild->dispColor = curNode->dispColor;
            }

        }
    }

    return imRes;
}



Image<RGB> CGraph::constructImageInverse()
{
    Image<RGB> imRes = imSource;
    imRes.fill(0);
    std::queue<Node *> fifo;

    OrderedQueueDouble <Node *> pq;

    vector<bool> processed(graph.size());

    RGB rgbmin(0, 0, 0);

    /** Warning : nodes[0] does not exist !!!! */
    for (int i = 0; i < graph.size(); i++) {

        graph[i]->dispColor = rgbmin;

        RGB value = graph[i]->color;

        int R = value[0];
        int G = value[1];
        int B = value[2];
        pq.put(-(R + G + B), graph[i]);
        processed[i] = false;
    }

    /** Special case for the root*/
    pq.put(-1, root);


    while (!pq.empty()) {
        Node *curNode = pq.get();

        if (curNode->active == true ) {
            curNode->dispColor = curNode->color;
        }

        paintNode(imRes, curNode, curNode->dispColor);

        for (int c = 0; c < curNode->childs.size(); c++) {
            Node *curChild = curNode->childs[c];
            if (curChild->active == false) {
//            if(curNode->dispColor>curChild->dispColor)
                curChild->dispColor = curNode->dispColor;
            }

        }
    }

    return imRes;
}



bool CGraph::notComparable(Image<RGB> &im, Point<TCoord> &p, Point<TCoord> &q)
{
    if (!(im(p) < im(q) ) && !(im(p) > im(q)) && !(im(p) == im(q)))
        return true;
    else return false;

}

int CGraph::writeDot(const char *filename)
{
    if (root != 0)
    {
        std::ofstream file(filename, std::ios_base::trunc  | std::ios_base::binary);
        if (!file)
        {
            std::cerr << "File I/O error\n";
            return 0;
        }

        /** Maximum label of the graph **/
        int labelMax = graph.size() + 1;
        bool isActive[labelMax];
        for (int i = 0; i < labelMax; i++) isActive[i] = true;

        file << "digraph G {\n";

        std::queue <Node *> fifo;
        fifo.push(root);
        while (!fifo.empty() )
        {
            Node *tmp = fifo.front();
            fifo.pop();

            std::stringstream nodeName;
            nodeName.str("");
            nodeName << "\"" << tmp->index << ",("

                     // << tmp->color[0]<<","<< tmp->color[1]<<","<< tmp->color[2]<< "):("

                     << (int)tmp->closest_sig_anc.size() << "," << (int)tmp->object_id << "," << (int)tmp->fathers.size() <<
                     " a=" << tmp->area << ")\"";
            // if(!tmp->active)
            //     file << "\t" << nodeName.str() << "[style=filled, fillcolor=grey];\n";
            if (tmp->sn1 > 0 ) {
                file << "\t" << nodeName.str() << "[style=filled, fillcolor=yellow];\n";
            }
            if (tmp->object_id >= 0 ) {
                file << "\t" << nodeName.str() << "[style=filled, fillcolor=red];\n";
            }
            for (int i = 0; i < tmp->childs.size(); i++)
            {
                std::stringstream nodeNameChild;
                nodeNameChild << "\"" << tmp->childs[i]->index << ",("

                              // << tmp->childs[i]->color[0]<<","<< tmp->childs[i]->color[1]<<","<< tmp->childs[i]->color[2]<< "):("

                              << (int)tmp->childs[i]->closest_sig_anc.size() << "," << (int)tmp->childs[i]->object_id << "," <<
                              (int)tmp->childs[i]->fathers.size() << " a=" << tmp->childs[i]->area  << ")\"";

                file << "\t" <<
                     nodeName.str() << "->" << nodeNameChild.str() << ";\n";

                if (isActive[tmp->childs[i]->index] == true)
                {
                    fifo.push(tmp->childs[i]);
                    isActive[tmp->childs[i]->index] = false;
                }
            }

        }

        file << "}\n";
        file.close();
        return 1;
    }
    else
        return 0;
}

int CGraph::writeDotForHeatmap(const char *filename) {
    if (root != 0) {
        std::ofstream file(filename, std::ios_base::trunc  | std::ios_base::binary);
        if (!file) {
            std::cerr << "File I/O error\n";
            return 0;
        }

        /** Maximum label of the graph **/
        int labelMax = graph.size() + 1;
        bool isActive[labelMax];
        for (int i = 0; i < labelMax; i++) isActive[i] = true;
        file << "digraph G {\n";
        std::queue <Node *> fifo;
        fifo.push(root);
        while (!fifo.empty() ) {
            Node *tmp = fifo.front();
            fifo.pop();
            //Add node name
            std::stringstream nodeName;
            nodeName.str("");
            nodeName << tmp->index;
            //Add node reference pixels
            std::stringstream pix;
            pix.str("");
            // pix << "pixels=\" ";
            // for (int r = 0; r < tmp->regions.size(); r++) {
            //     vector<Point<TCoord> > points = rag->nodes[tmp->regions[r]]->pixels;
            //     for (int p = 0; p < points.size(); p++) {
            //         pix << points[p].get_index((int)imSource.getSizeX()) << ";";
            //     }
            // }
            // pix << "\"";

            //Add node attributes
            std::stringstream att;
            att.str("");
            att << "[" << "index=" << tmp->index
                // << ", " << "sn1_v=" << std::fixed << tmp->get_snes_v_min()
                // << ", " << "sn1_v=\"" << tmp->get_snes_v_min() <<"\""
                << ", " << "sn1_v=\"" << my::to_string(tmp->get_snes_v_min())  << "\""
                // << ", " << pix.str()
                << ", sn1=" << tmp->sn1
                << ", sim=" << tmp->similarity
                << ", obj_id=" << tmp->object_id
                << ", mb=" << tmp->writeDotForHeatmap_get_mb()
                << ", mdepth=" << tmp->depth
                << "]";
            file << "\t" << nodeName.str() << att.str() << ";\n";

            //Add relations with childs
            for (int i = 0; i < tmp->childs.size(); i++) {
                std::stringstream nodeNameChild;
                nodeNameChild << tmp->childs[i]->index;
                file << "\t" << nodeName.str() << "->" << nodeNameChild.str() << ";\n";

                //Push childs to queue
                if (isActive[tmp->childs[i]->index] == true) {
                    fifo.push(tmp->childs[i]);
                    isActive[tmp->childs[i]->index] = false;
                }
            }
        }
        file << "}\n";
        file.close();
        return 1;
    }
    else
        return 0;
}

int CGraph::writeDotSN(const char *filename) {
    // int start_id = 1082; //For 2DN8
    // int start_id = 873;//for 2DN4, real_test_im1
    // int start_id = 109;//for 2DN4, fake_test_3
    int start_id = 1871;// fake_test_4
    if (root != 0)
    {
        std::ofstream file(filename, std::ios_base::trunc  | std::ios_base::binary);
        if (!file)
        {
            std::cerr << "File I/O error\n";
            return 0;
        }

        /** Maximum label of the graph **/
        int labelMax = graph.size() + 1;
        bool isActive[labelMax];
        for (int i = 0; i < labelMax; i++) isActive[i] = true;

        file << "digraph G {\n";

        std::queue <Node *> fifo;
        fifo.push(graph[start_id]);
        // fifo.push(graph[1092]);
        // fifo.push(graph[1297]);

        while (!fifo.empty() )
        {
            Node *tmp = fifo.front();
            fifo.pop();
            if (tmp->sn1 <= 0) { //skip non-sn1 node
                continue;
            }

            std::stringstream nodeName;
            nodeName.str("");
            nodeName << "\"" << tmp->index << ":(" << (int)tmp->closest_sig_anc.size() << "," <<
                     (int)tmp->object_id << "," <<
                     (int)tmp->fathers.size() <<
                     ",d=" << tmp->depth <<
                     " a=" << tmp->area << ")\"";
            if (tmp->sn1 > 0 ) {
                file << "\t" << nodeName.str() << "[style=filled, fillcolor=yellow];\n";
            }
            if (tmp->object_id >= 0 ) {
                file << "\t" << nodeName.str() << "[style=filled, fillcolor=red];\n";
            }
            for (int i = 0; i < tmp->childs.size(); i++)
            {
                if (tmp->childs[i]->sn1 > 0) {
                    std::stringstream nodeNameChild;
                    nodeNameChild << "\"" << tmp->childs[i]->index << ":(" << (int)tmp->childs[i]->closest_sig_anc.size() << "," <<
                                  (int)tmp->childs[i]->object_id << "," <<
                                  (int)tmp->childs[i]->fathers.size() <<
                                  ",d=" << (int)tmp->childs[i]->depth <<
                                  " a=" << tmp->childs[i]->area  << ")\"";

                    file << "\t" <<
                         nodeName.str() << "->" << nodeNameChild.str() << ";\n";

                    if (isActive[tmp->childs[i]->index] == true)
                    {
                        fifo.push(tmp->childs[i]);
                        isActive[tmp->childs[i]->index] = false;
                    }
                }
            }

        }

        file << "}\n";

        file.close();
        return 1;
    }
    else
        return 0;
}

int CGraph::writeDotFrom(const char *filename, int start_id) {
    if (root != 0)
    {
        std::ofstream file(filename, std::ios_base::trunc  | std::ios_base::binary);
        if (!file)
        {
            std::cerr << "File I/O error\n";
            return 0;
        }

        /** Maximum label of the graph **/
        int labelMax = graph.size() + 1;
        bool isActive[labelMax];
        for (int i = 0; i < labelMax; i++) isActive[i] = true;

        file << "digraph G {\n";

        std::queue <Node *> fifo;
        if (start_id == -1) {
            fifo.push(root);
        } else {
            fifo.push(graph[start_id]);
            fifo.push(graph[1969]);
            fifo.push(graph[1421]);
        }
        while (!fifo.empty() )
        {
            Node *tmp = fifo.front();
            fifo.pop();

            std::stringstream nodeName;
            nodeName.str("");
            nodeName << "\"" << tmp->index << ":(" << (int)tmp->closest_sig_anc.size() << "," <<
                     (int)tmp->object_id << "," <<
                     (int)tmp->fathers.size() <<
                     ",d=" << tmp->depth <<
                     " a=" << tmp->area << ",(" << tmp->color[0] << "," << tmp->color[1] << "," << tmp->color[2] << ")\"";
            if (tmp->sn1 > 0 ) {
                file << "\t" << nodeName.str() << "[style=filled, fillcolor=yellow];\n";
            }
            if (tmp->object_id >= 0 ) {
                file << "\t" << nodeName.str() << "[style=filled, fillcolor=red];\n";
            }
            for (int i = 0; i < tmp->childs.size(); i++)
            {
                std::stringstream nodeNameChild;
                nodeNameChild << "\"" << tmp->childs[i]->index << ":(" << (int)tmp->childs[i]->closest_sig_anc.size() << "," <<
                              (int)tmp->childs[i]->object_id << "," <<
                              (int)tmp->childs[i]->fathers.size() <<
                              ",d=" << (int)tmp->childs[i]->depth <<
                              " a=" << tmp->childs[i]->area  << ",(" << tmp->childs[i]->color[0] << "," << tmp->childs[i]->color[1]
                              << "," << tmp->childs[i]->color[2] << ")\"";

                file << "\t" <<
                     nodeName.str() << "->" << nodeNameChild.str() << ";\n";

                if (isActive[tmp->childs[i]->index] == true)
                {
                    // if (tmp->childs[i]->index < 4000){
                    fifo.push(tmp->childs[i]);
                    isActive[tmp->childs[i]->index] = false;
                    // }
                }
            }

        }

        file << "}\n";

        file.close();
        return 1;
    }
    else
        return 0;
}

//Export graph for python visualization
int CGraph::writeDotForPython(const char *filename) {
    if (root != 0) {
        std::ofstream file(filename, std::ios_base::trunc  | std::ios_base::binary);
        if (!file) {
            std::cerr << "File I/O error\n";
            return 0;
        }

        /** Maximum label of the graph **/
        int labelMax = graph.size() + 1;
        bool isActive[labelMax];
        for (int i = 0; i < labelMax; i++) isActive[i] = true;
        file << "digraph G {\n";
        std::queue <Node *> fifo;
        fifo.push(root);
        while (!fifo.empty() ) {
            Node *tmp = fifo.front();
            fifo.pop();
            //Add node name
            std::stringstream nodeName;
            nodeName.str("");
            nodeName << tmp->index;
            //Add node reference pixels
            std::stringstream pix;
            pix.str("");
            pix << "pixels=\" ";
            for (int r = 0; r < tmp->regions.size(); r++) {
                vector<Point<TCoord> > points = rag->nodes[tmp->regions[r]]->pixels;
                for (int p = 0; p < points.size(); p++) {
                    pix << points[p].get_index((int)imSource.getSizeX()) << ";";
                }
            }
            pix << "\"";

            //Add node attributes
            std::stringstream att;
            att.str("");//std::fixed
            att << "[" << "index=" << tmp->index << ", " << "sn1_v=" << std::scientific << tmp->sn1_value << ", " << pix.str() <<
                ", sn1=" << tmp->sn1 << ", obj_id=" << tmp->object_id << ", mdepth=" << tmp->depth << "]";
            file << "\t" << nodeName.str() << att.str() << ";\n";

            //Add relations with childs
            for (int i = 0; i < tmp->childs.size(); i++) {
                std::stringstream nodeNameChild;
                nodeNameChild << tmp->childs[i]->index;
                file << "\t" << nodeName.str() << "->" << nodeNameChild.str() << ";\n";

                //Push childs to queue
                if (isActive[tmp->childs[i]->index] == true) {
                    fifo.push(tmp->childs[i]);
                    isActive[tmp->childs[i]->index] = false;
                }
            }
        }
        file << "}\n";
        file.close();
        return 1;
    }
    else
        return 0;
}

/** check if set n is equal to m */
bool CGraph::isEqual(Node *n, Node *m) {

    if (n->area != m->area) return false;

    for (int i = 0; i < n->pixels.size(); i++) {
        TOffset curPixel = n->pixels[i];
        bool curPixelIncluded = false;
        for (int j = 0; j < m->pixels.size(); j++) {
            if (curPixel == m->pixels[j])
            {
                curPixelIncluded = true;
            }
        }
        if (curPixelIncluded == false) return false;
    }

    return true;
}



/** Test if set n is included in m
*/
bool CGraph::isIncluded(Node *n, Node *m, vector<bool> &tmp)
{
    tmp.assign(tmp.size(), false);
    for (int i = 0; i < m->pixels.size(); i++)
        tmp[m->pixels[i]] = true;

    for (int i = 0; i < n->pixels.size(); i++)
        if (tmp[n->pixels[i]] == false) return false;
    return true;
}

void CGraph::computeLinks(ColorOrdering *order, vector <Node *> &nodes)
{
    vector<bool> tmp(imSource.getBufSize(), false);
    for (int i = 0; i < nodes.size(); i++) {
        std::cout << "computeLinks " << i << " / " << nodes.size() << endl;
        Node *n = nodes[i];
        for (int j = 0; j < nodes.size(); j++) {
            if (j != i && nodes[j]->area <= nodes[i]->area && order->islessequal(nodes[i]->color, nodes[j]->color)) {
                Node *m = nodes[j];
                if (!order->isequal(m->color, n->color) && isIncluded(m, n, tmp)) {
                    n->addChild(m);
                }
            }
        }
    }
}



CGraph::Node * CGraph::addRoot(vector <Node *> &nodes)
{
    Node *root = new Node(-1, 0 , 0);

    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i]->fathers.size() == 0) {
            root->addChild(nodes[i]);
        }
    }
    return root;
}


vector <CGraph::Node *> CGraph::minimalElements(ColorOrdering *order, vector <Node *> &nodes, vector <bool> &tmp) {
    vector <Node *> res;

    vector <bool> active(nodes.size(), true);
    for (int j = 0; j < nodes.size(); j++) {
        RGB i = nodes[j]->color;
        for (int k = 0; k < nodes.size(); k++) {
            if (k != j) {
                RGB value2 = nodes[k]->color;
                if (order->islessequal(i, value2) && isIncluded(nodes[k], nodes[j], tmp) ) {
                    active[k] = false;
                }
            }
        }
    }

    for (int j = 0; j < nodes.size(); j++) {
        if (active[j]) res.push_back(nodes[j]);
    }

    return res;
}

/** Compute transitive reduction of graph from its root
* For each node:
*  - keep as childs only the minimal elements of the childs
*/
void CGraph::computeTransitiveReduction(ColorOrdering *order, vector<Node *> &nodes)
{
    vector<bool> tmp(imSource.getBufSize(), false);
    for (int i = 0; i < nodes.size(); i++) {
        std::cout << "computeTransitiveReduction " << i << " / " << nodes.size() << endl;
        Node *curNode = nodes[i];
        curNode->childs = minimalElements(order, curNode->childs, tmp);
    }
}

/** Compute the nodes for G and \dot G component-graph
**/
vector<CGraph::Node *> CGraph::computeComponents(ColorOrdering *order, OrderedQueueDouble <RGB> &pq)
{
    int dx = imSource.getSizeX();
    int dy = imSource.getSizeY();

    vector <Node *> nodes;

    Image<bool> active(imSource.getSize());
    Image<bool> inQueue(imSource.getSize());

    inQueue.fill(false);
    std::queue<Point<TCoord > > fifo;

    int index = 1;

    while (!pq.empty())
    {
        std::cout << "pq.size() = " << pq.size() << endl;
        RGB value = pq.get();

        active.fill(true);

        int ncomposantes = 0;

        for (int y = 0; y < dy; y++)
            for (int x = 0; x < dx; x++) {

                if (active(x, y) && order->islessequal(value, imSource(x, y))) {
                    /** Construct a new node and begin a propagation
                    **/
                    Point <TCoord> p(x, y);
                    inQueue.fill(false);

                    fifo.push(p);
                    ncomposantes++;
                    inQueue(p) = true;

                    Node *n = new Node(index, value, 0);
                    index++;

                    while (!fifo.empty()) {
                        Point<TCoord> curPt = fifo.front();
                        fifo.pop();

                        active(curPt) = false;

                        n->area++;
                        n->pixels.push_back(imSource.getOffset(curPt));
                        for (int i = 0; i < connexity.getNbPoints(); i++) {
                            Point<TCoord> q = curPt + connexity.getPoint(i);

                            if (imSource.isPosValid(q)) {

                                if (inQueue(q) == false && order->islessequal(n->color, imSource(q)) ) {
                                    fifo.push(q);
                                    inQueue(q) = true;
                                }
                            }
                        }
                    }
                    nodes.push_back(n);
                }

            }
        if (ncomposantes != 0) {
            std::cout << "Level: " << (int)value[0] << " " << (int)value[1] << " " << (int)value[2] << "\n";
            std::cout << "N Composantes : " << ncomposantes << "\n";
        }
    }

    return nodes;
}



/**
* Compute component-graph G
* -compute all components
* -compute inclusion relations/ construct links
* -compute the transitive reduction
**/
int CGraph::computeGraphFull(ColorOrdering *order, CGraphWatcher *watcher)
{
    Image <RGB> im = this->imSource;
    OrderedQueueDouble <RGB> pq;

    vector<Node *> nodes;

    vector <RGB> colorProcessed;

    /** Put all colors in priority queue
    **/
    // OrderedQueueDouble <int> pq;
    /** Warning : nodes[0] does not exist !!!! */
    for (int i = 1; i < rag->nodes.size(); i++) {
        RGB value = rag->nodes[i]->color;
        F32 priority = order->getPriority(value);
        pq.put(priority , value);
    }

    // for(int r=0; r<256; r++)
    //     for(int g=0; g<256; g++)
    //         for(int b=0; b<256; b++)
    //         {
    //             RGB value(r,g,b);
    //             pq.put(order->getPriority(value),value);

    //         }

    // float step = pow(10, -13);
    // for(int r=0; r<30; r++)
    //     for(int g=0; g<45; g++)
    //         for(int b=0; b<45; b++)
    //         {
    //             RGB value(step*r, step*g, step*b);
    //             value.print_table();
    //             pq.put(order->getPriority(value),value);

    //         }

    std::cout << "Begin compute components\n";
    clock_t c1 = clock();

    nodes = computeComponents(order, pq);

    std::cout << "Number of nodes: " << nodes.size() << "\n";

    clock_t c2 = clock();
    std::cout << "End compute components Time : " << (double)(c2 - c1) * 1000 / CLOCKS_PER_SEC << "ms\n\n";

    std::cout << "Begin compute links\n";
    c1 = clock();
    computeLinks(order, nodes);
    c2 = clock();
    std::cout << "End compute links Time : " << (double)(c2 - c1) * 1000 / CLOCKS_PER_SEC << "ms\n\n";

    for (int i = 0; i < nodes.size(); i++)
        graph.push_back(nodes[i]);
    root = addRoot(nodes);

    writeDot("fulllinks.dot");
//    std::cout << "Begin compute transitive reduction\n";
//    c1=clock();
    computeTransitiveReduction(order, nodes);
//    c2=clock();
//    std::cout << "End compute transitive reduction time : " << (double)(c2-c1)*1000/CLOCKS_PER_SEC << "ms\n\n";

    writeDot("fullgraph.dot");
    return 0;
}

void CGraph::cal_attribute_isect(ColorOrdering *order) {
    //Visit tree top down
    OrderedQueueDouble <Node *> pq;
    vector<bool> processed(graph.size());
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(-order->getPriority(value), graph[i]);
    }
    int intersection = 0;
    while (!pq.empty()) {
        Node *curNode = pq.get();
        bool skip = curNode->isIntersectSkip();
        if (skip) {
            intersection += 1;
        }
    }
}

void CGraph::simplify_graph(ColorOrdering *order) {
    //Visit tree top down
    OrderedQueueDouble <Node *> pq;
    vector<bool> processed(graph.size());
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(-order->getPriority(value), graph[i]);
    }
    int counter = 0;
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->sn1 && curNode->intersect) {
            counter++;

        }
    }
}

//Similarity score = intersect/union
double get_score(std::set<int> set_a, std::set<int> set_b) {
    set<int> shared;
    set_intersection(set_a.begin(), set_a.end(), set_b.begin(), set_b.end(),
                     std::inserter(shared, shared.begin()));
    int max_area = shared.size();
    return (1.0 * max_area) / (set_a.size() + set_b.size() - max_area); // (intersect/union)
}

//Cal top similarity from all available nodes vs. ground truth set
void CGraph::compare_two_graph(CGraph *ct_cgraph) {
    int graph_size = graph.size();
    int graph_size_ct = ct_cgraph->graph.size();
    int found = 0;
    int counter = 0;
    std::cout << "ct = " << graph_size_ct << " cg = " << graph_size << endl;
    std::vector<float> v;

    for (int i = 0; i < graph_size_ct; i++) {
        Node *curNode_ct = ct_cgraph->graph[i];

        //Verify existence in CG
        std::cout << "checking i = " << i << endl;
        std::set<int> set_ct = curNode_ct->get_set_pixels(ct_cgraph->rag);
        if (set_ct.size() <= 1) {
            continue;
        }
        counter += 1;

        //Export node as image
        Image<RGB> one_node = ct_cgraph->visual_one_object(curNode_ct);
        char str[100];
        sprintf(str, "top_sim/two/ct_%d.ppm", i);
        one_node.save(str);

        float top_score = 0;
        int top_index = -1;
        for (int j = 0; j < graph_size; j++) {
            Node *curNode = graph[j];
            std::set<int> set_cg = curNode->get_set_pixels(rag);
            float score = get_score(set_ct, set_cg);
            if (score > top_score) {
                top_score = score;
                top_index = j;
            }
        }

        //Export node as image
        if (top_index >= 0) {
            Image<RGB> one_node = CGraph::visual_one_object(graph[top_index]);
            char str[100];
            sprintf(str, "top_sim/two/ct_%d_cg_%d_%f.ppm", i, top_index, top_score);
            one_node.save(str);
        }

        //Count
        if (top_score == 1) {
            found += 1;
        }
        v.push_back(top_score);
    }
    for (auto value : v) {
        std::cout << value << "; " ;
    }
    std::cout << "found " << found << "/" << counter << endl;
}

double CGraph::top_similar_node(std::set<int> set_pixels_gt, bool is_center_check) {
    // std::cout << " is check = "<< is_center_check << endl;
    double top = 0 ;//intersect/union in [0,1]
    int center_index = 10 * 50 + 15; //Obj center at [10, 15], img size (50, 25)
    int graph_size = graph.size();
    int top_index = 0;

    // //Check node = Union of childs + Added pixels
    // for (int i = 0; i < graph_size; i++){
    //     std::set<int> set_a = set_pixels_gt;
    //     std::set<int> set_b = graph[i]->get_set_pixels(rag);

    //     if (is_center_check){
    //         if (set_b.count(center_index) == 0){
    //             //This node doesn't include gt center -> skip
    //             continue;
    //         }
    //     }

    //     set<int> shared;
    //     set_intersection(set_a.begin(),set_a.end(),set_b.begin(),set_b.end(), std::inserter(shared,shared.begin()));
    //     int max_area = shared.size();
    //     float score = (1.0*max_area)/(set_a.size() + set_b.size() - max_area); // (intersect/union)
    //     if (score > top){
    //         top = score;
    //         top_index = i;
    //     }
    // }

    //Check node = Union of childs only
    for (int i = 0; i < graph_size; i++) {
        std::set<int> set_a = set_pixels_gt;
        std::set<int> set_b;
        //Only get Union of childs:
        Node *curNode = graph[i];
        // curNode->print_node();

        //Export ppm
        Image<RGB> one_node = CGraph::visual_one_object(curNode);
        char str[100];
        sprintf(str, "top_sim/ppm123/sim_%d.ppm", i);
        one_node.save(str);

        //Add child first
        int width = rag->get_imsource_width();
        for (int r = 0; r < curNode->regions.size(); r++) {
            std::vector<Point<TCoord>> points = rag->nodes[curNode->regions[r]]->pixels;
            for (int p = 0; p < points.size(); p++) {
                // points[p].print();
                if (set_b.count(points[p].get_index(width)) <= 0) {
                    set_b.insert(points[p].get_index(width));
                }
            }
        }


        for (int i = 0; i < curNode->childs.size(); i++) {
            // curNode->childs[i]->print_node();
            std::set<int> set_i = curNode->childs[i]->get_set_pixels(rag);
            if (!set_i.empty()) {
                set_b.insert(set_i.begin(), set_i.end());
                double score = get_score(set_a, set_b);
                if (score > top) {
                    top = score;
                    top_index = i;
                }
            }
        }

        // if (is_center_check){
        //     if (set_b.count(center_index) == 0){
        //         //This node doesn't include gt center -> skip
        //         continue;
        //     }
        // }

    }

    // // Visualize top score node
    // Image<RGB> one_node = CGraph::visual_one_object(graph[top_index]);
    // char str[100];
    // sprintf(str, "ct_top_sim_%d_%f.ppm", top_index, top);
    // one_node.save(str);

    // for (int i = 0; i < graph[top_index]->fathers.size(); i++){
    //     int par_index = graph[top_index]->fathers[i]->index;
    //     one_node = CGraph::visual_one_object(graph[par_index]);
    //     sprintf(str, "cgo_par_%d.ppm", par_index);
    //     one_node.save(str);
    // }
    // for (int i = 0; i < graph[top_index]->childs.size(); i++){
    //     int par_index = graph[top_index]->childs[i]->index;
    //     one_node = CGraph::visual_one_object(graph[par_index]);
    //     sprintf(str, "cgo_child_%d.ppm", par_index);
    //     one_node.save(str);
    // }

    return top;
}

static RGB synthesize_supremum(std::vector<CGraph::Node *> fathers) { //Supremum
    RGB syn(0, 0, 0);
    int size = fathers.size();
    for (int i = 0; i < size; i++) {
        syn.get_supremum(fathers[i]->color);
    }
    return syn;
}

//Check SN for one node
std::vector<double> sn_check(CGraph::Node* curNode, RGB &syn, double alpha) {
    std::cout << curNode->index << " ----------\n";
    std::vector<double> sn_v = std::vector<double> (curNode->N, -1);
    bool sn_all = false;
    double combined_p_norm = 0;
    for (int i = 0; i < curNode->N; i++) {
        if (curNode->color[i] == syn[i]) {
            continue;
        }
        double p_norm = curNode->powers_norm[i];
        combined_p_norm += p_norm;
        double area = curNode->area;
        if (area > 256 * 256) {
            p_norm = (256 * 256) * p_norm / area;
        }
        curNode->snes_v[i] = gsl_cdf_chisq_Q(p_norm, area);
        sn_v[i] = curNode->snes_v[i];
        curNode->snes[i] = curNode->snes_v[i] < alpha;
        std::cout << curNode->snes_v[i] << endl;

        // // OPTION 1: sn all iff all sn_i is sn
        // if (!curNode->snes[i]){
        //     sn_all = false;
        // }

        //OPTION 2: sn all iff any sn_i is sn
        if (curNode->snes[i]) {
            sn_all = true;
            // break;
        }
    }
    curNode->sn1 = sn_all;
    if (curNode->index == 497 or curNode->index == 1002) {
        // getchar();
    }

    return sn_v;
}

//MTO
//Add sn1 attribute true/false
void CGraph::cal_attribute_sn1(ColorOrdering *order) {
    // 1. Check significant
    // double alpha = pow(10, -6);//For testing
    //Visit tree bottom-up

    OrderedQueueDouble <Node *> pq;
    vector<bool> processed(graph.size());
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
        processed[i] = false;
    }
    /** Special case for the root*/
    // pq.put(-1,root);
    int sn1_counter = 0;
    int early_stop_counter = 0;
    int check_counter = 0;
    int img_size = imSource.getBufSize();
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (pq.size() % 1000 == 0) {
            std::cout << curNode->index << " sn check remain " << pq.size() << "/ " << graph.size() << endl;
        }
        RGB syn = synthesize_supremum(curNode->fathers);
        //Skip virtual root node and real_root//We can also add minimum area requirement here.
        int min_area = 0;//test
        if (curNode->index <= 1 || curNode->area <= min_area || curNode->area == img_size) {
            continue;
        }

        //Checking SN
        double combined_p_norm = 0.0;
        double combined_area = 3 * curNode->area;
        //1. Checking on saparate bands first
        bool sn_all = false;
        for (int i = 0; i < curNode->N; i++) {
            bool skip = false;
            // Check if this node at this band have the same value as its parent.
            if (curNode->color[i] == syn[i]) {
                //Skip: means marking this node at this band as NOT significant
                std::cout << "SN checking: skip node " << curNode->index << " at band " << i << endl;
                skip = true;
                continue;
            }

            double p_norm = curNode->powers_norm[i];
            combined_p_norm += p_norm;
            double area = curNode->area;
            if (area > 256 * 256) {
                p_norm = (256 * 256) * p_norm / area;
            }
            curNode->snes_v[i] = gsl_cdf_chisq_Q(p_norm, area);
            curNode->snes[i] = curNode->snes_v[i] < this->alpha;
            // std::cout << curNode->snes_v[i] <<endl;

            // // OPTION 1: sn all iff all sn_i is sn
            // if (!curNode->snes[i]){
            //     sn_all = false;
            // }

            //OPTION 2: sn all iff any sn_i is sn
            if (curNode->snes[i]) {
                sn_all = true;
                break;
            }
        }

        //Check sn on intermediate parents
        if (false) {
            // if (sn_all) {
            check_counter++;
            sn_all = false;//Now check again on intermediate parents.
            //Necessary conditions: SN nodes N need to be significant on Supremum of all intermediate parents(N, P)
            std::vector<Node*> ip_vec;
            bool early_stop = false;
            RGB ip_color(-1, -1, -1);
            for (int i = 0; i < curNode->fathers.size(); i++) {
                Node* far = curNode->fathers[i];
                std::vector<Node*> ip_vec_i = far->get_iparents(graph, order, rag, curNode, early_stop, ip_color);
                if (early_stop) {
                    // getchar();
                    // std::cout<< "early_stop \n";
                    break;
                }
                // std::cout << "ip_vec.size = " << ip_vec.size() << endl;
                ip_vec.insert(ip_vec.begin(), ip_vec_i.begin(), ip_vec_i.end());
            }

            //No intermediate parent found
            if (ip_color[0] < 0) {
                sn_all = true;//Already checked on synthesized_parents
                early_stop = true;
                // continue;
            }

            //Check sn on synthesized IParents
            if (early_stop && ip_color[0] >= 0) {
                early_stop_counter++;
                RGB other_syn = curNode->color;
                std::vector<double> pes = curNode->gen_powers_given_bg( gain_vec, sigma2_bg_vec, other_syn);
                for (int k = 0; k < curNode->N; k++) {
                    // if (curNode->color[k] == other_syn[k]) {
                    //     //Skip: means marking this node at this band as NOT significant
                    //     // std::cout<<"SN checking: skip node " << curNode->index << " at band " << k <<endl;
                    //     continue;
                    // }

                    double p_norm = pes[k];
                    double area = curNode->area;
                    if (area > 256 * 256) {
                        p_norm = (256 * 256) * p_norm / area;
                    }
                    double sni = gsl_cdf_chisq_Q(p_norm, area); //compare to this->alpha
                    // std::cout << sni << endl;
                    if (sni < this->alpha) {
                        sn_all = true;
                        std::cout << "early_stop true, keep curNode color as parent color" << endl;
                        break;
                    }
                }
            }

            if (!early_stop) {
                RGB other_syn = ip_color; //synthesize_supremum(ip_vec);
                // curNode->color.print_table();
                // other_syn.print_table();
                std::vector<double> pes = curNode->gen_powers_given_bg( gain_vec, sigma2_bg_vec, other_syn);
                for (int k = 0; k < curNode->N; k++) {
                    if (curNode->color[k] == other_syn[k]) {
                        //Skip: means marking this node at this band as NOT significant
                        // std::cout<<"SN checking: skip node " << curNode->index << " at band " << k <<endl;
                        continue;
                    }

                    double p_norm = pes[k];
                    double area = curNode->area;
                    if (area > 256 * 256) {
                        p_norm = (256 * 256) * p_norm / area;
                    }
                    double sni = gsl_cdf_chisq_Q(p_norm, area); //compare to this->alpha
                    // std::cout << sni << endl;
                    if (sni < this->alpha) {
                        sn_all = true;
                        break;
                    }
                }

                // //Check on combined bands refering to other_syn of intermediate parents
                // if (!sn_all && curNode->color[0] == other_syn[0] &&curNode->color[1] == other_syn[1] &&curNode->color[2] == other_syn[2]){
                //     sn1_counter++;
                //     std::cout <<"check combined bands. \n";
                //     double p_norm = pes[0] + pes[1] + pes[2];
                //     double area = 3*curNode->area;
                //     if (area > 256 * 256){
                //         p_norm = (256*256)*p_norm/area;
                //     }
                //     double sni = gsl_cdf_chisq_Q(p_norm, area); //compare to this->alpha
                //     std::cout << sni << endl;
                //     if (sni < this->alpha){
                //         sn_all = true;
                //     }
                //     getchar();
                // }

                // } else{
                //     //THIS IS WHEN current Node color == to its parent's color
                //     //Try to check child of curNode on parent colors.
                //     std::cout << "early_stop check sn \n";
                //     for (int c = 0; c < curNode->childs.size(); ++c){
                //         curNode->childs[c]->color.print_table();
                //         curNode->color.print_table();
                //         std::vector<double> c_v = sn_check(curNode->childs[c] , curNode->color, this->alpha);
                //     }

                //     std::vector<double> test = std::vector<double> (3, 0.0);
                //     RGB other_syn = ip_color;
                //     curNode->color.print_table();
                //     other_syn.print_table();
                //     std::vector<double> pes = curNode->gen_powers_given_bg( gain_vec, sigma2_bg_vec, syn);
                //     for (int k = 0; k < curNode->N; k++){
                //         double p_norm = pes[k];
                //         double area = curNode->area;
                //         if (area > 256 * 256){
                //             p_norm = (256*256)*p_norm/area;
                //         }
                //         test[k] = gsl_cdf_chisq_Q(p_norm, area);
                //         std::cout<< test[k] << endl;
                //     }
                //     // if (test[0] < this->alpha && test[1] < this->alpha && test[2] < this->alpha){
                //     if (test[0] < this->alpha || test[1] < this->alpha || test[2] < this->alpha){
                //         sn_all = true;
                //         sn1_counter++;
                //     }
            }

        }

        if (sn_all) {
            curNode->sn1 = sn_all;
            sn1_counter ++;
        }


        // 2. Now check combined band if sn1 is false in all saparate bands
        // if (!curNode->sn1){
        //     if (combined_area > 256*256){
        //         combined_p_norm = 256*256*combined_p_norm/combined_area;
        //     }
        //     curNode->sn1_value = gsl_cdf_chisq_Q(combined_p_norm, combined_area);
        //     curNode->sn1 = curNode->sn1_value < this->alpha;
        //     if (curNode->sn1){
        //         sn1_counter++;
        //         // curNode->print_node();
        //         // curNode->print_parents();
        //         // std::cout <<curNode->sn1_value << endl;
        //         // std::cout<<combined_area << endl;
        //         // for (int i = 0; i < curNode->N; i++){
        //         //     double p_norm = curNode->powers_norm[i];
        //         //     std::cout << p_norm << " " << i << endl;
        //         // }
        //         // std::cout << "different happens \n";
        //         // getchar();
        //     }
        // }
    }
    std::cout << "sn1_counter = " << sn1_counter << endl;
    std::cout << "stop/check = " << early_stop_counter << "/" << check_counter << endl;
    // getchar();

    // #2. Update closest_sig_anc (top-down) from root_id
    std::vector<int> up_processed = std::vector<int> (graph.size(), 0);
    for (int i = 0; i < graph.size(); i++) {
        up_processed[i] = graph[i]->fathers.size();
    }
    // if (root->sn1){
    //     std::cout<<"root is sn \n";
    //     update_closest_sn_anc_from_id(root, root, order, up_processed);
    // }else{
    //     update_closest_sn_anc_from_id(root, NULL, order, up_processed);
    // }

    //Test another solution for updating closest_sn_anc
    for (int i = 0; i < graph.size(); ++i) {
        update_closest_sn_anc_from_id_new(graph[i]);
    }
    std::cout << "Done update closest_sn_ancestor \n";

    //Visit tree bottom-up
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
    }
    pq.put(-1, root);
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->sn1) {
            update_parent_main_branch(curNode, order);
        }
    }
std: cout << "Done update main_branch \n";

}

//Keep only incomparable maximal nodes
void filter_ip_vec(std::vector<CGraph::Node*> &ip_vec, ColorOrdering* order, CGraph::Node* curNode, RGB& color_sup, RGB& area_sup) {
    //For each band, find: 1. closest color value; 2. largest area of equivalent color value.
    for (int i = 0; i < ip_vec.size(); ++i) {
        color_sup.get_supremum(ip_vec[i]->color);
    }

    for (int i = 0; i < curNode->N; ++i) {
        if (color_sup[i] == curNode->color[i]) {
            //Find largest area in this band
            int area_max = curNode->area;
            for (int j = 0; j < ip_vec.size(); ++j) {
                if (color_sup[i] == ip_vec[j]->color[i] && ip_vec[j]->area > area_max) {
                    area_max = ip_vec[j]->area;
                    std::cout << "band " << i << " update area_max " << area_max << endl;
                }
            }
            area_sup[i] = area_max;
        } else {
            //Use curNode's area, already initialized at the the beginning
        }
    }
    // std::cout << "filter_ip_vec " << curNode->area << " -> " << endl;
    // area_sup.print_table();
    // getchar();

    return;
}



// std::vector<CGraph::Node*> filter_ip_vec(std::vector<CGraph::Node*> &ip_vec, ColorOrdering* order){
//     //Store filtering info
//     std::vector<CGraph::Node*> filter;
//     if (ip_vec.empty()) {
//         return ip_vec;
//     }
//     std::vector<bool> keeps = std::vector<bool>(ip_vec.size(), true);

//     //Filtering
//     for (int v = 0; v < ip_vec.size(); ++v) {
//         CGraph::Node* vec_v = ip_vec[v];
//         // vec_v->print_node();
//         for (int i = v + 1; i < ip_vec.size(); ++i) {
//             CGraph::Node* vec_i = ip_vec[i];
//             if (order->islessequal(vec_v->color, vec_i->color)) {
//                 keeps[v] = false;
//                 continue;
//             }
//         }
//     }

//     //Get maximals
//     for (int i = 0; i < ip_vec.size(); ++i) {
//         if (keeps[i]) {
//             filter.push_back(ip_vec[i]);
//             // ip_vec[i]->print_node();
//         }
//     }
//     // std::cout << "filtered " << ip_vec.size() << " to " << filter.size() << endl;
//     // getchar();

//     return filter;
// }

//MTO
//Add sn1 attribute true/false
void CGraph::cal_attribute_sn2(ColorOrdering *order) {
    //0. Saving all sn values
    std::vector<float> sn_stat;

    // 1. Check significant
    //Visit tree bottom-up
    OrderedQueueDouble <Node *> pq;
    vector<bool> processed(graph.size());
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
        processed[i] = false;
    }
    /** Special case for the root*/
    // pq.put(-1,root);
    int sn1_counter = 0;
    int early_stop_counter = 0;
    int check_counter = 0;
    int img_size = imSource.getBufSize();
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (pq.size() % 1000 == 0) {
            std::cout << curNode->index << " sn check remain " << pq.size() << "/ " << graph.size() << endl;
        }
        RGB syn = synthesize_supremum(curNode->fathers);
        //Skip virtual root node and real_root//We can also add minimum area requirement here.
        int min_area = 0;//test
        if (curNode->index <= 1 || curNode->area <= min_area || curNode->area == img_size) {
            continue;
        }

        //Checking SN
        double combined_p_norm = 0.0;
        double combined_area = 3 * curNode->area;
        //1. Checking on saparate bands first
        bool sn_all = false;
        std::vector<float> tmp;
        for (int i = 0; i < curNode->N; i++) {
            bool skip = false;
            // Check if this node at this band have the same value as its parent.
            if (curNode->color[i] == syn[i]) {
                //Skip: means marking this node at this band as NOT significant
                std::cout << "SN checking: skip node " << curNode->index << " at band " << i << endl;
                skip = true;
                continue;
            }

            double p_norm = curNode->powers_norm[i];
            combined_p_norm += p_norm;
            double area = curNode->area;
            if (area > 256 * 256) {
                p_norm = (256 * 256) * p_norm / area;
            }
            curNode->snes_v[i] = gsl_cdf_chisq_Q(p_norm, area);
            tmp.push_back(curNode->snes_v[i]); //For analysis
            curNode->snes[i] = curNode->snes_v[i] < this->alpha;
            // std::cout << curNode->snes_v[i] <<endl;

            //OPTION 2: sn all iff any sn_i is sn
            if (curNode->snes[i]) {
                sn_all = true;
                break;
            }
        }

        //Check sn on intermediate parents
        if (false) {
            // if (sn_all) {
            tmp.clear();
            check_counter++;
            sn_all = false;//Now check again on intermediate parents.
            //Necessary conditions: SN nodes N need to be significant on Supremum of all intermediate parents(N, P)
            std::vector<Node*> ip_vec;
            for (int i = 0; i < curNode->fathers.size(); i++) {
                Node* far = curNode->fathers[i];
                std::vector<Node*> ip_vec_i = far->get_iparents_vec(graph, order, rag, curNode);
                // std::cout << "ip_vec.size = " << ip_vec.size() << endl;
                ip_vec.insert(ip_vec.begin(), ip_vec_i.begin(), ip_vec_i.end());
            }

            //TODO: Select incomparable maximal of Nodes in ip_vec
            RGB color_sup(-1, -1, -1);
            RGB area_sup(curNode->area, curNode->area, curNode->area);
            filter_ip_vec(ip_vec, order, curNode, color_sup, area_sup);
            RGB other_syn = color_sup;
            std::vector<double> pes = curNode->gen_powers_given_bg( gain_vec, sigma2_bg_vec, other_syn, area_sup);
            for (int k = 0; k < curNode->N; k++) {
                double p_norm = pes[k];
                double area = curNode->area;
                if (curNode->color[k] == other_syn[k]) {
                    area = area_sup[k];
                    // std::cout << curNode->area << " vs. " << area << endl;
                    // getchar();
                }
                if (area > 256 * 256) {
                    p_norm = (256 * 256) * p_norm / area;
                }
                double sni = gsl_cdf_chisq_Q(p_norm, area); //compare to this->alpha
                tmp.push_back(sni);
                // std::cout << sni << endl;
                if (sni < this->alpha) {
                    sn_all = true;
                    break;
                }
            }
        }

        sn_stat.insert( sn_stat.end(), tmp.begin(), tmp.end() );

        if (sn_all) {
            curNode->sn1 = sn_all;
            sn1_counter ++;
        }
    }
    std::cout << "sn1_counter = " << sn1_counter << endl;
    std::cout << "stop/check = " << early_stop_counter << "/" << check_counter << endl;
    // getchar();

    // #2. Update closest_sig_anc (top-down) from root_id
    std::vector<int> up_processed = std::vector<int> (graph.size(), 0);
    for (int i = 0; i < graph.size(); i++) {
        up_processed[i] = graph[i]->fathers.size();
    }

    //Test another solution for updating closest_sn_anc
    for (int i = 0; i < graph.size(); ++i) {
        update_closest_sn_anc_from_id_new(graph[i]);
    }
    std::cout << "Done update closest_sn_ancestor \n";

    //Visit tree bottom-up
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
    }
    pq.put(-1, root);
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->sn1) {
            update_parent_main_branch(curNode, order);
        }
    }
std: cout << "Done update main_branch \n";

    // for (int i = 0; i < sn_stat.size(); ++i){
    //     std::cout<< sn_stat[i] <<", ";
    // }
    // getchar();

}

//MTO
//Add sn1 attribute true/false
void CGraph::cal_attribute_sn_combined_band(ColorOrdering *order) {
    //0. Saving all sn values
    std::vector<float> sn_stat;

    // 1. Check significant
    //Visit tree bottom-up
    OrderedQueueDouble <Node *> pq;
    vector<bool> processed(graph.size());
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
        processed[i] = false;
    }
    /** Special case for the root*/
    // pq.put(-1,root);
    int sn1_counter = 0;
    int early_stop_counter = 0;
    int check_counter = 0;
    int img_size = imSource.getBufSize();
    RGB bg_color(0, 0, 0);
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (pq.size() % 1000 == 0) {
            std::cout << curNode->index << " sn check remain " << pq.size() << "/ " << graph.size() << endl;
        }
        RGB syn = synthesize_supremum(curNode->fathers);
        //Skip virtual root node and real_root//We can also add minimum area requirement here.
        int min_area = 0;//test
        if (curNode->index <= 1 || curNode->area <= min_area || curNode->area == img_size) {
            continue;
        }

        //Checking SN ONLY ON combined band
        double combined_p_norm = 0.0;
        double combined_area = 0; //3 * curNode->area;
        std::vector<float> tmp;
        for (int i = 0; i < curNode->N; i++) {
            if (curNode->color[i] == bg_color[i]) {
                //Extending in band_i would cover the whole image
                continue;
            }
            double p_norm = curNode->powers_norm[i];
            double area = curNode->area;
            if (area > 256 * 256) {
                combined_area += 256 * 256;
                p_norm = (256 * 256) * p_norm / area;
            } else {
                combined_area += area;
            }
            combined_p_norm += p_norm;

            curNode->snes_v[i] = gsl_cdf_chisq_Q(p_norm, area);
            curNode->snes[i] = curNode->snes_v[i] < this->alpha;
        }
        if (combined_area > 256 * 256) {
            combined_p_norm = 256 * 256 * combined_p_norm / combined_area;
        }
        curNode->sn1_value = gsl_cdf_chisq_Q(combined_p_norm, combined_area);
        curNode->sn1 = curNode->sn1_value < this->alpha;
        if (curNode->sn1) {
            sn1_counter++;
        }
    }

    std::cout << "sn1_counter combined only = " << sn1_counter << endl;
    std::cout << "stop/check = " << early_stop_counter << "/" << check_counter << endl;
    // getchar();

    // #2. Update closest_sig_anc (top-down) from root_id
    std::vector<int> up_processed = std::vector<int> (graph.size(), 0);
    for (int i = 0; i < graph.size(); i++) {
        up_processed[i] = graph[i]->fathers.size();
    }

    //Test another solution for updating closest_sn_anc
    for (int i = 0; i < graph.size(); ++i) {
        update_closest_sn_anc_from_id_new(graph[i]);
    }
    std::cout << "Done update closest_sn_ancestor \n";

    //Visit tree bottom-up
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
    }
    pq.put(-1, root);
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->sn1) {
            update_parent_main_branch(curNode, order);
        }
    }
std: cout << "Done update main_branch \n";
}

//MTO
//Add sn1 attribute true/false
void CGraph::cal_attribute_sn_both_sn(ColorOrdering *order) {
    //0. Saving all sn values
    std::vector<float> sn_stat;

    // 1. Check significant
    //Visit tree bottom-up
    OrderedQueueDouble <Node *> pq;
    vector<bool> processed(graph.size());
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
        processed[i] = false;
    }
    /** Special case for the root*/
    // pq.put(-1,root);
    int sn1_counter = 0;
    int early_stop_counter = 0;
    int check_counter = 0;
    int img_size = imSource.getBufSize();
    RGB bg_color(0, 0, 0);
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (pq.size() % 1000 == 0) {
            std::cout << curNode->index << " sn check remain " << pq.size() << "/ " << graph.size() << endl;
        }
        RGB syn = synthesize_supremum(curNode->fathers);
        //Skip virtual root node and real_root//We can also add minimum area requirement here.
        int min_area = 0;//test
        if (curNode->index <= 1 || curNode->area <= min_area || curNode->area == img_size) {
            continue;
        }

        //Checking SN ONLY ON combined band
        std::vector<float> tmp;
        for (int i = 0; i < curNode->N; i++) {
            if (curNode->color[i] == bg_color[i]) {
                //Extending in band_i would cover the whole image
                continue;
            }
            double p_norm = curNode->powers_norm[i];
            double area = curNode->area;
            if (area > 256 * 256) {
                p_norm = (256 * 256) * p_norm / area;
            }
            curNode->snes_v[i] = gsl_cdf_chisq_Q(p_norm, area);
            curNode->snes[i] = curNode->snes_v[i] < this->alpha;
        }
        //OPTION 1: sn all iff all sn_i is sn
        if (curNode->snes[0] && curNode->snes[1] && curNode->snes[2]) {
            curNode->sn1 = true;
        }
        if (curNode->sn1) {
            sn1_counter++;
        }
    }

    std::cout << "sn1_counter sn_both = " << sn1_counter << endl;
    std::cout << "stop/check = " << early_stop_counter << "/" << check_counter << endl;
    // getchar();

    // #2. Update closest_sig_anc (top-down) from root_id
    std::vector<int> up_processed = std::vector<int> (graph.size(), 0);
    for (int i = 0; i < graph.size(); i++) {
        up_processed[i] = graph[i]->fathers.size();
    }

    //Test another solution for updating closest_sn_anc
    for (int i = 0; i < graph.size(); ++i) {
        update_closest_sn_anc_from_id_new(graph[i]);
    }
    std::cout << "Done update closest_sn_ancestor \n";

    //Visit tree bottom-up
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
    }
    pq.put(-1, root);
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->sn1) {
            update_parent_main_branch(curNode, order);
        }
    }
std: cout << "Done update main_branch \n";
}

//Add sn1 attribute true/false
void CGraph::cal_attribute_sn_either_sn(ColorOrdering *order) {
    //0. Saving all sn values
    std::vector<float> sn_stat;

    // 1. Check significant
    //Visit tree bottom-up
    OrderedQueueDouble <Node *> pq;
    vector<bool> processed(graph.size());
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
        processed[i] = false;
    }
    /** Special case for the root*/
    // pq.put(-1,root);
    int sn1_counter = 0;
    int early_stop_counter = 0;
    int check_counter = 0;
    int img_size = imSource.getBufSize();
    RGB bg_color(0, 0, 0);
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (pq.size() % 1000 == 0) {
            std::cout << curNode->index << " sn check remain " << pq.size() << "/ " << graph.size() << endl;
        }
        RGB syn = synthesize_supremum(curNode->fathers);
        //Skip virtual root node and real_root//We can also add minimum area requirement here.
        int min_area = 0;//test
        if (curNode->index <= 1 || curNode->area <= min_area || curNode->area == img_size) {
            continue;
        }

        //Checking SN ONLY ON combined band
        std::vector<float> tmp;
        for (int i = 0; i < curNode->N; i++) {
            if (curNode->color[i] == bg_color[i]) {
                //Extending in band_i would cover the whole image
                continue;
            }
            double p_norm = curNode->powers_norm[i];
            double area = curNode->area;
            if (area > 256 * 256) {
                p_norm = (256 * 256) * p_norm / area;
            }
            curNode->snes_v[i] = gsl_cdf_chisq_Q(p_norm, area);
            curNode->snes[i] = curNode->snes_v[i] < this->alpha;
            if (curNode->snes[i]) {
                curNode->sn1 = true;
                // break;
            }
        }
        if (curNode->sn1) {
            sn1_counter++;
        }
    }

    std::cout << "sn1_counter sn_either_sn = " << sn1_counter << endl;
    std::cout << "stop/check = " << early_stop_counter << "/" << check_counter << endl;
    // getchar();

    // #2. Update closest_sig_anc (top-down) from root_id
    std::vector<int> up_processed = std::vector<int> (graph.size(), 0);
    for (int i = 0; i < graph.size(); i++) {
        up_processed[i] = graph[i]->fathers.size();
    }

    //Test another solution for updating closest_sn_anc
    for (int i = 0; i < graph.size(); ++i) {
        update_closest_sn_anc_from_id_new(graph[i]);
    }
    std::cout << "Done update closest_sn_ancestor \n";

    //Visit tree bottom-up
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
    }
    pq.put(-1, root);
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->sn1) {
            update_parent_main_branch(curNode, order);
        }
    }
std: cout << "Done update main_branch \n";
}

//Keep only incomparable maximal nodes
void check_band_by_band(std::vector<CGraph::Node*> &ip_vec, ColorOrdering* order, CGraph::Node* curNode, RGB& color_sup, RGB& area_sup, Image <RGB> &imSource) {
    color_sup = synthesize_supremum(curNode->fathers);
    for (int k = 0; k < curNode->N; ++k) {

        //We don't care about non-SN band
        if (!curNode->snes[k]) {
            continue;
        }

        //Find ip_color for band_k: maximum color, but < curNode node
        double color_k = color_sup[k];
        for (int i = 0; i < ip_vec.size(); ++i) {
            // condition: color_k < color < curNode
            if (ip_vec[i]->color[k] < curNode->color[k] && ip_vec[i]->color[k] > color_k) {
                color_k = ip_vec[i]->color[k];
            }
        }
        color_sup[k] = color_k;

        //Find ip_area for band_k: Union all ip that share curNode color
        // std::cout << "before: curNode->set_pixels.size = " << curNode->set_pixels.size() << endl;
        for (int i = 0; i < ip_vec.size(); ++i) {
            //curNode in band k is already belonged to another obj.
            if (ip_vec[i]->snes[k]) {
                curNode->snes[k] = false;
                // std::cout << "belonged to other: "<< ip_vec[i]->index <<" \n";
                // getchar();
                break;
            }

            //Non SN node that shares similar color
            if (curNode->color[k] == ip_vec[i]->color[k] && !ip_vec[i]->snes[k]) {
                curNode->set_pixels.insert(ip_vec[i]->set_pixels.begin(), ip_vec[i]->set_pixels.end());
            }
        }
        // std::cout << "after: curNode->set_pixels.size = " << curNode->set_pixels.size() << endl;
        area_sup[k] = curNode->set_pixels.size();

        //Updateing sigx, sigx2 at band_k
        curNode->sigx[k] = 0;
        curNode->sigx2[k] = 0;
        for (auto pixel : curNode->set_pixels) {
            if (imSource.getData()[pixel][k] < curNode->color[k]) {
                std::cout << "Wrong index \n";
                // getchar();
            }
            curNode->sigx[k] += imSource.getData()[pixel][k];
            curNode->sigx2[k] += imSource.getData()[pixel][k] * imSource.getData()[pixel][k];
        }
    }

    return;
}

void CGraph::cal_sn_one_node(Node *curNode, ColorOrdering *order) {
    // std::cout << "Debuging cal_sn_one_node for node " << curNode->index << endl;
    // curNode->print_snes();

    //Object with main_branch ~ obvious object.
    if (curNode->main_branch != NULL) {
        return;
    }
    //Check sn on intermediate parents
    bool sn_all = false;//Now check again on intermediate parents.
    //Necessary conditions: SN nodes N need to be significant on Supremum of all intermediate parents(N, P)
    std::vector<Node*> ip_vec;
    for (int i = 0; i < curNode->fathers.size(); i++) {
        Node* far = curNode->fathers[i];
        std::vector<Node*> ip_vec_i = far->get_iparents_vec(graph, order, rag, curNode);
        ip_vec.insert(ip_vec.begin(), ip_vec_i.begin(), ip_vec_i.end());
        // std::cout << "ip_vec.size = " << ip_vec.size() << endl;
    }

    //TODO: Select incomparable maximal of Nodes in ip_vec
    RGB color_sup(-1, -1, -1);
    RGB area_sup(curNode->area, curNode->area, curNode->area);

    //OPTION 1
    // filter_ip_vec(ip_vec, order, curNode, color_sup, area_sup);

    //OPTION 2
    // area_sup.print_table();
    curNode->set_pixels = curNode->get_set_pixels(rag);
    check_band_by_band(ip_vec, order, curNode, color_sup, area_sup, this->imSource);
    // area_sup.print_table();
    // getchar();

    RGB other_syn = color_sup;
    if (ip_vec.empty()) {
        //Check ok, node remain sn
        return;
    }
    std::vector<double> pes = curNode->gen_powers_given_bg( gain_vec, sigma2_bg_vec, other_syn, area_sup);
    for (int k = 0; k < curNode->N; k++) {
        if (!curNode->snes[k]) {
            // std::cout << "Skip, because band " << k << " non SN band\n";
            continue;
        }
        double p_norm = pes[k];
        double area = curNode->area;
        // if (curNode->color[k] == other_syn[k]) {
        area = area_sup[k];
        // std::cout << curNode->area << " vs. " << area << endl;
        std::cout << synthesize_supremum(curNode->fathers)[k] << " vs. " << color_sup[k] << endl;
        // getchar();
        // }
        if (area > 256 * 256) {
            p_norm = (256 * 256) * p_norm / area;
        }
        double sni = gsl_cdf_chisq_Q(p_norm, area); //compare to this->alpha
        std::cout << sni << endl;
        if (sni < this->alpha) {
            sn_all = true;
            // break;
        }
    }
    if (sn_all) {
        // std::cout << "Remain SN\n";
        // getchar();
    } else {
        curNode->object_id = -1;
        curNode->sn1 = false;
        // std::cout << "Toang\n";
    }
    // getchar();
}

//Add sn1 attribute true/false
void CGraph::cal_attribute_sn_all(ColorOrdering *order) {
    //0. Saving all sn values
    std::vector<float> sn_stat;

    // 1. Check significant
    //Visit tree bottom-up
    OrderedQueueDouble <Node *> pq;
    vector<bool> processed(graph.size());
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
        processed[i] = false;
    }
    /** Special case for the root*/
    // pq.put(-1,root);
    int sn1_counter = 0;
    int early_stop_counter = 0;
    int check_counter = 0;
    int img_size = imSource.getBufSize();
    RGB bg_color(0, 0, 0);

    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (pq.size() % 1000 == 0) {
            // std::cout << curNode->index << " sn check remain " << pq.size() << "/ " << graph.size() << endl;
        }
        RGB syn = synthesize_supremum(curNode->fathers);
        //Skip virtual root node and real_root//We can also add minimum area requirement here.
        int min_area = 0;//test
        if (curNode->index <= 1 || curNode->area <= min_area || curNode->area == img_size) {
            continue;
        }

        //Check on separate bands
        std::vector<float> tmp;
        double combined_p_norm = 0.0;
        double combined_area = 0;
        for (int i = 0; i < curNode->N; i++) {
            if (curNode->color[i] == bg_color[i]) {
                // curNode->color.print_table();
                // syn.print_table();
                // std::cout <<curNode->index << " same value with bg(0 0 0)\n";
                // getchar();
                //Extending in band_i would cover the whole image
                continue;
            }
            if (curNode->color[i] == syn[i]) {
                curNode->color.print_table();
                syn.print_table();
                std::cout << curNode->index << " same value with syn of parents\n";
                // getchar();
            }

            double p_norm = curNode->powers_norm[i];
            double area = curNode->area;
            if (area > 256 * 256) {
                combined_area += 256 * 256;
                p_norm = (256 * 256) * p_norm / area;
            } else {
                combined_area += area;
            }
            combined_p_norm += p_norm;
            curNode->snes_v[i] = gsl_cdf_chisq_Q(p_norm, area);
            curNode->snes[i] = curNode->snes_v[i] < this->alpha;
            if (curNode->snes[i]) {
                curNode->sn1 = true;
                // break;
            }
        }

        //Check on combined band
        if (!curNode->sn1) {
            if (combined_area > 256 * 256) {
                combined_p_norm = 256 * 256 * combined_p_norm / combined_area;
            }
            curNode->sn1_value = gsl_cdf_chisq_Q(combined_p_norm, combined_area);
            curNode->sn1 = curNode->sn1_value < this->alpha;
        }

        if (curNode->sn1) {
            sn1_counter++;
        }
    }

    std::cout << "sn1_counter cal_attribute_sn_all = " << sn1_counter << endl;
    std::cout << "stop/check = " << early_stop_counter << "/" << check_counter << endl;
    // getchar();

    // #2. Update closest_sig_anc (top-down) from root_id
    std::vector<int> up_processed = std::vector<int> (graph.size(), 0);
    for (int i = 0; i < graph.size(); i++) {
        up_processed[i] = graph[i]->fathers.size();
    }

    //Test another solution for updating closest_sn_anc
    for (int i = 0; i < graph.size(); ++i) {
        update_closest_sn_anc_from_id_new(graph[i]);
    }
    std::cout << "Done update closest_sn_ancestor \n";

    //Visit tree bottom-up
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
    }
    pq.put(-1, root);
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->sn1) {
            update_parent_main_branch(curNode, order);
        }
    }
std: cout << "Done update main_branch \n";
}

//Add sn1 attribute true/false parallel
void CGraph::cal_attribute_sn1_parallel(ColorOrdering *order) {
    omp_set_num_threads(2);
    int nthreads, tid;

    /* Fork a team of threads giving them their own copies of variables */
    #pragma omp parallel private(nthreads, tid)
    {

        /* Obtain thread number */
        tid = omp_get_thread_num();
        printf("Hello World from thread = %d\n", tid);

        /* Only master thread does this */
        if (tid == 0)
        {
            nthreads = omp_get_num_threads();
            printf("Number of threads = %d\n", nthreads);
        }

    } /* All threads join master thread and disband */

    #pragma omp parallel for// private(g) //num_threads(4)
    for (int g = 0; g < graph.size(); g++) {
        std::cout << "parallel sn checking " << g << "/ " << graph.size() << endl;
        int id = omp_get_thread_num();
        int total = omp_get_num_threads();
        printf("Greetings from process %d out of %d \n", id, total);
        int img_size = imSource.getBufSize();
        Node* curNode = graph[g];
        RGB syn = synthesize_supremum(curNode->fathers);
        //Skip virtual root node and real_root//We can also add minimum area requirement here.
        int min_area = 1;//test
        if (curNode->index <= 1 || curNode->area < min_area || curNode->area == img_size) {
            continue;
        }
        //Checking SN
        double combined_p_norm = 0.0;
        double combined_area = 3 * curNode->area;
        //1. Checking on saparate bands first
        bool sn_all = false;
        for (int i = 0; i < curNode->N; i++) {
            if (curNode->color[i] == syn[i]) {
                //Skip: means marking this node at this band as NOT significant
                continue;
            }

            double p_norm = curNode->powers_norm[i];
            combined_p_norm += p_norm;
            double area = curNode->area;
            if (area > 256 * 256) {
                p_norm = (256 * 256) * p_norm / area;
            }
            curNode->snes_v[i] = gsl_cdf_chisq_Q(p_norm, area);
            curNode->snes[i] = curNode->snes_v[i] < this->alpha;

            //OPTION 2: sn all iff any sn_i is sn
            if (curNode->snes[i]) {
                sn_all = true;
            }
        }

        //Check sn on intermediate parents
        if (sn_all) {
            sn_all = false;//Now check again on intermediate parents.
            std::vector<Node*> ip_vec;
            bool early_stop = false;
            RGB ip_color(0, 0, 0);
            for (int i = 0; i < curNode->fathers.size(); i++) {
                Node* far = curNode->fathers[i];
                std::vector<Node*> ip_vec_i = far->get_iparents(graph, order, rag, curNode, early_stop, ip_color);
                if (early_stop) {
                    // getchar();
                    break;
                }
                // std::cout << "ip_vec.size = " << ip_vec.size() << endl;
                ip_vec.insert(ip_vec.begin(), ip_vec_i.begin(), ip_vec_i.end());
            }
            RGB syn = ip_color;//synthesize_supremum(ip_vec);
            std::vector<double> pes = curNode->gen_powers_given_bg( gain_vec, sigma2_bg_vec, syn);
            for (int k = 0; k < curNode->N; k++) {
                double p_norm = pes[k];
                double area = curNode->area;
                if (area > 256 * 256) {
                    p_norm = (256 * 256) * p_norm / area;
                }
                double sni = gsl_cdf_chisq_Q(p_norm, area);
                if (sni < this->alpha) {
                    sn_all = true;
                    break;
                }
            }
        }

        if (sn_all) {
            curNode->sn1 = sn_all;
        }

        // 2. Now check combined band if sn1 is false in all saparate bands
        // if (!curNode->sn1){
        //     if (combined_area > 256*256){
        //         combined_p_norm = 256*256*combined_p_norm/combined_area;
        //     }
        //     curNode->sn1_value = gsl_cdf_chisq_Q(combined_p_norm, combined_area);
        //     curNode->sn1 = curNode->sn1_value < this->alpha;
        //     if (curNode->sn1){
        //         sn1_counter++;
        //         // curNode->print_node();
        //         // curNode->print_parents();
        //         // std::cout <<curNode->sn1_value << endl;
        //         // std::cout<<combined_area << endl;
        //         // for (int i = 0; i < curNode->N; i++){
        //         //     double p_norm = curNode->powers_norm[i];
        //         //     std::cout << p_norm << " " << i << endl;
        //         // }
        //         // std::cout << "different happens \n";
        //         // getchar();
        //     }
        // }
        // getchar();
    }

    // #2. Update closest_sig_anc (top-down) from root_id
    std::vector<int> up_processed = std::vector<int> (graph.size(), 0);
    for (int i = 0; i < graph.size(); i++) {
        up_processed[i] = graph[i]->fathers.size();
    }
    if (root->sn1) {
        update_closest_sn_anc_from_id(root, root, order, up_processed);
    } else {
        update_closest_sn_anc_from_id(root, NULL, order, up_processed);
    }
    //Visit tree bottom-up
    OrderedQueueDouble <Node *> pq;
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(order->getPriority(value), graph[i]);
    }
    pq.put(-1, root);
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->sn1) {
            update_parent_main_branch(curNode, order);
        }
    }
}

void update_closest_sn_anc_from_id_recursive(CGraph::Node* node_from, CGraph::Node* node_propagate, std::vector<bool> &processed) {
    //Non-processed node
    if (!processed[node_propagate->index]) {
        //Add sn_anc for node_propagate
        node_propagate->add_closest_sig_anc(node_from);
        processed[node_propagate->index] = true;
        //Propagate from non-SN node_propagate
        if (!node_propagate->sn1) {
            for (int i = 0; i < node_propagate->childs.size(); ++i) {
                update_closest_sn_anc_from_id_recursive(node_from, node_propagate->childs[i], processed);
            }
        }
    }


}

//New strategy: update to all its decendants
void CGraph::update_closest_sn_anc_from_id_new(Node* node_from) {
    if (node_from->sn1) {
        std::vector<bool> processed = std::vector<bool> (graph.size(), false);
        for (int i = 0; i < node_from->childs.size(); ++i) {
            Node* curChild = node_from->childs[i];
            update_closest_sn_anc_from_id_recursive(node_from, curChild, processed);
        }
    }
}

// #This function update closest_sn_anc attribute top-down from node_id
// #Be aware, sn_id could be NULL, since we gonna go top-down from root_id, and root_id is likely to be None
void CGraph::update_closest_sn_anc_from_id(Node* node_from, Node* node_sn, ColorOrdering *order, std::vector<int> &processed) {
    // std::cout<<"update_closest_sn_anc_from_id " << node_from->index << endl;
    // if (node_from->index != -1){
    //     if (processed[node_from->index] <= 0){
    //         return;
    //     }
    // }
    if (node_sn != NULL) {
        // std::cout<< "--------------update_closest_sn_anc_from_id "<< node_from->index << " "
        // << node_from->childs.size()<< " childs ,sn="<<node_sn->index << " processed size =" << processed.size()<<endl;
        // std::cout << "sn_queue of " << node_from->index << ": "<< node_from->closest_sig_anc.size() << ", processed " <<processed[node_from->index]<<endl;
    }
    if (node_from->index != -1) {
        processed[node_from->index]--;
    }
    for (int c = 0; c < node_from->childs.size(); c++) {
        Node *curChild = node_from->childs[c];
        // #1. Set closest_sig_anc[children of node_id] = sn_id
        // tree.closest_sig_anc[child_id] = sn_id
        if (node_sn != NULL) {
            bool add_sn = curChild->add_closest_sig_anc(node_sn);
            if (!add_sn) {
                continue;
            }
        }
        // curChild->closest_sig_anc.push_back(node_sn);

        // #2. Going down recursively
        // # if node_id is SN: update_closest_sn_anc_from_id(tree, child, node_id)
        // # else: update_closest_sn_anc_from_id(tree, child, sn_id)
        if (curChild->sn1) {
            update_closest_sn_anc_from_id(curChild, curChild, order, processed);
        } else {
            update_closest_sn_anc_from_id(curChild, node_sn, order, processed);
        }

    }
}

// # Update main_branch for a new SN node[id]
// # For the whole tree, must update bottom up
// # then main_branch(id) could look through all node below node id
void CGraph::update_parent_main_branch(Node* node_sn, ColorOrdering *order) {
    // # If node[id] does not have closest_sn_ancestor, nothing to update
    if (node_sn->closest_sig_anc.size() == 0) {
        //Check: it seems sig_anc vector atleast have NULL element -> size always > 0
        return;
    }
    // # Else: check compare main_branch
    for (int c = 0; c < node_sn->closest_sig_anc.size(); c++) {
        Node *cursn1 = node_sn->closest_sig_anc[c];
        if (cursn1 == NULL) {
            continue;
        }
        if (cursn1->main_branch == NULL) {
            cursn1->main_branch = node_sn;
        } else {
            cursn1->main_branch = max_area(node_sn, cursn1->main_branch);
        }
    }
}

// # Find object for Gos, return list of node id marked as object
std::vector<CGraph::Node *> CGraph::gos_find_object(ColorOrdering *order) {
    std::vector<CGraph::Node *> objs;
    int num_objects = 0;
    int num_objects_nested = 0;

    // # Node visit Top-down
    OrderedQueueDouble <Node *> pq;
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(-order->getPriority(value), graph[i]);
    }
    while (!pq.empty()) {
        Node *curNode = pq.get();
        // # 1. #No SN node, no obj; One pixel node, no obj
        if (!curNode->sn1 || curNode->area <= 1) {
            continue;
        }
        if (curNode->sn1) {
            float p_threshold = pow(10, 18);
            if (curNode->powers_norm[0] + curNode->powers_norm[1] + curNode->powers_norm[2]  > p_threshold) {
                num_objects++;
                curNode->object_id = num_objects;
                objs.push_back(curNode);
            }
        }

    }
    this->num_obj = num_objects;
    return objs;
}

//Query the only pixel of main_leave node
Point<TCoord> node_to_pixel(CGraph::Node* curNode, RAGraph *rag) {
    // std::cout<<"node_to_pixel \n";
    int width = rag->get_imsource_width();
    for (int r = 0; r < curNode->regions.size(); r++) {
        std::vector<Point<TCoord>> points = rag->nodes[curNode->regions[r]]->pixels;
        for (int p = 0; p < points.size(); p++) {
            // points[p].print();
            return points[p];
        }
    }
    return NULL;
}

//TODO: a and b may share more than largest child node ???
//(object) a vs. (mbranch_of_snanc) b
bool is_merge(CGraph::Node* a, CGraph::Node* b, std::vector<CGraph::Node *> graph, RAGraph *rag, std::vector<float> &merge_stat) {
    if (a->index == b->index) {
        return true;
    }
    // return false;

    if (a->get_main_leave() == b->get_main_leave()) {
        // std::cout << "Duplicate detected: same main_leave \n";
        return true;
    }
    return false;

    // std::cout << "Main leave of " << a->index << " is "<< a->get_main_leave() << endl;
    // std::cout << "Main leave of " << b->index << " is "<< b->get_main_leave() << endl;
    //Compare shared area vs. a and b areas
    // int threshold = a->area / 3;//Since shared nodes which have area < threshold could not change merge decision (only depend on >= a/2 node)
    // std::set<int> set_a = a->get_set_childs(rag);
    // std::set<int> set_b = b->get_set_childs(rag);
    std::set<int> set_a = a->get_set_pixels(rag);
    std::set<int> set_b = b->get_set_pixels(rag);

    set<int> shared;
    set_intersection(set_a.begin(), set_a.end(), set_b.begin(), set_b.end(),
                     std::inserter(shared, shared.begin()));
    //Specity largest area node that shared between a and b
    if (shared.size() == 0) {
        merge_stat.push_back(0.0);
        return false;
    }
    int max_area = shared.size();

    //TODO TESTING THIS IF STATEMENT
    //Using closest object acestor, we can identify potential extended object of object, then apply similarity metric to remove the duplications.
    if (max_area == a->area) { //shared = a->area: mean a is totally included in b.
        return false;
    }
    float stat = (1.0 * max_area) / (a->area + b->area - max_area); // (intersect/union)
    float stat_2 = (1.0 * max_area) / (a->area); //(intersect/smaller)
    // if (max_area >= (a->area)/2 || max_area >= (b->area)/2){
    // if (max_area >= (a->area + b->area - max_area)/3){
    std::cout << "STATS " << stat << endl;
    if ((1.0 * max_area) / (a->area + b->area - max_area) > 0.5) {
        merge_stat.push_back(stat_2);
        return true;
    } else {
        merge_stat.push_back(stat_2);
        return false;
    }
}

bool is_similar(CGraph::Node* a, CGraph::Node* b, std::vector<CGraph::Node *> graph, RAGraph *rag, std::vector<float> &merge_stat) {
    if (a->index == b->index) {
        return true;
    }

    Point<TCoord> a_m_leave = node_to_pixel(graph[a->get_main_leave()], rag);
    Point<TCoord> b_m_leave = node_to_pixel(graph[b->get_main_leave()], rag);
    double min_distance = 10.0;
    // std::cout << a_m_leave.get_distance(b_m_leave) << endl;
    if (a_m_leave.get_distance(b_m_leave) < min_distance) {
        // std::cout<<"merged close min_distance \n";
        return true;
    }
    return false;
}

bool is_merge_extended(CGraph::Node* a, CGraph::Node* b, std::vector<CGraph::Node *> graph, RAGraph *rag, std::vector<float> &merge_stat) {
    if (a->index == b->index) {
        return true;
    }
    //get_set_closest_obj_anc
    std::set<int> set_a = a->get_set_closest_obj_anc();
    std::set<int> set_b = b->get_set_closest_obj_anc();
    set<int> shared;
    set_intersection(set_a.begin(), set_a.end(), set_b.begin(), set_b.end(), std::inserter(shared, shared.begin()));
    if (shared.size() == 0) {
        return false;
    } else {
        //1. In this case: a and b have inclusion/parenthood relationship
        //This case anyway will be detected in is_merge(), but early inclusion detection here can skip some unnecessary complexity.
        if (shared.count(a->index) > 0 || shared.count(b->index) > 0) {
            return false;
        }
        //2. Other: a and b share at least one object ancestor
        return is_merge(a, b, graph, rag, merge_stat);
    }
}

bool node_compare (CGraph::Node* i, CGraph::Node* j) { return ((i->area) > (j->area)); }

//Move down a node, return pointer to moved_node
CGraph::Node* move_down_one_node(CGraph::Node* obj, ColorOrdering *order, std::vector<float> gain_vec,
                                 std::vector<float> sigma2_bg_vec, float lambda) {
    // std::cout << "move_down_one_node from "<< obj->index  << " area " << obj->area<< endl;
    //1. Cal new_level(base, lambda, noise)= base + lambda*sqrt(base/gain + bg_var)
    // order->islessequal( curNode->color , tmp->fathers[a]->color)
    CGraph::Node* curNode = obj;
    //Scale lambda by size of object. Test adaptive scale factor
    RGB syn = synthesize_supremum(curNode->fathers);
    RGB new_level(0, 0, 0);
    for (int k = 0; k < curNode->N; k++) {
        float gain = gain_vec[k];
        float sigma2_bg = sigma2_bg_vec[k];
        new_level[k] = syn[k] + lambda * sqrt(syn[k] / gain + sigma2_bg);
    }
    //2. While f(P) < new_level: do
    CGraph::Node* new_node = curNode;
    while (order->islessequal(new_node->color, new_level)) {
        //2.1 Move to main_branch if possible, Continue
        if (new_node->main_branch != NULL) {
            new_node = new_node->main_branch;
            continue;
        } else {
            //2.2 Move to main_power_branch if possible, Continue
            CGraph::Node* m_b_power = new_node->get_main_branch_power();
            if (m_b_power != NULL) {
                new_node = m_b_power;
            } else {
                //Stop the loop
                break;
            }
        }
    }
    std::cout << "move_down_one_node: from " << obj->index << " to " << new_node->index << endl;
    return new_node;
}

//Adaptive moving: Two condition for each two objects:
// (NOT NECESSARY)1. similarity score get_score() < threshold (default = 0.5)
//2. Not containing other's leave
void CGraph::move_down_adaptive(std::vector<CGraph::Node *> &objs
                                , ColorOrdering *order, std::vector<float> gain_vec, std::vector<float> sigma2_bg_vec, float step) {
    // std::cout << "move_down_adaptive \n";
    for (int i = 0; i < objs.size(); i++) { //i:0->size
        // std::cout << "check objs " << i << "/" << objs.size() << endl;
        Node* mbranch = objs[i];
        std::set<int> set_a = mbranch->get_set_pixels(rag);
        Point<TCoord> m_leave_a = node_to_pixel(graph[mbranch->get_main_leave()], rag);
        int m_leave_a_int = m_leave_a.get_index(rag->get_imsource_width());
        for (int k = i + 1; k < objs.size(); k++) { //k:i->size
            //Finding the best move
            Node* object = objs[k];
            std::set<int> set_b = object->get_set_pixels(rag);
            Point<TCoord> m_leave_b = node_to_pixel(graph[object->get_main_leave()], rag);
            int m_leave_b_int = m_leave_b.get_index(rag->get_imsource_width());
            while (set_a.find(m_leave_b_int) != set_a.end() && set_b.find(m_leave_a_int) != set_b.end()) {
                Node *moved = move_down_one_node(object, order, gain_vec, sigma2_bg_vec, step);
                if (moved->index != object->index) {
                    object = moved;
                    set_b = object->get_set_pixels(rag);
                    std::cout << get_score(set_a, set_b) << endl;
                } else {
                    step = 2 * step;
                }
            }
            // Transfer object information
            if (object->index != objs[k]->index) {
                object->object_id = objs[k]->object_id;
                objs[k]->object_id = -1;
                objs[k] = object;
            }
        }
    }
    // std::cout << "end move_down_adaptive \n";
}

std::vector<CGraph::Node *> CGraph::move_down(std::vector<CGraph::Node *> &objs
        , ColorOrdering *order, std::vector<float> gain_vec, std::vector<float> sigma2_bg_vec, float lambda) {
    //Tracking compactness
    // for (int i = 0; i < objs.size(); i++){
    //     CGraph::Node* curNode = objs[i];
    //     //plot perimeter
    //     std::cout << "For object_id " << curNode->object_id <<":\n";
    //     while(curNode != NULL){
    //         // std::cout<< curNode->compact << " " << curNode->area<<  " ; ";
    //         curNode->gen_compactness(rag, order, imSource);
    //         std::cout << curNode->compact << "; ";
    //         if (curNode->main_branch != NULL){
    //             curNode = curNode->main_branch;
    //         } else{
    //             curNode = curNode->get_main_branch_power();
    //         }
    //     }
    //     std::cout<<endl;
    // }
    // getchar();

    std::cout << "move_down " << endl;
    std::vector<CGraph::Node *> objs_moved;
    float img_area = rag->get_imsource_width() * rag->get_imsource_height();
    for (int i = 0; i < objs.size(); i++) {
        //1. Cal new_level(base, lambda, noise)= base + lambda*sqrt(base/gain + bg_var)
        // order->islessequal( curNode->color , tmp->fathers[a]->color)
        CGraph::Node* curNode = objs[i];

        // //If object is isolate (i.e. no object descendants), just skip move_up
        // std::cout << "moving " << curNode->object_id << endl;
        // std::vector<Node*> obj_des = curNode->query_obj_descendants(graph, rag);
        // std::cout << "Innclude " << obj_des.size() << endl;
        // for (auto node : obj_des) {
        //     std::cout << node->object_id << endl;
        // }
        // if (obj_des.size() == 0) {
        //     objs_moved.push_back(curNode);
        //     continue;
        // }

        //Scale lambda by size of object. Test adaptive scale factor
        float lambda_scale = lambda;//(curNode->area) / img_area;
        // std::cout << "lambda_scale = " << lambda_scale << endl;
        RGB syn = synthesize_supremum(curNode->fathers);
        RGB new_level(0, 0, 0);
        for (int k = 0; k < curNode->N; k++) {
            float gain = gain_vec[k];
            float sigma2_bg = sigma2_bg_vec[k];
            new_level[k] = syn[k] + lambda_scale * sqrt(syn[k] / gain + sigma2_bg);
        }
        //2. While f(P) < new_level: do
        CGraph::Node* new_node = curNode;
        // std::cout << "curNode " << curNode->index << "; new_node " << new_node->index << endl;
        while (order->islessequal(new_node->color, new_level)) {
            //2.0. Only nove large objects (> 10*10 pixels)
            if (new_node->area < 10 * 10) {
                break;
            }

            //2.1 Move to main_branch if possible, Continue
            if (new_node->main_branch != NULL) {
                new_node = new_node->main_branch;
                // std::cout << "mb!= NULL: curNode " << curNode->index << "; new_node " << new_node->index << endl;
                continue;
            } else {
                //2.2 Move to main_power_branch if possible, Continue
                CGraph::Node* m_b_power = new_node->get_main_branch_power();
                if (m_b_power != NULL) {
                    new_node = m_b_power;
                    std::cout << "m_b_power!= NULL: curNode " << curNode->index << "; new_node " << new_node->index << endl;
                } else {
                    //Stop the loop
                    std::cout << "m_b_power== NULL: curNode " << curNode->index << "; new_node " << new_node->index << endl;
                    break;
                }

            }
        }
        //3. Update next index, if possible
        // getchar();
        // new_node->print_all();
        // getchar();
        // curNode->print_all();
        // getchar();
        if (new_node->index != curNode->index) {
            //set new_node as an object
            new_node->object_id = curNode->object_id;
            objs_moved.push_back(new_node);
            //release curNode as non-object
            curNode->object_id = -1;
            // std::cout << "new_node != curNode \n";
        } else {
            // std::cout << "new_node == curNode \n";
            objs_moved.push_back(curNode);
        }
        // std::cout << "Testing move_down() " << endl;
        // getchar();
    }

    return objs_moved;
}

// # Find object, return list of node id marked as object
std::vector<CGraph::Node *> CGraph::find_object(ColorOrdering *order) {
    std::vector<CGraph::Node *> objs;//all objs
    std::vector<CGraph::Node *> objs_explicit;
    int num_objects = 0;
    int num_objects_nested = 0;
    int num_isect = 0;

    // # Node visit Top-down
    OrderedQueueDouble <Node *> pq;
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(-order->getPriority(value), graph[i]);
    }
    while (!pq.empty()) {
        Node *curNode = pq.get();
        // # 1. #No SN node, no obj; One pixel node, no obj
        if (!curNode->sn1 || curNode->area <= 1) {
            // std::cout <<"find_object db 1"<<endl;
            continue;
        }
        // # 2. SN node, but no closest_sn_ancestor, mark as obj
        //remark: closet_sig_anc always add NULL node at the end of vector, then size(closet_sn_anc) always >=1
        if (curNode->closest_sig_anc.size() == 0) {
            // std::cout <<"find_object db 2"<<endl;
            num_objects++;
            curNode->object_id = num_objects;
            // objs.push_back(curNode);
            objs_explicit.push_back(curNode);
            if (curNode->intersect) {
                num_isect ++;
            }
            // getchar();
            continue;
        }

        // # 3. Check main_branch(closest_sn_ancestor != id)
        //This loop should be replace by a function that check extended_object more efficient.
        int flag_exd_obj = 1; //If found any mbranch(sn_node) == curNode->index, then mark flag = 0
        for (int c = 0; c < curNode->closest_sig_anc.size(); c++) {
            Node *cursn1 = curNode->closest_sig_anc[c];
            if (cursn1->main_branch->index == curNode->index) {
                flag_exd_obj = 0;
                break;
            }
        }
        if (flag_exd_obj) {
            // std::cout<< "Found extended object" << endl;
            // getchar();
        }
        if (flag_exd_obj) {
            // std::cout << "Found extended object at index "<<curNode->index <<endl;
            num_objects_nested++;
            num_objects++;
            curNode->object_id = num_objects;
            objs.push_back(curNode);
            if (curNode->intersect) {
                num_isect ++;
            }
        }
    }

    //Find duplicate object
    std::vector<CGraph::Node *> objs_refined;//Store final refined object (refined explicit obj + refined extended obj)
    std::vector<float> merge_stat;

    //1. Check all explicit objects, i.e. no sn_anc
    // std::cout << "check explicit objs "<< objs_explicit.size() <<endl;
    if (objs_explicit.size() == 1 ) { //Only one explicit object
        objs_refined.push_back(objs_explicit[0]);
    } else if (objs_explicit.size() > 1) { // Multiple explicit objs
        //Sort explicit objects
        std::sort(objs_explicit.begin(), objs_explicit.end(), node_compare);
        std::vector<bool> merger = std::vector<bool>(objs_explicit.size(), false);
        for (int i = 0; i < objs_explicit.size(); i++) { //i:0->size
            // std::cout << "check explicit objs "<<i<<"/"<< objs_explicit.size() <<endl;
            for (int k = i + 1; k < objs_explicit.size(); k++) { //k:i->size
                //obj(at k) vs. m_branch(at i)
                Node* object = objs_explicit[k];
                Node* mbranch = objs_explicit[i];
                if (!merger[k]) { //if obj(at k) is NOT MERGE to any others, then consider merge to mbranch if satisify
                    if (is_merge(object, mbranch, graph, rag, merge_stat)) {
                        merger[k] = true;
                        object->index_explicit_merge_to = mbranch->index_explicit_merge_to;
                        object->object_id = -1;
                    }
                }
            }
        }
        for (int i = 0; i < merger.size(); i++) {
            if (!merger[i]) {
                objs_refined.push_back(objs_explicit[i]);
            }
        }
    }
    //2. Check extended object vs. its main_branch, i.e. check sn_anc
    std::vector<CGraph::Node *> objs_etd_refined;
    //2.1. Step 1: compare with main_branch of sn_anc
    // #pragma omp parallel for private(i)
    for (int i = 0; i < objs.size(); i++) {
        // std::cout << "check extended objs "<<i<<"/"<< objs.size() <<endl;
        Node* obj = objs[i];
        for (int j = 0; j < obj->closest_sig_anc.size(); j++) { //ofcourse explicit will be skip since its sn_anc is empty
            Node* far = obj->closest_sig_anc[j];
            Node* far_mbranch = far->main_branch;
            //Now compare obj vs. far_m_branch. If you merge with any far_mbranch, you're not an obj anymore
            if (is_merge(obj, far_mbranch, graph, rag, merge_stat)) {
                //Mark obj as non-object
                obj->object_id = -1;
                break;
            }
        }
        //Store refined obj
        if (obj->object_id >= 0) {
            objs_etd_refined.push_back(obj);
        }
    }
    //2.2. Step 2: Check among extended objs
    if (objs_etd_refined.size() == 1 ) { //Only one  object
        objs_refined.push_back(objs_etd_refined[0]);
    } else if (objs_etd_refined.size() > 1) { // Multiple objs
        //Sort  objects
        std::sort(objs_etd_refined.begin(), objs_etd_refined.end(), node_compare);
        std::vector<bool> merger = std::vector<bool>(objs_etd_refined.size(), false);
        for (int i = 0; i < objs_etd_refined.size(); i++) { //i:0->size
            std::cout << "check extended objs among " << i << "/" << objs_etd_refined.size() << endl;
            for (int k = i + 1; k < objs_etd_refined.size(); k++) { //k:i->size
                //obj(at k) vs. m_branch(at i)
                Node* object = objs_etd_refined[k];
                Node* mbranch = objs_etd_refined[i];
                if (!merger[k]) { //if obj(at k) is NOT MERGE to any others, then consider merge to mbranch if satisify
                    // if(is_merge_extended(object, mbranch, graph, rag, merge_stat)){
                    if (is_merge(object, mbranch, graph, rag, merge_stat)) {
                        merger[k] = true;
                        object->object_id = -1;
                    }
                }
            }
        }
        for (int i = 0; i < merger.size(); i++) {
            if (!merger[i]) {
                objs_refined.push_back(objs_etd_refined[i]);
            }
        }
    }

    //3. Re-index object_id for refined objects
    //Also sort objs_refined for later visualiation (larger ones draw first)
    std::sort(objs_refined.begin(), objs_refined.end(), node_compare);
    for (int i = 0; i < objs_refined.size(); i++) {
        objs_refined[i]->object_id = i + 1; //This step is necessary for later visualization function (constructImageTos())
    }

    //4. Check between all objs.
    std::vector<CGraph::Node *> objs_all;
    std::vector<bool> merger = std::vector<bool>(objs_refined.size(), false);
    for (int i = 0; i < objs_refined.size(); i++) { //i:0->size
        std::cout << "check all objs among " << i << "/" << objs_refined.size() << endl;
        for (int k = i + 1; k < objs_refined.size(); k++) { //k:i->size
            Node* object = objs_refined[k];
            Node* mbranch = objs_refined[i];
            if (!merger[k]) { //if obj(at k) is NOT MERGE to any others, then consider merge to mbranch if satisify
                if (is_merge(object, mbranch, graph, rag, merge_stat)) {
                    merger[k] = true;
                    object->object_id = -1;
                }
            }
        }
    }
    for (int i = 0; i < merger.size(); i++) {
        if (!merger[i]) {
            objs_all.push_back(objs_refined[i]);
        }
    }

    return objs_all;

    // return objs_refined;
}

bool is_on_border(int index, int w, int h, int border) {

    //Left or right borders
    if (index % w < border || index % w > w - border) {
        return true;
    }

    //top or bottom borders
    if (index / w < border || index / w > h - border) {
        return true;
    }

    return false;
}

// # Find object, return list of node id marked as object
std::vector<CGraph::Node *> CGraph::find_object_merger(ColorOrdering *order) {
    std::vector<CGraph::Node *> objs;//all objs
    std::vector<float> merge_stat;
    int num_objects = 0;

    // # Node visit Top-down
    OrderedQueueDouble <Node *> pq;
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(-order->getPriority(value), graph[i]);
    }
    while (!pq.empty()) {
        Node *curNode = pq.get();
        // # 1. #No SN node, no obj; One pixel node, no obj
        if (!curNode->sn1 || curNode->area <= 1) {
            continue;
        }

        // # 2. SN node, but no closest_sn_ancestor, mark as obj
        if (curNode->closest_sig_anc.size() == 0) {
            num_objects++;
            curNode->object_id = num_objects;
            curNode->obj_extd_level = 0;//Explicite obj
            objs.push_back(curNode);
            // std::cout << "found explicit " << curNode->index << endl;
            continue;
        }

        // # 3. Check main_branch(closest_sn_ancestor != id)
        int flag_exd_obj = 1; //If found any mbranch(sn_node) == curNode->index, then mark flag = 0
        for (int c = 0; c < curNode->closest_sig_anc.size(); c++) {
            Node *cursn1 = curNode->closest_sig_anc[c];
            //Check: are they growth to the same leave.
            if (is_merge(curNode, cursn1, graph, rag, merge_stat)) {
                flag_exd_obj = 0;
                break;
            }
        }
        if (flag_exd_obj) {
            num_objects++;
            curNode->object_id = num_objects;
            curNode->obj_extd_level = 1; //Extended obj
            // std::cout << "found extended " << curNode->index << endl;
            objs.push_back(curNode);
        }
    }

    //3. Re-index object_id for refined objects
    //Also sort objs_refined for later visualiation (larger ones draw first)
    std::sort(objs.begin(), objs.end(), node_compare);
    for (int i = 0; i < objs.size(); i++) {
        objs[i]->object_id = i + 1; //This step is necessary for later visualization function (constructImageTos())
    }

    //4. Check between all objs.
    std::vector<CGraph::Node *> objs_all;
    std::vector<bool> merger = std::vector<bool>(objs.size(), false);
    for (int i = 0; i < objs.size(); i++) { //i:0->size
        // std::cout << "check objs " << i << "/" << objs.size() << endl;
        for (int k = i + 1; k < objs.size(); k++) { //k:i->size
            Node* object = objs[k];
            Node* mbranch = objs[i];
            if (!merger[k]) { //if obj(at k) is NOT MERGE to any others, then consider merge to mbranch if satisify
                // if(is_merge(object, mbranch, graph, rag, merge_stat)){
                if (is_similar(object, mbranch, graph, rag, merge_stat)) {
                    merger[k] = true;
                    // std::cout << "merged " << object->index << " to " << mbranch->index << endl;
                    object->object_id = -1;
                }
            }
        }
    }
    for (int i = 0; i < merger.size(); i++) {
        if (!merger[i]) {
            objs_all.push_back(objs[i]);
        }
    }
    std::cout << "Remain " << objs_all.size() << endl;

    // //Testing
    // merger = std::vector<bool>(objs_all.size(), false);
    // for (int i = 0; i < objs_all.size(); i++){//i:0->size
    //     std::cout << "check objs "<<i<<"/"<< objs_all.size() <<endl;
    //     for (int k = i+1; k < objs_all.size(); k++){//k:i->size
    //         Node* object = objs_all[k];
    //         Node* mbranch = objs_all[i];
    //         if (!merger[k]){//if obj(at k) is NOT MERGE to any others, then consider merge to mbranch if satisify
    //             if(is_similar(object, mbranch, graph, rag, merge_stat)){
    //                 // merger[k] = true;
    //                 std::cout << "merged " << object->index << " to " << mbranch->index << endl;
    //                 // object->object_id = -1;
    //             }
    //         }
    //     }
    // }
    // getchar();

    //ANOTHER TEST
    std::vector<CGraph::Node *> objs_no_borders;
    int w = rag->get_imsource_width();
    int h = rag->get_imsource_height();
    int border = 0;
    for (int i = 0; i < objs_all.size(); ++i) {
        //Check borders
        Node* curNode = objs_all[i];
        Point<TCoord> m_leave = node_to_pixel(graph[curNode->get_main_leave()], rag);
        // m_leave.print();
        if (is_on_border(m_leave.get_index(w), w, h, border)) {
            //Skip this object
            curNode->object_id = -1;
            // curNode->print_all();
            std::cout << "Skip bordering object \n";
            // getchar();
        } else {
            objs_no_borders.push_back(curNode);
        }
    }

    return objs_no_borders;

    // return objs_all;

    // return objs;
}

// # Find object, return list of node id marked as object
std::vector<CGraph::Node *> CGraph::find_objectNEW(ColorOrdering *order,
        std::vector<float> gain_vec, std::vector<float> sigma2_bg_vec, float lambda) {
    std::vector<CGraph::Node *> objs;//all objs
    std::vector<CGraph::Node *> objs_explicit;
    int num_objects = 0;
    int num_objects_nested = 0;
    int num_isect = 0;

    // # Node visit Top-down
    OrderedQueueDouble <Node *> pq;
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(-order->getPriority(value), graph[i]);
    }
    while (!pq.empty()) {
        Node *curNode = pq.get();
        // # 1. #No SN node, no obj; One pixel node, no obj
        if (!curNode->sn1 || curNode->area <= 1) {
            // std::cout <<"find_object db 1"<<endl;
            continue;
        }
        // # 2. SN node, but no closest_sn_ancestor, mark as obj
        //remark: closet_sig_anc always add NULL node at the end of vector, then size(closet_sn_anc) always >=1
        if (curNode->closest_sig_anc.size() == 0) {
            // std::cout <<"find_object db 2"<<endl;
            num_objects++;
            curNode->object_id = num_objects;
            // objs.push_back(curNode);
            objs_explicit.push_back(curNode);
            if (curNode->intersect) {
                num_isect ++;
            }
            continue;
        }

        // # 3. Check main_branch(closest_sn_ancestor != id)
        //This loop should be replace by a function that check extended_object more efficient.
        int flag_exd_obj = 1; //If found any mbranch(sn_node) == curNode->index, then mark flag = 0
        for (int c = 0; c < curNode->closest_sig_anc.size(); c++) {
            Node *cursn1 = curNode->closest_sig_anc[c];
            if (cursn1->main_branch->index == curNode->index) {
                flag_exd_obj = 0;
                break;
            }
        }
        if (flag_exd_obj) {
            // std::cout<< "Found extended object" << endl;
            // getchar();
        }
        if (flag_exd_obj) {
            num_objects_nested++;
            num_objects++;
            curNode->object_id = num_objects;
            objs.push_back(curNode);
            if (curNode->intersect) {
                num_isect ++;
            }
        }
    }

    //Find duplicate object
    std::vector<CGraph::Node *> objs_refined;//Store final refined object (refined explicit obj + refined extended obj)
    std::vector<float> merge_stat;

    //1. Check all explicit objects, i.e. no sn_anc
    // std::cout << "check explicit objs "<< objs_explicit.size() <<endl;
    if (objs_explicit.size() == 1 ) { //Only one explicit object
        objs_refined.push_back(objs_explicit[0]);
    } else if (objs_explicit.size() > 1) { // Multiple explicit objs
        //Sort explicit objects
        std::sort(objs_explicit.begin(), objs_explicit.end(), node_compare);
        std::vector<bool> merger = std::vector<bool>(objs_explicit.size(), false);
        for (int i = 0; i < objs_explicit.size(); i++) { //i:0->size
            // std::cout << "check explicit objs "<<i<<"/"<< objs_explicit.size() <<endl;
            for (int k = i + 1; k < objs_explicit.size(); k++) { //k:i->size
                //obj(at k) vs. m_branch(at i)
                Node* object = objs_explicit[k];
                Node* object_moved = move_down_one_node(object, order, gain_vec, sigma2_bg_vec, lambda);
                Node* mbranch = objs_explicit[i];
                Node* mbranch_moved = move_down_one_node(mbranch, order, gain_vec, sigma2_bg_vec, lambda);

                if (!merger[k]) { //if obj(at k) is NOT MERGE to any others, then consider merge to mbranch if satisify
                    if (is_merge(object, mbranch, graph, rag, merge_stat)) {
                        // if(is_merge(object_moved, mbranch_moved, graph, rag, merge_stat)){
                        // std::cout<<"------------- merged " << object->index <<" to "<< mbranch->index <<endl;
                        merger[k] = true;
                        object->index_explicit_merge_to = mbranch->index_explicit_merge_to;
                        object->object_id = -1;
                    }
                }
            }
        }
        for (int i = 0; i < merger.size(); i++) {
            if (!merger[i]) {
                objs_refined.push_back(objs_explicit[i]);
            }
        }
    }
    // std::cout <<"No. of detected explicited objs after step 1 = "<<objs_refined.size()<<endl;
    // getchar();

    //2. Check extended object vs. its main_branch, i.e. check sn_anc
    std::vector<CGraph::Node *> objs_etd_refined;
    //2.1. Step 1: compare with main_branch of sn_anc
    // #pragma omp parallel for private(i)
    for (int i = 0; i < objs.size(); i++) {
        // std::cout << "check extended objs "<<i<<"/"<< objs.size() <<endl;
        Node* obj = objs[i];
        Node* obj_moved = move_down_one_node(obj, order, gain_vec, sigma2_bg_vec, lambda);
        for (int j = 0; j < obj->closest_sig_anc.size(); j++) { //ofcourse explicit will be skip since its sn_anc is empty
            Node* far = obj->closest_sig_anc[j];
            Node* far_mbranch = far->main_branch;
            Node* far_mbranch_moved = move_down_one_node(far_mbranch, order, gain_vec, sigma2_bg_vec, lambda);
            //Now compare obj vs. far_m_branch. If you merge with any far_mbranch, you're not an obj anymore
            if (is_merge(obj, far_mbranch, graph, rag, merge_stat)) {
                // if(is_merge(obj_moved, far_mbranch_moved, graph, rag, merge_stat)){
                //Mark obj as non-object
                obj->object_id = -1;
                break;
            }
        }
        //Store refined obj
        if (obj->object_id >= 0) {
            objs_etd_refined.push_back(obj);
        }
    }
    // std::cout <<"No. of detected objs after step 2 "<<objs_refined.size()<<endl;
    // getchar();

    //2.2. Step 2: Check among extended objs
    // std::cout <<"Check among extended objects \n";
    if (objs_etd_refined.size() == 1 ) { //Only one  object
        objs_refined.push_back(objs_etd_refined[0]);
    } else if (objs_etd_refined.size() > 1) { // Multiple objs
        //Sort  objects
        std::sort(objs_etd_refined.begin(), objs_etd_refined.end(), node_compare);
        std::vector<bool> merger = std::vector<bool>(objs_etd_refined.size(), false);
        for (int i = 0; i < objs_etd_refined.size(); i++) { //i:0->size
            // std::cout << "check extended objs among "<<i<<"/"<< objs_etd_refined.size() <<endl;
            for (int k = i + 1; k < objs_etd_refined.size(); k++) { //k:i->size
                //obj(at k) vs. m_branch(at i)
                Node* object = objs_etd_refined[k];
                Node* object_moved = move_down_one_node(object, order, gain_vec, sigma2_bg_vec, lambda);
                Node* mbranch = objs_etd_refined[i];
                Node* mbranch_moved = move_down_one_node(mbranch, order, gain_vec, sigma2_bg_vec, lambda);
                if (!merger[k]) { //if obj(at k) is NOT MERGE to any others, then consider merge to mbranch if satisify
                    // if(is_merge_extended(object, mbranch, graph, rag, merge_stat)){
                    if (is_merge(object, mbranch, graph, rag, merge_stat)) {
                        // if(is_merge(object_moved, mbranch_moved, graph, rag, merge_stat)){
                        merger[k] = true;
                        object->object_id = -1;
                    }
                }
            }
        }
        for (int i = 0; i < merger.size(); i++) {
            if (!merger[i]) {
                objs_refined.push_back(objs_etd_refined[i]);
            }
        }
    }
    // std::cout <<"No. of detected objs after check among extended objs step 3 "<<objs_refined.size()<<endl;
    // getchar();

    //3. Re-index object_id for refined objects
    //Also sort objs_refined for later visualiation (larger ones draw first)
    std::sort(objs_refined.begin(), objs_refined.end(), node_compare);
    for (int i = 0; i < objs_refined.size(); i++) {
        objs_refined[i]->object_id = i + 1; //This step is necessary for later visualization function (constructImageTos())
    }

    //4. Check between all objs.
    std::vector<CGraph::Node *> objs_all;
    std::vector<bool> merger = std::vector<bool>(objs_refined.size(), false);
    for (int i = 0; i < objs_refined.size(); i++) { //i:0->size
        std::cout << "check all objs among " << i << "/" << objs_refined.size() << endl;
        for (int k = i + 1; k < objs_refined.size(); k++) { //k:i->size
            Node* object = objs_refined[k];
            Node* mbranch = objs_refined[i];
            if (!merger[k]) { //if obj(at k) is NOT MERGE to any others, then consider merge to mbranch if satisify
                if (is_merge(object, mbranch, graph, rag, merge_stat)) {
                    std::cout << "merge " << k << " to " << i ;
                    merger[k] = true;
                    object->object_id = -1;
                }
            }
        }
    }
    for (int i = 0; i < merger.size(); i++) {
        if (!merger[i]) {
            objs_all.push_back(objs_refined[i]);
        }
    }

    return objs_all;

    // return objs_refined;
}

// compare node areas, return larger node index
CGraph::Node* CGraph::max_area(CGraph::Node* new_node, CGraph::Node* old_node) {
    if (new_node->area > old_node->area) {
        return new_node;
    } else {
        return old_node;
    }
}


//High light selected objects
Image<RGB> CGraph::constructImageToS(ColorOrdering *order) {
    Image<RGB> imRes = imSource;
    imRes.fill(0);
    OrderedQueueDouble <Node *> pq;
    RGB rgbmin(0, 0, 0);
    for (int i = 0; i < graph.size(); i++) {
        graph[i]->dispColor = rgbmin;
        RGB value = graph[i]->color;
        pq.put(-order->getPriority(value), graph[i]); //Top-down: Root->leave
    }
    pq.put(-1, root);

    int counter = 0;
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->object_id > 0) {
            counter++;
            curNode->dispColor = curNode->get_obj_corlor(this->num_obj, this->rgbs);
        }

        paintNode(imRes, curNode, curNode->dispColor);

        for (int c = 0; c < curNode->childs.size(); c++) {
            Node *curChild = curNode->childs[c];
            if (curChild->object_id <= 0) {
                curChild->dispColor = curNode->dispColor;
            }
        }
    }
    return imRes;
}

void CGraph::visual_one_object_recursive(Image<RGB> &imRes, CGraph::Node* node, RGB &color, std::vector<int> &up_processed) {
    paintNode(imRes, node, color);
    up_processed[node->index]  = 1;
    for (int i = 0; i < node->childs.size(); i++) { //Paint childs
        Node* curChild = node->childs[i];
        if (!up_processed[curChild->index]) {
            visual_one_object_recursive(imRes, curChild, color, up_processed);
        }
    }
}


Image<RGB> CGraph::visual_one_object(CGraph::Node* object) {
    Image<RGB> imRes = imSource;
    imRes.fill(0);
    std::vector<int> up_processed = std::vector<int> (graph.size(), 0);
    if (object->object_id <= 0) {
        RGB red(255, 0, 0);
        visual_one_object_recursive(imRes, object, red, up_processed);
    } else {
        visual_one_object_recursive(imRes, object, rgbs[object->object_id - 1], up_processed);
    }
    return imRes;
}

Image<RGB> CGraph::visual_all_object(std::vector<CGraph::Node*> objs) {
    Image<RGB> imRes = imSource;
    imRes.fill(0);
    //Paint one-by-one
    for (int i = 0; i < objs.size(); i++) {
        Node* object = objs[i];
        std::vector<int> up_processed = std::vector<int> (graph.size(), 0);
        visual_one_object_recursive(imRes, object, rgbs[object->object_id - 1], up_processed);
    }
    return imRes;

}

Image<RGB> CGraph::visual_all_object_by_objid(std::vector<CGraph::Node*> objs) {
    Image<RGB> imRes = imSource;
    imRes.fill(0);
    //Paint one-by-one
    for (int i = 0; i < objs.size(); i++) {
        Node* object = objs[i];
        std::vector<int> up_processed = std::vector<int> (graph.size(), 0);
        RGB value(object->object_id, object->object_id, object->object_id);
        visual_one_object_recursive(imRes, object, value, up_processed);
        // std::cout << object->object_id << " object->object_id" << endl;
        // getchar();
    }
    return imRes;
}
int CGraph::export_fits(Image<RGB> &all_obj, const char* fits_name) {
    int w = imSource.getSizeX();
    int h = imSource.getSizeY();
    fitsfile *fptr;       /* pointer to the FITS file; defined in fitsio.h */
    int status, ii, jj;
    long  fpixel = 1, naxis = 2, nelements, exposure;
    long naxes[2] = { w, h };
    short array[h][w];
    status = 0;         /* initialize status before calling fitsio routines */
    fits_create_file(&fptr, fits_name, &status);   /* create new file */
    /* Create the primary array image (16-bit short integer pixels */
    fits_create_img(fptr, SHORT_IMG, naxis, naxes, &status);
    for (jj = 0; jj < naxes[1]; jj++) {
        for (ii = 0; ii < naxes[0]; ii++) {
            array[jj][ii] = all_obj(ii, jj)[0];
        }
    }
    nelements = naxes[0] * naxes[1];          /* number of pixels to write */
    /* Write the array of integers to the image */
    fits_write_img(fptr, TSHORT, fpixel, nelements, array[0], &status);
    fits_close_file(fptr, &status);            /* close the file */
    fits_report_error(stderr, status);  /* print out any error messages */
    return ( status );
}

void CGraph::trace_sn_values(int index) {
    Node* curNode;
    if (index == -1) { //root
        curNode = root;
    } else {
        curNode = graph[index];
    }
    float value = curNode->sn1_value;
    Node* deepest;
    int depth = -1;
    for (int i = 0; i < curNode->childs.size(); i++) {
        if (curNode->childs[i]->depth > depth) {
            deepest = curNode->childs[i];
            depth = curNode->childs[i]->depth;
        }
    }
    if (depth != -1) {
        trace_sn_values(deepest->index);
    }
}
//highlight each object into an image
Image<RGB> CGraph::constructImageGoS(ColorOrdering *order, int object_id) {
    std::cout << "Start constructImageGoS for id = " << object_id << endl;
    Image<RGB> imRes = imSource;
    imRes.fill(0);
    OrderedQueueDouble <Node *> pq;
    RGB rgbmin(0, 0, 0);
    for (int i = 0; i < graph.size(); i++) {
        graph[i]->dispColor = rgbmin;
        RGB value = graph[i]->color;
        pq.put(-order->getPriority(value), graph[i]);
    }
    pq.put(-1, root);

    RGB rgb_compare(-1, -1, -1);
    int counter = 0;
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->object_id == object_id) {
            counter++;
            curNode->dispColor = curNode->get_obj_corlor(this->num_obj, this->rgbs);
            rgb_compare = curNode->dispColor;
        }
        if (curNode->dispColor.is_equal(rgb_compare)) {
        }
        paintNode(imRes, curNode, curNode->dispColor);
        for (int c = 0; c < curNode->childs.size(); c++) {
            Node *curChild = curNode->childs[c];
            if (!curChild->dispColor.is_equal(rgbmin)) { //Already visited and setup color values
                // std::cout << "Already set up "<< curChild->index << endl;
                // getchar();
                continue;
            }
            if (curChild->dispColor.is_equal(rgbmin)) {
                if (curNode->dispColor.is_equal(rgb_compare)) {
                }
                counter++;
                curChild->dispColor = curNode->dispColor;
            }
        }
    }
    return imRes;
}

//Test showing gos objects
void CGraph::visual_gos(ColorOrdering *order) {
    OrderedQueueDouble <Node *> pq;
    for (int i = 0; i < graph.size(); i++) {
        RGB value = graph[i]->color;
        pq.put(-order->getPriority(value), graph[i]);
    }
    while (!pq.empty()) {
        Node *curNode = pq.get();
        if (curNode->object_id > 0) {
            Image<RGB> gos = constructImageGoS(order, curNode->object_id);
            char str[100];
            sprintf(str, "one_gos_%d.ppm", curNode->object_id);
            gos.save(str);
        }

    }
}

void CGraph::writeObjectToCSV(const char *filename, std::vector<CGraph::Node*> objs){
    std::ofstream myfile;
    myfile.open(filename);// ("Example.csv");
    myfile << "id, x, y, ra, dec, area\n";
    for (int i = 0; i < objs.size(); ++i){
        Node* curNode = objs[i];
        // curNode->print_node();
        // Point<TCoord> m_leave_a = node_to_pixel(graph[mbranch->get_main_leave()], rag);
        Point<TCoord> m_leave = node_to_pixel(graph[curNode->get_main_leave()], rag);
        myfile << i <<", "<< m_leave.get_x()<<", "<<m_leave.get_y()<<", "<< "ra"<<", "<<"dec"<<", "<<curNode->area << "\n";

    }
    myfile.close();
    std::cout << "writeObjectToCSV "<< filename <<endl;
}
