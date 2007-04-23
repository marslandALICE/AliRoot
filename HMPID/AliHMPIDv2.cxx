
// **************************************************************************
// * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
// *                                                                        *
// * Author: The ALICE Off-line Project.                                    *
// * Contributors are mentioned in the code where appropriate.              *
// *                                                                        *
// * Permission to use, copy, modify and distribute this software and its   *
// * documentation strictly for non-commercial purposes is hereby granted   *
// * without fee, provided that the above copyright notice appears in all   *
// * copies and that both the copyright notice and this permission notice   *
// * appear in the supporting documentation. The authors make no claims     *
// * about the suitability of this software for any purpose. It is          *
// * provided "as is" without express or implied warranty.                  *
// **************************************************************************


#include "AliHMPIDv2.h"     //class header
#include "AliHMPIDParam.h"  //StepManager()
#include "AliHMPIDHit.h"    //Hits2SDigs(),StepManager()
#include "AliHMPIDDigit.h"  //Digits2Raw(), Raw2SDigits()
#include "AliRawReader.h"  //Raw2SDigits()
#include <TVirtualMC.h>    //StepManager() for gMC
#include <TPDGCode.h>      //StepHistory() 
#include <AliStack.h>      //StepManager(),Hits2SDigits()
#include <AliLoader.h>        //Hits2SDigits()
#include <AliRunLoader.h>     //Hits2SDigits()
#include <AliConst.h>
#include <AliPDG.h>
#include <AliMC.h>            //StepManager()      
#include <AliRun.h>           //CreateMaterials()    
#include <AliMagF.h>          //CreateMaterials()
#include <TGeoManager.h>      //CreateGeometry()
#include <TF1.h>              //DefineOpticalProperties()
#include <TF2.h>              //DefineOpticalProperties()
#include <TLorentzVector.h>   //IsLostByFresnel() 
#include <AliCDBManager.h>    //CreateMaterials()
#include <AliCDBEntry.h>      //CreateMaterials()
 
ClassImp(AliHMPIDv2)    
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::AddAlignableVolumes()const
{
// Associates the symbolic volume name with the corresponding volume path. Interface method from AliModule invoked from AliMC
// Arguments: none
//   Returns: none   
  for(Int_t i=AliHMPIDDigit::kMinCh;i<=AliHMPIDDigit::kMaxCh;i++)
    gGeoManager->SetAlignableEntry(Form("/HMPID/Chamber%i",i),Form("ALIC_1/Hmp_%i",i));           //clm ??? 
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::CreateMaterials()
{
// Definition of available HMPID materials  
// Arguments: none
//   Returns: none    
  AliDebug(1,"Start v2 HMPID.");
    
    //clm update material definition later on from Antonello
    
//data from PDG booklet 2002     density [gr/cm^3] rad len [cm] abs len [cm]    
  Float_t   aAir[4]={12,14,16,36}    ,   zAir[4]={6,7,8,18} ,   wAir[4]={0.000124,0.755267,0.231781,0.012827} , dAir=0.00120479; Int_t nAir=4;//mixture 0.9999999
  Float_t aC6F14[2]={ 12.01 , 18.99} , zC6F14[2]={ 6 , 9}   , wC6F14[2]={6 , 14} , dC6F14=1.68    ; Int_t nC6F14=-2;
  Float_t  aSiO2[2]={ 28.09 , 15.99} ,  zSiO2[2]={14 , 8}   ,  wSiO2[2]={1 ,  2} ,  dSiO2=2.64    ; Int_t  nSiO2=-2; 
  Float_t   aCH4[2]={ 12.01 ,  1.01} ,   zCH4[2]={ 6 , 1}   ,   wCH4[2]={1 ,  4} ,   dCH4=7.17e-4 ; Int_t   nCH4=-2; 
  Float_t   aCsI[2]={132.90 ,126.90} ,   zCsI[2]={55 ,53}   ,   wCsI[2]={1 ,  1} ,   dCsI=0.1     ; Int_t   nCsI=-2; 
  
  Float_t     aRoha= 12.01 ,               zRoha=  6 ,                              dRoha=  0.10 ,   radRoha= 18.80 , absRoha=  86.3/dRoha; //special material- quasi quartz
  Float_t       aCu= 63.55 ,                 zCu= 29 ,                                dCu=  8.96 ,     radCu=  1.43 ,   absCu= 134.9/dCu  ;
  Float_t        aW=183.84 ,                  zW= 74 ,                                 dW= 19.30 ,      radW=  0.35 ,    absW= 185.0/dW   ;
  Float_t       aAl= 26.98 ,                 zAl= 13 ,                                dAl=  2.70 ,     radAl=  8.90 ,   absAl= 106.4/dAl  ;
  Float_t       aAr= 39.94 ,                 zAr= 18 ,                                dAr=  1.396e-3,  radAr=  14.0 ,   absAr= 117.2/dAr  ;   
           
    Int_t   matId=0;                           //tmp material id number
    Int_t   unsens =  0, sens=1;               //sensitive or unsensitive medium
    Int_t   itgfld = gAlice->Field()->Integ(); //type of field intergration 0 no field -1 user in guswim 1 Runge Kutta 2 helix 3 const field along z
    Float_t maxfld = gAlice->Field()->Max();   //max field value
    Float_t tmaxfd = -10.0;                    //max deflection angle due to magnetic field in one step
    Float_t deemax = - 0.2;                    //max fractional energy loss in one step   
    Float_t stemax = - 0.1;                    //mas step allowed [cm]
    Float_t epsil  =   0.001;                  //abs tracking precision [cm]   
    Float_t stmin  = - 0.001;                  //min step size [cm] in continius process transport, negative value: choose it automatically
    
    AliMixture(++matId,"Air"  ,aAir  ,zAir  ,dAir  ,nAir  ,wAir  ); AliMedium(kAir  ,"Air"  ,matId, unsens, itgfld, maxfld, tmaxfd, stemax, deemax, epsil, stmin);
    AliMixture(++matId,"C6F14",aC6F14,zC6F14,dC6F14,nC6F14,wC6F14); AliMedium(kC6F14,"C6F14",matId, unsens, itgfld, maxfld, tmaxfd, stemax, deemax, epsil, stmin);      
    AliMixture(++matId,"SiO2" ,aSiO2 ,zSiO2 ,dSiO2 ,nSiO2 ,wSiO2 ); AliMedium(kSiO2 ,"SiO2" ,matId, unsens, itgfld, maxfld, tmaxfd, stemax, deemax, epsil, stmin);    
    AliMixture(++matId,"CH4"  ,aCH4  ,zCH4  ,dCH4  ,nCH4  ,wCH4  ); AliMedium(kCH4  ,"CH4"  ,matId, unsens, itgfld, maxfld, tmaxfd, stemax, deemax, epsil, stmin);  
    AliMixture(++matId,"CsI"  ,aCsI  ,zCsI  ,dCsI  ,nCsI  ,wCsI  ); AliMedium(kCsI  ,"CsI"  ,matId,   sens, itgfld, maxfld, tmaxfd, stemax, deemax, epsil, stmin);//sensitive
  
    AliMixture(++matId ,"Neo" ,aSiO2 ,zSiO2 ,dSiO2 ,nSiO2 ,wSiO2 ); AliMedium(kNeo,"Neo" , matId, unsens, itgfld, maxfld, tmaxfd, stemax, deemax, epsil, stmin); //clm neoceram
    AliMaterial(++matId,"Roha",aRoha,zRoha,dRoha,radRoha,absRoha);  AliMedium(kRoha,"Roha", matId, unsens, itgfld, maxfld, tmaxfd, stemax, deemax, epsil, stmin); //Roha->honeycomb

    
    AliMaterial(++matId,"Cu"  ,aCu  ,zCu  ,dCu  ,radCu  ,absCu  );  AliMedium(kCu  ,"Cu"  , matId, unsens, itgfld, maxfld, tmaxfd, stemax, deemax, epsil, stmin);
    AliMaterial(++matId,"W"   ,aW   ,zW   ,dW   ,radW   ,absW   );  AliMedium(kW   ,"W"   , matId, unsens, itgfld, maxfld, tmaxfd, stemax, deemax, epsil, stmin);
    AliMaterial(++matId,"Al"  ,aAl  ,zAl  ,dAl  ,radAl  ,absAl  );  AliMedium(kAl  ,"Al"  , matId, unsens, itgfld, maxfld, tmaxfd, stemax, deemax, epsil, stmin);
    AliMaterial(++matId,"Ar"  ,aAr  ,zAr  ,dAr  ,radAr  ,absAr  );  AliMedium(kAr  ,"Ar"  , matId, unsens, itgfld, maxfld, tmaxfd, stemax, deemax, epsil, stmin);
    
    DefineOpticalProperties();
}//void AliHMPID::CreateMaterials()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::CreateGeometry()
{
//Creates detailed geometry simulation (currently GEANT volumes tree)         
  AliDebug(1,"Start main.");
  if(!gMC->IsRootGeometrySupported()) return;                
 
 Double_t cm=1,mm=0.1*cm,um=0.001*mm;//default is cm
 
  TGeoMedium *al   =gGeoManager->GetMedium("HMPID_Al");    
  TGeoMedium *ch4  =gGeoManager->GetMedium("HMPID_CH4");    
  TGeoMedium *roha =gGeoManager->GetMedium("HMPID_Roha");   
  TGeoMedium *neoc =gGeoManager->GetMedium("HMPID_Neo");
  TGeoMedium *c6f14=gGeoManager->GetMedium("HMPID_C6F14");  
  TGeoMedium *sio2 =gGeoManager->GetMedium("HMPID_SiO2");   
  TGeoMedium *cu   =gGeoManager->GetMedium("HMPID_Cu");     
  TGeoMedium *w    =gGeoManager->GetMedium("HMPID_W");      
  TGeoMedium *csi  =gGeoManager->GetMedium("HMPID_CsI");    
  TGeoMedium *ar   =gGeoManager->GetMedium("HMPID_Ar");     

  TGeoVolume *hmp=gGeoManager->MakeBox ("Hmp",ch4,1681*mm/2, 1466*mm/2,(2*80*mm+2*41*mm)/2);//2033P1  z from p84 TDR  

  TString title=GetTitle();
  if(title.Contains("TestBeam")  )
      {
        gGeoManager->GetVolume("ALIC")->AddNode(hmp,0,new TGeoTranslation(5.0*mm/2 ,  5.0*mm/2, 1000.0*mm));
      }
  else
    {
      for(Int_t iCh=AliHMPIDDigit::kMinCh;iCh<=AliHMPIDDigit::kMaxCh;iCh++){//place 7 chambers
      TGeoHMatrix *pMatrix=new TGeoHMatrix;
      AliHMPIDParam::IdealPosition(iCh,pMatrix);
      gGeoManager->GetVolume("ALIC")->AddNode(hmp,iCh,pMatrix);
       }
     }

  TGeoRotation *rot=new TGeoRotation("HwireRot"); rot->RotateY(90); //rotate wires around Y to be along X (initially along Z)
  TGeoVolume *sbo=gGeoManager->MakeBox ("Hsbo",ch4  , 1419*mm/2 , 1378.00*mm/2 ,   50.5*mm/2);//2072P1
  TGeoVolume *cov=gGeoManager->MakeBox ("Hcov",al   , 1419*mm/2 , 1378.00*mm/2 ,    0.5*mm/2);  
  TGeoVolume *hon=gGeoManager->MakeBox ("Hhon",roha , 1359*mm/2 , 1318.00*mm/2 ,   49.5*mm/2);  
  TGeoVolume *rad=gGeoManager->MakeBox ("Hrad",c6f14, 1330*mm/2 ,  413.00*mm/2 ,   24.0*mm/2); //2011P1
  TGeoVolume *neo=gGeoManager->MakeBox ("Hneo",neoc , 1330*mm/2 ,  413.00*mm/2 ,    4.0*mm/2); 
  TGeoVolume *win=gGeoManager->MakeBox ("Hwin",sio2 , 1330*mm/2 ,  413.00*mm/2 ,    5.0*mm/2); 
  TGeoVolume *si1=gGeoManager->MakeBox ("Hsi1",sio2 , 1330*mm/2 ,    5.00*mm/2 ,   15.0*mm/2);    
  TGeoVolume *si2=gGeoManager->MakeBox ("Hsi2",neoc ,   10*mm/2 ,  403.00*mm/2 ,   15.0*mm/2);    
  TGeoVolume *spa=gGeoManager->MakeTube("Hspa",sio2 ,    0*mm   ,    5.00*mm   ,   15.0*mm/2);         
  TGeoVolume *fr4=gGeoManager->MakeBox ("Hfr4",ch4  , 1407*mm/2 , 1366.00*mm/2 ,   15.0*mm/2);//2043P1 
  TGeoVolume *f4a=gGeoManager->MakeBox ("Hf4a",al   , 1407*mm/2 , 1366.00*mm/2 ,   10.0*mm/2); 
  TGeoVolume *f4i=gGeoManager->MakeBox ("Hf4i",ch4  , 1323*mm/2 , 1296.00*mm/2 ,   10.0*mm/2); 
  TGeoVolume *col=gGeoManager->MakeTube("Hcol",cu   ,    0*mm   ,  100.00*um   , 1323.0*mm/2);
  TGeoVolume *sec=gGeoManager->MakeBox ("Hsec",ch4  ,  648*mm/2 ,  411.00*mm/2 ,   45.5*mm/2);//sec=gap+ppf
  TGeoVolume *ppf=gGeoManager->MakeBox ("Hppf",al   ,  648*mm/2 ,  411.00*mm/2 ,   40.0*mm/2);//2001P2
  TGeoVolume *lar=gGeoManager->MakeBox ("Hlar",ar   ,  181*mm/2 ,   89.25*mm/2 ,   38.3*mm/2);//2001P2
  TGeoVolume *smo=gGeoManager->MakeBox ("Hsmo",ar   ,  114*mm/2 ,   89.25*mm/2 ,   38.3*mm/2);//2001P2
  TGeoVolume *gap=gGeoManager->MakeBox ("Hgap",ch4  ,  640*mm/2 ,  403.20*mm/2 ,    5.5*mm/2);//gap=pad+ano+cat
  TGeoVolume *cat=gGeoManager->MakeTube("Hcat",cu   ,    0*mm   ,   50.00*um   ,    8.0*mm/2); 
  TGeoVolume *ano=gGeoManager->MakeTube("Hano",w    ,    0*mm   ,   20.00*um   ,    8.0*mm/2); 
  TGeoVolume *pad=gGeoManager->MakeBox ("Hpad",csi  ,    8*mm/2 ,    8.40*mm/2 ,    1.0*mm/2);      
//
// ^ Y   z=         z=-12mm      z=98.25mm               ALIC->7xHmp (virtual)-->1xHsbo (virtual) --->2xHcov (real) 2072P1
// |  ____________________________________                                    |                   |-->1xHhon (real) 2072P1
// | |   ______     ____          ______  |                                   |
//   |  |      |   |    |   *    |      | |                                   |->3xHrad (virtual) --->1xHneo (real) 2011P1
//   |  |50.5mm|   |24mm|   *    |45.5mm| |                                   |                   |-->1xHwin (real) 2011P1
//   |  |      |   |    |   *    |      | |                                   |                   |-->2xHsi1 (real) 2011P1
//   |  |      |   |____|   *    |______| |                                   |                   |-->2xHsi2 (real) 2011P1
//   |  |      |    ____    *     ______  |                                   |                   |->30xHspa (real) 2011P1
//   |  |      |   |    |   *    |      | |                                   |
//   |  |      |   |    |   *    |      | |                                   |->1xHfr4 (vitual) --->1xHf4a (real)---->1xHf4i(virtual) 2043P1 
//   |  |  sb  |   | rad|   *    |      | |                                   |                  |-->322xHcol (real) 2043P1
//   |  |      |   |____|   *    |______| |                                   |
//   |  |      |    ____    *     ______  |                                   |->6xHsec (virtual) --> 1xHppf(real) ---->8xHlar (virtual) 2001P1
//   |  |      |   |    |   *    |      | |                                                                        |--->8xHsmo (virtual) 2001P1     
//   |  |      |   |    |   *    |      | |                                   |               
//   |  |      |   |    |   *    |      | |                                   |-> 1xHgap (virtual) --->48xHrow (virtual) -->80xHcel (virtual) -->4xHcat (real) from p84 TDR 
//   |  |______|   |____|   *    |______| |                                                                                                  |-->2xHano (real) from p84 TDR                                  
//   |____________________________________|                                                                                                  |-->1xHpad (real) from p84 TDR 
//                                                       --->Z 
  hmp->AddNode(sbo ,1,new TGeoTranslation(   0*mm,   0*mm, -73.75*mm));                     //p.84 TDR
     sbo->AddNode(hon ,1,new TGeoTranslation(  0*mm,0*mm,      0*mm)); //2072P1
     sbo->AddNode(cov ,1,new TGeoTranslation(  0*mm,0*mm,    +25*mm)); 
     sbo->AddNode(cov ,2,new TGeoTranslation(  0*mm,0*mm,    -25*mm)); 
  hmp->AddNode(rad,2,new TGeoTranslation(   0*mm,+434*mm, -12.00*mm)); 
  hmp->AddNode(rad,1,new TGeoTranslation(   0*mm,   0*mm, -12.00*mm)); 
  hmp->AddNode(rad,0,new TGeoTranslation(   0*mm,-434*mm, -12.00*mm)); 
    rad->AddNode(neo,1,new TGeoTranslation(   0*mm,   0*mm, -10.0*mm));
    rad->AddNode(win,1,new TGeoTranslation(   0*mm,   0*mm,   9.5*mm));
    rad->AddNode(si1,1,new TGeoTranslation(   0*mm,-204*mm,  -0.5*mm)); rad->AddNode(si1,2,new TGeoTranslation(   0*mm,+204*mm,  -0.5*mm));
    rad->AddNode(si2,1,new TGeoTranslation(-660*mm,   0*mm,  -0.5*mm)); rad->AddNode(si2,2,new TGeoTranslation(+660*mm,   0*mm,  -0.5*mm));
    for(Int_t i=0;i<3;i++) for(Int_t j=0;j<10;j++) rad->AddNode(spa,10*i+j,new TGeoTranslation(-1330*mm/2+116*mm+j*122*mm,(i-1)*105*mm,-0.5*mm));
  hmp->AddNode(fr4,1,new TGeoTranslation(   0*mm,   0*mm,   9.00*mm));                     //p.84 TDR
  for(int i=1;i<=322;i++)  fr4->AddNode(col,i,new TGeoCombiTrans( 0*mm, -1296/2*mm+i*4*mm,-5*mm,rot)); //F4 2043P1
                           fr4->AddNode(f4a,1,new TGeoTranslation(   0*mm,0*mm, 2.5*mm));    
                                        f4a->AddNode(f4i,1,new TGeoTranslation(   0*mm,0*mm,   0*mm));
  hmp->AddNode(sec,4,new TGeoTranslation(-335*mm,+433*mm,  98.25*mm)); hmp->AddNode(sec,5,new TGeoTranslation(+335*mm,+433*mm,  98.25*mm));
  hmp->AddNode(sec,2,new TGeoTranslation(-335*mm,   0*mm,  98.25*mm)); hmp->AddNode(sec,3,new TGeoTranslation(+335*mm,   0*mm,  98.25*mm));
  hmp->AddNode(sec,0,new TGeoTranslation(-335*mm,-433*mm,  98.25*mm)); hmp->AddNode(sec,1,new TGeoTranslation(+335*mm,-433*mm,  98.25*mm));
    sec->AddNode(gap,1,new TGeoTranslation(0,0,-20.00*mm));
  TGeoVolume *row=          gap->Divide("Hrow",2,48,0,0);//along Y->48 rows
  TGeoVolume *cel=          row->Divide("Hcel",1,80,0,0);//along X->80 cells
      cel->AddNode(cat,1,new TGeoCombiTrans (0,  3.15*mm , -2.70*mm , rot)); //4 cathode wires
      cel->AddNode(ano,1,new TGeoCombiTrans (0,  2.00*mm , -0.29*mm , rot)); //2 anod wires
      cel->AddNode(cat,2,new TGeoCombiTrans (0,  1.05*mm , -2.70*mm , rot)); 
      cel->AddNode(cat,3,new TGeoCombiTrans (0, -1.05*mm , -2.70*mm , rot)); 
      cel->AddNode(ano,2,new TGeoCombiTrans (0, -2.00*mm , -0.29*mm , rot)); 
      cel->AddNode(cat,4,new TGeoCombiTrans (0, -3.15*mm , -2.70*mm , rot));   
      cel->AddNode(pad,1,new TGeoTranslation(0,  0.00*mm ,  2.25*mm));       //1 pad  
    sec->AddNode(ppf,1,new TGeoTranslation(0,0,  2.75*mm));
// ^ Y  single cell                                                5.5mm CH4 = 1*mm CsI + 4.45*mm CsI x cath +0.05*mm safety margin         
// |      ______________________________           
// |     |                              |          ^                            ||     
//       |                              | 1.05mm                                ||     
// 2.2*mm| xxxxxxxxxxxxxxxxxxxxxxxxxxxx |--              50um  x                || cat shift  x=0mm , y= 3.15mm , z=-2.70mm       
//       |                              |                                       ||     
//       |                              |                                       ||     
// __    |  ..........................  | 2.1mm                    20un .       ||  ano shift x=0mm , y= 2.00mm , z=-0.29mm   
//       |                              |                                       ||     
//       |                              |                                       ||     
//       | xxxxxxxxxxxxxxxxxxxxxxxxxxxx |--                    x                ||  cat shift x=0mm , y= 1.05mm , z=-2.70mm   
//       |                              |                                       ||     
//       |                              |         8.4mm                         ||   
// 4*mm  |                              | 2.1mm                                 ||  pad shift x=0mm , y= 0.00mm , z=2.25*mm   
//       |                              |                                       ||  
//       |                              |                                       ||  
//       | xxxxxxxxxxxxxxxxxxxxxxxxxxxx |--                    x                ||  cat shift x=0mm , y=-1.05mm , z=-2.70mm   
//       |                              |                                       ||  
//       |                              |                                       ||    
// __    |  ..........................  | 2.1mm                         . 2.04mm||  ano shift x=0mm , y=-2.00mm , z=-0.29mm   
//       |                              |                                       ||  
//       |                              |                                       ||  
//       | xxxxxxxxxxxxxxxxxxxxxxxxxxxx |--                    x    4.45mm      ||  cat shift x=0mm , y=-3.15mm , z=-2.70mm   
// 2.2*mm|                              |                                       ||  
//       |                              | 1.05mm                                ||         
//       |______________________________|          v                            ||    
//       <             8 mm             >                          
//                                   ----->X                                 ----->Z
  ppf->AddNode(lar,1,new TGeoTranslation(-224.5*mm,-151.875*mm,  0.85*mm));
  ppf->AddNode(lar,2,new TGeoTranslation(-224.5*mm,- 50.625*mm,  0.85*mm));
  ppf->AddNode(lar,3,new TGeoTranslation(-224.5*mm,+ 50.625*mm,  0.85*mm));
  ppf->AddNode(lar,4,new TGeoTranslation(-224.5*mm,+151.875*mm,  0.85*mm));
  ppf->AddNode(lar,5,new TGeoTranslation(+224.5*mm,-151.875*mm,  0.85*mm));
  ppf->AddNode(lar,6,new TGeoTranslation(+224.5*mm,- 50.625*mm,  0.85*mm));
  ppf->AddNode(lar,7,new TGeoTranslation(+224.5*mm,+ 50.625*mm,  0.85*mm));
  ppf->AddNode(lar,8,new TGeoTranslation(+224.5*mm,+151.875*mm,  0.85*mm));
  ppf->AddNode(smo,1,new TGeoTranslation(- 65.0*mm,-151.875*mm,  0.85*mm));
  ppf->AddNode(smo,2,new TGeoTranslation(- 65.0*mm,- 50.625*mm,  0.85*mm));
  ppf->AddNode(smo,3,new TGeoTranslation(- 65.0*mm,+ 50.625*mm,  0.85*mm));
  ppf->AddNode(smo,4,new TGeoTranslation(- 65.0*mm,+151.875*mm,  0.85*mm));
  ppf->AddNode(smo,5,new TGeoTranslation(+ 65.0*mm,-151.875*mm,  0.85*mm));
  ppf->AddNode(smo,6,new TGeoTranslation(+ 65.0*mm,- 50.625*mm,  0.85*mm));
  ppf->AddNode(smo,7,new TGeoTranslation(+ 65.0*mm,+ 50.625*mm,  0.85*mm));
  ppf->AddNode(smo,8,new TGeoTranslation(+ 65.0*mm,+151.875*mm,  0.85*mm)); 



  AliDebug(1,"Stop v2. HMPID option");  
}//CreateGeometry()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::Init()
{
// This methode defines ID for sensitive volumes, i.e. such geometry volumes for which there are if(gMC->CurrentVolID()==XXX) statements in StepManager()
// Arguments: none
//   Returns: none      
  AliDebug(1,"Start v2 HMPID.");    
  fIdPad     = gMC->VolId("Hpad");
  fIdCell    = gMC->VolId("Hcel");
  AliDebug(1,"Stop v2 HMPID.");    
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::DefineOpticalProperties()
{
// Optical properties definition.
  const Int_t kNbins=30;       //number of photon energy points
  Float_t emin=5.5,emax=8.5;         //Photon energy range,[eV]
  Float_t aEckov [kNbins]; 
  Float_t aAbsRad[kNbins], aAbsWin[kNbins], aAbsGap[kNbins], aAbsMet[kNbins];
  Float_t aIdxRad[kNbins], aIdxWin[kNbins], aIdxGap[kNbins], aIdxMet[kNbins], aIdxPc[kNbins]; 
  Float_t                                                    aQeAll [kNbins], aQePc [kNbins];

  TF2 *pRaIF=new TF2("HidxRad","sqrt(1+0.554*(1239.84/x)^2/((1239.84/x)^2-5796)-0.0005*(y-20))"                                       ,emin,emax,0,50); //DiMauro mail temp 0-50 degrees C
  TF1 *pWiIF=new TF1("HidxWin","sqrt(1+46.411/(10.666*10.666-x*x)+228.71/(18.125*18.125-x*x))"                                        ,emin,emax);      //SiO2 idx TDR p.35
  TF1 *pGaIF=new TF1("HidxGap","1+0.12489e-6/(2.62e-4 - x*x/1239.84/1239.84)"                                                         ,emin,emax);      //?????? from where  

  TF1 *pRaAF=new TF1("HabsRad","(x<7.8)*(gaus+gaus(3))+(x>=7.8)*0.0001"                                                               ,emin,emax);  //fit from DiMauro data 28.10.03 
  pRaAF->SetParameters(3.20491e16,-0.00917890,0.742402,3035.37,4.81171,0.626309);
  TF1 *pWiAF=new TF1("HabsWin","(x<8.2)*(818.8638-301.0436*x+36.89642*x*x-1.507555*x*x*x)+(x>=8.2)*0.0001"                            ,emin,emax);  //fit from DiMauro data 28.10.03 
  TF1 *pGaAF=new TF1("HabsGap","(x<7.75)*6512.399+(x>=7.75)*3.90743e-2/(-1.655279e-1+6.307392e-2*x-8.011441e-3*x*x+3.392126e-4*x*x*x)",emin,emax);  //????? from where  
  
  TF1 *pQeF =new TF1("Hqe"    ,"0+(x>6.07267)*0.344811*(1-exp(-1.29730*(x-6.07267)))"                                                 ,emin,emax);  //fit from DiMauro data 28.10.03  
                            
  for(Int_t i=0;i<kNbins;i++){
    Float_t eV=emin+0.1*i;  //Ckov energy in eV
    aEckov [i] =1e-9*eV;    //Ckov energy in GeV
    aAbsRad[i]=pRaAF->Eval(eV); aIdxRad[i]=1.292;//pRaIF->Eval(eV,20);      //Simulation for 20 degress C       
    aAbsWin[i]=pWiAF->Eval(eV); aIdxWin[i]=1.5787;//pWiIF->Eval(eV);
    aAbsGap[i]=pGaAF->Eval(eV); aIdxGap[i]=1.0005;//pGaIF->Eval(eV);   
    aQeAll[i] =1;                     //QE for all other materials except for PC must be 1.  
    aAbsMet[i] =0.0001;                aIdxMet[i]=0;                                             //metal ref idx must be 0 in order to reflect photon
                                       aIdxPc [i]=1;           aQePc [i]=pQeF->Eval(eV);         //PC ref idx must be 1 in order to apply photon to QE conversion 
                                       
  }
  gMC->SetCerenkov((*fIdtmed)[kC6F14]    , kNbins, aEckov, aAbsRad  , aQeAll , aIdxRad );    
  gMC->SetCerenkov((*fIdtmed)[kSiO2]     , kNbins, aEckov, aAbsWin  , aQeAll , aIdxWin );    
  gMC->SetCerenkov((*fIdtmed)[kCH4]      , kNbins, aEckov, aAbsGap  , aQeAll , aIdxGap );    
  gMC->SetCerenkov((*fIdtmed)[kCu]       , kNbins, aEckov, aAbsMet  , aQeAll , aIdxMet );    
  gMC->SetCerenkov((*fIdtmed)[kW]        , kNbins, aEckov, aAbsMet  , aQeAll , aIdxMet ); //n=0 means reflect photons       
  gMC->SetCerenkov((*fIdtmed)[kCsI]      , kNbins, aEckov, aAbsMet  , aQePc  , aIdxPc  ); //n=1 means convert photons    
  gMC->SetCerenkov((*fIdtmed)[kAl]       , kNbins, aEckov, aAbsMet  , aQeAll , aIdxMet );    
  delete pRaAF;delete pWiAF;delete pGaAF; delete pRaIF; delete pWiIF; delete pGaIF; delete pQeF;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Bool_t AliHMPIDv2::IsLostByFresnel()
{
// Calculate probability for the photon to be lost by Fresnel reflection.
  TLorentzVector p4;
  Double_t mom[3],localMom[3];
  gMC->TrackMomentum(p4);   mom[0]=p4(1);   mom[1]=p4(2);   mom[2]=p4(3);
  localMom[0]=0; localMom[1]=0; localMom[2]=0;
  gMC->Gmtod(mom,localMom,2);
  Double_t localTc    = localMom[0]*localMom[0]+localMom[2]*localMom[2];
  Double_t localTheta = TMath::ATan2(TMath::Sqrt(localTc),localMom[1]);
  Double_t cotheta = TMath::Abs(TMath::Cos(localTheta));
  if(gMC->GetRandom()->Rndm() < Fresnel(p4.E()*1e9,cotheta,1)){
    AliDebug(1,"Photon lost");
    return kTRUE;
  }else
    return kFALSE;
}//IsLostByFresnel()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::GenFee(Float_t qtot)
{
// Generate FeedBack photons for the current particle. To be invoked from StepManager().
// eloss=0 means photon so only pulse height distribution is to be analysed.
  TLorentzVector x4;
  gMC->TrackPosition(x4); 
  Int_t iNphotons=gMC->GetRandom()->Poisson(0.02*qtot);  //# of feedback photons is proportional to the charge of hit
  AliDebug(1,Form("N photons=%i",iNphotons));
  Int_t j;
  Float_t cthf, phif, enfp = 0, sthf, e1[3], e2[3], e3[3], vmod, uswop,dir[3], phi,pol[3], mom[4];
//Generate photons
  for(Int_t i=0;i<iNphotons;i++){//feedbacks loop
    Double_t ranf[2];
    gMC->GetRandom()->RndmArray(2,ranf);    //Sample direction
    cthf=ranf[0]*2-1.0;
    if(cthf<0) continue;
    sthf = TMath::Sqrt((1 - cthf) * (1 + cthf));
    phif = ranf[1] * 2 * TMath::Pi();
    
    if(Double_t randomNumber=gMC->GetRandom()->Rndm()<=0.57)
      enfp = 7.5e-9;
    else if(randomNumber<=0.7)
      enfp = 6.4e-9;
    else
      enfp = 7.9e-9;
    

    dir[0] = sthf * TMath::Sin(phif);    dir[1] = cthf;    dir[2] = sthf * TMath::Cos(phif);
    gMC->Gdtom(dir, mom, 2);
    mom[0]*=enfp;    mom[1]*=enfp;    mom[2]*=enfp;
    mom[3] = TMath::Sqrt(mom[0]*mom[0]+mom[1]*mom[1]+mom[2]*mom[2]);
    
    // Polarisation
    e1[0]=      0;    e1[1]=-dir[2];    e1[2]= dir[1];
    e2[0]=-dir[1];    e2[1]= dir[0];    e2[2]=      0;
    e3[0]= dir[1];    e3[1]=      0;    e3[2]=-dir[0];
    
    vmod=0;
    for(j=0;j<3;j++) vmod+=e1[j]*e1[j];
    if (!vmod) for(j=0;j<3;j++) {
      uswop=e1[j];
      e1[j]=e3[j];
      e3[j]=uswop;
    }
    vmod=0;
    for(j=0;j<3;j++) vmod+=e2[j]*e2[j];
    if (!vmod) for(j=0;j<3;j++) {
      uswop=e2[j];
      e2[j]=e3[j];
      e3[j]=uswop;
    }
    
    vmod=0;  for(j=0;j<3;j++) vmod+=e1[j]*e1[j];  vmod=TMath::Sqrt(1/vmod);  for(j=0;j<3;j++) e1[j]*=vmod;    
    vmod=0;  for(j=0;j<3;j++) vmod+=e2[j]*e2[j];  vmod=TMath::Sqrt(1/vmod);  for(j=0;j<3;j++) e2[j]*=vmod;
    
    phi = gMC->GetRandom()->Rndm()* 2 * TMath::Pi();
    for(j=0;j<3;j++) pol[j]=e1[j]*TMath::Sin(phi)+e2[j]*TMath::Cos(phi);
    gMC->Gdtom(pol, pol, 2);
    Int_t outputNtracksStored;    
    gAlice->GetMCApp()->PushTrack(1,                             //transport
                     gAlice->GetMCApp()->GetCurrentTrackNumber(),//parent track 
                     50000051,                                   //PID
		     mom[0],mom[1],mom[2],mom[3],                //track momentum  
                     x4.X(),x4.Y(),x4.Z(),x4.T(),                //track origin 
                     pol[0],pol[1],pol[2],                       //polarization
		     kPFeedBackPhoton,                           //process ID   
                     outputNtracksStored,                        //on return how many new photons stored on stack
                     1.0);                                       //weight
  }//feedbacks loop
  AliDebug(1,"Stop.");
}//GenerateFeedbacks()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::Hits2SDigits()
{
// Interface method ivoked from AliSimulation to create a list of sdigits corresponding to list of hits. Every hit generates one or more sdigits.
// Arguments: none
//   Returns: none   
  AliDebug(1,"Start.");
  for(Int_t iEvt=0;iEvt < GetLoader()->GetRunLoader()->GetNumberOfEvents();iEvt++){                //events loop
    GetLoader()->GetRunLoader()->GetEvent(iEvt);                          //get next event
  
    if(!GetLoader()->TreeH()) {GetLoader()->LoadHits();                    }
    if(!GetLoader()->TreeS()) {GetLoader()->MakeTree("S"); MakeBranch("S");}//to
          
    for(Int_t iEnt=0;iEnt<GetLoader()->TreeH()->GetEntries();iEnt++){//prims loop
      GetLoader()->TreeH()->GetEntry(iEnt);
      Hit2Sdi(Hits(),SdiLst());
    }//prims loop
    GetLoader()->TreeS()->Fill();
    GetLoader()->WriteSDigits("OVERWRITE");
    SdiReset();
  }//events loop  
  GetLoader()->UnloadHits();
  GetLoader()->UnloadSDigits();  
  AliDebug(1,"Stop.");
}//Hits2SDigits()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::Hit2Sdi(TClonesArray *pHitLst,TClonesArray *pSdiLst)
{
// Converts list of hits to list of sdigits. 
// Arguments: pHitLst  - list of hits provided not empty
//            pSDigLst - list of sdigits where to store the results
//   Returns: none         
  for(Int_t iHit=0;iHit<pHitLst->GetEntries();iHit++){         //hits loop
    AliHMPIDHit *pHit=(AliHMPIDHit*)pHitLst->At(iHit);         //get pointer to current hit   
    pHit->Hit2Sdi(pSdiLst);                                    //convert this hit to list of sdigits     
  }//hits loop loop
}//Hits2Sdi()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::Digits2Raw()
{
// Interface method invoked by AliSimulation to create raw data streams from digits. Events loop is done in AliSimulation
// Arguments: none
//   Returns: none    
  AliDebug(1,"Start.");
  GetLoader()->LoadDigits();
  TTree * treeD = GetLoader()->TreeD();
  if(!treeD) {
    AliError("No digits tree!");
    return;
  }
  treeD->GetEntry(0);
  
  AliHMPIDDigit::WriteRaw(DigLst());
    
  GetLoader()->UnloadDigits();
  AliDebug(1,"Stop.");      
}//Digits2Raw()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Float_t AliHMPIDv2::Fresnel(Float_t ene,Float_t pdoti, Bool_t pola)
{
// Correction for Fresnel   ???????????
// Arguments:   ene - photon energy [GeV],
//              PDOTI=COS(INC.ANG.), PDOTR=COS(POL.PLANE ROT.ANG.)
//   Returns:  
    Float_t en[36] = {5.0,5.1,5.2,5.3,5.4,5.5,5.6,5.7,5.8,5.9,6.0,6.1,6.2,
		      6.3,6.4,6.5,6.6,6.7,6.8,6.9,7.0,7.1,7.2,7.3,7.4,7.5,7.6,7.7,
		      7.8,7.9,8.0,8.1,8.2,8.3,8.4,8.5};
    Float_t csin[36] = {2.14,2.21,2.33,2.48,2.76,2.97,2.99,2.59,2.81,3.05,
			2.86,2.53,2.55,2.66,2.79,2.96,3.18,3.05,2.84,2.81,2.38,2.11,
			2.01,2.13,2.39,2.73,3.08,3.15,2.95,2.73,2.56,2.41,2.12,1.95,
			1.72,1.53};
    Float_t csik[36] = {0.,0.,0.,0.,0.,0.196,0.408,0.208,0.118,0.49,0.784,0.543,
	 		0.424,0.404,0.371,0.514,0.922,1.102,1.139,1.376,1.461,1.253,0.878,
			0.69,0.612,0.649,0.824,1.347,1.571,1.678,1.763,1.857,1.824,1.824,
			1.714,1.498};
    Float_t xe=ene;
    Int_t  j=Int_t(xe*10)-49;
    Float_t cn=csin[j]+((csin[j+1]-csin[j])/0.1)*(xe-en[j]);
    Float_t ck=csik[j]+((csik[j+1]-csik[j])/0.1)*(xe-en[j]);

    //FORMULAE FROM HANDBOOK OF OPTICS, 33.23 OR
    //W.R. HUNTER, J.O.S.A. 54 (1964),15 , J.O.S.A. 55(1965),1197

    Float_t sinin=TMath::Sqrt(1-pdoti*pdoti);
    Float_t tanin=sinin/pdoti;

    Float_t c1=cn*cn-ck*ck-sinin*sinin;
    Float_t c2=4*cn*cn*ck*ck;
    Float_t aO=TMath::Sqrt(0.5*(TMath::Sqrt(c1*c1+c2)+c1));
    Float_t b2=0.5*(TMath::Sqrt(c1*c1+c2)-c1);
    
    Float_t rs=((aO-pdoti)*(aO-pdoti)+b2)/((aO+pdoti)*(aO+pdoti)+b2);
    Float_t rp=rs*((aO-sinin*tanin)*(aO-sinin*tanin)+b2)/((aO+sinin*tanin)*(aO+sinin*tanin)+b2);
    

    //CORRECTION FACTOR FOR SURFACE ROUGHNESS
    //B.J. STAGG  APPLIED OPTICS, 30(1991),4113

    Float_t sigraf=18.;
    Float_t lamb=1240/ene;
    Float_t fresn;
 
    Float_t  rO=TMath::Exp(-(4*TMath::Pi()*pdoti*sigraf/lamb)*(4*TMath::Pi()*pdoti*sigraf/lamb));

    if(pola)
    {
	Float_t pdotr=0.8;                                 //DEGREE OF POLARIZATION : 1->P , -1->S
	fresn=0.5*(rp*(1+pdotr)+rs*(1-pdotr));
    }
    else
	fresn=0.5*(rp+rs);
      
    fresn = fresn*rO;
    return fresn;
}//Fresnel()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::Print(Option_t *option)const
{
// Debug printout
  TObject::Print(option);
}//void AliHMPID::Print(Option_t *option)const
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Bool_t AliHMPIDv2::Raw2SDigits(AliRawReader *pRR)
{
// Interface methode ivoked from AliSimulation to create a list of sdigits from raw digits. Events loop is done in AliSimulation
// Arguments: pRR- raw reader 
//   Returns: kTRUE on success (currently ignored in AliSimulation::ConvertRaw2SDigits())      
  AliHMPIDDigit sdi; //tmp sdigit, raw digit will be converted to it
  
  if(!GetLoader()->TreeS()) {MakeTree("S");  MakeBranch("S");}
    
  TClonesArray *pSdiLst=SdiLst(); Int_t iSdiCnt=0; //tmp list of sdigits for all chambers
  pRR->Select("HMPID",0,13);//select all HMPID DDL files
  UInt_t w32=0;
  while(pRR->ReadNextInt(w32)){//raw records loop (in selected DDL files)
    UInt_t ddl=pRR->GetDDLID(); //returns 0,1,2 ... 13
    sdi.Raw(ddl,w32);  
    new((*pSdiLst)[iSdiCnt++]) AliHMPIDDigit(sdi); //add this digit to the tmp list
  }//raw records loop
  GetLoader()->TreeS()->Fill(); GetLoader()->WriteSDigits("OVERWRITE");//write out sdigits
  SdiReset();
  return kTRUE;
}//Raw2SDigits
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::StepCount()
{
// Count number of ckovs created  
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::StepHistory()
{
// This methode is invoked from StepManager() in order to print out 
  static Int_t iStepN;
  const char *sParticle;
  switch(gMC->TrackPid()){
    case kProton:      sParticle="PROTON"    ;break;
    case kNeutron:     sParticle="neutron"   ;break;
    case kGamma:       sParticle="gamma"     ;break;
    case 50000050:     sParticle="CKOV"      ;break;
    case kPi0:         sParticle="Pi0"       ;break;  
    case kPiPlus:      sParticle="Pi+"       ;break;  
    case kPiMinus:     sParticle="Pi-"       ;break;  
    case kElectron:    sParticle="electron"  ;break;  
    default:           sParticle="not known" ;break;
  }

  TString flag="fanny combination";
  if(gMC->IsTrackAlive())
      if(gMC->IsTrackEntering())      flag="enters to";
      else if(gMC->IsTrackExiting())  flag="exits from";
      else if(gMC->IsTrackInside())   flag="inside";
  else
      if(gMC->IsTrackStop())          flag="stoped in";        
  


  
  Int_t vid=0,copy=0;
  TString path=gMC->CurrentVolName(); path.Prepend("-");path.Prepend(gMC->CurrentVolOffName(1));//current volume and his mother are always there
  vid=gMC->CurrentVolOffID(2,copy);  if(vid) {path.Prepend("-");path.Prepend(gMC->VolName(vid));}
  vid=gMC->CurrentVolOffID(3,copy);  if(vid) {path.Prepend("-");path.Prepend(gMC->VolName(vid));}
 
  
  Printf("Step %i: %s (%i) %s %s m=%.6f GeV q=%.1f dEdX=%.4f Etot=%.4f",iStepN,sParticle,gMC->TrackPid(),flag.Data(),path.Data(),gMC->TrackMass(),gMC->TrackCharge(),gMC->Edep()*1e9,gMC->Etot());
  
  Double_t gMcTrackPos[3]; gMC->TrackPosition(gMcTrackPos[0],gMcTrackPos[1],gMcTrackPos[2]);
  Double_t  gMcTrackPosLoc[3]; gMC->Gmtod(gMcTrackPos,gMcTrackPosLoc,1);
  Printf("gMC Track Position (MARS) x: %5.3lf, y: %5.3lf, z: %5.3lf (r: %5.3lf) ---> (LOC) x: %5.3f, y: %5.3f, z: %5.3f",gMcTrackPos[0],gMcTrackPos[1],gMcTrackPos[2],TMath::Sqrt(gMcTrackPos[0]*gMcTrackPos[0]+gMcTrackPos[1]*gMcTrackPos[1]+gMcTrackPos[2]*gMcTrackPos[2]),gMcTrackPosLoc[0],gMcTrackPosLoc[1],gMcTrackPosLoc[2]);
  

  
  Printf("Step %i: tid=%i flags alive=%i disap=%i enter=%i exit=%i inside=%i out=%i stop=%i new=%i",
                            iStepN, gAlice->GetMCApp()->GetCurrentTrackNumber(),
                            gMC->IsTrackAlive(), gMC->IsTrackDisappeared(),gMC->IsTrackEntering(), gMC->IsTrackExiting(),
                            gMC->IsTrackInside(),gMC->IsTrackOut(),        gMC->IsTrackStop(),     gMC->IsNewTrack());
  
  Float_t a,z,den,rad,abs; a=z=den=rad=abs=-1;
  Int_t mid=gMC->CurrentMaterial(a,z,den,rad,abs);
  Printf("Step %i: mid=%i a=%7.2f z=%7.2f den=%9.4f rad=%9.2f abs=%9.2f\n\n",iStepN,mid,a,z,den,rad,abs);
  
  TArrayI proc;  gMC->StepProcesses(proc); 
  Printf("Processes in this step:");
  for ( int i = 0 ; i < proc.GetSize(); i++)
  {
    Printf("%s",TMCProcessName[proc.At(i)]);
  }
  Printf("End process list");
  
  iStepN++;
}//StepHistory()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void AliHMPIDv2::StepManager()
{
// Full Step Manager.
// Arguments: none
//   Returns: none           
//  StepHistory(); return; //uncomment to print tracks history
 //  StepCount(); return;     //uncomment to count photons
  
  Int_t   copy; //volume copy aka node
  
//Treat photons    
  if((gMC->TrackPid()==50000050||gMC->TrackPid()==50000051)&&gMC->CurrentVolID(copy)==fIdPad){   //photon (Ckov or feedback) hit PC (fIdPad)
    if(gMC->Edep()>0){                                                                           //photon survided QE test i.e. produces electron
      if(IsLostByFresnel()){ gMC->StopTrack(); return;}                                          //photon lost due to fersnel reflection on PC       
                       gMC->CurrentVolOffID(5,copy);                                             //current chamber since geomtry tree is Hmp-Hsec-Hgap-Hrow-Hcel-Hpad
      Int_t   tid=     gMC->GetStack()->GetCurrentTrackNumber();                                 //take TID
      Int_t   pid=     gMC->TrackPid();                                                          //take PID
      Float_t etot=    gMC->Etot();                                                              //total hpoton energy, [GeV] 
      Double_t x[3];   gMC->TrackPosition(x[0],x[1],x[2]);                                       //take MARS position at entrance to PC
      Float_t xl,yl;   AliHMPIDParam::Instance()->Mars2Lors(copy,x,xl,yl);                       //take LORS position
       if ( yl < 0  ) Printf("-------------------> SUPER PROBLEM PHOTON>>> Ch: %d, x[]: %f %f %f (MARS)-> xl: %f yl: %f",copy,x[0],x[1],x[2],xl,yl);
      new((*fHits)[fNhits++])AliHMPIDHit(copy,etot,pid,tid,xl,yl,x);                             //HIT for photon, position at P, etot will be set to Q
      GenFee(etot);                                                                              //generate feedback photons etot is modified in hit ctor to Q of hit
    }//photon hit PC and DE >0 
  }//photon hit PC
  
//Treat charged particles  
  static Float_t eloss;                                                                           //need to store mip parameters between different steps    
  static Double_t in[3];
  if(gMC->TrackCharge() && gMC->CurrentVolID(copy)==fIdCell){                                     //charged particle in amplification gap (fIdCell)
    if(gMC->IsTrackEntering()||gMC->IsNewTrack()) {                                               //entering or newly created
      eloss=0;                                                                                    //reset Eloss collector                         
      gMC->TrackPosition(in[0],in[1],in[2]);                                                      //take position at the entrance
    }else if(gMC->IsTrackExiting()||gMC->IsTrackStop()||gMC->IsTrackDisappeared()){               //exiting or disappeared
      eloss              +=gMC->Edep();                                                           //take into account last step Eloss
                          gMC->CurrentVolOffID(4,copy);                                           //take current chamber since geometry tree is Hmp-Hsec-Hgap-Hrow-Hcel
      Int_t tid=          gMC->GetStack()->GetCurrentTrackNumber();                               //take TID
      Int_t pid=          gMC->TrackPid();                                                        //take PID
      Double_t out[3];    gMC->TrackPosition(out[0],out[1],out[2]);                               //take MARS position at exit
      out[0]=0.5*(out[0]+in[0]);                                                                  //>
      out[1]=0.5*(out[1]+in[1]);                                                                  //take hit position at the anod plane
      out[2]=0.5*(out[2]+in[2]);                                                                  //>
      Float_t xl,yl;AliHMPIDParam::Instance()->Mars2Lors(copy,out,xl,yl);                         //take LORS position
       if ( yl < 0  ) Printf("-------------------> SUPER PROBLEM CHARGED>>> Ch: %d, x[]: %f %f %f (MARS)-> xl: %f yl: %f",copy,out[0],out[1],out[2],xl,yl);
      new((*fHits)[fNhits++])AliHMPIDHit(copy,eloss,pid,tid,xl,yl,out);                           //HIT for MIP, position near anod plane, eloss will be set to Q 
      GenFee(eloss);                                                                              //generate feedback photons 
    }else                                                                                         //just going inside
      eloss          += gMC->Edep();                                                              //collect this step eloss 
  }//MIP in GAP
}//StepManager()
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
