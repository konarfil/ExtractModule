#ifndef EXTRACT_MODULE_H
#define EXTRACT_MODULE_H

#include <bayeux/dpp/chain_module.h>
#include <falaise/snemo/services/service_handle.h>

#include <TTree.h>
#include <TFile.h>

#include "track_info.h"

class extract_module : public dpp::chain_module
{

public:
  // Constructor
  extract_module();

  // Destructor
  virtual ~extract_module();

  // Initialisation function
  virtual void initialize (const datatools::properties &,
                           datatools::service_manager &,
			   dpp::module_handle_dict_type &);

  // Event processing function
  dpp::chain_module::process_status process (datatools::things & event);
  
private:
  TTree* tree; // a tree to save all the extracted values into
  TFile* save_file;

  int  event_number_; // number of event
  bool flag_event_; // 0 = did not pass cuts, 1 = passed
  int track_number_; // number of reconstructed tracks
  std::vector<track_info> tracks_;

  snemo::service_handle<snemo::geometry_svc> geo_manager_{};

  // number of calibration sources
  static const int calib_source_rows_ = 7;
  static const int calib_source_columns_ = 6;

  // dimensions of the ellipse for calibration source vertex cut
  double source_cut_ellipse_Y_;
  double source_cut_ellipse_Z_;

  // Y and Z coorinates of centres of calibration sources
  double calib_source_Y_[calib_source_rows_][calib_source_columns_];
  double calib_source_Z_[calib_source_rows_][calib_source_columns_];

  // Macro to register the module
  DPP_MODULE_REGISTRATION_INTERFACE(extract_module);

};
#endif