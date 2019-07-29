// LumiCal
//      sensor stracture from front to back  :
//          1. 0.150 kapton front.
//          1.2. 0.010 epoxy glue
//          2.1. 0.020 AL on sensor
//          2.2. 0.320 Si sensor
//          2.3. 0.020 AL on sensor
//          3.1. 0.040 epoxy (condactive - not take into acount)
//          3.2. 0.025 cupper (on kapton)
//          3.3. 0.065 kapton back
//          3.4. 0.020 epoxy glue
//          total 0.670  

#include "LCDetectorConstruction.hh"
#include <iostream>
#include <sstream>
#include <string>
#include "globals.hh"
#include <G4VisAttributes.hh>

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Polycone.hh"
#include "G4Polyhedra.hh"
#include "G4EllipticalTube.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4PVParameterised.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"

#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "G4SDManager.hh"
#include "G4Region.hh"
#include "G4ProductionCuts.hh"

#include "G4Colour.hh"

#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"
#include "G4NistManager.hh"
#include "G4ios.hh"

#include "G4GeometryTolerance.hh"
#include "G4GeometryManager.hh"

#include "G4UserLimits.hh"

#include "G4SystemOfUnits.hh"
//  logicMetalV->SetVisAttributes( G4VisAttributes::Invisible );


LCDetectorConstruction::LCDetectorConstruction()
    :logicWorld(0),
    physicWorld(0),
    logicSlot(0),
    logicSensor(0),
    logicAbsorber(0),
    logicFanoutFront(0),
    logicFanoutBack(0),
    logicAl(0),
    logicSi(0){}

LCDetectorConstruction::~LCDetectorConstruction(){}

G4VPhysicalVolume* LCDetectorConstruction::Construct(){
    World();
    Absorber();
    Sensor();
    Slot();

    // SENSITIVE DETECTOR
    G4SDManager *SDman = G4SDManager::GetSDMpointer();

    // Initialize the Sensitive Detector
    G4double cellPitch = (rSensorMax - rSensorMin - 2.*rSensorGap)/64.;
    G4double cell0Rad = rSensorMin + rSensorGap + cellPitch/2.;
    G4double phiSector = 7.5*deg;
    SensDet = new LCSensitiveDetector("LumiCalSD",  // name
                                      "LCHitsCollectionName", //This name connected with LCEventAction file EndofEvent extraction. Dont change it
                                      cell0Rad,    // inner LC radius
                                      cellPitch,    // radial cell size
                                      phiSector,    // angular cell width
                                      64);       // # cells in the rad dir

    // Cells are the sensitive detectors
    SDman->AddNewDetector(SensDet);
   
    logicSi->SetSensitiveDetector(SensDet);

    return physicWorld;
}


void LCDetectorConstruction::World(){
    //G4PVPlacement constructor: rotation, origin, logic, name, mother, binary, copy
    G4double xWorldSize = 1000.*mm;
    G4double yWorldSize = 1000.*mm;
    G4double zWorldSize = 10000.*mm;

    Air = materials->FindOrBuildMaterial("G4_AIR");

    G4Box *solidWorld = new G4Box("World", xWorldSize, yWorldSize, zWorldSize);
    logicWorld = new G4LogicalVolume(solidWorld, Air, "World", 0, 0, 0);
    physicWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 1, 1);
    G4VisAttributes* colorWorld = new G4VisAttributes(G4Color(1., 1., 1., 0.1));
    logicWorld->SetVisAttributes(colorWorld);
}

void LCDetectorConstruction::Absorber(){
    // For simulation PL absorber is used (W:95%, Ni: 2.5%, Cu: 2.5%)
    xAbsorberSize = 70.0*mm;
    yAbsorberSize = 70.0*mm;
    zAbsorberSize = 1.75*mm;
    
    W = materials->FindOrBuildMaterial("G4_W");
    Ni =materials->FindOrBuildMaterial("G4_Ni");
    Cu = materials->FindOrBuildMaterial("G4_Cu");

    matAbsorber = new G4Material("matAbsorber", 18.0*g/cm3, 3);
    matAbsorber->AddMaterial(W,0.95); 
    matAbsorber->AddMaterial(Ni,0.025); 
    matAbsorber->AddMaterial(Cu,0.025);  

    G4Box *solidAbsorber = new G4Box("solidAbsorber", xAbsorberSize, yAbsorberSize, zAbsorberSize);
    logicAbsorber = new G4LogicalVolume(solidAbsorber, matAbsorber, "logicAbsorber", 0, 0, 0); 

    G4VisAttributes* absColor = new G4VisAttributes(G4Color(0., 0., 1.));
    logicAbsorber->SetVisAttributes(absColor);

}

void LCDetectorConstruction::Sensor(){
    xSensorSize = 70.0*mm;
    ySensorSize = 70.0*mm;
    zSensorSize = 0.395*mm;

    rSensorMin = 80.*mm;
    rSensorMax = 195.2*mm;
    rSensorGap = 1.2*mm;

    //FanoutFront sizes
    G4double zFanoutFrontEpoxy = 0.065*mm;
    G4double zFanoutFrontKapton = 0.050*mm;
    G4double zFanoutFrontCu = 0.035*mm;
    G4double zFanoutFront = 0.5*(zFanoutFrontEpoxy + zFanoutFrontKapton + zFanoutFrontCu);

    //FanoutBack sizes
    G4double zFanoutBackEpoxy = 0.060*mm;
    G4double zFanoutBackKapton = 0.065*mm;
    G4double zFanoutBackCu = 0.025*mm;
    G4double zFanoutBack = 0.5*(zFanoutBackEpoxy + zFanoutBackKapton + zFanoutBackCu);

    //All needed materials for 
    Si = materials->FindOrBuildMaterial("G4_Si");
    Al = materials->FindOrBuildMaterial("G4_Al");
    Carbon = materials->FindOrBuildMaterial("G4_C");
    Kapton = materials->FindOrBuildMaterial("G4_KAPTON");
    Cu = materials->FindOrBuildMaterial("G4_Cu");
    H = new G4Element("Hydrogen", "H", 1., 1.01*g/mole);
    C = new G4Element("Carbon", "C", 6., 12.01*g/mole);
    O = new G4Element("Oxygen", "O", 8., 16.00*g/mole);

    Epoxy = new G4Material("Epoxy",1.3*g/cm3, 3);
    Epoxy->AddElement(H, 0.1310);
    Epoxy->AddElement(C, 0.5357);
    Epoxy->AddElement(O, 0.3333);

    matFiber = new G4Material("matFiber", 1.6*g/cm3, 2);
    matFiber->AddMaterial(Carbon, 0.5);
    matFiber->AddMaterial(Epoxy, 0.5);

    // densities for Fanout material
    G4double densityFanoutFront = (Epoxy->GetDensity()*zFanoutFrontEpoxy/(2.*zFanoutFront) + Kapton->GetDensity()*zFanoutFrontKapton/(2.*zFanoutFront) + 0.5*Cu->GetDensity()*zFanoutFrontCu/(2.*zFanoutFront));
    G4double densityFanoutBack = (Epoxy->GetDensity()*zFanoutBackEpoxy/(2.*zFanoutBack) + Kapton->GetDensity()*zFanoutBackKapton/(2.*zFanoutBack) + 0.5*Cu->GetDensity()*zFanoutBackCu/(2.*zFanoutBack));


    matFanoutFront = new G4Material("matFanoutFront", densityFanoutFront, 3);
    matFanoutFront->AddMaterial(Cu, zFanoutFrontCu/(2.*zFanoutFront));
    matFanoutFront->AddMaterial(Kapton, zFanoutFrontKapton/(2.*zFanoutFront));
    matFanoutFront->AddMaterial(Epoxy, zFanoutFrontEpoxy/(2.*zFanoutFront));

    matFanoutBack = new G4Material("matFanoutBack", densityFanoutBack, 3);
    matFanoutBack->AddMaterial(Cu, zFanoutBackCu/(2.*zFanoutBack));
    matFanoutBack->AddMaterial(Kapton, zFanoutBackKapton/(2.*zFanoutBack));
    matFanoutBack->AddMaterial(Epoxy, zFanoutBackEpoxy/(2.*zFanoutBack));

    G4Box *solidSensor = new G4Box("solidSensor", xSensorSize, ySensorSize, zSensorSize);
    G4Tubs *solidFanoutFront = new G4Tubs("solidFanoutFront", rSensorMin, rSensorMax, zFanoutFront, 75.*deg, 30.*deg);
    G4Tubs *solidFanoutBack = new G4Tubs("solidFanoutBack", rSensorMin, rSensorMax, zFanoutBack, 75.*deg, 30.*deg);
    G4Tubs *solidSi = new G4Tubs("solidSi", rSensorMin+rSensorGap, rSensorMax-rSensorGap, 0.5*0.32*mm, 75.*deg, 30.*deg);
    G4Tubs *solidAl = new G4Tubs("solidAl", rSensorMin+rSensorGap, rSensorMax-rSensorGap, 0.5*0.02*mm, 75.*deg, 30.*deg);


    logicSensor = new G4LogicalVolume(solidSensor, matFiber, "logicSensor", 0, 0, 0);
    logicFanoutFront = new G4LogicalVolume(solidFanoutFront, matFanoutFront, "logicFanoutFront", 0, 0, 0); 
    logicFanoutBack = new G4LogicalVolume(solidFanoutBack, matFanoutBack, "logicFanoutBack", 0, 0, 0); 
    logicSi = new G4LogicalVolume(solidSi, Si, "logicSi", 0, 0, 0);
    logicAl = new G4LogicalVolume(solidAl, Al, "logicAl", 0, 0, 0);

    logicSensor->SetVisAttributes(G4VisAttributes::Invisible);
    logicFanoutFront->SetVisAttributes(G4VisAttributes::Invisible);
    logicFanoutBack->SetVisAttributes(G4VisAttributes::Invisible);
    logicAl->SetVisAttributes(G4VisAttributes::Invisible);
    G4VisAttributes* sensorColor = new G4VisAttributes(G4Color(0.5, 0.5, 0.));

    logicSi->SetVisAttributes(sensorColor);


    G4double zSensorPos = -zSensorSize + zFanoutFront;
    G4double ySensorPos = -(rSensorMin + 0.5*(rSensorMax - rSensorMin));

    new G4PVPlacement(0, G4ThreeVector(0., ySensorPos, zSensorPos), logicFanoutFront, "SensorFanFront", logicSensor, false, 1, 1);
    zSensorPos += zFanoutFront + 0.5*0.02*mm;
    new G4PVPlacement(0, G4ThreeVector(0., ySensorPos, zSensorPos), logicAl, "SensorAlFront", logicSensor, false, 1, 1);
    zSensorPos += (0.5*0.02 + 0.5*0.32)*mm;
    new G4PVPlacement(0, G4ThreeVector(0., ySensorPos, zSensorPos),logicSi, "SensorSi", logicSensor, false, 1, 1);
    zSensorPos += (0.5*0.32 + 0.5*0.02)*mm;
    new G4PVPlacement(0, G4ThreeVector(0., ySensorPos, zSensorPos), logicAl, "SensorAlBack", logicSensor, false, 1, 1);
    zSensorPos += 0.5*0.02*mm + zFanoutBack;
    new G4PVPlacement(0, G4ThreeVector(0., ySensorPos, zSensorPos), logicFanoutBack, "SensorFanBack", logicSensor, false, 1, 1);
}

void LCDetectorConstruction::Slot(){
    xSlotSize = 150.0*mm;
    ySlotSize = 150.0*mm;
    zSlotSize = 2.25*mm;

    Air = materials->FindOrBuildMaterial("G4_AIR");

    G4Box *solidSlot = new G4Box("solidSlot", xSlotSize, ySlotSize, zSlotSize);
    logicSlot = new G4LogicalVolume(solidSlot, Air, "logicSlot", 0, 0, 0);

    logicSlot->SetVisAttributes(G4VisAttributes::Invisible);

    new G4PVPlacement(0, G4ThreeVector(0., 0., -zSlotSize+zAbsorberSize), logicAbsorber, "Absorber", logicSlot, false, 1);
    new G4PVPlacement(0, G4ThreeVector(0., 0., zSlotSize-zSensorSize), logicSensor, "Sensor", logicSlot, false, 1, 1);

    G4double ySlotPos = - 22./64. * (195.2 - 80.)*mm;
    G4double zSlotPos = 3300.*mm;
    for(int i=0; i<20; i++){
        std::ostringstream stringStream;
        stringStream<<"Slot"<<i+1;
        G4String nameSlot = stringStream.str();
        new G4PVPlacement(0, G4ThreeVector(0., ySlotPos, zSlotPos + zSlotSize), logicSlot, nameSlot, logicWorld, false, i+1, 1);
        zSlotPos += 2.*zSlotSize + 0.002*mm;
    }

}