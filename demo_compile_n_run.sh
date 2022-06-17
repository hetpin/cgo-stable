#!/bin/bash  
echo "Shell script to compile then run demo cgraph"
sh ./demo_compile.sh
# ./demo /blade/home/nguyentx/Data/AkuMockMulti/color_mock/cluster1_g.fits \
#  /blade/home/nguyentx/Data/AkuMockMulti/color_mock/cluster1_r.fits \
#  /blade/home/nguyentx/Data/AkuMockMulti/color_mock/cluster1_i.fits

# ./demo /Volumes/DATA/BladeESIEE/cluster1_g.fits \
# /Volumes/DATA/BladeESIEE/cluster1_r.fits \
# /Volumes/DATA/BladeESIEE/cluster1_i.fits

# ./demo data/band_1.fits data/band_2.fits data/band_3.fits

#Last case: running on im1.fits
# ./demo data/reprojected/im1_g.fits data/reprojected/reprjted_im1_r.fits data/reprojected/reprjted_im1_u.fits
# ./demo data/reprojected/im1_g.fits data/reprojected/im1_g.fits data/reprojected/im1_g.fits
# ./demo data/reprojected/reprjted_im1_r.fits data/reprojected/reprjted_im1_r.fits data/reprojected/reprjted_im1_r.fits
# ./demo data/reprojected/reprjted_im1_u.fits data/reprojected/reprjted_im1_u.fits data/reprojected/reprjted_im1_u.fits

# ./demo data/sim2/snr_-0.93/band_1_fake_test_6_snr_-0.93_i_0_ori.fits \
# data/sim2/snr_-0.93/band_2_fake_test_6_snr_-0.93_i_0_ori.fits \
# data/sim2/snr_-0.93/band_3_fake_test_6_snr_-0.93_i_0_ori.fits \
#  0.000001 data/sim2/snr_-0.93/segmaps_fake_test_6_snr_-0.93.fits

#running on stripe 82
# ./demo data/f0363_g.rec.fits data/reprojected/reprjted_f0363_i.rec.fits data/reprojected/reprjted_f0363_i.rec.fits

# ./demo data/J23_g.fits data/reprojected/reprjted_J23_r.fits data/reprojected/reprjted_J23_i.fits

# ./demo data/J23_g.fits data/J23_g.fits data/J23_g.fits

# ./demo data/band_1.fits data/band_2.fits data/band_3.fits

#Check supremum and no_supremum
# ./demo data/sup_J23_g.fits data/sup_J23_g.fits data/sup_J23_g.fits
# ./demo data/J23_g.fits data/J23_g.fits data/J23_g.fits

#Alex
# ./demo data/alex/i_KiDS_6000161325908.0_53.17222542289311_-27.76524068222039.fits \
# data/alex/repr_g_KiDS_6000161325908.0_53.17222542289311_-27.76524068222039.fits \
# data/alex/repr_r_KiDS_6000161325908.0_53.17222542289311_-27.76524068222039.fits

# ./demo data/alex/i_KiDS_6000155321189.0_53.15154189283351_-27.85497499550366.fits \
# data/alex/repr_g_KiDS_6000155321189.0_53.15154189283351_-27.85497499550366.fits \
# data/alex/repr_r_KiDS_6000155321189.0_53.15154189283351_-27.85497499550366.fits

# data/alex/i_KiDS_6000161325908.0_53.17222542289311_-27.76524068222039.fits \
# data/alex/i_KiDS_6000161325908.0_53.17222542289311_-27.76524068222039.fits

# ./demo data/alex/cluster1_g_10_10.fits \
# data/alex/cluster1_g_10_10.fits \
# data/alex/cluster1_g_10_10.fits

# ./demo data/alex/cluster1_r_10_10.fits \
# data/alex/cluster1_r_10_10.fits \
# data/alex/cluster1_r_10_10.fits

# ./demo data/alex/cluster1_g_10_10.fits \
# data/alex/cluster1_i_10_10.fits \
# data/alex/cluster1_i_10_10.fits

#SINGLE TREE
# ./demo /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simsquare/snr_-3.95/band_1_fake_test_6_snr_-3.95_i_0_ori.fits \
# /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simsquare/snr_-3.95/band_1_fake_test_6_snr_-3.95_i_0_ori.fits \
# /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simsquare/snr_-3.95/band_1_fake_test_6_snr_-3.95_i_0_ori.fits\

# # SINGLE GRAPH
# ./demo /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simsquare/snr_-3.95/band_1_fake_test_6_snr_-3.95_i_0_ori.fits \
# /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simsquare/snr_-3.95/band_2_fake_test_6_snr_-3.95_i_0_ori.fits \
# /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simsquare/snr_-3.95/band_3_fake_test_6_snr_-3.95_i_0_ori.fits \

# 0.000001 /Users/hetpin/Downloads/cgoJune/cgo_bounddata/simsquare/snr_-3.95/segmaps_fake_test_6_snr_-3.95.fits

# #MULTI Detect ok on graph
# ./demo /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simmulti/snr_-2.97/band_1_fake_test_6_snr_-2.97_i_0_ori.fits \
# /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simmulti/snr_-2.97/band_2_fake_test_6_snr_-2.97_i_0_ori.fits \
# /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simmulti/snr_-2.97/band_3_fake_test_6_snr_-2.97_i_0_ori.fits\

# # #MULTI Detect not_ok on graph
# ./demo /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simmulti/snr_-6.5/band_1_fake_test_6_snr_-6.5_i_0_ori.fits \
# /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simmulti/snr_-6.5/band_2_fake_test_6_snr_-6.5_i_0_ori.fits \
# /Users/hetpin/Downloads/cgoJune/cgo_bound/data/simmulti/snr_-6.5/band_3_fake_test_6_snr_-6.5_i_0_ori.fits\

#Debug get_main_leave()
# ./demo /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_g_4250_4250.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_r_4250_4250.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_i_4250_4250.fits 1e-6
  # int w = 110;// 300;
  # int h = 90;//150;
  # int start_i = 270;
  # int start_j = 180;
# ./demo /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_i_3750_3750.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_g_3750_3750.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_r_3750_3750.fits 1e-6

# ./demo /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_g_5000_5000.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_r_5000_5000.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_i_5000_5000.fits 1e-6

# ./demo /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_g_4750_3750.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_r_4750_3750.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_i_4750_3750.fits 1e-6

# #CHECKING CGO WITH THIS TEST
# ./demo /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_r_4250_4000.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_i_4250_4000.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_g_4250_4000.fits 1e-6

./demo /Volumes/DATA/BladeESIEE/Alex/r/repr_KiDS_r_6000511208377_53.146419_-27.77840.fits \
/Volumes/DATA/BladeESIEE/Alex/u/repr_KiDS_u_6000511208377_53.146419_-27.77840.fits \
/Volumes/DATA/BladeESIEE/Alex/g/repr_KiDS_g_6000511208377_53.146419_-27.77840.fits 1e-6

# ./demo /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_i_4250_4000.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_i_4250_4000.fits \
# /Volumes/DATA/BladeESIEE/data_overlap_500/cluster1_i_4250_4000.fits 1e-6

# ./demo data/alex/cluster1_g_10_10.fits \
# data/alex/cluster1_r_10_10.fits \
# data/alex/cluster1_i_10_10.fits 0.000001

# ./demo data/alex/cluster1_g_10_10.fits \
# data/alex/cluster1_r_10_10.fits \
# data/alex/cluster1_r_10_10.fits

# ./demo data/alex/cluster1_g_10_4.fits \
# data/alex/cluster1_r_10_4.fits \
# data/alex/cluster1_i_10_4.fits

# ./demo data/alex/cluster1_g_4_13.fits \
# data/alex/cluster1_r_4_13.fits \
# data/alex/cluster1_i_4_13.fits

# ./demo data/alex/cluster1_g_4_3.fits \
# data/alex/cluster1_r_4_3.fits \
# data/alex/cluster1_i_4_3.fits

# ./demo data/alex/cluster1_g_4_3.fits \
# data/alex/cluster1_g_4_3.fits \
# data/alex/cluster1_g_4_3.fits

# ./demo data/alex/cluster1_r_4_3.fits \
# data/alex/cluster1_r_4_3.fits \
# data/alex/cluster1_r_4_3.fits

# ./demo data/alex/cluster1_i_4_3.fits \
# data/alex/cluster1_i_4_3.fits \
# data/alex/cluster1_i_4_3.fits

# ./demo data/fake_4_ex/fake_test_4_explicit_g.fits data/fake_4_ex/fake_test_4_explicit_g.fits data/fake_4_ex/fake_test_4_explicit_r.fits

# ./demo data/fake_4_ex/fake_test_4_explicit_r.fits data/fake_4_ex/fake_test_4_explicit_r.fits data/fake_4_ex/fake_test_4_explicit_r.fits

# ./demo data/fake_4_ex/fake_test_4_explicit_g.fits data/fake_4_ex/fake_test_4_explicit_g.fits data/fake_4_ex/fake_test_4_explicit_g.fits

# ./demo /Volumes/DATA/BladeESIEE/data/cluster1_i_21_30.fits /Volumes/DATA/BladeESIEE/data/cluster1_g_21_30.fits /Volumes/DATA/BladeESIEE/data/cluster1_r_21_30.fits

