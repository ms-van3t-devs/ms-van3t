/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 SIGNET Lab, Department of Information Engineering,
 * University of Padova
 * Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering,
 * New York University
 * Copyright (c) 2021 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "three-gpp-channel-model-param.h"

#include "ns3/log.h"
#include "ns3/phased-array-model.h"
#include "ns3/node.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/integer.h"
#include <algorithm>
#include <random>
#include "ns3/log.h"
#include <ns3/simulator.h>
#include "ns3/mobility-model.h"
#include "ns3/pointer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ThreeGppChannelModelParam");

NS_OBJECT_ENSURE_REGISTERED (ThreeGppChannelModelParam);



ThreeGppChannelModelParam::ThreeGppChannelModelParam () :ThreeGppChannelModel ()
{
  NS_LOG_FUNCTION (this);
}

ThreeGppChannelModelParam::~ThreeGppChannelModelParam ()
{
  NS_LOG_FUNCTION (this);
}

void
ThreeGppChannelModelParam::DoDispose ()
{
  ThreeGppChannelModel::DoDispose();
}

TypeId
ThreeGppChannelModelParam::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ThreeGppChannelModelParam")
    .SetGroupName ("Spectrum")
    .SetParent<ThreeGppChannelModel> ()
    .AddConstructor<ThreeGppChannelModelParam> ()
    .AddAttribute ("Ro",
                   "Cross polarization correlation parameter.",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&ThreeGppChannelModelParam::SetRo),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ParametrizedCorrelation",
                   "Whether the parameter value Ro will be used as the term for the correlation or "
                   "the 3gpp term: std::sqrt (1 / k). When true Ro will be used, otherwise, 3gpp term.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&ThreeGppChannelModelParam::m_parametrizedCorrelation),
                   MakeBooleanChecker ())


  ;
  return tid;
}

void
ThreeGppChannelModelParam::SetRo (double ro)
{
  m_Ro = ro;
}

Ptr<MatrixBasedChannelModel::ChannelMatrix>
ThreeGppChannelModelParam::GetNewChannel (Ptr<const ThreeGppChannelParams> channelParams,
                                     Ptr<const ParamsTable> table3gpp,
                                     const Ptr<const MobilityModel> sMob,
                                     const Ptr<const MobilityModel> uMob,
                                     Ptr<const PhasedArrayModel> sAntenna,
                                     Ptr<const PhasedArrayModel> uAntenna
                                     ) const
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_frequency > 0.0, "Set the operating frequency first!");

  // create a channel matrix instance
  Ptr<ChannelMatrix> channelMatrix = Create<ChannelMatrix> ();
  channelMatrix->m_generatedTime = Simulator::Now ();
  channelMatrix->m_nodeIds = std::make_pair (sMob->GetObject<Node> ()->GetId (), uMob->GetObject<Node> ()->GetId ());

  // check if channelParams structure is generated in direction s-to-u or u-to-s
  bool isSameDirection = (channelParams->m_nodeIds == channelMatrix->m_nodeIds);

  MatrixBasedChannelModel::Double2DVector rayAodRadian;
  MatrixBasedChannelModel::Double2DVector rayAoaRadian;
  MatrixBasedChannelModel::Double2DVector rayZodRadian;
  MatrixBasedChannelModel::Double2DVector rayZoaRadian;

  // if channel params is generated in the same direction in which we
  // generate the channel matrix, angles and zenit od departure and arrival are ok,
  // just set them to corresponding variable that will be used for the generation
  // of channel matrix, otherwise we need to flip angles and zenits of departure and arrival
  if (isSameDirection)
    {
      rayAodRadian = channelParams->m_rayAodRadian;
      rayAoaRadian = channelParams->m_rayAoaRadian;
      rayZodRadian = channelParams->m_rayZodRadian;
      rayZoaRadian = channelParams->m_rayZoaRadian;
    }
  else
    {
      rayAodRadian = channelParams->m_rayAoaRadian;
      rayAoaRadian = channelParams->m_rayAodRadian;
      rayZodRadian = channelParams->m_rayZoaRadian;
      rayZoaRadian = channelParams->m_rayZodRadian;
    }


  //Step 11: Generate channel coefficients for each cluster n and each receiver
  // and transmitter element pair u,s.

  Complex3DVector H_NLOS; // channel coefficients H_NLOS [u][s][n],
  // where u and s are receive and transmit antenna element, n is cluster index.
  uint64_t uSize = uAntenna->GetNumberOfElements ();
  uint64_t sSize = sAntenna->GetNumberOfElements ();

  Complex3DVector H_usn;  //channel coffecient H_usn[u][s][n];
  // NOTE Since each of the strongest 2 clusters are divided into 3 sub-clusters,
  // the total cluster will be numReducedCLuster + 4.

  H_usn.resize (uSize);
  for (uint64_t uIndex = 0; uIndex < uSize; uIndex++)
    {
      H_usn[uIndex].resize (sSize);
      for (uint64_t sIndex = 0; sIndex < sSize; sIndex++)
        {
          H_usn[uIndex][sIndex].resize (channelParams->m_reducedClusterNumber);
        }
    }

  NS_ASSERT (channelParams->m_reducedClusterNumber <= channelParams->m_clusterPhase.size ());
  NS_ASSERT (channelParams->m_reducedClusterNumber <= channelParams->m_clusterPower.size ());
  NS_ASSERT (channelParams->m_reducedClusterNumber <= channelParams->m_crossPolarizationPowerRatios.size ());
  NS_ASSERT (channelParams->m_reducedClusterNumber <= rayZoaRadian.size ());
  NS_ASSERT (channelParams->m_reducedClusterNumber <= rayZodRadian.size ());
  NS_ASSERT (channelParams->m_reducedClusterNumber <= rayAoaRadian.size ());
  NS_ASSERT (channelParams->m_reducedClusterNumber <= rayAodRadian.size ());
  NS_ASSERT (table3gpp->m_raysPerCluster <= channelParams->m_clusterPhase[0].size ());
  NS_ASSERT (table3gpp->m_raysPerCluster <= channelParams->m_crossPolarizationPowerRatios[0].size ());
  NS_ASSERT (table3gpp->m_raysPerCluster <= rayZoaRadian[0].size ());
  NS_ASSERT (table3gpp->m_raysPerCluster <= rayZodRadian[0].size ());
  NS_ASSERT (table3gpp->m_raysPerCluster <= rayAoaRadian[0].size ());
  NS_ASSERT (table3gpp->m_raysPerCluster <= rayAodRadian[0].size ());

  double x = sMob->GetPosition ().x - uMob->GetPosition ().x;
  double y = sMob->GetPosition ().y - uMob->GetPosition ().y;
  double distance2D = sqrt (x * x + y * y);
  // NOTE we assume hUT = min (height(a), height(b)) and
  // hBS = max (height (a), height (b))
  double hUt = std::min (sMob->GetPosition ().z, uMob->GetPosition ().z);
  double hBs = std::max (sMob->GetPosition ().z, uMob->GetPosition ().z);
  // compute the 3D distance using eq. 7.4-1
  double distance3D = std::sqrt (distance2D * distance2D + (hBs - hUt) * (hBs - hUt));

  Angles sAngle (uMob->GetPosition (), sMob->GetPosition ());
  Angles uAngle (sMob->GetPosition (), uMob->GetPosition ());

  // The following for loops computes the channel coefficients
  for (uint64_t uIndex = 0; uIndex < uSize; uIndex++)
    {
      Vector uLoc = uAntenna->GetElementLocation (uIndex);

      for (uint64_t sIndex = 0; sIndex < sSize; sIndex++)
        {

          Vector sLoc = sAntenna->GetElementLocation (sIndex);

          for (uint8_t nIndex = 0; nIndex < channelParams->m_reducedClusterNumber; nIndex++)
            {
              //Compute the N-2 weakest cluster, assuming 0 slant angle and a
              //polarization slant angle configured in the array (7.5-22)
              if (nIndex != channelParams->m_cluster1st && nIndex != channelParams->m_cluster2nd)
                {
                  std::complex<double> rays (0,0);
                  for (uint8_t mIndex = 0; mIndex < table3gpp->m_raysPerCluster; mIndex++)
                    {
                      DoubleVector initialPhase = channelParams->m_clusterPhase[nIndex][mIndex];

                      double Ro = 0;
                      if (m_parametrizedCorrelation)
                        {
                          Ro = m_Ro;
                        }
                      else
                        {
                          double k = channelParams->m_crossPolarizationPowerRatios[nIndex][mIndex];
                          Ro = std::sqrt (1 / k);
                        }

                      //lambda_0 is accounted in the antenna spacing uLoc and sLoc.
                      double rxPhaseDiff = 2 * M_PI * (sin (rayZoaRadian[nIndex][mIndex]) * cos (rayAoaRadian[nIndex][mIndex]) * uLoc.x
                                                       + sin (rayZoaRadian[nIndex][mIndex]) * sin (rayAoaRadian[nIndex][mIndex]) * uLoc.y
                                                       + cos (rayZoaRadian[nIndex][mIndex]) * uLoc.z);

                      double txPhaseDiff = 2 * M_PI * (sin (rayZodRadian[nIndex][mIndex]) * cos (rayAodRadian[nIndex][mIndex]) * sLoc.x
                                                       + sin (rayZodRadian[nIndex][mIndex]) * sin (rayAodRadian[nIndex][mIndex]) * sLoc.y
                                                       + cos (rayZodRadian[nIndex][mIndex]) * sLoc.z);
                      // NOTE Doppler is computed in the CalcBeamformingGain function and is simplified to only account for the center angle of each cluster.

                      double rxFieldPatternPhi, rxFieldPatternTheta, txFieldPatternPhi, txFieldPatternTheta;
                      std::tie (rxFieldPatternPhi, rxFieldPatternTheta) = uAntenna->GetElementFieldPattern (Angles (rayAoaRadian[nIndex][mIndex], rayZoaRadian[nIndex][mIndex]));
                      std::tie (txFieldPatternPhi, txFieldPatternTheta) = sAntenna->GetElementFieldPattern (Angles (rayAodRadian[nIndex][mIndex], rayZodRadian[nIndex][mIndex]));

                      rays += (exp (std::complex<double> (0, initialPhase[0])) * rxFieldPatternTheta * txFieldPatternTheta +
                               +exp (std::complex<double> (0, initialPhase[1])) * Ro * rxFieldPatternTheta * txFieldPatternPhi +
                               +exp (std::complex<double> (0, initialPhase[2])) * Ro * rxFieldPatternPhi * txFieldPatternTheta +
                               +exp (std::complex<double> (0, initialPhase[3])) * rxFieldPatternPhi * txFieldPatternPhi)
                        * exp (std::complex<double> (0, rxPhaseDiff))
                        * exp (std::complex<double> (0, txPhaseDiff));
                    }
                  rays *= sqrt (channelParams->m_clusterPower[nIndex] / table3gpp->m_raysPerCluster);
                  H_usn[uIndex][sIndex][nIndex] = rays;
                }
              else  //(7.5-28)
                {
                  std::complex<double> raysSub1 (0,0);
                  std::complex<double> raysSub2 (0,0);
                  std::complex<double> raysSub3 (0,0);

                  for (uint8_t mIndex = 0; mIndex < table3gpp->m_raysPerCluster; mIndex++)
                    {
                      double Ro = 0;
                      if (m_parametrizedCorrelation)
                        {
                          Ro = m_Ro;
                        }
                      else
                        {
                          double k = channelParams->m_crossPolarizationPowerRatios[nIndex][mIndex];
                          Ro = std::sqrt (1 / k);
                        }

                      //ZML:Just remind me that the angle offsets for the 3 subclusters were not generated correctly.

                      DoubleVector initialPhase = channelParams->m_clusterPhase[nIndex][mIndex];
                      double rxPhaseDiff = 2 * M_PI * (sin (rayZoaRadian[nIndex][mIndex]) * cos (rayAoaRadian[nIndex][mIndex]) * uLoc.x
                                                       + sin (rayZoaRadian[nIndex][mIndex]) * sin (rayAoaRadian[nIndex][mIndex]) * uLoc.y
                                                       + cos (rayZoaRadian[nIndex][mIndex]) * uLoc.z);
                      double txPhaseDiff = 2 * M_PI * (sin (rayZodRadian[nIndex][mIndex]) * cos (rayAodRadian[nIndex][mIndex]) * sLoc.x
                                                       + sin (rayZodRadian[nIndex][mIndex]) * sin (rayAodRadian[nIndex][mIndex]) * sLoc.y
                                                       + cos (rayZodRadian[nIndex][mIndex]) * sLoc.z);

                      double rxFieldPatternPhi, rxFieldPatternTheta, txFieldPatternPhi, txFieldPatternTheta;
                      std::tie (rxFieldPatternPhi, rxFieldPatternTheta) = uAntenna->GetElementFieldPattern (Angles (rayAoaRadian[nIndex][mIndex], rayZoaRadian[nIndex][mIndex]));
                      std::tie (txFieldPatternPhi, txFieldPatternTheta) = sAntenna->GetElementFieldPattern (Angles (rayAodRadian[nIndex][mIndex], rayZodRadian[nIndex][mIndex]));

                      switch (mIndex)
                        {
                          case 9:
                          case 10:
                          case 11:
                          case 12:
                          case 17:
                          case 18:
                            raysSub2 += (exp (std::complex<double> (0, initialPhase[0])) * rxFieldPatternTheta * txFieldPatternTheta +
                                         +exp (std::complex<double> (0, initialPhase[1])) * Ro * rxFieldPatternTheta * txFieldPatternPhi +
                                         +exp (std::complex<double> (0, initialPhase[2])) * Ro * rxFieldPatternPhi * txFieldPatternTheta +
                                         +exp (std::complex<double> (0, initialPhase[3])) * rxFieldPatternPhi * txFieldPatternPhi)
                              * exp (std::complex<double> (0, rxPhaseDiff))
                              * exp (std::complex<double> (0, txPhaseDiff));
                            break;
                          case 13:
                          case 14:
                          case 15:
                          case 16:
                            raysSub3 += (exp (std::complex<double> (0, initialPhase[0])) * rxFieldPatternTheta * txFieldPatternTheta +
                                         +exp (std::complex<double> (0, initialPhase[1])) * Ro * rxFieldPatternTheta * txFieldPatternPhi +
                                         +exp (std::complex<double> (0, initialPhase[2])) * Ro * rxFieldPatternPhi * txFieldPatternTheta +
                                         +exp (std::complex<double> (0, initialPhase[3])) * rxFieldPatternPhi * txFieldPatternPhi)
                              * exp (std::complex<double> (0, rxPhaseDiff))
                              * exp (std::complex<double> (0, txPhaseDiff));
                            break;
                          default:                      //case 1,2,3,4,5,6,7,8,19,20
                            raysSub1 += (exp (std::complex<double> (0, initialPhase[0])) * rxFieldPatternTheta * txFieldPatternTheta +
                                         +exp (std::complex<double> (0, initialPhase[1])) * Ro * rxFieldPatternTheta * txFieldPatternPhi +
                                         +exp (std::complex<double> (0, initialPhase[2])) * Ro * rxFieldPatternPhi * txFieldPatternTheta +
                                         +exp (std::complex<double> (0, initialPhase[3])) * rxFieldPatternPhi * txFieldPatternPhi)
                              * exp (std::complex<double> (0, rxPhaseDiff))
                              * exp (std::complex<double> (0, txPhaseDiff));
                            break;
                        }
                    }
                  raysSub1 *= sqrt (channelParams->m_clusterPower[nIndex] / table3gpp->m_raysPerCluster);
                  raysSub2 *= sqrt (channelParams->m_clusterPower[nIndex] / table3gpp->m_raysPerCluster);
                  raysSub3 *= sqrt (channelParams->m_clusterPower[nIndex] / table3gpp->m_raysPerCluster);
                  H_usn[uIndex][sIndex][nIndex] = raysSub1;
                  H_usn[uIndex][sIndex].push_back (raysSub2);
                  H_usn[uIndex][sIndex].push_back (raysSub3);

                  NS_LOG_DEBUG ("H_usn[uIndex][sIndex][nIndex]:"<< H_usn[uIndex][sIndex][nIndex]<< " uIndex:"<<uIndex<<", sIndex:"<<sIndex<<"nIndex:"<< +nIndex);
                }
            }
          if (channelParams->m_losCondition == ChannelCondition::LOS) //(7.5-29) && (7.5-30)
            {
              std::complex<double> ray (0,0);
              double rxPhaseDiff = 2 * M_PI * (sin (uAngle.GetInclination ()) * cos (uAngle.GetAzimuth ()) * uLoc.x
                                               + sin (uAngle.GetInclination ()) * sin (uAngle.GetAzimuth ()) * uLoc.y
                                               + cos (uAngle.GetInclination ()) * uLoc.z);
              double txPhaseDiff = 2 * M_PI * (sin (sAngle.GetInclination ()) * cos (sAngle.GetAzimuth ()) * sLoc.x
                                               + sin (sAngle.GetInclination ()) * sin (sAngle.GetAzimuth ()) * sLoc.y
                                               + cos (sAngle.GetInclination ()) * sLoc.z);

              double rxFieldPatternPhi, rxFieldPatternTheta, txFieldPatternPhi, txFieldPatternTheta;
              std::tie (rxFieldPatternPhi, rxFieldPatternTheta) = uAntenna->GetElementFieldPattern (Angles (uAngle.GetAzimuth (), uAngle.GetInclination ()));
              std::tie (txFieldPatternPhi, txFieldPatternTheta) = sAntenna->GetElementFieldPattern (Angles (sAngle.GetAzimuth (), sAngle.GetInclination ()));

              double lambda = 3e8 / m_frequency; // the wavelength of the carrier frequency

              ray = (rxFieldPatternTheta * txFieldPatternTheta - rxFieldPatternPhi * txFieldPatternPhi)
                * exp (std::complex<double> (0, -2 * M_PI * distance3D / lambda))
                * exp (std::complex<double> (0, rxPhaseDiff))
                * exp (std::complex<double> (0, txPhaseDiff));

              double K_linear = pow (10, channelParams->m_K_factor / 10);
              // the LOS path should be attenuated if blockage is enabled.
              H_usn[uIndex][sIndex][0] = sqrt (1 / (K_linear + 1)) * H_usn[uIndex][sIndex][0] + sqrt (K_linear / (1 + K_linear)) * ray / pow (10, channelParams->m_attenuation_dB[0] / 10);           //(7.5-30) for tau = tau1
              double tempSize = H_usn[uIndex][sIndex].size ();
              for (uint8_t nIndex = 1; nIndex < tempSize; nIndex++)
                {
                  H_usn[uIndex][sIndex][nIndex] *= sqrt (1 / (K_linear + 1)); //(7.5-30) for tau = tau2...taunN
                  NS_LOG_DEBUG ("LOS H_usn[uIndex][sIndex][nIndex]:"<< H_usn[uIndex][sIndex][nIndex]<< " uIndex:"<<uIndex<<", sIndex:"<<sIndex<<"nIndex:"<< +nIndex);
                }

            }
        }
    }

  NS_LOG_DEBUG ("Husn (sAntenna, uAntenna):" << sAntenna->GetId () << ", " << uAntenna->GetId ());
  for (auto& i:H_usn)
    {
      for (auto& j:i)
        {
          for (auto& k:j)
            {
              NS_LOG_DEBUG (" " << k << ",");
            }
        }
    }
  NS_LOG_INFO ("size of coefficient matrix =[" << H_usn.size () << "][" << H_usn[0].size () << "][" << H_usn[0][0].size () << "]");
  channelMatrix->m_channel = H_usn;
  return channelMatrix;
}

}  // namespace ns3
