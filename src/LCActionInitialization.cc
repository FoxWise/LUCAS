#include "LCActionInitialization.hh"
#include "LCPrimaryGeneratorAction.hh"
#include "LCRunAction.hh"
#include "LCEventAction.hh"

LCActionInitialization::LCActionInitialization():G4VUserActionInitialization(){}

LCActionInitialization::~LCActionInitialization(){}

void LCActionInitialization::BuildForMaster() const{SetUserAction(new LCRunAction);}

void LCActionInitialization::Build() const{
    SetUserAction(new LCPrimaryGeneratorAction);
    LCRunAction *runAction = new LCRunAction;
    SetUserAction(runAction);
    SetUserAction(new LCEventAction(runAction));
}