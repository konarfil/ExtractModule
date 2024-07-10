#ifndef TRACK_INFO_H
#define TRACK_INFO_H

#include <TVector3.h>
#include <vector>
#include <TObject.h>

class track_info : public TObject
{
  public:
    track_info();
    ~track_info();

    TVector3 get_source_vertex() const;
    TVector3 get_calo_vertex() const;
    TVector3 get_calo_dist() const;
    TVector3 get_source_dist() const;
    std::string get_calo_type() const;

    void set_source_vertex(const TVector3& vertex);
    void set_calo_vertex(const TVector3& vertex);
    void set_calo_dist(const TVector3& dist);
    void set_source_dist(const TVector3& dist);
    void set_calo_type(const std::string& type);

  private:
    TVector3 source_vertex;
    TVector3 calo_vertex;
    TVector3 calo_dist;
    TVector3 source_dist;
    std::string calo_type;
  ClassDef(track_info, 1);
};
#endif