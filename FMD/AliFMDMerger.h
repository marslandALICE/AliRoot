#ifndef ALIFMDMERGER_H
#define ALIFMDMERGER_H
/* Copyright(c) 1998-2000, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

// #include "AliMerger.h"
// #include "AliMergable.h"
#include "TRandom.h"
#include "AliDetector.h"

typedef enum {kDigitize=0, kMerge = 1} MergeMode_t;

class AliFMDMerger {
 public:
  
  AliFMDMerger();
  virtual ~AliFMDMerger();
  
  
  // Initialize merging and digitisation
  virtual void Init();
  
  // Do the main work
  void Digitise() ;
  Int_t PutNoise(Int_t charge){return (Int_t)(gRandom->Gaus(charge,500));}
  TClonesArray *SDigits() const {return fSDigits;}
  // TClonesArray *Digits() const {return fDigits;}
 
  void ReadDigit(Int_t a[][20][150], Int_t);
  
  // Setters -> Later Communication with gAlice 
  void SetSignalEventNumber(Int_t i)     {fEvNrSig = i;}
  void SetBackgroundEventNumber(Int_t i) {fEvNrBgr = i;}    
  void SetBackgroundFileName(char* file) {fFnBgr = file;}        
  void SetSignalFileName(char* file)     {fFnSig = file;}        
  void SetMode(MergeMode_t mode) {fMerge = mode;}
	
    enum {kBgTag = -1};
      
 private:    
    // Open the bgr file
    TFile *InitBgr();
    
 private:
     TClonesArray *fDigits;               // ! array with digits
    // Int_t fDigits;
    TClonesArray *fSDigits      ; // List of summable digits
    Int_t fEvNrSig;                 // signal     event number
    Int_t fEvNrBgr;                 // background event number    
    MergeMode_t fMerge;             // merging type kDigitize, kMerge
    char  *fFnBgr;                  // background file name
    char  *fFnSig;                  // signal file name
    TFile *fBgrFile;                // Pointer to background file
    
    ClassDef(AliFMDMerger,0)
};    
#endif

