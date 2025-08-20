#include "volume_sensor.h"
#include "esphome/core/log.h"
#include <algorithm>
#include <cmath>

namespace esphome {
namespace volume_sensor {

static const char *const TAG = "volume_sensor.sensor";

float VolumeSensor::map_value_float(float x, float in_min, float in_max,
                                    float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void VolumeSensor::setup() {
  #if defined(ESP32)
    // Set attenuation to 0db to be as close to esp8266 as possible
    analogSetPinAttenuation(this->pin_, ADC_0db);
    // Set ADC resolution to 10 bits (0-1023) on ESP32
    analogReadResolution(10);
  #endif
}

void VolumeSensor::dump_config() {
  LOG_SENSOR("", "Volume Sensor", this);
  ESP_LOGCONFIG(TAG, "  Pin: %u", this->pin_);
  ESP_LOGCONFIG(TAG, "  Sampling Duration: %ums", this->sampling_duration_);
  ESP_LOGCONFIG(TAG, "  Publish Interval: %ums", this->publish_interval_);
  ESP_LOGCONFIG(TAG, "  Sensitivity: %.2f", this->sensitivity_);
  if (this->adc_min_cal_ != -1) {
    ESP_LOGCONFIG(
        TAG, "  Mode: Calibrated Range (ADC: %d-%d), adjusted by Sensitivity",
        this->adc_min_cal_, this->adc_max_cal_);
  } else {
    ESP_LOGCONFIG(TAG,
                  "  Mode: Dynamic Range (0-1023), adjusted by Sensitivity");
  }
  LOG_SENSOR("  ", "Raw Max Sensor", this->raw_max_sensor_);
  LOG_SENSOR("  ", "Raw Min Sensor", this->raw_min_sensor_);
  LOG_SENSOR("  ", "Percentage Sensor", this->percentage_sensor_);
  LOG_SENSOR("  ", "DB Sensor", this->db_sensor_);
}

void VolumeSensor::update() {
  uint16_t signal_max = 0;
  uint16_t signal_min = 1023;
  unsigned long start_time = millis();
  while (millis() - start_time < this->sampling_duration_) {
    uint16_t sample = analogRead(this->pin_);
    signal_max = std::max(signal_max, sample);
    signal_min = std::min(signal_min, sample);
  }
  uint16_t current_amplitude = signal_max - signal_min;

  if (current_amplitude > this->max_amplitude_in_window_) {
    this->max_amplitude_in_window_ = current_amplitude;
    this->peak_signal_max_in_window_ = signal_max;
    this->peak_signal_min_in_window_ = signal_min;
  }

  unsigned long now = millis();
  if (now - this->last_publish_time_ < this->publish_interval_) {
    return;
  }

  this->last_publish_time_ = now;
  uint16_t peak_amplitude = this->max_amplitude_in_window_;

  if (this->raw_max_sensor_ != nullptr) {
    this->raw_max_sensor_->publish_state(this->peak_signal_max_in_window_);
  }
  if (this->raw_min_sensor_ != nullptr) {
    this->raw_min_sensor_->publish_state(this->peak_signal_min_in_window_);
  }

  float base_min = 0.0f;
  float base_max = 1023.0f;
  if (this->adc_min_cal_ != -1 && this->adc_max_cal_ != -1) {
    base_min = this->adc_min_cal_;
    base_max = this->adc_max_cal_;
  }
  const float MIN_SENSITIVITY = 10.0f;
  const float MAX_SENSITIVITY = 100.0f;
  const float MIN_RANGE_SPAN = 1.0f;
  float final_max = this->map_value_float(this->sensitivity_, MIN_SENSITIVITY,
                                          MAX_SENSITIVITY, base_max,
                                          base_min + MIN_RANGE_SPAN);
  final_max = std::max(base_min + MIN_RANGE_SPAN, final_max);
  float final_range = final_max - base_min;
  if (final_range <= 0)
    final_range = 1.0f;
  int adjusted_amplitude = peak_amplitude;

  if (this->percentage_sensor_ != nullptr) {
    float percentage =
        this->map_value_float(adjusted_amplitude, 0, final_range, 0.0, 100.0);
    percentage = std::max(0.0f, std::min(100.0f, percentage));
    this->percentage_sensor_->publish_state(percentage);
  }
  if (this->db_sensor_ != nullptr || !this->is_internal()) {
    float mapped_amplitude =
        this->map_value_float(adjusted_amplitude, 0, final_range, 1.5, 1023.0);
    if (mapped_amplitude <= 0)
      mapped_amplitude = 1.0;
    float db_value = 36.5f * log10f(mapped_amplitude);
    if (this->db_sensor_ != nullptr) {
      this->db_sensor_->publish_state(db_value);
    } else {
      this->publish_state(db_value);
    }
  }

  this->max_amplitude_in_window_ = 0;
  this->peak_signal_max_in_window_ = 0;
  this->peak_signal_min_in_window_ = 1023;
}

} // namespace volume_sensor
} // namespace esphome
