//utils.h
// #include "cgraph.h"
// #include "Types.h"
// #include <gsl/gsl_cdf.h>

template <class I1, class I2>
bool have_common_element(I1 first1, I1 last1, I2 first2, I2 last2) {
	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2)
			++first1;
		else if (*first2 < *first1)
			++first2;
		else
			return true;
	}
	return false;
}

// std::vector<double> sn_check(CGraph::Node* curNode, RGB &syn, double alpha){
// 	std::vector<double> sn_v = std::vector<double> (curNode->N, -1);
//     bool sn_all = false;
//     double combined_p_norm = 0;
//     for (int i = 0; i < curNode->N; i++){
//         if (curNode->color[i] == syn[i]){
//             continue;
//         }
//         double p_norm = curNode->powers_norm[i];
//         combined_p_norm += p_norm;
//         double area = curNode->area;
//         if (area > 256 * 256){
//             p_norm = (256*256)*p_norm/area;
//         }
//         curNode->snes_v[i] = gsl_cdf_chisq_Q(p_norm, area);
//         sn_v[i] = curNode->snes_v[i];
//         curNode->snes[i] = curNode->snes_v[i] < alpha;
//         std::cout << curNode->snes_v[i] <<endl;

//         // // OPTION 1: sn all iff all sn_i is sn
//         // if (!curNode->snes[i]){
//         //     sn_all = false;
//         // }

//         //OPTION 2: sn all iff any sn_i is sn
//         if (curNode->snes[i]){
//             sn_all = true;
//             // break;
//         }
//     }
//     curNode->sn1 = sn_all;
//     return sn_v;
// }
