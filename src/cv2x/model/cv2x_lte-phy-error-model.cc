/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.

 * We would appreciate acknowledgement if the software is used.

 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 *
 * Modified by: NIST
 */

#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <stdint.h>
#include <ns3/math.h>
#include <ns3/cv2x_lte-phy-error-model.h>

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("cv2x_LtePhyErrorModel");


  /** 
   * Table of SINR for the physical uplink shared channel
   * SINR range is provided for each MCS and HARQ Tx
   * Index is defined by 4 * MCS + HARQ Tx
   */
static const double PuschAwgnSisoBlerCurveXaxis[116][3] = {
{-9.6,-4.6,0.2},
{-12.6,-7.2,0.2},
{-15.2,-8.8,0.2},
{-15.6,-9.8,0.2},
{-7.4,-3.0,0.2},
{-10.4,-5.2,0.2},
{-11.8,-7.0,0.2},
{-13.4,-8.0,0.2},
{-6.0,-2.6,0.2},
{-9.0,-5.0,0.2},
{-10.8,-6.8,0.2},
{-12.2,-8.4,0.2},
{-5.4,-2.0,0.2},
{-8.4,-4.6,0.2},
{-10.0,-6.4,0.2},
{-11.4,-7.6,0.2},
{-4.4,-1.2,0.2},
{-7.4,-3.8,0.2},
{-9.2,-5.4,0.2},
{-10.8,-6.6,0.2},
{-3.4,-0.4,0.2},
{-6.2,-2.2,0.2},
{-7.8,-4.8,0.2},
{-9.6,-6.0,0.2},
{-2.4,0.4,0.2},
{-5.2,-2.4,0.2},
{-7.2,-4.0,0.2},
{-8.2,-5.4,0.2},
{-1.0,1.4,0.2},
{-4.2,-1.2,0.2},
{-6.0,-3.2,0.2},
{-7.4,-4.8,0.2},
{-0.4,2.0,0.2},
{-3.6,-1.0,0.2},
{-5.6,-2.2,0.2},
{-6.8,-3.8,0.2},
{0.6,2.8,0.2},
{-3.0,-0.6,0.2},
{-4.8,-1.8,0.2},
{-6.0,-3.4,0.2},
{1.8,3.4,0.2},
{-2.4,-0.6,0.2},
{-4.2,-2.2,0.2},
{-5.6,-3.6,0.2},
{2.6,4.8,0.2},
{-0.4,1.6,0.2},
{-1.8,0.2,0.2},
{-2.6,-0.8,0.2},
{3.6,5.4,0.2},
{0.2,2.0,0.2},
{-1.4,0.8,0.2},
{-2.2,-0.4,0.2},
{4.4,6.4,0.2},
{1.0,3.0,0.2},
{-0.8,1.2,0.2},
{-1.8,0.2,0.2},
{5.2,7.0,0.2},
{1.4,3.4,0.2},
{-0.2,1.4,0.2},
{-1.2,0.4,0.2},
{6.0,7.8,0.2},
{2.0,3.8,0.2},
{0.2,2.0,0.2},
{-0.8,1.0,0.2},
{6.6,8.4,0.2},
{2.6,4.2,0.2},
{0.6,2.4,0.2},
{-0.4,1.2,0.2},
{7.0,8.8,0.2},
{2.6,4.4,0.2},
{1.0,2.8,0.2},
{-0.2,1.4,0.2},
{7.8,9.6,0.2},
{3.2,4.8,0.2},
{1.4,3.0,0.2},
{0.2,1.8,0.2},
{8.6,10.4,0.2},
{3.6,5.4,0.2},
{1.8,3.6,0.2},
{0.6,2.2,0.2},
{9.8,11.6,0.2},
{4.4,6.0,0.2},
{2.4,4.0,0.2},
{1.2,2.6,0.2},
{10.4,12.2,0.2},
{6.0,7.6,0.2},
{3.2,5.0,0.2},
{1.8,3.4,0.2},
{11.2,13.0,0.2},
{6.4,8.0,0.2},
{3.6,5.4,0.2},
{2.2,3.6,0.2},
{12.0,13.6,0.2},
{6.6,8.2,0.2},
{4.0,7.2,0.2},
{2.4,4.0,0.2},
{12.6,14.4,0.2},
{7.0,8.6,0.2},
{4.4,5.8,0.2},
{2.8,4.2,0.2},
{13.4,15.2,0.2},
{7.2,8.6,0.2},
{4.6,6.2,0.2},
{3.0,4.4,0.2},
{14.0,15.8,0.2},
{7.4,9.4,0.2},
{5.0,6.6,0.2},
{3.4,4.8,0.2},
{14.8,16.6,0.2},
{7.8,9.6,0.2},
{5.4,7.0,0.2},
{3.6,5.2,0.2},
{17.2,18.8,0.2},
{9.2,11.0,0.2},
{6.6,8.0,0.2},
{4.8,6.2,0.2}
};

  /** 
   * BLER values for the physical uplink shared channel
   * One dimension version of original table of [116][33] 
   * Index is computed based on MCS, HARQ, and SINR
   */
static const double PuschAwgnSisoBlerCurveYaxis[3828] = {
1,0.9992,0.998,0.9984,0.996,0.9948,0.9868,0.9792,0.9544,0.9384,0.9052,0.8532,0.798,0.6936,0.6048,0.4948,0.3864,0.2972,0.198,0.1388,0.0832,0.0432,0.0228,0.0108,0.0044,0.002,0,0,0,0,0,0,0,
1,0.9992,0.9992,0.9984,0.9984,0.9936,0.9908,0.9776,0.9672,0.9408,0.9016,0.8688,0.7828,0.7156,0.626,0.5096,0.3944,0.2924,0.2044,0.1392,0.0836,0.0452,0.028,0.0128,0.0056,0.0044,0.0008,0,0,0,0,0,0,
1,0.9996,1,1,1,0.9996,0.9996,0.9988,0.9984,0.9968,0.9904,0.9772,0.9688,0.9536,0.9188,0.876,0.806,0.7336,0.6424,0.5384,0.424,0.306,0.222,0.1484,0.0828,0.0528,0.0268,0.0116,0.0056,0.0032,0.0008,0.0016,0,
1,0.9996,0.9996,0.9968,0.9944,0.994,0.9892,0.9748,0.9664,0.9408,0.914,0.8508,0.7884,0.7032,0.6136,0.5188,0.3944,0.3124,0.2088,0.1388,0.078,0.0408,0.0208,0.0104,0.0052,0.0028,0.0016,0,0.0004,0,0,0,0,
1,0.9988,0.9996,0.9992,0.9984,0.9956,0.988,0.97,0.95,0.8992,0.8388,0.7616,0.6416,0.512,0.3912,0.2492,0.1704,0.0996,0.05,0.0232,0.0104,0.0036,0.0012,0,0,0,0,0,0,0,0,0,0,
1,0.9996,1,0.9996,0.9976,0.9924,0.988,0.9764,0.95,0.902,0.8352,0.7752,0.638,0.5068,0.3912,0.2624,0.1768,0.0964,0.0468,0.02,0.01,0.0028,0.0008,0.0004,0,0.0004,0,0,0,0,0,0,0,
1,0.9996,0.998,0.994,0.9888,0.9732,0.9564,0.9156,0.8488,0.78,0.6856,0.5532,0.402,0.2776,0.1916,0.1164,0.0568,0.0292,0.016,0.0028,0.0004,0.0008,0,0.0004,0,0,0,0,0,0,0,0,0,
1,0.9996,1,0.9984,0.998,0.9916,0.9912,0.9736,0.9528,0.9008,0.8476,0.74,0.64,0.512,0.4016,0.2588,0.1528,0.0876,0.0556,0.0164,0.008,0.0024,0.0004,0.0004,0,0,0.0004,0,0,0,0,0,0,
1,0.998,0.994,0.9916,0.9872,0.9648,0.9188,0.876,0.7804,0.6476,0.512,0.3632,0.2308,0.1412,0.0808,0.0336,0.0144,0.0032,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.9968,0.9924,0.9832,0.9528,0.908,0.852,0.7588,0.6272,0.5028,0.3452,0.2148,0.1268,0.068,0.0228,0.0124,0.0048,0.0016,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9964,0.9936,0.984,0.9684,0.9188,0.8524,0.7628,0.6576,0.512,0.3572,0.2256,0.1396,0.0604,0.0296,0.01,0.0032,0.0004,0.0016,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9972,0.9968,0.992,0.98,0.9544,0.9188,0.8376,0.7476,0.618,0.4704,0.3548,0.2188,0.1308,0.0592,0.0292,0.01,0.004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9996,0.9952,0.99,0.9776,0.948,0.9096,0.8128,0.6976,0.58,0.4088,0.2836,0.166,0.0764,0.038,0.0132,0.0028,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9996,0.994,0.9892,0.9724,0.9568,0.8948,0.7872,0.6876,0.52,0.3716,0.2348,0.132,0.0592,0.03,0.0112,0.0024,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9976,0.9924,0.974,0.946,0.896,0.81,0.6932,0.5448,0.3808,0.2412,0.1456,0.0732,0.0384,0.0104,0.002,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9984,0.9992,0.9956,0.9864,0.97,0.942,0.8812,0.7776,0.6548,0.5208,0.3704,0.2316,0.1272,0.0516,0.0248,0.008,0.0048,0.0016,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9988,0.9972,0.9864,0.9604,0.9172,0.8404,0.7188,0.576,0.412,0.2648,0.1448,0.0772,0.0324,0.0148,0.0032,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9988,0.9952,0.9872,0.962,0.9368,0.8408,0.7028,0.5548,0.388,0.2444,0.1276,0.0508,0.0224,0.0044,0.0008,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9996,0.9948,0.9896,0.9656,0.9256,0.8532,0.75,0.5968,0.4176,0.262,0.1436,0.068,0.0284,0.0088,0.0016,0.0012,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,1,0.9996,0.9992,0.9956,0.9884,0.9648,0.9168,0.8264,0.6904,0.5452,0.3756,0.2308,0.1216,0.0496,0.0148,0.0052,0.0012,0.0004,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9992,0.9992,0.9944,0.9832,0.946,0.8848,0.7708,0.592,0.4032,0.2288,0.1152,0.0488,0.0204,0.0052,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9984,0.994,0.9776,0.9448,0.882,0.7464,0.5756,0.3992,0.2316,0.1072,0.0428,0.0152,0.004,0.0004,0,0,0,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.998,0.9948,0.9816,0.954,0.8908,0.782,0.626,0.4224,0.2512,0.1236,0.0456,0.0136,0.0024,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,1,1,0.9988,0.9916,0.9748,0.9308,0.8608,0.7236,0.5492,0.3796,0.206,0.1092,0.034,0.0096,0.0024,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9992,0.9988,0.992,0.9672,0.9192,0.816,0.6552,0.4372,0.2428,0.1168,0.0436,0.014,0.0036,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.9964,0.9872,0.9476,0.8688,0.738,0.5416,0.3056,0.1648,0.0608,0.0232,0.0036,0.0016,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9988,0.9948,0.9836,0.9532,0.8792,0.7316,0.5516,0.3416,0.172,0.0736,0.0236,0.0076,0.0016,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9972,0.9944,0.98,0.9396,0.8336,0.7056,0.5032,0.3096,0.1484,0.0568,0.0168,0.0052,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9932,0.9788,0.9344,0.8396,0.6808,0.4552,0.2448,0.1252,0.044,0.014,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9888,0.9656,0.8952,0.7632,0.5624,0.346,0.164,0.0624,0.016,0.0072,0.0028,0,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9976,0.9812,0.9536,0.868,0.7188,0.504,0.2988,0.138,0.0528,0.0128,0.0016,0,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9976,0.9848,0.9452,0.8432,0.6936,0.4612,0.2484,0.1112,0.034,0.0088,0.0016,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.998,0.992,0.9712,0.9128,0.7916,0.6032,0.3752,0.1744,0.0748,0.0164,0.0044,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.998,0.99,0.9616,0.896,0.7316,0.516,0.3044,0.1148,0.0392,0.0068,0.0012,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9976,0.9896,0.9552,0.854,0.6912,0.4688,0.2592,0.1164,0.0368,0.0068,0.002,0,0,0,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.996,0.986,0.9496,0.8456,0.6408,0.4164,0.2268,0.0808,0.0156,0.0036,0.0004,0,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9972,0.9832,0.9368,0.822,0.6452,0.416,0.2032,0.0748,0.0204,0.0048,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9916,0.9684,0.8888,0.7328,0.4828,0.2524,0.0988,0.02,0.0032,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9936,0.9656,0.8868,0.7216,0.4848,0.2608,0.1012,0.0272,0.0068,0.0016,0.0004,0,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.998,0.9892,0.9468,0.8464,0.6368,0.42,0.1724,0.0604,0.0124,0.0008,0,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.968,0.896,0.76,0.552,0.296,0.156,0.048,0.012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.996,0.96,0.864,0.708,0.464,0.228,0.068,0.024,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.996,0.98,0.932,0.792,0.5,0.284,0.112,0.044,0.012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9924,0.952,0.876,0.62,0.356,0.168,0.064,0.008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9984,0.9944,0.9696,0.884,0.7056,0.4076,0.1872,0.0572,0.0108,0.002,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9916,0.9432,0.8036,0.5388,0.236,0.0748,0.012,0.0016,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9944,0.95,0.8272,0.5232,0.2292,0.0532,0.01,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9972,0.9736,0.8444,0.552,0.2356,0.0516,0.006,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.998,0.976,0.8968,0.7208,0.4448,0.2068,0.07,0.01,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.998,0.9816,0.9092,0.694,0.3604,0.1364,0.0312,0.004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.998,0.9784,0.8648,0.5912,0.2848,0.068,0.008,0.0008,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9892,0.9056,0.6764,0.298,0.07,0.01,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9948,0.9644,0.8816,0.6716,0.3856,0.1448,0.0408,0.0096,0.0016,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9944,0.9576,0.84,0.5612,0.2776,0.08,0.0112,0.002,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.998,0.9752,0.8516,0.578,0.2304,0.0544,0.008,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9992,0.976,0.858,0.5472,0.2032,0.0384,0.0044,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9972,0.9824,0.9388,0.764,0.4952,0.2316,0.068,0.0152,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.9888,0.9296,0.7248,0.4144,0.1584,0.0296,0.0044,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9984,0.9724,0.874,0.5732,0.2512,0.0576,0.0072,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.9672,0.8324,0.5128,0.1524,0.024,0.0016,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9984,0.9892,0.9384,0.7844,0.5344,0.2584,0.0672,0.0128,0.002,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9884,0.9376,0.7132,0.3944,0.124,0.024,0.002,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9968,0.9636,0.8092,0.4676,0.1572,0.0296,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9888,0.9184,0.6504,0.2688,0.0588,0.008,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.9888,0.9432,0.818,0.516,0.2528,0.0848,0.012,0.0028,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.992,0.9356,0.7124,0.3968,0.1224,0.0164,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9956,0.952,0.7724,0.4128,0.1164,0.0152,0.002,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.978,0.8656,0.5116,0.1436,0.0176,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9944,0.958,0.8256,0.5796,0.2808,0.0892,0.0212,0.0024,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9944,0.952,0.7832,0.456,0.1688,0.0288,0.0016,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.9792,0.8308,0.4876,0.1592,0.0204,0.0024,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.9836,0.8696,0.5252,0.162,0.022,0.002,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9932,0.9652,0.8544,0.6092,0.3064,0.1088,0.0268,0.0036,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.9776,0.8456,0.5152,0.1848,0.034,0.0032,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9824,0.8816,0.5608,0.186,0.0312,0.002,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9984,0.9872,0.8512,0.476,0.1292,0.0156,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.9944,0.9616,0.8724,0.6084,0.3148,0.1008,0.03,0.0032,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9984,0.9892,0.8896,0.6136,0.2332,0.0456,0.0032,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9912,0.904,0.6424,0.2424,0.0416,0.0036,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.98,0.8224,0.4436,0.0956,0.006,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9904,0.9352,0.7756,0.4664,0.2152,0.0564,0.0116,0.0016,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.9672,0.8204,0.446,0.1328,0.016,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9932,0.9048,0.6428,0.224,0.0304,0.0024,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.998,0.956,0.7084,0.266,0.038,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.996,0.9744,0.8728,0.6352,0.3344,0.106,0.028,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9984,0.972,0.8628,0.5456,0.2152,0.0452,0.0032,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9988,0.9612,0.7928,0.4352,0.1208,0.0204,0.002,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9984,0.9856,0.8704,0.5304,0.1668,0.0228,0.0024,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9984,0.9828,0.9216,0.7436,0.4376,0.1628,0.0356,0.0076,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9904,0.9452,0.7516,0.4024,0.1064,0.0188,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9928,0.9128,0.6628,0.288,0.0564,0.0064,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.998,0.9604,0.7348,0.334,0.07,0.0076,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9948,0.9636,0.8468,0.5896,0.3024,0.0876,0.0176,0.0036,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9704,0.832,0.5056,0.178,0.028,0.0024,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9736,0.8076,0.4508,0.1328,0.0216,0.0004,0,0,0,0,0,0,0,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9852,0.8582,0.4952,0.1424,0.0124,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9976,0.9808,0.9008,0.6876,0.382,0.1364,0.0308,0.002,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9928,0.9324,0.6848,0.2952,0.0672,0.008,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9984,0.9568,0.7564,0.3904,0.0924,0.0104,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9992,0.9616,0.7312,0.3208,0.056,0.004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9896,0.9376,0.794,0.5196,0.2176,0.058,0.0104,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.996,0.9416,0.7264,0.3396,0.0832,0.008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9796,0.85,0.4876,0.1308,0.0152,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.998,0.9744,0.7704,0.3552,0.0532,0.0036,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9988,0.9936,0.9648,0.8408,0.5556,0.2676,0.0752,0.012,0.002,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9988,0.9696,0.8016,0.4212,0.1108,0.0152,0.0004,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.984,0.8644,0.5256,0.1532,0.022,0.0012,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9948,0.9356,0.6424,0.2,0.0304,0.0008,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9964,0.9652,0.8532,0.6004,0.2884,0.092,0.0152,0.0036,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9968,0.9704,0.8224,0.4376,0.1012,0.0136,0.0016,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.998,0.96,0.7648,0.3704,0.0832,0.0056,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.9868,0.842,0.4444,0.1012,0.0076,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9944,0.9536,0.8196,0.5356,0.2408,0.0756,0.01,0.002,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9996,0.996,0.9452,0.7292,0.3404,0.0712,0.004,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9976,0.9514,0.7112,0.2812,0.0376,0.004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,0.9872,0.856,0.4268,0.078,0.0024,0.0004,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

  /** 
   * Table of SINR for the physical sidelink discovery channel
   * SINR range is provided for each HARQ Tx
   * Index is defined by HARQ Tx
   */
static const double PsdchAwgnSisoBlerCurveXaxis[4][3] = {
{-0.8,2.4,0.2},
{-4.4,-0.4,0.2},
{-6.0,-1.6,0.2},
{-7.2,-3.4,0.2}
};

  /** 
   * BLER values for the physical sidelink discovery channel
   * One dimension version of original table of [116][33] 
   * Index is computed based on MCS, HARQ, and SINR
   */
static const double PsdchAwgnSisoBlerCurveYaxis[92] = {
1,0.9984,0.9972,0.9936,0.98,0.9524,0.9148,0.8192,0.7016,0.5628,0.3812,0.2616,0.1224,0.0708,0.0252,0.0108,0.0028,0,0,0,0,0,0,
1,0.9996,0.9996,0.9964,0.9908,0.9752,0.944,0.8692,0.7884,0.6448,0.47,0.2956,0.188,0.0828,0.0296,0.0136,0.0036,0.002,0.0004,0.0008,0,0,0,
1,0.9996,0.994,0.9888,0.974,0.9344,0.8676,0.7572,0.6104,0.4436,0.2816,0.1676,0.0704,0.034,0.012,0.0028,0.0016,0.0008,0.0004,0,0.0004,0.0004,0,
1,0.9988,0.9956,0.9896,0.9648,0.9332,0.8548,0.742,0.58,0.4272,0.2528,0.1384,0.0756,0.0248,0.0108,0.0032,0,0,0.0004,0,0,0,0};

  /** 
   * Table of SINR for the physical sidelink control channel
   * SINR range is provided for each HARQ Tx
   * Index is defined by HARQ Tx
   */
static const double PscchAwgnSisoBlerCurveXaxis[1][3] = {
{-6.2,1.2,0.2}
};

  /** 
   * BLER values for the physical sidelink control channel
   * One dimension version of original table of [116][33] 
   * Index is computed based on MCS, HARQ, and SINR
   */
static const double PscchAwgnSisoBlerCurveYaxis[38] = {
1,0.9883,0.9784,0.9721,0.9631,0.9517,0.9325,0.9133,0.8783,0.849,0.8048,0.7565,0.6958,0.6325,0.5703,0.5049,0.4273,0.3733,0.2989,0.2437,0.1932,0.1467,0.1126,0.0785,0.0592,0.0423,0.0265,0.0186,0.011,0.0072,0.0031,0.0026,0.0011,0.0008,0.0003,0.0002,0.0001,0};

  /** 
   * Table of SINR for the physical sidelink broadcast channel
   * SINR range is provided for each MCS and HARQ Tx
   * Index is defined by 4 * MCS + HARQ Tx
   */
static const double PsbchAwgnSisoBlerCurveXaxis[1][3] = {
{-13.2,-4.8,0.2}
};

  /** 
   * BLER values for the physical sidelink broadcast channel
   * One dimension version of original table of [116][33] 
   * Index is computed based on MCS, HARQ, and SINR
   */
static const double PsbchAwgnSisoBlerCurveYaxis[43] = {
1,0.9979,0.9969,0.9958,0.995,0.9912,0.9912,0.9846,0.9766,0.9661,0.9587,0.9409,0.9217,0.8995,0.8787,0.8373,0.7927,0.744,0.6976,0.6348,0.5662,0.5032,0.4392,0.3677,0.3068,0.2522,0.2011,0.1504,0.1254,0.0878,0.0635,0.0436,0.032,0.0211,0.0146,0.008,0.0063,0.0046,0.0033,0.0017,0.001,0.0004,0};


int16_t
cv2x_LtePhyErrorModel::GetRowIndex (uint16_t mcs, uint8_t harq)
{
  NS_LOG_FUNCTION (mcs << (uint16_t) harq);
  return 4*mcs + harq;
}

int16_t
cv2x_LtePhyErrorModel::GetColIndex (double val, double min, double max, double step)
{
  NS_LOG_FUNCTION (val << min << max << step);
  //std::cout << "GetColIndex val=" << val << " min=" << min << " max=" << max << " step=" << step << std::endl;
  int16_t index = -1; //out of range
  if (val >= min)
    {
      val=std::min(val, max); //must avoid overflow
      index = (val-min)/step;
    }
  return index;
}

double
cv2x_LtePhyErrorModel::GetBlerValue (const double (*xtable)[XTABLE_SIZE], const double (*ytable), const uint16_t ysize, uint16_t mcs, uint8_t harq, double sinr)
{
  NS_LOG_FUNCTION (mcs << (uint16_t) harq << sinr);
  double sinrDb = 10 * std::log10 (sinr);
  int16_t rIndex = GetRowIndex (mcs, harq);
  double bler = 1;

  //std::cout << "sinrDb=" << sinrDb << " min=" << xtable[rIndex][0] << " max=" << xtable[rIndex][1] << std::endl;
  if (sinrDb < xtable[rIndex][0])
    {
      bler = 1;
    } 
  else if (sinrDb  > xtable[rIndex][1]) 
    {
      bler = 0;
    } 
  else
    {    
      int16_t index1 = std::floor((sinrDb-xtable[rIndex][0])/xtable[rIndex][2]);
      int16_t index2 = std::ceil((sinrDb-xtable[rIndex][0])/xtable[rIndex][2]);
      if (index1 != index2) 
	{
	  //interpolate
	  double sinr1 = std::pow (10, (xtable[rIndex][0] + index1*xtable[rIndex][2]) / 10);
	  double sinr2 = std::pow (10, (xtable[rIndex][0] + index2*xtable[rIndex][2]) / 10);
	  double bler1 = ytable[rIndex*ysize+index1];
	  double bler2 = ytable[rIndex*ysize+index2];
	  bler = bler1 + (bler2-bler1)*(sinr-sinr1)/(sinr2-sinr1);
	}
      else
	{
	  bler = ytable[rIndex*ysize+index1];
	}
    }
  return bler;
}

double
cv2x_LtePhyErrorModel::GetSinrValue (const double (*xtable)[XTABLE_SIZE], const double (*ytable), const uint16_t ysize, uint16_t mcs, uint8_t harq, double bler)
{
  double sinr = 0;
  int16_t rIndex = GetRowIndex (mcs, harq);
  uint16_t index = 0;
  while (ytable[rIndex*ysize+index] > bler) 
    {
      index++;
    }

  if (ytable[rIndex*ysize+index] < bler) 
    {
      double sinr1 = std::pow (10, (xtable[rIndex][0] + (index-1)*xtable[rIndex][2]) / 10);
      double sinr2 = std::pow (10, (xtable[rIndex][0] + index*xtable[rIndex][2]) / 10);
      double bler1 = ytable[rIndex*ysize+index-1];
      double bler2 = ytable[rIndex*ysize+index];
      sinr = sinr1 + (bler - bler1) * (sinr2-sinr1) / (bler2-bler1);
    } 
  else
    {
      //last or equal element
      sinr = std::pow (10, (xtable[rIndex][0] + index*xtable[rIndex][2]) / 10);
    }
  return sinr;
}

double 
cv2x_LtePhyErrorModel::GetValueForIndex (uint16_t index, double min, double max, double step)
{
  NS_LOG_FUNCTION (index << min << max << step);
  //std::cout << "GetValueForIndex " << index << " " << min << " " << max << " " << step << std::endl;
  double value = 1;
  if (index > -1) 
    {
      value = min + index * step;
    }
  return value;
}

uint16_t
cv2x_LtePhyErrorModel::GetBlerIndex (double bler, uint16_t row, const double (*ytable), const uint16_t ysize)
{
  uint16_t index = 0;
  while (ytable[row*ysize+index] > bler) 
    {
      index++;
    }
  return index-1; //return the index of the last column where bler is above the target one
}

TbErrorStats_t
cv2x_LtePhyErrorModel::GetBler (const double (*xtable)[XTABLE_SIZE], const double (*ytable), const uint16_t ysize, uint16_t mcs, uint8_t harq, double prevSinr, double newSinr)
  {
    NS_LOG_FUNCTION (mcs << (uint16_t) harq << prevSinr << newSinr);

    TbErrorStats_t tbStat;
    tbStat.tbler = 1;

    if (harq > 0 && prevSinr != newSinr)
      {
	//must combine previous and new transmission
	double prevBler = GetBlerValue (xtable, ytable, ysize, mcs, harq, prevSinr);
	double newBler = GetBlerValue (xtable, ytable, ysize, mcs, harq, newSinr);
	//compute effective BLER
	if (prevBler == 1 && newBler == 1) 
	  {
	    //if both have BLER of 1, the interpolation will not work
	    tbStat.tbler = 1;
	    tbStat.sinr = prevSinr > newSinr ? prevSinr : newSinr;
	  } 
	else
	  { 
	    if (newSinr > prevSinr) 
	      {
		tbStat.tbler = (prevBler + newBler * newSinr / prevSinr) / (1 + newSinr / prevSinr);
	      }
	    else 
	      {
		tbStat.tbler = (prevBler + newBler * prevSinr / newSinr) / (1 + prevSinr / newSinr);
	      }
	    //reverse lookup to find effective SINR
	    tbStat.sinr = GetSinrValue (xtable, ytable, ysize, mcs, harq, tbStat.tbler);
	  }
	NS_LOG_DEBUG ("prevBler=" << prevBler << " newBler=" << newBler << " bler=" << tbStat.tbler);	
	
      } else 
      {
	//first transmission or the SINR did not change
	tbStat.tbler = GetBlerValue (xtable, ytable, ysize, mcs, harq, newSinr);
	//double tmp = GetSinrValue (xtable, ytable, mcs, harq, tbStat.tbler);
	//std::cout << " newSinr=" << newSinr << " reverse lookup=" << tmp << std::endl;
	tbStat.sinr = newSinr;       
      }
    NS_LOG_INFO ("bler=" << tbStat.tbler << ", sinr=" << tbStat.sinr);
    //std::cout << "bler=" << tbStat.tbler << ", sinr=" << tbStat.sinr << std::endl;
    return tbStat;
  }


TbErrorStats_t
cv2x_LtePhyErrorModel::GetPsschBler (LteFadingModel fadingChannel, LteTxMode txmode, uint16_t mcs, double sinr, HarqProcessInfoList_t harqHistory)
{
  //Check mcs values
  if (mcs > 20) 
    {
      NS_FATAL_ERROR ("PSSCH modulation cannot exceed 20");
    }

  //Find the table to use
  const double (*xtable)[XTABLE_SIZE];
  const double *ytable;
  double ysize = 0;

  switch (fadingChannel) {
  case AWGN:
    switch (txmode) {
    case SISO:
      xtable = PuschAwgnSisoBlerCurveXaxis;
      ytable = PuschAwgnSisoBlerCurveYaxis;
      ysize = PUSCH_AWGN_SIZE;
      break;
    default: 
      NS_FATAL_ERROR ("Transmit mode "<< txmode << " not supported in AWGN channel");
    }
    break;
  default:
    NS_FATAL_ERROR ("Fading channel " << fadingChannel << " not supported");
  }

  TbErrorStats_t tbStat;
  if (harqHistory.size() == 0)
    {
      tbStat = GetBler (xtable, ytable, ysize, mcs, 0, 0,  sinr);
    }
  else 
    {
      tbStat = GetBler (xtable, ytable, ysize, mcs, harqHistory.size(), harqHistory[harqHistory.size()-1].m_sinr,  sinr);
    }

  return tbStat;
}

TbErrorStats_t
cv2x_LtePhyErrorModel::GetPsdchBler (LteFadingModel fadingChannel, LteTxMode txmode, double sinr, HarqProcessInfoList_t harqHistory)
{
  //Find the table to use
  const double (*xtable)[XTABLE_SIZE];
  const double *ytable;
  double ysize = 0;

  switch (fadingChannel) {
  case AWGN:
    switch (txmode) {
    case SISO:
      xtable = PsdchAwgnSisoBlerCurveXaxis;
      ytable = PsdchAwgnSisoBlerCurveYaxis;
      ysize = PSDCH_AWGN_SIZE;
      break;
    default: 
      NS_FATAL_ERROR ("Transmit mode "<< txmode << " not supported in AWGN channel");
    }
    break;
  default:
    NS_FATAL_ERROR ("Fading channel " << fadingChannel << " not supported");
  }

  TbErrorStats_t tbStat;
  if (harqHistory.size() == 0)
    {
      tbStat = GetBler (xtable, ytable, ysize, 0 /*since no mcs used*/, 0, 0,  sinr);
    }
  else 
    {
      tbStat = GetBler (xtable, ytable, ysize, 0 /*since no mcs used*/, harqHistory.size(), harqHistory[harqHistory.size()-1].m_sinr,  sinr);
    }

  return tbStat;
}

TbErrorStats_t
cv2x_LtePhyErrorModel::GetPscchBler (LteFadingModel fadingChannel, LteTxMode txmode, double sinr)
{
  //Find the table to use
  const double (*xtable)[XTABLE_SIZE];
  const double *ytable;
  double ysize = 0;

  switch (fadingChannel) {
  case AWGN:
    switch (txmode) {
    case SISO:
      xtable = PscchAwgnSisoBlerCurveXaxis;
      ytable = PscchAwgnSisoBlerCurveYaxis;
      ysize = PSCCH_AWGN_SIZE;
      break;
    default: 
      NS_FATAL_ERROR ("Transmit mode "<< txmode << " not supported in AWGN channel");
    }
    break;
  default:
    NS_FATAL_ERROR ("Fading channel " << fadingChannel << " not supported");
  }

  TbErrorStats_t tbStat = GetBler (xtable, ytable, ysize, 0 /*since no mcs used*/, 0, 0,  sinr);

  return tbStat;
}

TbErrorStats_t
cv2x_LtePhyErrorModel::GetPuschBler (LteFadingModel fadingChannel, LteTxMode txmode, uint16_t mcs, double sinr, HarqProcessInfoList_t harqHistory)
{
  //Check mcs values
  if (mcs > 28) 
    {
      NS_FATAL_ERROR ("PUSCH modulation cannot exceed 28");
    }

  //Find the table to use
  const double (*xtable)[XTABLE_SIZE];
  const double *ytable;
  double ysize = 0;

  switch (fadingChannel) {
  case AWGN:
    switch (txmode) {
    case SISO:
      xtable = PuschAwgnSisoBlerCurveXaxis;
      ytable = PuschAwgnSisoBlerCurveYaxis;
      ysize = PUSCH_AWGN_SIZE;
      break;
    default: 
      NS_FATAL_ERROR ("Transmit mode "<< txmode << " not supported in AWGN channel");
    }
    break;
  default:
    NS_FATAL_ERROR ("Fading channel " << fadingChannel << " not supported");
  }

  TbErrorStats_t tbStat;
  if (harqHistory.size() == 0)
    {
      tbStat = GetBler (xtable, ytable, ysize, mcs, 0, 0,  sinr);
    }
  else 
    {
      tbStat = GetBler (xtable, ytable, ysize, mcs, harqHistory.size(), harqHistory[harqHistory.size()-1].m_sinr,  sinr);
    }

  return tbStat;
}

TbErrorStats_t
cv2x_LtePhyErrorModel::GetPsbchBler (LteFadingModel fadingChannel, LteTxMode txmode, double sinr)
{
  //Find the table to use
  const double (*xtable)[XTABLE_SIZE];
  const double *ytable;
  double ysize = 0;

  switch (fadingChannel) {
  case AWGN:
    switch (txmode) {
    case SISO:
      xtable = PsbchAwgnSisoBlerCurveXaxis;
      ytable = PsbchAwgnSisoBlerCurveYaxis;
      ysize = PSBCH_AWGN_SIZE;
      break;
    default:
      NS_FATAL_ERROR ("Transmit mode "<< txmode << " not supported in AWGN channel");
    }
    break;
  default:
    NS_FATAL_ERROR ("Fading channel " << fadingChannel << " not supported");
  }

  TbErrorStats_t tbStat = GetBler (xtable, ytable, ysize, 0 /*since no mcs used*/, 0, 0,  sinr);

  return tbStat;
}


} // namespace ns3
