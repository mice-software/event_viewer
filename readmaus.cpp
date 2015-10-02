#include "readmaus.h"

#include "JsonCppStreamer/IRStream.hh"


ReadMAUS::ReadMAUS()
{
    // initialise all detectors as NOT being read out. We'll turn these on
    // when we set up the object prior to reading a file

    set_2011_TOF0_TOF1_Rayner_calibration(); // this does nothing atm
    initialise_detector_positions();
    spillRange = 2000;
    spillBegin = 0;
    spillEnd = spillBegin + spillRange;
}

ReadMAUS::~ReadMAUS(){

}

void ReadMAUS::initialise_detector_positions(){
    // some initialisation to make sure that we'll always have SOME value for where
    // detectors are.
    QVector<double> tof0_location;
    tof0_location << 0.0 << 0.0 << 5285.66;

    QVector<double> tof1_location;
    tof1_location << 0.0 << 0.0 << 12922.00;

    QVector<double> tof2_location;
    tof2_location << 0.0 << 0.0 << 21127.27;

    QVector<double> no_offset;
    no_offset << 0.0 << 0.0 << 0.0;

    SetDetectorPositions(tof0_location, tof1_location, no_offset, no_offset, tof2_location);
}

void ReadMAUS::SetDetectorPositions(QVector<double> tof0_location, QVector<double> tof1_location,
                                    QVector<double> tku_location, QVector<double> tkd_location,
                                    QVector<double> tof2_location){
    /*
     * Set locations of TOF0--2 and offsets in (x, y, z) for all (inc. trackers) detectors.
     *
     * These values are defined in the 'Settings Window', accessible from the main gui window.
     */

    TOF0_xOffset = tof0_location.at(0);
    TOF0_yOffset = tof0_location.at(1);
    TOF0_z = tof0_location.at(2);

    TOF1_xOffset = tof1_location.at(0);
    TOF1_yOffset = tof1_location.at(1);
    TOF1_z = tof1_location.at(2);

    TOF2_xOffset = tof2_location.at(0);
    TOF2_yOffset = tof2_location.at(1);
    TOF2_z = tof2_location.at(2);

    TKU_xOffset = tku_location.at(0);
    TKU_yOffset = tku_location.at(1);
    TKU_zOffset = tku_location.at(2);

    TKD_xOffset = tkd_location.at(0);
    TKD_yOffset = tkd_location.at(1);
    TKD_zOffset = tkd_location.at(2);
}

void ReadMAUS::SetSpillRange(int spill_range){
    spillRange = spill_range;
    spillEnd = spillBegin + spillRange;
}

void ReadMAUS::SetStartingSpill(int start_spill){
    spillBegin = start_spill;
    spillEnd = spillBegin + spillRange;
}

QHash<int, QHash<int, QVector<QVector<double> > > > ReadMAUS::Read(QString fileToOpen){
    particle_info.clear();
    particles_in_event.clear();
    particles_in_spill.clear();

    particles_in_spill.clear();
    particles_in_event.clear();

    // start reading data:
    MAUS::Data data;
    irstream infile(fileToOpen.toStdString().c_str(), "Spill");

    // iterate over events:

    while(infile >> readEvent != NULL){
        infile >> branchName("data") >> data;
        spill = data.GetSpill();

        if(spill != NULL && spill->GetDaqEventType() == "physics_event"){
            /*
             * We've found a spill that contains some data. Next we iterate over
             * all of the different event types and put the interesting information
             * into our ROOT file
             */
            spillNumber = spill->GetSpillNumber();
            if(spillNumber >= spillBegin && spillNumber < spillEnd){
                readParticleEvent();
            }

        }
        if(spillNumber >= spillBegin && spillNumber < spillEnd){
            add_to_spills();
        }
        if(spillNumber > spillEnd){
            break;
        }

    }

    return particles_in_spill;
}

void ReadMAUS::readParticleEvent(){
    for(size_t i = 0; i < spill->GetReconEvents()->size(); ++i){
        /*
         * For now we're only going to look at TOF events. Other events will
         * need adding here.
         */
        reset_particle_variables();
        reconstructed_event_number = i;

        tof_event = (*spill->GetReconEvents())[i]->GetTOFEvent();
        scifi_event = (*spill->GetReconEvents())[i]->GetSciFiEvent();

        if(tof_event != NULL){
            // there are hits at TOFs, we should try and do something with them
            particle_at_TOF0();
            particle_at_TOF1();
        }

        if(scifi_event != NULL){
            particle_at_tracker(); // this function needs renaming
        }

        if(tof_event != NULL){
            particle_at_TOF2();
        }

        add_to_events();
    }
}

void ReadMAUS::add_to_events(){
    particle_info.clear();
    particle_x.clear();
    particle_y.clear();
    particle_z.clear();

    particle_px.clear();
    particle_py.clear();
    particle_pz.clear();
    particle_t.clear();

    // add offsets as defined in Settings Window
    particle_x << TOF0_x + TOF0_xOffset
               << TOF1_x + TOF1_xOffset
               << TKU_plane1_x + TKU_xOffset
               << TKU_plane2_x + TKU_xOffset
               << TKU_plane3_x + TKU_xOffset
               << TKU_plane4_x + TKU_xOffset
               << TKU_plane5_x + TKU_xOffset
               << TKD_plane1_x + TKD_xOffset
               << TKD_plane2_x + TKD_xOffset
               << TKD_plane3_x  + TKD_xOffset
               << TKD_plane4_x  + TKD_xOffset
               << TKD_plane5_x  + TKD_xOffset
               << TOF2_x + TOF2_xOffset;

    particle_y << TOF0_y + TOF0_yOffset
               << TOF1_y + TOF1_yOffset
               << TKU_plane1_y + TKU_yOffset
               << TKU_plane2_y + TKU_yOffset
               << TKU_plane3_y + TKU_yOffset
               << TKU_plane4_y + TKU_yOffset
               << TKU_plane5_y + TKU_yOffset
               << TKD_plane1_y + TKD_yOffset
               << TKD_plane2_y + TKD_yOffset
               << TKD_plane3_y  + TKD_yOffset
               << TKD_plane4_y  + TKD_yOffset
               << TKD_plane5_y  + TKD_yOffset
               << TOF2_y + TOF2_yOffset;

    if(TOF0_x != TMath::Infinity()){
        particle_z << TOF0_z;
    }
    else{
        particle_z << TMath::Infinity();
    }

    if(TOF1_x != TMath::Infinity()){
        particle_z << TOF1_z;
    }
    else{
        particle_z << TMath::Infinity();
    }

    if(TKU_plane1_x != TMath::Infinity()){
        particle_z << TKU_plane1_z + TKU_zOffset
                   << TKU_plane2_z + TKU_zOffset
                   << TKU_plane3_z + TKU_zOffset
                   << TKU_plane4_z + TKU_zOffset
                   << TKU_plane5_z + TKU_zOffset ;
    }
    else{
        particle_z << TMath::Infinity() << TMath::Infinity() << TMath::Infinity()
                   << TMath::Infinity() << TMath::Infinity();
    }

    if(TKD_plane1_x != TMath::Infinity()){
        particle_z << TKD_plane1_z + TKD_zOffset
                   << TKD_plane2_z + TKD_zOffset
                   << TKD_plane3_z + TKD_zOffset
                   << TKD_plane4_z + TKD_zOffset
                   << TKD_plane5_z + TKD_zOffset ;
    }
    else{
        particle_z << TMath::Infinity() << TMath::Infinity() << TMath::Infinity()
                   << TMath::Infinity() << TMath::Infinity();
    }

    if(TOF2_x != TMath::Infinity()){
        particle_z << TOF2_z;
    }
    else{
        particle_z << TMath::Infinity();
    }




    /*
     * Add momentum to just tracker slots, and add time to just tof slots
     */
    particle_px << TMath::Infinity() << TMath::Infinity() // TOF0 & TOF1
                << TKU_plane1_px << TKU_plane2_px << TKU_plane3_px
                << TKU_plane4_px << TKU_plane5_px << TKD_plane1_px
                << TKD_plane2_px << TKD_plane3_px << TKD_plane4_px
                << TKD_plane5_px << TMath::Infinity();

    particle_py << TMath::Infinity() << TMath::Infinity() // TOF0 & TOF1
                << TKU_plane1_py << TKU_plane2_py << TKU_plane3_py
                << TKU_plane4_py << TKU_plane5_py << TKD_plane1_py
                << TKD_plane2_py << TKD_plane3_py << TKD_plane4_py
                << TKD_plane5_py << TMath::Infinity();

    particle_pz << TMath::Infinity() << TMath::Infinity() // TOF0 & TOF1
                << TKU_plane1_pz << TKU_plane2_pz << TKU_plane3_pz
                << TKU_plane4_pz << TKU_plane5_pz << TKD_plane1_pz
                << TKD_plane2_pz << TKD_plane3_pz << TKD_plane4_pz
                << TKD_plane5_pz << TMath::Infinity();

    particle_t << TOF0_hitTime << TOF1_hitTime // TOF0 & TOF1
                << TMath::Infinity() << TMath::Infinity() << TMath::Infinity()
                << TMath::Infinity() << TMath::Infinity() << TMath::Infinity()
                << TMath::Infinity() << TMath::Infinity() << TMath::Infinity()
                << TMath::Infinity() << TOF2_hitTime;

    particle_info << particle_x << particle_y << particle_z << particle_t << particle_px
                  << particle_py << particle_pz;

    particles_in_event.insert(reconstructed_event_number, particle_info);
}

void ReadMAUS::add_to_spills(){
    particles_in_spill.insert(spillNumber, particles_in_event);
}




void ReadMAUS::reset_particle_variables(){
    /*
     * We want to avoid 'double writing' a particle to our ROOT file if, for example, we have a new
     * vertical slab hit but have old information about a horizontal slab hit when keeping only
     * slab hit data
     *
     * By setting double variables to TMath::Infinity(), we have an easy way of identifying them later
     * in further analysis code.  Ints get set to -1.
     */

    TOF0_xPixel = TMath::Infinity();
    TOF0_yPixel = TMath::Infinity();
    TOF0_x = TMath::Infinity();
    TOF0_y = TMath::Infinity();
    TOF0_hSlab_raw_t0 = TMath::Infinity();
    TOF0_hSlab_raw_t1 = TMath::Infinity();
    TOF0_hSlab_t0 = TMath::Infinity();
    TOF0_hSlab_t1 = TMath::Infinity();
    TOF0_hSlab = -1;
    TOF0_vSlab = -1;

    TOF1_xPixel = TMath::Infinity();
    TOF1_yPixel = TMath::Infinity();
    TOF1_x = TMath::Infinity();
    TOF1_y = TMath::Infinity();
    TOF1_hSlab_raw_t0 = TMath::Infinity();
    TOF1_hSlab_raw_t1 = TMath::Infinity();
    TOF1_hSlab_t0 = TMath::Infinity();
    TOF1_hSlab_t1 = TMath::Infinity();
    TOF1_hSlab = -1;
    TOF1_vSlab = -1;

    TOF2_xPixel = TMath::Infinity();
    TOF2_yPixel = TMath::Infinity();
    TOF2_x = TMath::Infinity();
    TOF2_y = TMath::Infinity();
    TOF2_hSlab_raw_t0 = TMath::Infinity();
    TOF2_hSlab_raw_t1 = TMath::Infinity();
    TOF2_hSlab_t0 = TMath::Infinity();
    TOF2_hSlab_t1 = TMath::Infinity();
    TOF2_hSlab = -1;
    TOF2_vSlab = -1;

    TKU_plane1_x = TMath::Infinity();
    TKU_plane1_y = TMath::Infinity();
    TKU_plane1_z = TMath::Infinity();

    TKU_plane2_x = TMath::Infinity();
    TKU_plane2_y = TMath::Infinity();
    TKU_plane2_z = TMath::Infinity();

    TKU_plane3_x = TMath::Infinity();
    TKU_plane3_y = TMath::Infinity();
    TKU_plane3_z = TMath::Infinity();

    TKU_plane4_x = TMath::Infinity();
    TKU_plane4_y = TMath::Infinity();
    TKU_plane4_z = TMath::Infinity();

    TKU_plane5_x = TMath::Infinity();
    TKU_plane5_y = TMath::Infinity();
    TKU_plane5_z = TMath::Infinity();

    TKD_plane1_x = TMath::Infinity();
    TKD_plane1_y = TMath::Infinity();
    TKD_plane1_z = TMath::Infinity();

    TKD_plane2_x = TMath::Infinity();
    TKD_plane2_y = TMath::Infinity();
    TKD_plane2_z = TMath::Infinity();

    TKD_plane3_x = TMath::Infinity();
    TKD_plane3_y = TMath::Infinity();
    TKD_plane3_z = TMath::Infinity();

    TKD_plane4_x = TMath::Infinity();
    TKD_plane4_y = TMath::Infinity();
    TKD_plane4_z = TMath::Infinity();

    TKD_plane5_x = TMath::Infinity();
    TKD_plane5_y = TMath::Infinity();
    TKD_plane5_z = TMath::Infinity();
}

void ReadMAUS::particle_at_TOF0(){
    spacePoints_at_TOF0();
}

void ReadMAUS::spacePoints_at_TOF0(){
    // need two slab hits at TOF0 (one in each plane) for
    // this particle to have passed through TOF0

    MAUS::TOFEventSlabHit slab_hits = tof_event->GetTOFEventSlabHit();
    MAUS::TOFEventSpacePoint space_points = tof_event->GetTOFEventSpacePoint();

    MAUS::TOFSlabHit tof0_slab_hits;
    MAUS::TOFSpacePoint tof0_space_points;

    /*
     * 1. Loop over TOF space points
     *    a. Get horizontal and vertical slab numbers
     *    b. Get spill and particle event numbers
     *
     * 2. Loop over TOF slab hits
     *    a. If horizontal and vertical slabs == slabs from space points, get PMT times
     *    b. Check also that spill and particle event numbers are the same
     *    c. Then: get TOF pixel by slab hits and pmt timing
     */

    int horizontalHit = -1;
    int verticalHit = -1;
    int spillNo = -1;
    int particleNo = -1;

    // 1. Loop over TOF0 space points:
    for(size_t i = 0; i < space_points.GetTOF0SpacePointArray().size(); ++i){

        tof0_space_points = space_points.GetTOF0SpacePointArray()[i];
        spillNo = tof0_space_points.GetPhysEventNumber();
        particleNo = tof0_space_points.GetPartEventNumber();
        horizontalHit = tof0_space_points.GetSlabx(); // returns slabs oriented along the x-axis
        verticalHit = tof0_space_points.GetSlaby();   // returns slabs oriented along the y-axis

        // 2. Loop over slab hits and look for matches:
        for(size_t j = 0; j < slab_hits.GetTOF0SlabHitArray().size(); ++j){
            tof0_slab_hits = slab_hits.GetTOF0SlabHitArray()[j];
            if(tof0_slab_hits.GetPlane() == 0){
                // horizontal slab hit
                TOF0_hSlab = tof0_slab_hits.GetSlab();
                TOF0_hSlab_raw_t0 = tof0_slab_hits.GetPmt0().GetRawTime();
                TOF0_hSlab_raw_t1 = tof0_slab_hits.GetPmt1().GetRawTime();
                TOF0_hSlab_t0 = tof0_slab_hits.GetPmt0().GetTime();
                TOF0_hSlab_t1 = tof0_slab_hits.GetPmt1().GetTime();
            }
            else if(tof0_slab_hits.GetPlane() == 1){
                TOF0_vSlab = tof0_slab_hits.GetSlab();
                TOF0_vSlab_raw_t0 = tof0_slab_hits.GetPmt0().GetRawTime();
                TOF0_vSlab_raw_t1 = tof0_slab_hits.GetPmt1().GetRawTime();
                TOF0_vSlab_t0 = tof0_slab_hits.GetPmt0().GetTime();
                TOF0_vSlab_t1 = tof0_slab_hits.GetPmt1().GetTime();
            }

            if((TOF0_hSlab == horizontalHit) && (TOF0_vSlab == verticalHit)){
                // we have a pixel
                get_TOF0_pixel_xy();
                TOF0_hitTime = tof0_space_points.GetTime();
            }
        }
    }
}

void ReadMAUS::particle_at_TOF1(){
    spacePoints_at_TOF1();
}

void ReadMAUS::spacePoints_at_TOF1(){
    // need two slab hits at TOF1 (one in each plane) for
    // this particle to have passed through TOF1

    MAUS::TOFEventSlabHit slab_hits = tof_event->GetTOFEventSlabHit();
    MAUS::TOFEventSpacePoint space_points = tof_event->GetTOFEventSpacePoint();

    MAUS::TOFSlabHit tof1_slab_hits;
    MAUS::TOFSpacePoint tof1_space_points;

    /*
     * 1. Loop over TOF space points
     *    a. Get horizontal and vertical slab numbers
     *    b. Get spill and particle event numbers
     *
     * 2. Loop over TOF slab hits
     *    a. If horizontal and vertical slabs == slabs from space points, get PMT times
     *    b. Check also that spill and particle event numbers are the same
     *    c. Then: get TOF pixel by slab hits and pmt timing
     */

    int horizontalHit = -1;
    int verticalHit = -1;
    int spillNo = -1;
    int particleNo = -1;

    // 1. Loop over TOF1 space points:
    for(size_t i = 0; i < space_points.GetTOF1SpacePointArray().size(); ++i){
        tof1_space_points = space_points.GetTOF1SpacePointArray()[i];
        spillNo = tof1_space_points.GetPhysEventNumber();
        particleNo = tof1_space_points.GetPartEventNumber();
        horizontalHit = tof1_space_points.GetSlabx(); // returns slabs oriented along the x-axis
        verticalHit = tof1_space_points.GetSlaby();   // returns slabs oriented along the y-axis

        // 2. Loop over slab hits and look for matches:
        for(size_t j = 0; j < slab_hits.GetTOF1SlabHitArray().size(); ++j){
            tof1_slab_hits = slab_hits.GetTOF1SlabHitArray()[j];
            if(tof1_slab_hits.GetPlane() == 0){
                // horizontal slab hit
                TOF1_hSlab = tof1_slab_hits.GetSlab();
                TOF1_hSlab_raw_t0 = tof1_slab_hits.GetPmt0().GetRawTime();
                TOF1_hSlab_raw_t1 = tof1_slab_hits.GetPmt1().GetRawTime();
                TOF1_hSlab_t0 = tof1_slab_hits.GetPmt0().GetTime();
                TOF1_hSlab_t1 = tof1_slab_hits.GetPmt1().GetTime();
            }
            else if(tof1_slab_hits.GetPlane() == 1){
                TOF1_vSlab = tof1_slab_hits.GetSlab();
                TOF1_vSlab_raw_t0 = tof1_slab_hits.GetPmt0().GetRawTime();
                TOF1_vSlab_raw_t1 = tof1_slab_hits.GetPmt1().GetRawTime();
                TOF1_vSlab_t0 = tof1_slab_hits.GetPmt0().GetTime();
                TOF1_vSlab_t1 = tof1_slab_hits.GetPmt1().GetTime();
            }

            if((TOF1_hSlab == horizontalHit) && (TOF1_vSlab == verticalHit)){
                // we have a pixel
                get_TOF1_pixel_xy();
                TOF1_hitTime = tof1_space_points.GetTime();
            }
        }
    }
}



void ReadMAUS::particle_at_TOF2(){
    spacePoints_at_TOF2();
}


void ReadMAUS::spacePoints_at_TOF2(){
    // need two slab hits at TOF2 (one in each plane) for
    // this particle to have passed through TOF2

    MAUS::TOFEventSlabHit slab_hits = tof_event->GetTOFEventSlabHit();
    MAUS::TOFEventSpacePoint space_points = tof_event->GetTOFEventSpacePoint();

    MAUS::TOFSlabHit tof2_slab_hits;
    MAUS::TOFSpacePoint tof2_space_points;

    /*
     * 1. Loop over TOF space points
     *    a. Get horizontal and vertical slab numbers
     *    b. Get spill and particle event numbers
     *
     * 2. Loop over TOF slab hits
     *    a. If horizontal and vertical slabs == slabs from space points, get PMT times
     *    b. Check also that spill and particle event numbers are the same
     *    c. Then: get TOF pixel by slab hits and pmt timing
     */

    int horizontalHit = -1;
    int verticalHit = -1;

    // 1. Loop over TOF2 space points:
    for(size_t i = 0; i < space_points.GetTOF2SpacePointArray().size(); ++i){

        tof2_space_points = space_points.GetTOF2SpacePointArray()[i];
        horizontalHit = tof2_space_points.GetSlabx(); // returns slabs oriented along the x-axis
        verticalHit = tof2_space_points.GetSlaby();   // returns slabs oriented along the y-axis

        // 2. Loop over slab hits and look for matches:
        for(size_t j = 0; j < slab_hits.GetTOF2SlabHitArray().size(); ++j){
            tof2_slab_hits = slab_hits.GetTOF2SlabHitArray()[j];
            if(tof2_slab_hits.GetPlane() == 0){
                // horizontal slab hit
                TOF2_hSlab = tof2_slab_hits.GetSlab();
                TOF2_hSlab_raw_t0 = tof2_slab_hits.GetPmt0().GetRawTime();
                TOF2_hSlab_raw_t1 = tof2_slab_hits.GetPmt1().GetRawTime();
                TOF2_hSlab_t0 = tof2_slab_hits.GetPmt0().GetTime();
                TOF2_hSlab_t1 = tof2_slab_hits.GetPmt1().GetTime();
            }
            else if(tof2_slab_hits.GetPlane() == 1){
                TOF2_vSlab = tof2_slab_hits.GetSlab();
                TOF2_vSlab_raw_t0 = tof2_slab_hits.GetPmt0().GetRawTime();
                TOF2_vSlab_raw_t1 = tof2_slab_hits.GetPmt1().GetRawTime();
                TOF2_vSlab_t0 = tof2_slab_hits.GetPmt0().GetTime();
                TOF2_vSlab_t1 = tof2_slab_hits.GetPmt1().GetTime();
            }


            if((TOF2_hSlab == horizontalHit) && (TOF2_vSlab == verticalHit)){
                // we have a pixel
                get_TOF2_pixel_xy();
                TOF2_hitTime = tof2_space_points.GetTime();
            }
        }
    }
}




void ReadMAUS::get_TOF0_pixel_xy(){
    /*
     * Horizontal slabs give us the y-coordinate, vertical slabs give us the x-coordinate
     *
     * TOF0 has 10 slabs (horizontal and vertical).  If it is perfectly on-axis, this puts
     * x = y = 0 on the boundary between slab 4 and 5.
     *
     * So vertical slab 4 is centred on (0 + TOF0_slab_width)/2
     *     --> vertical slab 4 at x = 0 + (TOF0_slab_width)/2
     *     --> vertical slab 3 at x = x_4 + TOF0_slab_width
     *     --> vertical slab 2 at x = x_3 + TOF0_slab_width
     *
     * etc...
     */
    double TOF0_slab_width = 40.0;

    double horizontal_slab_0, horizontal_slab_1, horizontal_slab_2, horizontal_slab_3,
            horizontal_slab_4, horizontal_slab_5, horizontal_slab_6, horizontal_slab_7,
            horizontal_slab_8, horizontal_slab_9;

    double vertical_slab_0, vertical_slab_1, vertical_slab_2, vertical_slab_3,
            vertical_slab_4, vertical_slab_5, vertical_slab_6, vertical_slab_7,
            vertical_slab_8, vertical_slab_9;

    vertical_slab_4 = 0.0 + 0.5*TOF0_slab_width;
    vertical_slab_3 = vertical_slab_4 + TOF0_slab_width;
    vertical_slab_2 = vertical_slab_3 + TOF0_slab_width;
    vertical_slab_1 = vertical_slab_2 + TOF0_slab_width;
    vertical_slab_0 = vertical_slab_1 + TOF0_slab_width;

    vertical_slab_5 = 0.0 - 0.5*TOF0_slab_width;
    vertical_slab_6 = vertical_slab_5 - TOF0_slab_width;
    vertical_slab_7 = vertical_slab_6 - TOF0_slab_width;
    vertical_slab_8 = vertical_slab_7 - TOF0_slab_width;
    vertical_slab_9 = vertical_slab_8 - TOF0_slab_width;

    horizontal_slab_4 = 0.0 - 0.5*TOF0_slab_width;
    horizontal_slab_3 = horizontal_slab_4 - TOF0_slab_width;
    horizontal_slab_2 = horizontal_slab_3 - TOF0_slab_width;
    horizontal_slab_1 = horizontal_slab_2 - TOF0_slab_width;
    horizontal_slab_0 = horizontal_slab_1 - TOF0_slab_width;

    horizontal_slab_5 = 0.0 + 0.5*TOF0_slab_width;
    horizontal_slab_6 = horizontal_slab_5 + TOF0_slab_width;
    horizontal_slab_7 = horizontal_slab_6 + TOF0_slab_width;
    horizontal_slab_8 = horizontal_slab_7 + TOF0_slab_width;
    horizontal_slab_9 = horizontal_slab_8 + TOF0_slab_width;

    double x, y;
    if(TOF0_vSlab == 0){
        x = vertical_slab_0;
    }
    else if(TOF0_vSlab == 1){
        x = vertical_slab_1;
    }
    else if(TOF0_vSlab == 2){
        x = vertical_slab_2;
    }
    else if(TOF0_vSlab == 3){
        x = vertical_slab_3;
    }
    else if(TOF0_vSlab == 4){
        x = vertical_slab_4;
    }
    else if(TOF0_vSlab == 5){
        x = vertical_slab_5;
    }
    else if(TOF0_vSlab == 6){
        x = vertical_slab_6;
    }
    else if(TOF0_vSlab == 7){
        x = vertical_slab_7;
    }
    else if(TOF0_vSlab == 8){
        x = vertical_slab_8;
    }
    else if(TOF0_vSlab == 9){
        x = vertical_slab_9;
    }
    else{
        std::cerr << "Received Vertical TOF Slab number " << TOF0_vSlab << " which is out of bounds (0, 9)\n";
        x = TMath::Infinity();
    }


    if(TOF0_hSlab == 0){
        y = horizontal_slab_0;
    }
    else if(TOF0_hSlab == 1){
        y = horizontal_slab_1;
    }
    else if(TOF0_hSlab == 2){
        y = horizontal_slab_2;
    }
    else if(TOF0_hSlab == 3){
        y = horizontal_slab_3;
    }
    else if(TOF0_hSlab == 4){
        y = horizontal_slab_4;
    }
    else if(TOF0_hSlab == 5){
        y = horizontal_slab_5;
    }
    else if(TOF0_hSlab == 6){
        y = horizontal_slab_6;
    }
    else if(TOF0_hSlab == 7){
        y = horizontal_slab_7;
    }
    else if(TOF0_hSlab == 8){
        y = horizontal_slab_8;
    }
    else if(TOF0_hSlab == 9){
        y = horizontal_slab_9;
    }
    else{
        std::cerr << "Received Horizontal TOF Slab number " << TOF0_hSlab << " which is out of bounds (0, 9)\n";
        y = TMath::Infinity();
    }

    TOF0_xPixel = x;
    TOF0_yPixel = y;

    if(TOF0_horizontal_slab_calibrations.at(TOF0_hSlab) != TMath::Infinity()){
        TOF0_x = 0.5*calibrated_c_eff*(TOF0_hSlab_raw_t0 - TOF0_hSlab_raw_t1 + TOF0_horizontal_slab_calibrations.at(TOF0_hSlab));
    }
    else{
        TOF0_x = TOF0_xPixel;
    }
    if(TOF0_vertical_slab_calibrations.at(TOF0_vSlab) != TMath::Infinity()){
        TOF0_y = 0.5*calibrated_c_eff*(TOF0_vSlab_raw_t0 - TOF0_vSlab_raw_t1 + TOF0_vertical_slab_calibrations.at(TOF0_vSlab));
    }
    else{
        TOF0_y = TOF0_yPixel;
    }

    TOF0_x = TOF0_x + TOF0_xOffset;
    TOF0_y = TOF0_y + TOF0_yOffset;
}

void ReadMAUS::get_TOF1_pixel_xy(){
    /*
     * Horizontal slabs give us the y-coordinate, vertical slabs give us the x-coordinate
     *
     * TOF1 has 7 slabs (horizontal and vertical).  If it is perfectly on-axis, this puts
     * x = y = 0 in the middle of slab 3
     *
     * So vertical slab 3 is centred on 0
     *     --> vertical slab 2 at x = 0 + TOF1_slab_width
     *     --> vertical slab 1 at x = x_2 + TOF1_slab_width
     *     --> vertical slab 0 at x = x_1 + TOF1_slab_width
     *
     * etc...
     */

    double TOF1_slab_width = 60.0; // mm

    double horizontal_slab_0, horizontal_slab_1, horizontal_slab_2, horizontal_slab_3,
            horizontal_slab_4, horizontal_slab_5, horizontal_slab_6;

    double vertical_slab_0, vertical_slab_1, vertical_slab_2, vertical_slab_3,
            vertical_slab_4, vertical_slab_5, vertical_slab_6;

    vertical_slab_3 = 0.0;
    vertical_slab_2 = vertical_slab_3 + TOF1_slab_width;
    vertical_slab_1 = vertical_slab_3 + TOF1_slab_width;
    vertical_slab_0 = vertical_slab_2 + TOF1_slab_width;

    vertical_slab_4 = vertical_slab_3 - TOF1_slab_width;
    vertical_slab_5 = vertical_slab_4 - TOF1_slab_width;
    vertical_slab_6 = vertical_slab_5 - TOF1_slab_width;

    horizontal_slab_3 = 0.0;
    horizontal_slab_2 = horizontal_slab_3 - TOF1_slab_width;
    horizontal_slab_1 = horizontal_slab_2 - TOF1_slab_width;
    horizontal_slab_0 = horizontal_slab_1 - TOF1_slab_width;

    horizontal_slab_4 = horizontal_slab_3 + TOF1_slab_width;
    horizontal_slab_5 = horizontal_slab_4 + TOF1_slab_width;
    horizontal_slab_6 = horizontal_slab_5 + TOF1_slab_width;

    double x, y;
    if(TOF1_vSlab == 0){
        x = vertical_slab_0;
    }
    else if(TOF1_vSlab == 1){
        x = vertical_slab_1;
    }
    else if(TOF1_vSlab == 2){
        x = vertical_slab_2;
    }
    else if(TOF1_vSlab == 3){
        x = vertical_slab_3;
    }
    else if(TOF1_vSlab == 4){
        x = vertical_slab_4;
    }
    else if(TOF1_vSlab == 5){
        x = vertical_slab_5;
    }
    else if(TOF1_vSlab == 6){
        x = vertical_slab_6;
    }
    else{
        std::cerr << "Received Vertical TOF Slab number " << TOF1_vSlab << " which is out of bounds (0, 6)\n";
        x = TMath::Infinity();
    }


    if(TOF1_hSlab == 0){
        y = horizontal_slab_0;
    }
    else if(TOF1_hSlab == 1){
        y = horizontal_slab_1;
    }
    else if(TOF1_hSlab == 2){
        y = horizontal_slab_2;
    }
    else if(TOF1_hSlab == 3){
        y = horizontal_slab_3;
    }
    else if(TOF1_hSlab == 4){
        y = horizontal_slab_4;
    }
    else if(TOF1_hSlab == 5){
        y = horizontal_slab_5;
    }
    else if(TOF1_hSlab == 6){
        y = horizontal_slab_6;
    }
    else{
        std::cerr << "Received Horizontal TOF Slab number " << TOF1_hSlab << " which is out of bounds (0, 6)\n";
        y = TMath::Infinity();
    }

    TOF1_xPixel = x;
    TOF1_yPixel = y;

    if(TOF1_horizontal_slab_calibrations.at(TOF1_hSlab) != TMath::Infinity()){
        TOF1_x = 0.5*calibrated_c_eff*(TOF1_hSlab_raw_t0 - TOF1_hSlab_raw_t1 + TOF1_horizontal_slab_calibrations.at(TOF1_hSlab));
    }
    else{
        TOF1_x = TOF1_xPixel;
    }
    if(TOF1_vertical_slab_calibrations.at(TOF1_vSlab) != TMath::Infinity()){
        TOF1_y = 0.5*calibrated_c_eff*(TOF1_vSlab_raw_t0 - TOF1_vSlab_raw_t1 + TOF1_vertical_slab_calibrations.at(TOF1_vSlab));
    }
    else{
        TOF1_y = TOF1_yPixel;
    }

    TOF1_x = TOF1_x + TOF1_xOffset;
    TOF1_y = TOF1_y + TOF1_yOffset;
}



void ReadMAUS::get_TOF2_pixel_xy(){
    /*
     * Horizontal slabs give us the y-coordinate, vertical slabs give us the x-coordinate
     *
     * TOF2 has 10 slabs (horizontal and vertical).  If it is perfectly on-axis, this puts
     * x = y = 0 on the boundary between slab 4 and 5.
     *
     * So vertical slab 4 is centred on (0 + TOF2_slab_width)/2
     *     --> vertical slab 4 at x = 0 + (TOF2_slab_width)/2
     *     --> vertical slab 3 at x = x_4 + TOF2_slab_width
     *     --> vertical slab 2 at x = x_3 + TOF2_slab_width
     *
     * etc...
     */

    double TOF2_slab_width = 60.0;

    double horizontal_slab_0, horizontal_slab_1, horizontal_slab_2, horizontal_slab_3,
            horizontal_slab_4, horizontal_slab_5, horizontal_slab_6, horizontal_slab_7,
            horizontal_slab_8, horizontal_slab_9;

    double vertical_slab_0, vertical_slab_1, vertical_slab_2, vertical_slab_3,
            vertical_slab_4, vertical_slab_5, vertical_slab_6, vertical_slab_7,
            vertical_slab_8, vertical_slab_9;

    vertical_slab_4 = 0.0 + 0.5*TOF2_slab_width;
    vertical_slab_3 = vertical_slab_4 + TOF2_slab_width;
    vertical_slab_2 = vertical_slab_3 + TOF2_slab_width;
    vertical_slab_1 = vertical_slab_2 + TOF2_slab_width;
    vertical_slab_0 = vertical_slab_1 + TOF2_slab_width;

    vertical_slab_5 = 0.0 - 0.5*TOF2_slab_width;
    vertical_slab_6 = vertical_slab_5 - TOF2_slab_width;
    vertical_slab_7 = vertical_slab_6 - TOF2_slab_width;
    vertical_slab_8 = vertical_slab_7 - TOF2_slab_width;
    vertical_slab_9 = vertical_slab_8 - TOF2_slab_width;

    horizontal_slab_4 = 0.0 - 0.5*TOF2_slab_width;
    horizontal_slab_3 = horizontal_slab_4 - TOF2_slab_width;
    horizontal_slab_2 = horizontal_slab_3 - TOF2_slab_width;
    horizontal_slab_1 = horizontal_slab_2 - TOF2_slab_width;
    horizontal_slab_0 = horizontal_slab_1 - TOF2_slab_width;

    horizontal_slab_5 = 0.0 + 0.5*TOF2_slab_width;
    horizontal_slab_6 = horizontal_slab_5 + TOF2_slab_width;
    horizontal_slab_7 = horizontal_slab_6 + TOF2_slab_width;
    horizontal_slab_8 = horizontal_slab_7 + TOF2_slab_width;
    horizontal_slab_9 = horizontal_slab_8 + TOF2_slab_width;

    double x, y;
    if(TOF2_vSlab == 0){
        x = vertical_slab_0;
    }
    else if(TOF2_vSlab == 1){
        x = vertical_slab_1;
    }
    else if(TOF2_vSlab == 2){
        x = vertical_slab_2;
    }
    else if(TOF2_vSlab == 3){
        x = vertical_slab_3;
    }
    else if(TOF2_vSlab == 4){
        x = vertical_slab_4;
    }
    else if(TOF2_vSlab == 5){
        x = vertical_slab_5;
    }
    else if(TOF2_vSlab == 6){
        x = vertical_slab_6;
    }
    else if(TOF2_vSlab == 7){
        x = vertical_slab_7;
    }
    else if(TOF2_vSlab == 8){
        x = vertical_slab_8;
    }
    else if(TOF2_vSlab == 9){
        x = vertical_slab_9;
    }
    else{
        std::cerr << "Received Vertical TOF Slab number " << TOF2_vSlab << " which is out of bounds (0, 9)\n";
        x = TMath::Infinity();
    }


    if(TOF2_hSlab == 0){
        y = horizontal_slab_0;
    }
    else if(TOF2_hSlab == 1){
        y = horizontal_slab_1;
    }
    else if(TOF2_hSlab == 2){
        y = horizontal_slab_2;
    }
    else if(TOF2_hSlab == 3){
        y = horizontal_slab_3;
    }
    else if(TOF2_hSlab == 4){
        y = horizontal_slab_4;
    }
    else if(TOF2_hSlab == 5){
        y = horizontal_slab_5;
    }
    else if(TOF2_hSlab == 6){
        y = horizontal_slab_6;
    }
    else if(TOF2_hSlab == 7){
        y = horizontal_slab_7;
    }
    else if(TOF2_hSlab == 8){
        y = horizontal_slab_8;
    }
    else if(TOF2_hSlab == 9){
        y = horizontal_slab_9;
    }
    else{
        std::cerr << "Received Horizontal TOF Slab number " << TOF2_hSlab << " which is out of bounds (0, 9)\n";
        y = TMath::Infinity();
    }

    TOF2_xPixel = x;
    TOF2_yPixel = y;

    if(TOF2_horizontal_slab_calibrations.at(TOF2_hSlab) != TMath::Infinity()){
        TOF2_x = 0.5*calibrated_c_eff*(TOF2_hSlab_raw_t0 - TOF2_hSlab_raw_t1 + TOF2_horizontal_slab_calibrations.at(TOF2_hSlab));
    }
    else{
        TOF2_x = TOF2_xPixel;
    }
    if(TOF2_vertical_slab_calibrations.at(TOF2_vSlab) != TMath::Infinity()){
        TOF2_y = 0.5*calibrated_c_eff*(TOF2_vSlab_raw_t0 - TOF2_vSlab_raw_t1 + TOF2_vertical_slab_calibrations.at(TOF2_vSlab));
    }
    else{
        TOF2_y = TOF2_yPixel;
    }

    TOF2_x = TOF2_x + TOF2_xOffset;
    TOF2_y = TOF2_y + TOF2_yOffset;

}



void ReadMAUS::set_2011_TOF0_TOF1_Rayner_calibration(){
    calibrated_c_eff = 135.2e-3; //mm per ps (as particle time at TOF is in ps)

    // calibrations go in order of increasing slab number:
    TOF0_horizontal_slab_calibrations << TMath::Infinity()  // slab 0 is uncalibrated in 2011
                                      << TMath::Infinity()  // slab 1 is uncalibrated in 2011
                                      << TMath::Infinity()  // 234.1 in 2011
                                      << TMath::Infinity()  // 294.2 in 2011
                                      << TMath::Infinity()  // 351.9 in 2011
                                      << TMath::Infinity()  // 321.0 in 2011
                                      << TMath::Infinity()  // 357.5 in 2011
                                      << TMath::Infinity()  // 219.7 in 2011
                                      << TMath::Infinity()  // slab 8 is uncalibrated in 2011
                                      << TMath::Infinity(); // slab 9 is uncalibrated in 2011

    TOF0_vertical_slab_calibrations << TMath::Infinity() // slab 0 is uncalibrated in 2011
                                    << TMath::Infinity()  // 194.3 in 2011
                                    << TMath::Infinity()  // 202.7 in 2011
                                    << TMath::Infinity()  // 238.1 in 2011
                                    << TMath::Infinity()  // 268.8 in 2011
                                    << TMath::Infinity()  // 232.7 in 2011
                                    << TMath::Infinity()  // 232.1 in 2011
                                    << TMath::Infinity()  // 221.6 in 2011
                                    << TMath::Infinity()  // 330.9 in 2011
                                    << TMath::Infinity(); // slab 9 is uncalibrated in 2011

    TOF1_horizontal_slab_calibrations << TMath::Infinity()  // -42.2 in 2011
                                      << TMath::Infinity()  // 6.2 in 2011
                                      << TMath::Infinity()  // -1.0 in 2011
                                      << TMath::Infinity()  // 35.0 in 2011
                                      << TMath::Infinity()  // 33.8 in 2011
                                      << TMath::Infinity()  // 29.4 in 2011
                                      << TMath::Infinity();  // 32.6 in 2011

    TOF1_vertical_slab_calibrations << TMath::Infinity() // slab 0 is uncalibrated in 2011
                                    << TMath::Infinity()  // -3.4 in 2011
                                    << TMath::Infinity()  // -39.8 in 2011
                                    << TMath::Infinity()  // 34.6 in 2011
                                    << TMath::Infinity()  // 35.8 in 2011
                                    << TMath::Infinity()  // 9.8 in 2011
                                    << TMath::Infinity();  // 4.2 in 2011

    TOF2_horizontal_slab_calibrations << TMath::Infinity()
                                      << TMath::Infinity()
                                      << TMath::Infinity()
                                      << TMath::Infinity()
                                      << TMath::Infinity()
                                      << TMath::Infinity()
                                        << TMath::Infinity()
                                        << TMath::Infinity()
                                        << TMath::Infinity()
                                      << TMath::Infinity();

    TOF2_vertical_slab_calibrations << TMath::Infinity()
                                    << TMath::Infinity()
                                    << TMath::Infinity()
                                    << TMath::Infinity()
                                    << TMath::Infinity()
                                    << TMath::Infinity()
                                    << TMath::Infinity()
                                    << TMath::Infinity()
                                    << TMath::Infinity()
                                    << TMath::Infinity();

}


void ReadMAUS::particle_at_tracker(){
    /*
     * Currently this function just spits out information to the terminal window. Need to
     * (a) figure out what info I really want to strip off, and (b) make sure I've nabbed
     * the correct info!
     */

    int tracker, station;
    MAUS::ThreeVector position;

    std::vector<MAUS::SciFiTrack*> tracks = scifi_event->scifitracks();
    std::vector<MAUS::SciFiTrack*>::iterator track_iter;

    for(track_iter = tracks.begin(); track_iter != tracks.end(); ++track_iter){

        std::vector<MAUS::SciFiTrackPoint*> track_points = (*track_iter)->scifitrackpoints();
        std::vector<MAUS::SciFiTrackPoint*>::iterator track_point_iter;

        for(track_point_iter = track_points.begin(); track_point_iter != track_points.end(); ++track_point_iter){
            MAUS::SciFiTrackPoint* point = (*track_point_iter);
            tracker = point->tracker();
            station = point->station();
            position = point->pos();


            if(tracker == 0){
                if(point->station() == 1){
                    TKU_plane1_x = point->pos().x();
                    TKU_plane1_y = point->pos().y();
                    TKU_plane1_z = point->pos().z();

                    TKU_plane1_px = point->mom().x();
                    TKU_plane1_py = point->mom().y();
                    TKU_plane1_pz = point->mom().z();
                }
                else if(point->station() == 2){
                    TKU_plane2_x = point->pos().x();
                    TKU_plane2_y = point->pos().y();
                    TKU_plane2_z = point->pos().z();

                    TKU_plane2_px = point->mom().x();
                    TKU_plane2_py = point->mom().y();
                    TKU_plane2_pz = point->mom().z();
                }
                else if(point->station() == 3){
                    TKU_plane3_x = point->pos().x();
                    TKU_plane3_y = point->pos().y();
                    TKU_plane3_z = point->pos().z();

                    TKU_plane3_px = point->mom().x();
                    TKU_plane3_py = point->mom().y();
                    TKU_plane3_pz = point->mom().z();
                }
                else if(point->station() == 4){
                    TKU_plane4_x = point->pos().x();
                    TKU_plane4_y = point->pos().y();
                    TKU_plane4_z = point->pos().z();

                    TKU_plane4_px = point->mom().x();
                    TKU_plane4_py = point->mom().y();
                    TKU_plane4_pz = point->mom().z();
                }
                else{
                    TKU_plane5_x = point->pos().x();
                    TKU_plane5_y = point->pos().y();
                    TKU_plane5_z = point->pos().z();

                    TKU_plane5_px = point->mom().x();
                    TKU_plane5_py = point->mom().y();
                    TKU_plane5_pz = point->mom().z();
                }
            }

            else{
                if(point->station() == 1){
                    TKD_plane1_x = point->pos().x();
                    TKD_plane1_y = point->pos().y();
                    TKD_plane1_z = point->pos().z();

                    TKD_plane1_px = point->mom().x();
                    TKD_plane1_py = point->mom().y();
                    TKD_plane1_pz = point->mom().z();
                }
                else if(point->station() == 2){
                    TKD_plane2_x = point->pos().x();
                    TKD_plane2_y = point->pos().y();
                    TKD_plane2_z = point->pos().z();

                    TKD_plane2_px = point->mom().x();
                    TKD_plane2_py = point->mom().y();
                    TKD_plane2_pz = point->mom().z();
                }
                else if(point->station() == 3){
                    TKD_plane3_x = point->pos().x();
                    TKD_plane3_y = point->pos().y();
                    TKD_plane3_z = point->pos().z();

                    TKD_plane3_px = point->mom().x();
                    TKD_plane3_py = point->mom().y();
                    TKD_plane3_pz = point->mom().z();
                }
                else if(point->station() == 4){
                    TKD_plane4_x = point->pos().x();
                    TKD_plane4_y = point->pos().y();
                    TKD_plane4_z = point->pos().z();

                    TKD_plane4_px = point->mom().x();
                    TKD_plane4_py = point->mom().y();
                    TKD_plane4_pz = point->mom().z();
                }
                else{
                    TKD_plane5_x = point->pos().x();
                    TKD_plane5_y = point->pos().y();
                    TKD_plane5_z = point->pos().z();

                    TKD_plane5_px = point->mom().x();
                    TKD_plane5_py = point->mom().y();
                    TKD_plane5_pz = point->mom().z();
                }
            }
        }

    }

}

