/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#ifndef NR_SL_COMM_PRE_CONFIG_RESOURCE_POOL_FACTORY_H
#define NR_SL_COMM_PRE_CONFIG_RESOURCE_POOL_FACTORY_H

#include <ns3/simple-ref-count.h>
#include <ns3/lte-rrc-sap.h>
#include "nr-sl-comm-resource-pool-factory.h"

namespace ns3 {

/** Class to configure and generate resource pools */
class NrSlCommPreconfigResourcePoolFactory : public NrSlCommResourcePoolFactory
{
public:
  NrSlCommPreconfigResourcePoolFactory ();
  virtual ~NrSlCommPreconfigResourcePoolFactory ();

  /**
   * \brief Create pool
   *
   * Inherited from NrSlCommResourcePoolFactory class
   *
   * \return The struct of type LteRrcSap::SlResourcePoolNr defining the SL pool
   */
  const LteRrcSap::SlResourcePoolNr CreatePool (void) override;


private:
  LteRrcSap::SlResourcePoolNr m_pool; //!< Sidelink communication pool
};

}

#endif /* NR_SL_COMM_PRE_CONFIG_RESOURCE_POOL_FACTORY_H */
