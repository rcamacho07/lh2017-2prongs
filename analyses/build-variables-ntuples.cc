///////////////////////////////////////////////////////////////////////////////////////////////////
//
/// Description to be added...
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "EventMixer.hh"
#include "CmdLine.hh"
#include "PU14.hh"
#include "fastjet/ClusterSequence.hh"
#include "SubstructureVariables.hh"
#include <fstream>

using namespace std;
using namespace fastjet;

int main (int argc, char ** argv) {
  CmdLine cmdline(argc,argv);
  // inputs read from command line
  int nev = cmdline.value<int>("-nev",1);  // first argument: command line option; second argument: default value
  double R = cmdline.value<double>("-R",1.0);

  string outname = cmdline.value<string>("-out");
  
  // some definitions
  JetDefinition jet_def(antikt_algorithm,R);       // the jet definition....

  double jet_rapmax = cmdline.value("-jet.rapmax",  2.5);
  double jet_ptmin  = cmdline.value("-jet.ptmin", 500.0);
  
  // selection of the hardest jets
  Selector sel_hard_jets = SelectorNHardest(2) * SelectorPtMin(jet_ptmin) * SelectorAbsRapMax(jet_rapmax);

  ostringstream header;
  header << "# Used jet definition: " << jet_def.description() << endl;
  
  //-------------------------------------------------------------------------
  // set up substructure tools
  SubstructureVariables subvars(cmdline);
  //TODO: add this once Fredericc has finished playing w the file
  //header << subvars.description() << "#" << endl;
  
  //------------------------------------------------------------------------
  // create mixer that will construct events by mixing hard and pileup
  // events read from files given from command line using 
  // -hard hard_events_file(.gz) -pileup pileup_events_file(.gz)
  EventMixer mixer(&cmdline);
  header << "# EventGeneration: " << mixer.description() << endl;
  header << "#" << endl;
  
  assert(cmdline.all_options_used());

  //------------------------------------------------------------------------
  // prepare the output
  ofstream ostr(outname.c_str());
  ostr << header.str();
  ostr << "#columns: ";
  ostr << "mass_plain mass_loose mass_tight mass_trim ";
  for (const auto & name : {"tau1", "tau2", "1e2", "2e3", "3e3"}){
    for (int beta=1; beta<=2; ++beta){
      for (const auto & level : {"plain", "loose", "tight"}){
        ostr << name << "_beta" << beta << "_" << level << " ";
      }
    }
  }
  ostr << endl;
  
  //------------------------------------------------------------------------
  // loop over events
  int iev = 0;
  int periodic_iev_output=10;
  int nentries = 0;
  while ( mixer.next_event() && iev < nev ) {
     // increment event number    
     iev++;
     
     // extract particles from event 
     vector<PseudoJet> full_event = mixer.particles() ;
     if (iev % periodic_iev_output == 0){
       cout << "Event " << iev << endl;
       if (iev == 15 * periodic_iev_output) periodic_iev_output*=10;
     }
     
     ClusterSequence cs_hard(full_event,jet_def);
     vector<PseudoJet> jets = sel_hard_jets(cs_hard.inclusive_jets());

     // loop over the jets
     for (const PseudoJet &jet : jets){
       subvars.set_jet(jet);
       ++nentries;
       
       // handle masses
       for (int igroom = 0; igroom<4; ++igroom){
         ostr << subvars.m((SubstructureVariables::groom) igroom) << " ";
       }

       // tau1
       for (int ibeta = 0; ibeta<2; ++ibeta){
         for (int igroom = 0; igroom<3; ++igroom){
           ostr << subvars.tau1((SubstructureVariables::groom) igroom,
                                (SubstructureVariables::beta) ibeta) << " ";
         }
       }

       // tau2
       for (int ibeta = 0; ibeta<2; ++ibeta){
         for (int igroom = 0; igroom<3; ++igroom){
           ostr << subvars.tau2((SubstructureVariables::groom) igroom,
                                (SubstructureVariables::beta) ibeta) << " ";
         }
       }

       // 1e2
       for (int ibeta = 0; ibeta<2; ++ibeta){
         for (int igroom = 0; igroom<3; ++igroom){
           ostr << subvars.ecfg_v1_N2((SubstructureVariables::groom) igroom,
                                      (SubstructureVariables::beta) ibeta) << " ";
         }
       }

       // 2e3
       for (int ibeta = 0; ibeta<2; ++ibeta){
         for (int igroom = 0; igroom<3; ++igroom){
           ostr << subvars.ecfg_v2_N3((SubstructureVariables::groom) igroom,
                                      (SubstructureVariables::beta) ibeta) << " ";
         }
       }
       
       // 3e3
       for (int ibeta = 0; ibeta<2; ++ibeta){
         for (int igroom = 0; igroom<3; ++igroom){
           ostr << subvars.ecfg_v3_N3((SubstructureVariables::groom) igroom,
                                      (SubstructureVariables::beta) ibeta) << " ";
         }
       }

       ostr << endl;


     } // end of loop over jets
  } // end of loop over events
  ostr << "#Nentries=" << nentries << endl;
  
  return 0;
}  
