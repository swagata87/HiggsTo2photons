#include "HiggsAnalysis/HiggsTo2photons/interface/GlobeConversions.h"

#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

GlobeConversions::GlobeConversions(const edm::ParameterSet& iConfig, const char* n): nome(n) {

  debug_level = iConfig.getParameter<int>("Debug_Level");
  doFastSim = iConfig.getParameter<bool>("doFastSim");
  doAodSim = iConfig.getParameter<bool>("doAodSim");

  // allConversions
  allConversionsColl =  iConfig.getParameter<edm::InputTag>("ConvertedPhotonColl");

  // SUPER CLUSTERS
  hybridSuperClusterColl = iConfig.getParameter<edm::InputTag>("HybridSuperClusterColl");
  endcapSuperClusterColl = iConfig.getParameter<edm::InputTag>("EndcapSuperClusterColl");

  // get cut thresholds
  gCUT = new GlobeCuts(iConfig);

  conv_nHitsBeforeVtx = new std::vector<std::vector<unsigned short> >; conv_nHitsBeforeVtx->clear();

}

void GlobeConversions::defineBranch(TTree* tree) {

  conv_p4 = new TClonesArray("TLorentzVector",MAX_CONVERTEDPHOTONS);
  tree->Branch("conv_n", &conv_n, "conv_n/I");
  //// conversion quantities 
  tree->Branch("conv_p4", "TClonesArray", &conv_p4, 32000, 0);
  tree->Branch("conv_ntracks",&conv_ntracks,"conv_ntracks[conv_n]/I");
  tree->Branch("conv_pairinvmass",&conv_pairinvmass,"conv_pairinvmass[conv_n]/F");
  tree->Branch("conv_paircotthetasep",&conv_paircotthetasep,"conv_paircotthetasep[conv_n]/F");
  tree->Branch("conv_eoverp",&conv_eoverp,"conv_eoverp[conv_n]/F");
  tree->Branch("conv_distofminapproach",&conv_distofminapproach,"conv_distofminapproach[conv_n]/F");
  tree->Branch("conv_dphitrksatvtx",&conv_dphitrksatvtx,"conv_dphitrksatvtx[conv_n]/F");
  tree->Branch("conv_dphitrksatecal",&conv_dphitrksatecal,"conv_dphitrksatecal[conv_n]/F");
  tree->Branch("conv_detatrksatecal",&conv_detatrksatecal,"conv_detatrksatecal[conv_n]/F");
  tree->Branch("conv_quality",&conv_quality,"conv_quality[conv_n]/I");
  tree->Branch("conv_dxy",&conv_dxy,"conv_dxy[conv_n]/F"); // will not be filled because this will only be available from 420
  tree->Branch("conv_dz",&conv_dz,"conv_dz[conv_n]/F");    // will not be filled because this will only be available from 420
  tree->Branch("conv_lxy",&conv_lxy,"conv_lxy[conv_n]/F"); // will not be filled because this will only be available from 420
  tree->Branch("conv_lz",&conv_lz,"conv_lz[conv_n]/F");    // will not be filled because this will only be available from 420
  tree->Branch("conv_zofprimvtxfromtrks",&conv_zofprimvtxfromtrks,"conv_zofprimvtxfromtrks[conv_n]/F");
  tree->Branch("conv_nHitsBeforeVtx", "std::vector<std::vector<unsigned short> >", &conv_nHitsBeforeVtx);
  tree->Branch("conv_nSharedHits",&conv_nSharedHits,"conv_nSharedHits[conv_n]/I");

  tree->Branch("conv_validvtx",&conv_validvtx,"conv_validvtx[conv_n]/I");
  tree->Branch("conv_MVALikelihood",&conv_MVALikelihood,"conv_MVALikelihood[conv_n]/I");
  /// vertex quantities 
  tree->Branch("conv_chi2",&conv_chi2,"conv_chi2[conv_n]/F");
  tree->Branch("conv_chi2_probability",&conv_chi2_probability,"conv_chi2_probability[conv_n]/F");
  tree->Branch("conv_vtx_xErr",&conv_vtx_xErr,"conv_vtx_xErr[conv_n]/F");
  tree->Branch("conv_vtx_yErr",&conv_vtx_yErr,"conv_vtx_yErr[conv_n]/F");
  tree->Branch("conv_vtx_zErr",&conv_vtx_zErr,"conv_vtx_zErr[conv_n]/F");

  /// per track quantities
  tree->Branch("conv_tk1_dz",&conv_tk1_dz,"conv_tk1_dz[conv_n]/F");
  tree->Branch("conv_tk2_dz",&conv_tk2_dz,"conv_tk2_dz[conv_n]/F");
  tree->Branch("conv_tk1_dzerr",&conv_tk1_dzerr,"conv_tk1_dzerr[conv_n]/F");
  tree->Branch("conv_tk2_dzerr",&conv_tk2_dzerr,"conv_tk2_dzerr[conv_n]/F");
  tree->Branch("conv_tk1_nh",&conv_tk1_nh,"conv_tk1_nh[conv_n]/I");
  tree->Branch("conv_tk2_nh",&conv_tk2_nh,"conv_tk2_nh[conv_n]/I");
  tree->Branch("conv_ch1ch2",&conv_ch1ch2,"conv_ch1ch2[conv_n]/I");
  tree->Branch("conv_tk1_d0",&conv_tk1_d0,"conv_tk1_d0[conv_n]/F");
  tree->Branch("conv_tk1_pout",&conv_tk1_pout,"conv_tk1_pout[conv_n]/F");
  tree->Branch("conv_tk1_pin",&conv_tk1_pin,"conv_tk1_pin[conv_n]/F");
  tree->Branch("conv_tk2_d0",&conv_tk2_d0,"conv_tk2_d0[conv_n]/F");
  tree->Branch("conv_tk2_pout",&conv_tk2_pout,"conv_tk2_pout[conv_n]/F");
  tree->Branch("conv_tk2_pin",&conv_tk2_pin,"conv_tk2_pin[conv_n]/F");





  conv_vtx = new TClonesArray("TVector3", MAX_CONVERTEDPHOTONS);
  tree->Branch("conv_vtx", "TClonesArray", &conv_vtx, 32000, 0);
  conv_pair_momentum = new TClonesArray("TVector3", MAX_CONVERTEDPHOTONS);
  tree->Branch("conv_pair_momentum", "TClonesArray", &conv_pair_momentum, 32000, 0);
  conv_refitted_momentum = new TClonesArray("TVector3", MAX_CONVERTEDPHOTONS);
  tree->Branch("conv_refitted_momentum", "TClonesArray", &conv_refitted_momentum, 32000, 0);

}

bool GlobeConversions::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {

  if (debug_level > 9) 
    {
      std::cout << "GlobeConversions: Start analyze" << std::endl;
    }
  // get collections
  edm::Handle<reco::ConversionCollection> convH;
  iEvent.getByLabel(allConversionsColl, convH);

  if (debug_level > 9) {
    std::cout << "GlobeConversions: Start analyze" << std::endl;
  }

  edm::Handle<reco::SuperClusterCollection> superClustersHybridH; 
  edm::Handle<reco::SuperClusterCollection> superClustersEndcapH; 
  edm::Handle<reco::BasicClusterShapeAssociationCollection> hybridClusterShapeBarrelH; 
  edm::Handle<reco::BasicClusterShapeAssociationCollection> basicClusterShapeEndcapH; 

  iEvent.getByLabel(barrelHybridClusterShapeColl, hybridClusterShapeBarrelH);
  iEvent.getByLabel(endcapBasicClusterShapeColl, basicClusterShapeEndcapH);

  iEvent.getByLabel(hybridSuperClusterColl,superClustersHybridH);
  iEvent.getByLabel(endcapSuperClusterColl, superClustersEndcapH);

  if (debug_level > 9) {
    std::cout << "GlobeConversions: allConversions collection size: "<< convH->size() << std::endl;
  }


  // now have collections
  conv_p4->Clear();
  conv_vtx->Clear();
  conv_pair_momentum->Clear();
  conv_refitted_momentum->Clear();
  conv_nHitsBeforeVtx->clear();
   
  conv_n = 0;

  if(debug_level>9)std::cout << "GlobeConversions: allConversions" << std::endl;

  for( reco::ConversionCollection::const_iterator  iConv = convH->begin(); iConv != convH->end(); iConv++) {
    if (conv_n >= MAX_CONVERTEDPHOTONS) {
      std::cout << "GlobeConversions: WARNING TOO MANY CONVERSIONS: " << convH->size() << " (allowed " << MAX_CONVERTEDPHOTONS << ")" << std::endl;
      break;
    }
    reco::Conversion localConv = reco::Conversion(*iConv);

    if(gCUT->cut(localConv)) continue;

    new ((*conv_p4)[conv_n]) TLorentzVector();
    new ((*conv_vtx)[conv_n]) TVector3();
    new ((*conv_pair_momentum)[conv_n]) TVector3();
    new ((*conv_refitted_momentum)[conv_n]) TVector3();

    if(debug_level>9)std::cout << "GlobeConversions: -21 "<< std::endl;

    ((TLorentzVector *)conv_p4->At(conv_n))->SetXYZT(localConv.refittedPair4Momentum().px(), localConv.refittedPair4Momentum().py(), localConv.refittedPair4Momentum().pz(), localConv.refittedPair4Momentum().energy());

    if(debug_level>9)
      std::cout << "GlobeConversions: -22 "<< std::endl;

    //GlobalPoint pSc(localPho.superCluster()->position().x(),  localPho.superCluster()->position().y(), localPho.superCluster()->position().z());
    if(debug_level>9)std::cout << "GlobeConversions: -23 "<< std::endl;

    conv_scind[conv_n] = -1;
    if(debug_level>9)std::cout << "GlobeConversions: -3 "<< std::endl;

    int index = 0;
    if ( localConv.caloCluster().size() ) {

      if(debug_level>9)std::cout << "GlobeConversions: 0 "<< std::endl;
      for(int isuperClusterType=0; isuperClusterType<3; ++isuperClusterType) {
        if (isuperClusterType == 0) {
          for(reco::SuperClusterCollection::size_type j = 0; j<superClustersHybridH->size(); ++j){
	    
            reco::SuperClusterRef sc(superClustersHybridH, j);
	    
            //apply cuts
            if(gCUT->cut(*sc))continue;
            //passed cuts
            if (  sc.id() == localConv.caloCluster()[0].id() && sc.key() == localConv.caloCluster()[0].key() ) {
              conv_scind[conv_n] = index;
              break;
            }
            index++;
          }
        }
	
        if (isuperClusterType == 2) {
          for(reco::SuperClusterCollection::size_type j = 0; j<superClustersEndcapH->size(); ++j){
	    
            reco::SuperClusterRef sc(superClustersEndcapH, j);
            //apply cuts
            if(gCUT->cut(*sc))continue;
            //passed cuts
	    
            if ( sc.id() == localConv.caloCluster()[0].id() && sc.key() == localConv.caloCluster()[0].key() ) {
              conv_scind[conv_n] = index;
              break;
            }
            index++;
          }
        }
      }
    }

    conv_ntracks[conv_n]=0;
    conv_pairinvmass[conv_n]=-999.;
    conv_paircotthetasep[conv_n]=-999.;
    conv_eoverp[conv_n]=-999.;
    conv_distofminapproach[conv_n]=-999.;
    conv_dphitrksatvtx[conv_n]=-999.;
    conv_dphitrksatecal[conv_n]=-999.;
    conv_detatrksatecal[conv_n]=-999.;
    conv_quality[conv_n]=-999.;
    conv_dxy[conv_n]=-999.;
    conv_dz[conv_n]=-999.;
    conv_lxy[conv_n]=-999.;
    conv_lz[conv_n]=-999.;
    conv_nSharedHits[conv_n]=0;
    conv_zofprimvtxfromtrks[conv_n]=-999.;
    conv_tk1_d0[conv_n]=-999.;
    conv_tk1_pout[conv_n]=-999.;
    conv_tk1_pin[conv_n]=-999.;
    conv_tk2_d0[conv_n]=-999.;
    conv_tk2_pout[conv_n]=-999.;
    conv_tk2_pin[conv_n]=-999.;
    conv_validvtx[conv_n]=0;
    conv_MVALikelihood[conv_n]=-999.;
    conv_ch1ch2[conv_n] = -999.;
    conv_tk1_dz[conv_n]=-999.;
    conv_tk1_dzerr[conv_n]=-999.;
    conv_tk2_dz[conv_n]=-999.;
    conv_tk2_dzerr[conv_n]=-999.;
    conv_vtx_xErr[conv_n]=-999.;
    conv_vtx_yErr[conv_n]=-999.;
    conv_vtx_zErr[conv_n]=-999.;

    ((TVector3 *)conv_vtx->At(conv_n))->SetXYZ(-999, -999, -999);
    ((TVector3 *)conv_pair_momentum->At(conv_n))->SetXYZ(-999, -999, -999);
    ((TVector3 *)conv_refitted_momentum->At(conv_n))->SetXYZ(-999, -999, -999);

    if (debug_level>9) std::cout << "Looking For Valid Conversion" << std::endl;
    if (debug_level>9) std::cout << "Checking Vertex Validity" << std::endl;

    conv_validvtx[conv_n]=localConv.conversionVertex().isValid();
    if ( !localConv.conversionVertex().isValid() ) continue;
    reco::Vertex vtx=localConv.conversionVertex();
    
    
    ((TVector3 *) conv_vtx->At(conv_n))->SetXYZ(vtx.x(), vtx.y(), vtx.z());
    ((TVector3 *) conv_pair_momentum->At(conv_n))->SetXYZ(localConv.pairMomentum().x(), localConv.pairMomentum().y(), localConv.pairMomentum().z());
    ((TVector3 *) conv_refitted_momentum->At(conv_n))->SetXYZ(localConv.refittedPairMomentum().x(), localConv.refittedPairMomentum().y(), localConv.refittedPairMomentum().z());


    conv_chi2[conv_n]=vtx.chi2();
    conv_chi2_probability[conv_n]=ChiSquaredProbability(vtx.chi2(), vtx.ndof());
    conv_vtx_xErr[conv_n]= vtx.xError();
    conv_vtx_yErr[conv_n]= vtx.yError();
    conv_vtx_zErr[conv_n]= vtx.zError();
    conv_ntracks[conv_n]=localConv.nTracks();
    conv_MVALikelihood[conv_n]=localConv.MVAout();


    if( localConv.nTracks()) {
      const std::vector<edm::RefToBase<reco::Track> > tracks = localConv.tracks();
      for (unsigned int i=0; i<tracks.size(); i++) {
        if(i==0) {
          conv_tk1_dz[conv_n]=tracks[i]->dz();
          conv_tk1_dzerr[conv_n]=tracks[i]->dzError();
          conv_ch1ch2[conv_n]=tracks[i]->charge();
        }
        else if(i==1) {
          conv_tk2_dz[conv_n]=tracks[i]->dz();
          conv_tk2_dzerr[conv_n]=tracks[i]->dzError();
          conv_ch1ch2[conv_n]*=tracks[i]->charge();
        }
      }
    }

    
    conv_pairinvmass[conv_n]=localConv.pairInvariantMass();
    conv_paircotthetasep[conv_n]=localConv.pairCotThetaSeparation();
    // will work in 420 conv_eoverp[conv_n]=localConv.EoverPrefittedTracks();
    conv_zofprimvtxfromtrks[conv_n]=localConv.zOfPrimaryVertexFromTracks();
    conv_distofminapproach[conv_n]=localConv.distOfMinimumApproach();
    conv_dphitrksatvtx[conv_n]=localConv.dPhiTracksAtVtx();
    //conv_dphitrksatecal[conv_n]=localConv.dPhiTracksAtEcal();
    //conv_detatrksatecal[conv_n]=localConv.dEtaTracksAtEcal();
    //commented out for now since these will only be available in cmssw_420
    conv_dxy[conv_n]=localConv.dxy();
    conv_dz[conv_n]=localConv.dz();
    conv_lxy[conv_n]=localConv.lxy();
    conv_lz[conv_n]=localConv.lz();

    int flag=-999;
    if (localConv.quality(reco::Conversion::arbitratedMerged)) flag= 1;
    if (localConv.quality(reco::Conversion::generalTracksOnly)) flag =2;
    if (localConv.quality(reco::Conversion::arbitratedEcalSeeded)) flag=3;
    conv_quality[conv_n]=flag;
    
    std::vector<unsigned short> tmp;
    for (unsigned int i=0; i<localConv.nHitsBeforeVtx().size(); ++i) {
      tmp.push_back(static_cast<unsigned short>(localConv.nHitsBeforeVtx()[i]));
    }

    conv_nHitsBeforeVtx->push_back(tmp);
    conv_nSharedHits[conv_n] = localConv.nSharedHits();

    if(localConv.tracks().size() > 0) {
      conv_tk1_d0[conv_n]=localConv.tracksSigned_d0()[0];
      conv_tk1_pout[conv_n]=sqrt(localConv.tracksPout()[0].Mag2());
      conv_tk1_pin[conv_n]=sqrt(localConv.tracksPin()[0].Mag2());
    }
    
    if(localConv.tracks().size() > 1) {
      conv_tk2_d0[conv_n]=localConv.tracksSigned_d0()[1];
      conv_tk2_pout[conv_n]=sqrt(localConv.tracksPout()[1].Mag2());
      conv_tk2_pin[conv_n]=sqrt(localConv.tracksPin()[1].Mag2());
    }
    
    conv_n++;
  }


  if(debug_level>9)
    std::cout << "End Conversion" << std::endl;

  return true;
}
