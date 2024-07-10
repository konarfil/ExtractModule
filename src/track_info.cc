#include "track_info.h"

ClassImp(track_info);

track_info::track_info()
{
    source_vertex = TVector3(std::nan(""), std::nan(""), std::nan(""));
    calo_vertex = TVector3(std::nan(""), std::nan(""), std::nan(""));
    calo_dist = TVector3(std::nan(""), std::nan(""), std::nan(""));
    source_dist = TVector3(std::nan(""), std::nan(""), std::nan(""));
    calo_type = "";
}

track_info::~track_info()
{

}

TVector3 track_info::get_source_vertex() const {
    return source_vertex;
}

TVector3 track_info::get_calo_vertex() const {
    return calo_vertex;
}

TVector3 track_info::get_calo_dist() const {
    return calo_dist;
}

TVector3 track_info::get_source_dist() const {
    return source_dist;
}

std::string track_info::get_calo_type() const {
    return calo_type;
}

void track_info::set_source_vertex(const TVector3& vertex) {
    source_vertex = vertex;
}

void track_info::set_calo_vertex(const TVector3& vertex) {
    calo_vertex = vertex;
}

void track_info::set_calo_dist(const TVector3& dist) {
    calo_dist = dist;
}

void track_info::set_source_dist(const TVector3& dist) {
    source_dist = dist;
}

void track_info::set_calo_type(const std::string& type) {
    calo_type = type;
}