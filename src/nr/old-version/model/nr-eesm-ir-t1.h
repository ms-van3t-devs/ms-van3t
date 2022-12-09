/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef NR_EESM_IR_T1_H
#define NR_EESM_IR_T1_H

#include "nr-eesm-t1.h"
#include "nr-eesm-ir.h"

namespace ns3 {

/**
 * \ingroup error-models
 * \brief The NrEesmIrT1 class
 *
 * Class that implements the IR-HARQ combining with Table 1. It can be used
 * directly in the code.
 */
class NrEesmIrT1 : public NrEesmIr
{
public:
  /**
   * \brief Get the type id of the object
   * \return the type id of the object
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrEesmIrT1 constructor
   */
  NrEesmIrT1 ();
  /**
   * \brief ~NrEesmIrT1 deconstructor
   */
  virtual ~NrEesmIrT1 ();

protected:
  //inherited
  virtual const std::vector<double> * GetBetaTable () const override;
  virtual const std::vector<double> * GetMcsEcrTable () const override;
  virtual const SimulatedBlerFromSINR * GetSimulatedBlerFromSINR () const override;
  virtual const std::vector<uint8_t> * GetMcsMTable () const override;
  virtual const std::vector<double> * GetSpectralEfficiencyForMcs () const override;
  virtual const std::vector<double> * GetSpectralEfficiencyForCqi () const override;

private:
  NrEesmT1 m_t1; //!< The reference table
};

} // namespace ns3

#endif // NR_EESM_IR_T1_H
