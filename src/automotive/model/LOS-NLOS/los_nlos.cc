#include "los_nlos.h"

namespace ns3
{
ChannelCondition::LosConditionValue
LOS_NLOS::GetLosNlos(std::tuple<float, float> xy1,
                      std::tuple<float, float> xy2,
                      std::vector<std::tuple<float, float>> other_vehicles)
{
  float x1 = std::get<0>(xy1);
  float y1 = std::get<1>(xy1);
  float x2 = std::get<0>(xy2);
  float y2 = std::get<1>(xy2);

  float m_v1_v2 = 0, q_v1_v2;
  bool vertical_v1_v2 = (x1 == x2);
  if (!vertical_v1_v2)
    {
      m_v1_v2 = (y1 - y2) / (x1 - x2);
      q_v1_v2 = y1 - m_v1_v2 * x1;
    }

  for (auto& building : m_buildings)
    {
      auto& corners = building.second;
      uint8_t n = corners.size();
      for (uint8_t i = 0; i < n - 1; i++)
        {
          float x_b1 = std::get<0>(corners[i]);
          float y_b1 = std::get<1>(corners[i]);
          float x_b2 = std::get<0>(corners[i + 1]);
          float y_b2 = std::get<1>(corners[i + 1]);

          float m_b1_b2 = 0, q_b1_b2;
          bool vertical_b1_b2 = (x_b1 == x_b2);
          if (!vertical_b1_b2)
            {
              m_b1_b2 = (y_b1 - y_b2) / (x_b1 - x_b2);
              q_b1_b2 = y_b1 - m_b1_b2 * x_b1;
            }

          float inter_x, inter_y;
          if (!vertical_v1_v2 && !vertical_b1_b2)
            {
              if (m_v1_v2 == m_b1_b2)
                continue; // Parallel lines
              inter_x = (q_b1_b2 - q_v1_v2) / (m_v1_v2 - m_b1_b2);
              inter_y = m_v1_v2 * inter_x + q_v1_v2;
            }
          else if (vertical_v1_v2 && !vertical_b1_b2)
            {
              inter_x = x1;
              inter_y = m_b1_b2 * inter_x + q_b1_b2;
            }
          else if (!vertical_v1_v2 && vertical_b1_b2)
            {
              inter_x = x_b1;
              inter_y = m_v1_v2 * inter_x + q_v1_v2;
            }
          else
            {
              // both vertical
              continue;
            }

          // Check if intersection is within both segments
          if (inter_x >= std::min(x1, x2) && inter_x <= std::max(x1, x2) &&
              inter_x >= std::min(x_b1, x_b2) && inter_x <= std::max(x_b1, x_b2) &&
              inter_y >= std::min(y1, y2) && inter_y <= std::max(y1, y2) &&
              inter_y >= std::min(y_b1, y_b2) && inter_y <= std::max(y_b1, y_b2))
            {
              return ChannelCondition::LosConditionValue::NLOS;
            }
        }
    }

  for (auto veh : other_vehicles)
    {
      if (!vertical_v1_v2)
        {
          float x3 = std::get<0>(veh);
          float y3 = std::get<1>(veh);
          float y_test = m_v1_v2 * x3 + q_v1_v2;
          if (y_test - y3 < m_threshold_nlosv)
            {
              return ChannelCondition::LosConditionValue::NLOSv;
            }
        }
    }

  return ChannelCondition::LosConditionValue::LOS;
}

}

