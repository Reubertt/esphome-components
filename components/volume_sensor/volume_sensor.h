#pragma once
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace volume_sensor {

class VolumeSensor : public sensor::Sensor, public PollingComponent {
public:
  void set_pin(uint8_t pin) { this->pin_ = pin; }
  void set_sampling_duration(uint32_t duration) { this->sampling_duration_ = duration; }
  void set_publish_interval(uint32_t interval) { this->publish_interval_ = interval; }
  void set_sensitivity(float sensitivity) { this->sensitivity_ = sensitivity; }
  void set_adc_min_calibration(uint16_t val) { this->adc_min_cal_ = val; }
  void set_adc_max_calibration(uint16_t val) { this->adc_max_cal_ = val; }
  void set_db_sensor(sensor::Sensor *sensor) { this->db_sensor_ = sensor; }
  void set_raw_max_sensor(sensor::Sensor *sensor) { this->raw_max_sensor_ = sensor; }
  void set_raw_min_sensor(sensor::Sensor *sensor) { this->raw_min_sensor_ = sensor; }
  void set_percentage_sensor(sensor::Sensor *sensor) { this->percentage_sensor_ = sensor; }

  void update() override;
  void setup() override;
  void dump_config() override;

protected:
  float map_value_float(float x, float in_min, float in_max, float out_min,
                        float out_max);

  uint8_t pin_;
  uint32_t sampling_duration_;
  uint32_t publish_interval_;
  float sensitivity_;
  int16_t adc_min_cal_{-1};
  int16_t adc_max_cal_{-1};

  sensor::Sensor *raw_max_sensor_{nullptr};
  sensor::Sensor *raw_min_sensor_{nullptr};
  sensor::Sensor *percentage_sensor_{nullptr};
  sensor::Sensor *db_sensor_{nullptr};

  uint32_t last_publish_time_{0};
  uint16_t max_amplitude_in_window_{0};

  uint16_t peak_signal_max_in_window_{0};
  uint16_t peak_signal_min_in_window_{1023};
};

} // namespace volume_sensor
} // namespace esphome
