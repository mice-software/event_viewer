#ifndef ReadMAUS_H
#define ReadMAUS_H

#include <iostream>
#include "string.h"
#include <TFile.h>
#include <TTree.h>
#include <QString>
#include "TMath.h"
#include <QVector>


#include <DataStructure/Spill.hh>
#include <DataStructure/TOFEvent.hh>
#include <DataStructure/Data.hh>
#include <DataStructure/ReconEvent.hh>
#include <DataStructure/SciFiEvent.hh>
#include <DataStructure/SciFiTrack.hh>
#include <DataStructure/SciFiTrackPoint.hh>
#include <DataStructure/SciFiSpacePoint.hh>
#include <DataStructure/SciFiStraightPRTrack.hh>
#include <DataStructure/SciFiCluster.hh>
#include <DataStructure/SciFiDigit.hh>
#include <DataStructure/SciFiChannelId.hh>
#include <DataStructure/Hit.hh>
#include <DataStructure/Track.hh>
#include <DataStructure/ThreeVector.hh>

#include <QHash>



class ReadMAUS
{
public:
    ReadMAUS();
    ~ReadMAUS();

    QHash<int, QHash<int, QVector<QVector<double> > > > Read(QString fileToOpen);
    void SetDetectorPositions(QVector<double> tof0_location, QVector<double> tof1_location,
                              QVector<double> tku_location, QVector<double> tkd_location,
                              QVector<double> tof2_location);
    void SetSpillRange(int spill_range);
    void SetStartingSpill(int start_spill);

private:
    MAUS::Spill *spill;
    MAUS::TOFEvent *tof_event;
    MAUS::SciFiEvent *scifi_event;

    int reconstructed_event_number, spillNumber;
    int spillRange, spillBegin, spillEnd;

    double TOF0_xPixel, TOF0_yPixel, TOF0_x, TOF0_y; // (x, y) positions of hits
    double TOF0_hSlab_t0, TOF0_hSlab_t1, TOF0_vSlab_t0, TOF0_vSlab_t1; // PMT time at TOF0
    double TOF0_hSlab_raw_t0, TOF0_hSlab_raw_t1, TOF0_vSlab_raw_t0, TOF0_vSlab_raw_t1; // raw (uncalibrated?) PMT times
    int TOF0_hSlab, TOF0_vSlab;
    double TOF0_hitTime; // time TOF0 was hit, for time-of-flight calculations
    double TOF0_z, TOF0_xOffset, TOF0_yOffset;

    double TOF1_xPixel, TOF1_yPixel, TOF1_x, TOF1_y;
    double TOF1_hSlab_t0, TOF1_hSlab_t1, TOF1_vSlab_t0, TOF1_vSlab_t1; // PMT time at TOF1
    double TOF1_hSlab_raw_t0, TOF1_hSlab_raw_t1, TOF1_vSlab_raw_t0, TOF1_vSlab_raw_t1; // raw (uncalibrated?) PMT times
    int TOF1_hSlab, TOF1_vSlab; // slabs hit in TOF0 and TOF1
    double TOF1_hitTime;
    double TOF1_z, TOF1_xOffset, TOF1_yOffset;

    double TOF2_xPixel, TOF2_yPixel, TOF2_x, TOF2_y;
    double TOF2_hSlab_t0, TOF2_hSlab_t1, TOF2_vSlab_t0, TOF2_vSlab_t1; // PMT time at TOF1
    double TOF2_hSlab_raw_t0, TOF2_hSlab_raw_t1, TOF2_vSlab_raw_t0, TOF2_vSlab_raw_t1; // raw (uncalibrated?) PMT times
    int TOF2_hSlab, TOF2_vSlab; // slabs hit in TOF0 and TOF1
    double TOF2_hitTime;
    double TOF2_z, TOF2_xOffset, TOF2_yOffset;

    double TKU_plane1_x, TKU_plane1_y, TKU_plane1_z;
    double TKU_plane2_x, TKU_plane2_y, TKU_plane2_z;
    double TKU_plane3_x, TKU_plane3_y, TKU_plane3_z;
    double TKU_plane4_x, TKU_plane4_y, TKU_plane4_z;
    double TKU_plane5_x, TKU_plane5_y, TKU_plane5_z;

    double TKU_plane1_px, TKU_plane1_py, TKU_plane1_pz;
    double TKU_plane2_px, TKU_plane2_py, TKU_plane2_pz;
    double TKU_plane3_px, TKU_plane3_py, TKU_plane3_pz;
    double TKU_plane4_px, TKU_plane4_py, TKU_plane4_pz;
    double TKU_plane5_px, TKU_plane5_py, TKU_plane5_pz;
    double TKU_xOffset, TKU_yOffset, TKU_zOffset;

    double TKD_plane1_x, TKD_plane1_y, TKD_plane1_z;
    double TKD_plane2_x, TKD_plane2_y, TKD_plane2_z;
    double TKD_plane3_x, TKD_plane3_y, TKD_plane3_z;
    double TKD_plane4_x, TKD_plane4_y, TKD_plane4_z;
    double TKD_plane5_x, TKD_plane5_y, TKD_plane5_z;

    double TKD_plane1_px, TKD_plane1_py, TKD_plane1_pz;
    double TKD_plane2_px, TKD_plane2_py, TKD_plane2_pz;
    double TKD_plane3_px, TKD_plane3_py, TKD_plane3_pz;
    double TKD_plane4_px, TKD_plane4_py, TKD_plane4_pz;
    double TKD_plane5_px, TKD_plane5_py, TKD_plane5_pz;
    double TKD_xOffset, TKD_yOffset, TKD_zOffset;


    void readParticleEvent();
    void reset_particle_variables();
    void add_to_events();
    void add_to_spills();

    void particle_at_TOF0();
    void slabHits_at_TOF0();
    void spacePoints_at_TOF0();

    void particle_at_TOF1();
    void slabHits_at_TOF1();
    void spacePoints_at_TOF1();

    void particle_at_TOF2();
    void slabHits_at_TOF2();
    void spacePoints_at_TOF2();

    void particle_at_tracker();

    void get_TOF0_pixel_xy();
    void get_TOF1_pixel_xy();
    void get_TOF2_pixel_xy();

    void set_2011_TOF0_TOF1_Rayner_calibration();
    void initialise_detector_positions();
    QVector<double> TOF0_horizontal_slab_calibrations;
    QVector<double> TOF1_horizontal_slab_calibrations;
    QVector<double> TOF2_horizontal_slab_calibrations;
    QVector<double> TOF0_vertical_slab_calibrations;
    QVector<double> TOF1_vertical_slab_calibrations;
    QVector<double> TOF2_vertical_slab_calibrations;
    double calibrated_c_eff;

    QVector<double> particle_x;
    QVector<double> particle_y;
    QVector<double> particle_z;
    QVector<double> particle_t;
    QVector<double> particle_px;
    QVector<double> particle_py;
    QVector<double> particle_pz;
    QVector<double> particle_E;

    QVector<QVector<double> > particle_info;
    QHash<int, QVector<QVector<double> > > particles_in_event;

    QHash<int, QHash<int, QVector<QVector<double> > > > particles_in_spill;


};

#endif // ReadMAUS_H
