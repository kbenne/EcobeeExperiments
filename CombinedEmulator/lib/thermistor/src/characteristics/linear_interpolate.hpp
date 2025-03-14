#ifndef _THERMISTOR_CHARACTERISTICS_LINEAR_INTERPOLATE_HPP_
#define _THERMISTOR_CHARACTERISTICS_LINEAR_INTERPOLATE_HPP_

#include "../characteristic_curve.hpp"

#include <vector>
#include <utility>
#include <initializer_list>

namespace thermistor {
  class LinearInterpolate : CharacteristicCurve {

    const std::vector<std::pair<float, float>> m_lookup_table;

    template <size_t x_index = 0, size_t y_index = 1>
    constexpr inline float get_slope(std::pair<float,float> a, std::pair<float,float> b) const {
      return (std::get<y_index>(a) - std::get<y_index>(b))/(std::get<x_index>(a) - std::get<x_index>(b));
    }

    template <size_t x_index = 0, size_t y_index = 1>
    constexpr inline float point_slope(std::pair<float,float> point, float slope, float x_1) const {
      float x = std::get<x_index>(point);
      float y = std::get<y_index>(point);
      float dx = x_1 - x;
      float dy = dx * slope;
      return y + dy;
    }

    template <size_t x_index = 0, size_t y_index = 1>
    auto lookup(float domain) const {
      if (m_lookup_table.size() == 1) {
        return std::get<y_index>(m_lookup_table[0]);
      }

      auto table_it = m_lookup_table.begin();
      auto last_point = *(table_it++);
      auto current_point = last_point;
    
      for(; table_it != m_lookup_table.end(); table_it++) {
        last_point = current_point;
        current_point = *table_it;
        if (get<x_index>(current_point) > value) {
          break;
        }
      }
    
      return point_slope<x_index, y_index>(current_point, get_slope<x_index, y_index>(current_point, last_point), domain);
    }

  public:
    template<class InputIt>
    LinearInterpolate(const InputIt &begin, const InputIt &end) {
      auto size = std::distance(begin, end);
      m_lookup_table.reserve(size);
      std::copy(begin, end, m_lookup_table.begin());
    }

    template <typename T>
    LinearInterpolate(const T &lookup_table) : LinearInterpolate(lookup_table.begin(), lookup_table.end()) {}

    LinearInterpolate(const std::initializer_list<std::pair<float, float>> lookup_table) : LinearInterpolate(lookup_table.begin(), lookup_table.end()) {}

    uint32_t temperature_to_resistance(float temperature) const override;

    float resistance_to_temperature(uint32_t resistance) const override;
  };
}

#endif