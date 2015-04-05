#include <DD4hep/DetFactoryHelper.h>
#include <XML/Layering.h>
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"

#include <string>


static DD4hep::Geometry::Ref_t create_detector(DD4hep::Geometry::LCDD& lcdd,
					       DD4hep::XML::Handle_t xmlHandle,
					       DD4hep::Geometry::SensitiveDetector sens) {

  std::cout << __PRETTY_FUNCTION__  << std::endl;
  std::cout << "Here is my LHCal"  << std::endl;
  std::cout << " and this is the sensitive detector: " << &sens  << std::endl;
  sens.setType("calorimeter");
  //Materials
  DD4hep::Geometry::Material air = lcdd.air();

  //Access to the XML File
  DD4hep::XML::DetElement xmlLHCal = xmlHandle;
  const std::string detName = xmlLHCal.nameStr();

  DD4hep::Geometry::DetElement sdet ( detName, xmlLHCal.id() );
  DD4hep::Geometry::Volume motherVol = lcdd.pickMotherVolume(sdet);

  // --- create an envelope volume and position it into the world ---------------------
  
  DD4hep::Geometry::Volume envelope = DD4hep::XML::createPlacedEnvelope( lcdd,  xmlHandle , sdet ) ;
  
  if( lcdd.buildType() == DD4hep::BUILD_ENVELOPE ) return sdet ;
  
  //-----------------------------------------------------------------------------------

  DD4hep::XML::Dimension dimensions =  xmlLHCal.dimensions();

  //LHCal Dimensions
  const double lhcalInnerR = dimensions.inner_r();
  const double lhcalwidth = dimensions.width();
  const double lhcalheight = dimensions.height();
  const double lhcalInnerZ = dimensions.inner_z();
  const double lhcaloffset = dimensions.offset();
  const double lhcalThickness = DD4hep::Layering(xmlLHCal).totalThickness();
  const double lhcalCentreZ = lhcalInnerZ+lhcalThickness*0.5;

  double LHcal_cell_size      = lcdd.constant<double>("LHcal_cell_size");
  double LHCal_outer_radius   = lcdd.constant<double>("LHCal_outer_radius");
  //========== fill data for reconstruction ============================
  DDRec::LayeredCalorimeterData* caloData = new DDRec::LayeredCalorimeterData ;
  caloData->layoutType = DDRec::LayeredCalorimeterData::EndcapLayout ;
  caloData->inner_symmetry = 0  ; // hard code cernter pipe hole
  caloData->outer_symmetry = 4  ; // outer box
  caloData->phi0 = 0 ;

  /// extent of the calorimeter in the r-z-plane [ rmin, rmax, zmin, zmax ] in mm.
  caloData->extent[0] = lhcalInnerR ;
  caloData->extent[1] = LHCal_outer_radius ;
  caloData->extent[2] = lhcalInnerZ ;
  caloData->extent[3] = lhcalInnerZ + lhcalThickness ;

  // counter for the current layer to be placed
  int thisLayerId = 1;

  //Parameters we have to know about
  DD4hep::XML::Component xmlParameter = xmlLHCal.child(_Unicode(parameter));
  const double mradFullCrossingAngle  = xmlParameter.attr< double >(_Unicode(crossingangle));
  std::cout << " The crossing angle is: " << mradFullCrossingAngle << " radian"  << std::endl;

  // this needs the full crossing angle, because the beamCal is centred on the
  // outgoing beampipe, and the incoming beampipe is a full crossing agle away
  const DD4hep::Geometry::Position    incomingBeamPipePosition( -1.0*lhcaloffset, 0.0, 0.0);
  //The extra parenthesis are paramount!
  const DD4hep::Geometry::Rotation3D  incomingBeamPipeRotation( ( DD4hep::Geometry::RotationY( 0 ) ) );
  const DD4hep::Geometry::Transform3D incomingBPTransform( incomingBeamPipeRotation, incomingBeamPipePosition );

  //Envelope to place the layers in
  DD4hep::Geometry::Box envelopeBox (lhcalwidth*0.5, lhcalheight*0.5, lhcalThickness*0.5 );
  DD4hep::Geometry::Tube incomingBeamPipe (0.0, lhcalInnerR, lhcalThickness);//we want this to be longer than the LHCal

  DD4hep::Geometry::SubtractionSolid LHCalModule (envelopeBox, incomingBeamPipe, incomingBPTransform);
  DD4hep::Geometry::Volume     envelopeVol(detName+"_module",LHCalModule,air);
  envelopeVol.setVisAttributes(lcdd,xmlLHCal.visStr());


  ////////////////////////////////////////////////////////////////////////////////
  // Create all the layers
  ////////////////////////////////////////////////////////////////////////////////

  //Loop over all the layer (repeat=NN) sections
  //This is the starting point to place all layers, we need this when we have more than one layer block
  double referencePosition = -lhcalThickness*0.5;
  for(DD4hep::XML::Collection_t coll(xmlLHCal,_U(layer)); coll; ++coll)  {
    DD4hep::XML::Component xmlLayer(coll); //we know this thing is a layer


    //This just calculates the total size of a single layer
    //Why no convenience function for this?
    double layerThickness = 0;

    for(DD4hep::XML::Collection_t l(xmlLayer,_U(slice)); l; ++l)
      layerThickness += xml_comp_t(l).thickness();

    std::cout << "Total Length "    << lhcalThickness/dd4hep::cm  << " cm" << std::endl;
    std::cout << "Layer Thickness " << layerThickness/dd4hep::cm << " cm" << std::endl;

    //Loop for repeat=NN
    for(int i=0, repeat=xmlLayer.repeat(); i<repeat; ++i)  {

      std::string layer_name = detName + DD4hep::XML::_toString(thisLayerId,"_layer%d");
      DD4hep::Geometry::Box layer_base(lhcalwidth*0.5, lhcalheight*0.5,layerThickness*0.5);
      DD4hep::Geometry::SubtractionSolid layer_subtracted;
      layer_subtracted = DD4hep::Geometry::SubtractionSolid(layer_base, incomingBeamPipe, incomingBPTransform);
     

      DD4hep::Geometry::Volume layer_vol(layer_name,layer_subtracted,air);


      int sliceID=1;
      double inThisLayerPosition = -layerThickness*0.5;

      double radiator_thickness = 0.0;

      for(DD4hep::XML::Collection_t collSlice(xmlLayer,_U(slice)); collSlice; ++collSlice)  {
	DD4hep::XML::Component compSlice = collSlice;
	const double      sliceThickness = compSlice.thickness();
	const std::string sliceName = layer_name + DD4hep::XML::_toString(sliceID,"slice%d");
	DD4hep::Geometry::Material   slice_mat  = lcdd.material(compSlice.materialStr());

	DD4hep::Geometry::Box sliceBase(lhcalwidth*0.5, lhcalheight*0.5,sliceThickness*0.5);
	DD4hep::Geometry::SubtractionSolid slice_subtracted;

	slice_subtracted = DD4hep::Geometry::SubtractionSolid(sliceBase, incomingBeamPipe, incomingBPTransform);

	DD4hep::Geometry::Volume slice_vol (sliceName,slice_subtracted,slice_mat);

	if ( compSlice.materialStr().compare("TungstenDens24") == 0 ) radiator_thickness = sliceThickness;

	if ( compSlice.isSensitive() )  {
	  slice_vol.setSensitiveDetector(sens);
	}

	slice_vol.setAttributes(lcdd,compSlice.regionStr(),compSlice.limitsStr(),compSlice.visStr());
	DD4hep::Geometry::PlacedVolume pv = layer_vol.placeVolume(slice_vol, DD4hep::Geometry::Position(0,0,inThisLayerPosition+sliceThickness*0.5));

	pv.addPhysVolID("slice",sliceID);
	inThisLayerPosition += sliceThickness;

	++sliceID;
      }//For all slices in this layer

      //-----------------------------------------------------------------------------------------
      DDRec::LayeredCalorimeterData::Layer caloLayer ;
      
      caloLayer.distance = lhcalCentreZ + referencePosition+0.5*layerThickness ;
      caloLayer.thickness = layerThickness ;
      caloLayer.absorberThickness = radiator_thickness ;
      caloLayer.cellSize0 = LHcal_cell_size ;
      caloLayer.cellSize1 = LHcal_cell_size ;
      
      caloData->layers.push_back( caloLayer ) ;
      //-----------------------------------------------------------------------------------------

      //Why are we doing this for each layer, this just needs to be done once and then placed multiple times
      //Do we need unique IDs for each piece?
      layer_vol.setVisAttributes(lcdd,xmlLayer.visStr());

      DD4hep::Geometry::Position layer_pos(0,0,referencePosition+0.5*layerThickness);
      referencePosition += layerThickness;
      DD4hep::Geometry::PlacedVolume pv = envelopeVol.placeVolume(layer_vol,layer_pos);
      pv.addPhysVolID("layer",thisLayerId);

      ++thisLayerId;

    }//for all layers

  }// for all layer collections

 
  const DD4hep::Geometry::Position bcForwardPos (lhcaloffset,0.0, lhcalCentreZ);
  const DD4hep::Geometry::Position bcBackwardPos(-1.0*lhcaloffset,0.0,-lhcalCentreZ);
  const DD4hep::Geometry::Rotation3D bcForwardRot ( DD4hep::Geometry::RotationY( 0 ) );
  const DD4hep::Geometry::Rotation3D bcBackwardRot( DD4hep::Geometry::RotationY( M_PI )  );

  DD4hep::Geometry::PlacedVolume pv =
    envelope.placeVolume(envelopeVol, DD4hep::Geometry::Transform3D( bcForwardRot, bcForwardPos ) );
  pv.addPhysVolID("barrel", 1);

  DD4hep::Geometry::PlacedVolume pv2 =
    envelope.placeVolume(envelopeVol, DD4hep::Geometry::Transform3D( bcBackwardRot, bcBackwardPos ) );
  pv2.addPhysVolID("barrel", 2);

  sdet.addExtension< DDRec::LayeredCalorimeterData >( caloData ) ;

  return sdet;
}

DECLARE_DETELEMENT(LHCal,create_detector)
