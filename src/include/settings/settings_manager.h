#pragma once

#include <unordered_map>
#include "type/value.h"
#include "common/exception.h"
#include "settings/settings_param.h"

namespace terrier::settings {

/*
 * SettingsManager:
 * A wrapper for pg_settings table, does not store values in it.
 * Stores and triggers callbacks when a tunable parameter is changed.
 * Static class.
*/

class SettingsManager {
 public:
  static int32_t GetInt(Param param);
  static double GetDouble(Param param);
  static bool GetBool(Param param);
  static std::string GetString(Param param);

  static void SetInt(Param param, int32_t value);
  static void SetBool(Param param, bool value);
  static void SetString(Param param, const std::string &value);

  // Call this method in Catalog->Bootstrap
  // to store information into pg_settings
  void InitializeCatalog();

  const std::string GetInfo() const;

  void ShowInfo();

 private:

  // local information storage
  // name, value, description, default_value, is_mutable, is_persistent

  bool catalog_initialized_;

  void DefineSetting(Param param, const std::string &name,
                     const type::Value &value,
                     const std::string &description,
                     const type::Value &default_value,
                     const type::Value &min_value,
                     const type::Value &max_value,
                     bool is_mutable, bool is_persistent);

  static type::Value GetValue(Param param);
  static void SetValue(Param param, const type::Value &value);

};

}  // namespace terrier::settings
