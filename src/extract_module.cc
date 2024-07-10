#include <bayeux/mctools/simulated_data.h>
#include <geomtools/manager.h>
#include <geomtools/geometry_service.h>
#include <falaise/snemo/datamodels/calibrated_data.h>
#include <falaise/snemo/datamodels/geomid_utils.h>
#include <falaise/snemo/services/geometry.h>
#include <falaise/snemo/datamodels/particle_track_data.h>

#include <TVector3.h>

#include "CLHEP/Units/SystemOfUnits.h"

#include "extract_module.h"

DPP_MODULE_REGISTRATION_IMPLEMENT(extract_module, "ExtractModule");

extract_module::extract_module()
{
  tracks_ = {};
  save_file = new TFile("extracted_data.root", "RECREATE");
  tree = new TTree("Event", "Event information");
  tree->Branch("event_number", &event_number_);
  tree->Branch("flag_event", &flag_event_);
  tree->Branch("track_number", &track_number_);
  tree->Branch("tracks", &tracks_);
}

extract_module::~extract_module()
{
  save_file->cd();
  tree->Write();
  save_file->Close();
  delete save_file;
}

void extract_module::initialize (const datatools::properties & module_properties, datatools::service_manager & services, dpp::module_handle_dict_type &)
{
  event_number_ = 0;
  geo_manager_ = snemo::service_handle<snemo::geometry_svc>{services};

  // read calibration source cut parameters from conf file
  double source_cut_ellipse_Y, source_cut_ellipse_Z;
  if(module_properties.has_key("source_cut_ellipse_Y"))
    source_cut_ellipse_Y = module_properties.fetch_real("source_cut_ellipse_Y") / CLHEP::mm;
  else
    source_cut_ellipse_Y = 25.0 * CLHEP::mm;

  if(module_properties.has_key("source_cut_ellipse_Z"))
    source_cut_ellipse_Z = module_properties.fetch_real("source_cut_ellipse_Z") / CLHEP::mm;
  else
    source_cut_ellipse_Z = 30.0 * CLHEP::mm;

  source_cut_ellipse_Y_ = source_cut_ellipse_Y;
  source_cut_ellipse_Z_ = source_cut_ellipse_Z;

  // iterate through all calibration sources and save their Y and Z positions into arrays
  for(int i = 0;i < calib_source_rows_;i++)
  {
    for(int j = 0;j < calib_source_columns_;j++)
    {
      const geomtools::id_mgr & mgr = geo_manager_->get_id_mgr();
      geomtools::geom_id calib_spot_id;
      mgr.make_id("source_calibration_spot", calib_spot_id);
      mgr.set(calib_spot_id, "module", 0);
      mgr.set(calib_spot_id, "track", j);
      mgr.set(calib_spot_id, "position", i);
    
      const geomtools::mapping & mapping = geo_manager_->get_mapping();
      const geomtools::geom_info & calib_spot_ginfo = mapping.get_geom_info(calib_spot_id);
      const geomtools::placement & calib_spot_placement = calib_spot_ginfo.get_world_placement();
      const geomtools::vector_3d & calib_spot_pos  = calib_spot_placement.get_translation();

      calib_source_Y_[i][j] = calib_spot_pos.getY();
      calib_source_Z_[i][j] = calib_spot_pos.getZ();
    }
  }

  this->_set_initialized(true);
}

dpp::chain_module::process_status extract_module::process (datatools::things & event)
{
  // Skip processing if PTD bank is not present
  if (!event.has("PTD"))
  {
    std::cout << "======== no PTD bank in event " << event_number_++ << " ========" << std::endl;
    return dpp::base_module::PROCESS_SUCCESS;
  }

  // Skip processing if SD bank is not present
  if (!event.has("SD"))
  {
    std::cout << "======== no SD bank in event " << event_number_++ << " ========" << std::endl;
    return dpp::base_module::PROCESS_SUCCESS;
  }
  // Retrieve data banks
  const snemo::datamodel::particle_track_data & PTD = event.get<snemo::datamodel::particle_track_data>("PTD");
  const mctools::simulated_data & SD = event.get<mctools::simulated_data>("SD");

  track_number_ = PTD.particles().size(); // number of tracks
  tracks_.clear();
  flag_event_ = false;

  // iterate through all particles and find those coming from a calibration source and hitting an OM
  for (const datatools::handle<snemo::datamodel::particle_track> & particle : PTD.particles())
  {
    track_info track;

    bool vertex_close_to_a_calib_source = false;
    bool vertex_associated_to_a_calo = false;
    double assoc_source_Y, assoc_source_Z;
    datatools::handle<snemo::datamodel::vertex> first_vertex, second_vertex;

    for(const datatools::handle<snemo::datamodel::vertex> & vertex : particle->get_vertices())
    {
      if(vertex->is_on_reference_source_plane())
      {
        double ver_y = vertex->get_spot().get_position().getY();
        double ver_z = vertex->get_spot().get_position().getZ();
        for(int i = 0;i < calib_source_rows_;i++)
        {
          for(int j = 0;j < calib_source_columns_;j++)
          {
            double ellipse_distance = ((ver_y - calib_source_Y_[i][j])*(ver_y - calib_source_Y_[i][j])) / (source_cut_ellipse_Y_*source_cut_ellipse_Y_) +
                            ((ver_z - calib_source_Z_[i][j])*(ver_z - calib_source_Z_[i][j])) / (source_cut_ellipse_Z_*source_cut_ellipse_Z_);

            first_vertex = vertex;
            if(ellipse_distance < 1.0) // vertex is close to a calibration source
            {
                assoc_source_Y = calib_source_Y_[i][j];
                assoc_source_Z = calib_source_Z_[i][j];
                vertex_close_to_a_calib_source = true;
            }
          }
        }
      }

      // there is exactly one vertex associated to a calo hit
      if((vertex->is_on_main_calorimeter() || vertex->is_on_x_calorimeter()) && particle->get_associated_calorimeter_hits().size() == 1)
      {
        if(vertex->is_on_main_calorimeter())
        {
          track.set_calo_type("mwall");
        }
        else
        {
          track.set_calo_type("xcalo");
        }

        second_vertex = vertex;
        vertex_associated_to_a_calo = true;
      }
    }

    if(vertex_close_to_a_calib_source && vertex_associated_to_a_calo) // event passed the cuts
    {
        flag_event_ = true;
    }

    if(first_vertex.has_data())
    {
      const geomtools::vector_3d first_pos = first_vertex->get_spot().get_placement().get_translation();
      track.set_source_vertex(TVector3(first_pos.getX(), first_pos.getY(), first_pos.getZ()));

      if(vertex_close_to_a_calib_source)
      {
        // calculate distance between the calibration source and the second vertex
        track.set_source_dist(TVector3(0.0, first_pos.getY() - assoc_source_Y, first_pos.getZ() - assoc_source_Z));
      }
    }

    if(second_vertex.has_data())
    {
      const geomtools::vector_3d second_pos = second_vertex->get_spot().get_placement().get_translation();
      track.set_calo_vertex(TVector3(second_pos.getX(), second_pos.getY(), second_pos.getZ()));
    }

    if(SD.has_step_hits("calo") && second_vertex.has_data())
    {
      for(const datatools::handle<mctools::base_step_hit> & step_hit : SD.get_step_hits("calo"))
      {
        if(step_hit->get_geom_id() == second_vertex->get_geom_id())
        {
          double simulated_vertex_X = (step_hit->get_position_start().x() + step_hit->get_position_stop().x()) / 2.0;
          double simulated_vertex_Y = (step_hit->get_position_start().y() + step_hit->get_position_stop().y()) / 2.0;
          double simulated_vertex_Z = (step_hit->get_position_start().z() + step_hit->get_position_stop().z()) / 2.0;
          track.set_calo_dist(TVector3(track.get_calo_vertex().X() - simulated_vertex_X, track.get_calo_vertex().Y() - simulated_vertex_Y, track.get_calo_vertex().Z() - simulated_vertex_Z));
          break;
        }
      }
    }
    tracks_.push_back(track);
  }
  tree->Fill();
  event_number_++;

  return dpp::base_module::PROCESS_SUCCESS;
}