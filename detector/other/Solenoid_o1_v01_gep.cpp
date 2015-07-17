#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/Printout.h"
#include "XML/Layering.h"
#include "TGeoTrd2.h"
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"


using namespace std;
using namespace DD4hep;
using namespace DD4hep::Geometry;

static Ref_t create_detector(LCDD& lcdd, xml_h e, SensitiveDetector sens)  {
    static double tolerance = 0e0;
    
    xml_det_t     x_det     = e;
    int           det_id    = x_det.id();
    string        det_name  = x_det.nameStr();
    DetElement    sdet      (det_name,det_id);
    //Below needed only for reco structure
//     Layering layering(x_det);
//     double totalThickness = layering.totalThickness();
//     xml_dim_t dim = x_det.dimensions();
//     double detZ = dim.z();
//     double rmin = dim.rmin();
    
    // --- create an envelope volume and position it into the world ---------------------
    
    Volume envelope = XML::createPlacedEnvelope( lcdd,  e , sdet ) ;
    
    if( lcdd.buildType() == BUILD_ENVELOPE ) return sdet ;
    
    //-----------------------------------------------------------------------------------
    
    //DISABLE FOR NOW, TRY USING ENVELOPE VOLUME IN RECO.
    //TO ENABLE, UNCOMMENT LINE 93 AT THE END TOO
//     //Create caloData object to extend driver with data required for reconstruction
//     DDRec::LayeredCalorimeterData* caloData = new DDRec::LayeredCalorimeterData ;
//     caloData->layoutType = DDRec::LayeredCalorimeterData::BarrelLayout ;
//     /// extent of the calorimeter in the r-z-plane [ rmin, rmax, zmin, zmax ] in mm.
//     caloData->extent[0] = rmin ;
//     caloData->extent[1] = rmin + totalThickness ;
//     caloData->extent[2] = 0. ;
//     caloData->extent[3] = detZ;
    //No other information needed, e.g. no layers needed. This may change but for now pandora just needs dimensions
    
    Material air = lcdd.air();
    PlacedVolume pv;
    int n = 0;
    
    for(xml_coll_t i(x_det,_U(layer)); i; ++i, ++n)  {
        xml_comp_t x_layer = i;
        string  l_name = det_name+_toString(n,"_layer%d");
        double  z    = x_layer.outer_z();
        double  rmin = x_layer.inner_r();
        double  r    = rmin;
        DetElement layer(sdet,_toString(n,"layer%d"),x_layer.id());
        Tube    l_tub (rmin,2*rmin,z);
        Volume  l_vol(l_name,l_tub,air);
        int m = 0;
        
        for(xml_coll_t j(x_layer,_U(slice)); j; ++j, ++m)  {
            xml_comp_t x_slice = j;
            Material mat = lcdd.material(x_slice.materialStr());
            string s_name= l_name+_toString(m,"_slice%d");
            double thickness = x_slice.thickness();
            Tube   s_tub(r,r+thickness,z,2*M_PI);
            Volume s_vol(s_name, s_tub, mat);
            
            r += thickness;
            if ( x_slice.isSensitive() ) {
                sens.setType("tracker");
                s_vol.setSensitiveDetector(sens);
            }
            // Set Attributes
            s_vol.setAttributes(lcdd,x_slice.regionStr(),x_slice.limitsStr(),x_slice.visStr());
            pv = l_vol.placeVolume(s_vol);
            // Slices have no extra id. Take the ID of the layer!
            pv.addPhysVolID("slice",m);
        }
        l_tub.setDimensions(rmin,r,z);
        //cout << l_name << " " << rmin << " " << r << " " << z << endl;
        l_vol.setVisAttributes(lcdd,x_layer.visStr());
        
        pv = envelope.placeVolume(l_vol);
        pv.addPhysVolID("layer",n);
        layer.setPlacement(pv);
    }
    if ( x_det.hasAttr(_U(combineHits)) ) {
        sdet.setCombineHits(x_det.combineHits(),sens);
    }
    
//     sdet.addExtension< DDRec::LayeredCalorimeterData >( caloData ) ;
    
    return sdet;
    
}

DECLARE_DETELEMENT(Solenoid_o1_v01,create_detector)