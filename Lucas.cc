#include <sys/times.h>

// Include ROOT first to avoid shadowing 's' declaration warning due to ROOT/Geant4 names conflict
#include "LCRootOut.hh"

#include "LCDetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "QGSP_BERT.hh"
#include "LCRunAction.hh"
#include "LCEventAction.hh"

#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include <G4HadronicProcessStore.hh>
#include <G4ProductionCutsTable.hh>

int main(int argc, char** argv){
    tms fStartTime;
    clock_t StartTime = times(&fStartTime); // in miliseconds 

    G4UIExecutive* ui = 0;
    if(argc == 1) ui = new G4UIExecutive(argc, argv);

    G4RunManager *runManager = new G4RunManager;

    // Detectron construction and physics list
    runManager->SetUserInitialization(new LCDetectorConstruction);

    G4VUserPhysicsList *physicsList = new QGSP_BERT(0);
    physicsList->SetDefaultCutValue(0.005*mm);

    runManager->SetUserInitialization(physicsList);
 
    // User Action Classes
    runManager->SetUserAction(new PrimaryGeneratorAction);

    LCRootOut *theRootOut = 0;
    LCRunAction *theRunAction = 0;
    LCEventAction *theEventAction = 0;

    if(!ui){
        // batch mode
        theRootOut = new LCRootOut();
        theRunAction = new LCRunAction(theRootOut);
        theEventAction = new LCEventAction(theRootOut);
    }
    else{
        // interactive mode. Cant call ROOT in visualisation, or its crashes on BeamOn
        theRunAction = new LCRunAction();
        theEventAction = new LCEventAction();
    }


    runManager->SetUserAction(theRunAction);
    runManager->SetUserAction(theEventAction);

    G4VisManager *visManager = new G4VisExecutive("0");
    visManager->Initialise();

    G4HadronicProcessStore::Instance()->SetVerbose(0); 
    G4ProductionCutsTable::GetProductionCutsTable()->SetVerboseLevel(0);

    G4UImanager *uiManager = G4UImanager::GetUIpointer();

    if(!ui){
        // batch mode
        G4String command = "/control/execute ";
        G4String fileName = argv[1];
        uiManager->ApplyCommand(command+fileName);
    }
    else{
        // interactive mode
        uiManager->ApplyCommand("/control/macroPath /home/FoxWise/Documents/FCAL/LUCAS/");
        uiManager->ApplyCommand("/control/execute init_vis.mac");
        ui->SessionStart();
        delete ui;
    }


    delete visManager;
    delete runManager;

    time_t now;
    time(&now);
    tms fEndTime;
    
    clock_t EndTime = times(&fEndTime);
    G4double diff = 10.*( EndTime - StartTime )*ms ; 

    return 0;
}
