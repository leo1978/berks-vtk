/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRCalculatorFilter.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2009 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "vtkObjectFactory.h"
#include "vtkRCalculatorFilter.h"
#include "vtkRInterface.h"
#include "vtkErrorCode.h"
#include "vtkDataSet.h"
#include "vtkGraph.h"
#include "vtkFieldData.h"
#include "vtkArrayData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkCompositeDataSet.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDoubleArray.h"
#include "vtkTable.h"
#include "vtkTree.h"

#include <stdlib.h>
#include <string>
#include <vector>
#include <vtksys/ios/sstream>
#include <sys/stat.h>

#define BUFFER_SIZE 32768

vtkStandardNewMacro(vtkRCalculatorFilter);

class ArrNames
{

public:
  ArrNames(const char* Vname, const char* Rname)
    {
    this->VTKArrName = Vname;
    this->RarrName = Rname;
    };

  std::string VTKArrName;
  std::string RarrName;

};

class vtkRCalculatorFilterInternals
{

public:
  std::vector<ArrNames> PutArrNames;
  std::vector<ArrNames> GetArrNames;
  std::string PutTableName;
  std::string GetTableName;
  std::string PutTreeName;
  std::string GetTreeName;
};

vtkRCalculatorFilter::vtkRCalculatorFilter()
{

  this->ri = 0;
  this->Rscript = 0;
  this->RfileScript = 0;
  this->ScriptFname = 0;
  this->TimeOutput = 1;
  this->Routput = 1;
  this->BlockInfoOutput = 1;
  this->ScriptFname = 0;
  this->CurrentTime = 0;
  this->TimeRange = 0;
  this->TimeSteps = 0;
  this->BlockId = 0;
  this->NumBlocks = 0;
  this->Routput = 1;
  this->rcfi = new vtkRCalculatorFilterInternals();
  this->OutputBuffer = new char[BUFFER_SIZE];
  this->GetInputPortInformation(0)->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);

}

vtkRCalculatorFilter::~vtkRCalculatorFilter()
{

  delete this->rcfi;

  if(this->ri)
    {
    this->ri->Delete();
    }

  if(this->Rscript)
    {
    delete [] this->Rscript;
    }

  if(this->RfileScript)
    {
    delete [] this->RfileScript;
    }

  if(this->ScriptFname)
    {
    delete [] this->ScriptFname;
    }

  if(this->CurrentTime)
    {
    this->CurrentTime->Delete();
    }

  if(this->TimeRange)
    {
    this->TimeRange->Delete();
    }

  if(this->TimeSteps)
    {
    this->TimeSteps->Delete();
    }

  if(this->BlockId)
    {
    this->BlockId->Delete();
    }

  if(this->NumBlocks)
    {
    this->NumBlocks->Delete();
    }

  delete [] this->OutputBuffer;

}

void vtkRCalculatorFilter::PrintSelf(ostream& os, vtkIndent indent)
{

  this->Superclass::PrintSelf(os,indent);

  os << indent << "Rscript: "
     << (this->Rscript ? this->Rscript : "(none)") << endl;

  os << indent << "RfileScript: "
     << (this->RfileScript ? this->RfileScript : "(none)") << endl;

  os << indent << "ScriptFname: "
     << (this->ScriptFname ? this->ScriptFname : "(none)") << endl;

  os << indent << "Routput: "
     << (this->Routput ? "On" : "Off") << endl;

  os << indent << "TimeOutput: "
     << (this->TimeOutput ? "On" : "Off") << endl;

  os << indent << "BlockInfoOutput: "
     << (this->BlockInfoOutput ? "On" : "Off") << endl;

  os << indent << "CurrentTime: " << endl;

  if (this->CurrentTime)
    {
    this->CurrentTime->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent << "(none)" << endl;
    }

  os << indent << "TimeRange: " << endl;

  if (this->TimeRange)
    {
    this->TimeRange->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent << "(none)" << endl;
    }

  os << indent << "TimeSteps: " << endl;

  if (this->TimeSteps)
    {
    this->TimeSteps->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent << "(none)" << endl;
    }

  os << indent << "BlockId: " << endl;

  if (this->BlockId)
    {
    this->BlockId->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent << "(none)" << endl;
    }

  os << indent << "NumBlocks: " << endl;

  if (this->NumBlocks)
    {
    this->NumBlocks->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << indent << "(none)" << endl;
    }

}

//----------------------------------------------------------------------------
int vtkRCalculatorFilter::ProcessRequest(
    vtkInformation* request,
    vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{
  // create the output
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
    {
    return this->RequestDataObject(request, inputVector, outputVector);
    }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
int vtkRCalculatorFilter::RequestDataObject(
    vtkInformation*,
    vtkInformationVector** inputVector ,
    vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  if (!inInfo)
    {
    return 0;
    }
  vtkDataObject *input = inInfo->Get(vtkDataObject::DATA_OBJECT());

  if (input)
    {
    //one output port, but multiple InformationObjects()
    for(int i=0; i < this->GetNumberOfOutputPorts(); ++i)
      {
      vtkInformation* info = outputVector->GetInformationObject(i);
      vtkDataObject *output = info->Get(vtkDataObject::DATA_OBJECT());

      if (!output || !output->IsA(input->GetClassName()))
        {
        vtkDataObject* newOutput = input->NewInstance();
        info->Set(vtkDataObject::DATA_OBJECT(), newOutput);
        newOutput->Delete();
        }
      }
    return (1);
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkRCalculatorFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                      vtkInformationVector **inputVector,
                                      vtkInformationVector *outputVector)
{

  int result;

  if(!this->ri)
    {
    this->ri = vtkRInterface::New();
    }


  if(this->ScriptFname)
    {
    this->SetRscriptFromFile(this->ScriptFname);
    }

  if( (!this->Rscript) && (!this->RfileScript) )
    {
    return(1);
    }

  if(this->Routput)
    {
    this->ri->OutputBuffer(this->OutputBuffer, BUFFER_SIZE);
    }


  vtkInformation* outinfo = outputVector->GetInformationObject(0);
  vtkInformation* inpinfo = inputVector[0]->GetInformationObject(0);

  vtkDataObject* input = inpinfo->Get(vtkDataObject::DATA_OBJECT());
  vtkDataObject* output = outinfo->Get(vtkDataObject::DATA_OBJECT());

  //only one output port; the ouput type is assumed to be the same as the first input data object (backward compatibility)
  output->ShallowCopy(input);

  // For now: use the first input information for timing
  if(this->TimeOutput)
    {
    if ( inpinfo->Has(vtkStreamingDemandDrivenPipeline::TIME_STEPS()) )
      {
      int length = inpinfo->Length( vtkStreamingDemandDrivenPipeline::TIME_STEPS() );

      if(!this->TimeSteps)
        {
        this->TimeSteps = vtkDoubleArray::New();
        this->TimeSteps->SetNumberOfComponents(1);
        this->TimeSteps->SetNumberOfTuples(length);
        }
      else if(this->TimeSteps->GetNumberOfTuples() != length)
        {
        this->TimeSteps->SetNumberOfTuples(length);
        }

      for(int i = 0; i<length; i++)
        {
        this->TimeSteps->InsertValue(i,
          inpinfo->Get( vtkStreamingDemandDrivenPipeline::TIME_STEPS() )[i] );
        }

      this->ri->AssignVTKDataArrayToRVariable(this->TimeSteps, "VTK_TIME_STEPS");
      }

    if ( inpinfo->Has(vtkStreamingDemandDrivenPipeline::TIME_RANGE()) )
      {
      if(!this->TimeRange)
        {
        this->TimeRange = vtkDoubleArray::New();
        this->TimeRange->SetNumberOfComponents(1);
        this->TimeRange->SetNumberOfTuples(2);
        }

      this->TimeRange->InsertValue(0,inpinfo->Get( vtkStreamingDemandDrivenPipeline::TIME_RANGE() )[0] );

      this->TimeRange->InsertValue(1,inpinfo->Get( vtkStreamingDemandDrivenPipeline::TIME_RANGE() )[1] );

      this->ri->AssignVTKDataArrayToRVariable(this->TimeRange, "VTK_TIME_RANGE");
      }

    if ( input->GetInformation()->Has(vtkDataObject::DATA_TIME_STEP()) )
      {
      if(!this->CurrentTime)
        {
        this->CurrentTime = vtkDoubleArray::New();
        this->CurrentTime->SetNumberOfComponents(1);
        this->CurrentTime->SetNumberOfTuples(1);
        }

      this->CurrentTime->InsertValue(0,input->GetInformation()->Get( vtkDataObject::DATA_TIME_STEP()) );

      this->ri->AssignVTKDataArrayToRVariable(this->CurrentTime, "VTK_CURRENT_TIME");
      }
    }

  // assign vtk variables to R variables
  vtkDataSet * dataSetIn = 0;
  vtkGraph * graphIn = 0;
  vtkTree * treeIn = 0;
  vtkTable* tableIn = 0;
  vtkCompositeDataSet * compositeDataSetIn = 0;
  vtkArrayData * arrayDataIn = 0;
  int numberOfInputs =  inputVector[0]->GetNumberOfInformationObjects();
  for ( int i = 0; i < numberOfInputs; i++)
    {
    inpinfo = inputVector[0]->GetInformationObject(i);

    input = inpinfo->Get(vtkDataObject::DATA_OBJECT());

    dataSetIn = vtkDataSet::SafeDownCast(input);
    if (dataSetIn)
      {
      this->ProcessInputDataSet(dataSetIn);
      continue;
      }

    treeIn = vtkTree::SafeDownCast(input);
    if (treeIn)
      {
      this->ProcessInputTree(treeIn);
      continue;
      }

    graphIn = vtkGraph::SafeDownCast(input);
    if (graphIn)
      {
      this->ProcessInputGraph(graphIn);
      continue;
      }

    arrayDataIn = vtkArrayData::SafeDownCast(input);
    if (arrayDataIn)
      {
      this->ProcessInputArrayData(arrayDataIn);
      continue;
      }

    compositeDataSetIn = vtkCompositeDataSet::SafeDownCast(input);
    if (compositeDataSetIn)
      {
      this->ProcessInputCompositeDataSet(compositeDataSetIn);
      continue;
      }

    tableIn = vtkTable::SafeDownCast(input);
    if (tableIn)
      {
      this->ProcessInputTable(tableIn);
      continue;
      }



    }

  //run scripts
  if(this->Rscript)
    {
    result = this->ri->EvalRscript(this->Rscript);

    if(result)
      {
      vtkErrorMacro(<<"Failed to evaluate command string in R");
      return(1);
      }

    if(this->Routput)
      {
      cout << this->OutputBuffer << endl;
      }
    }

  if(this->RfileScript)
    {
    result = this->ri->EvalRscript(this->RfileScript);

    if(result)
      {
      vtkErrorMacro(<<"Failed to evaluate command string in R");
      return(1);
      }

    if(this->Routput)
      {
      cout << this->OutputBuffer << endl;
      }
    }


  // generate output
  vtkDataSet* dataSetOut = vtkDataSet::SafeDownCast(output);
  if (dataSetOut)
    {
    this->ProcessOutputDataSet(dataSetOut);
    return (1);
    }


  vtkCompositeDataSet* compositeDataSetOut= vtkCompositeDataSet::SafeDownCast(output);
  if (compositeDataSetOut)
    {
    this->ProcessOutputCompositeDataSet(compositeDataSetOut);
    return (1);
    }

  vtkArrayData* arrayDataOut = vtkArrayData::SafeDownCast(output);
  if (arrayDataOut)
    {
    this->ProcessOutputArrayData(arrayDataOut);
    return (1);
    }

  vtkTable* tableOut= vtkTable::SafeDownCast(output);
  if (tableOut)
    {
    this->ProcessOutputTable(tableOut);
    return (1);
    }

  vtkTree* treeOut = vtkTree::SafeDownCast(output);
  if (treeOut)
    {
    this->ProcessOutputTree(treeOut);
    return (1);
    }

  vtkGraph* graphOut = vtkGraph::SafeDownCast(output);
  if (graphOut)
    {
    int ncells = graphIn->GetNumberOfEdges();
    int npoints = graphIn->GetNumberOfVertices();
    this->ProcessOutputGraph(graphOut);
    return (1);
    }


  vtkErrorMacro(<<"Filter does not handle output data type");
  return(1);
}

//----------------------------------------------------------------------------
int vtkRCalculatorFilter::ProcessInputDataSet(vtkDataSet* dsIn)
{
  int ncells;
  int npoints;
  vtkDataSetAttributes* CellinFD = 0;
  vtkDataSetAttributes* PointinFD = 0;

  std::vector<ArrNames>::iterator VectorIterator;
  vtkDataArray* currentArray = 0;

  CellinFD = dsIn->GetCellData();
  PointinFD = dsIn->GetPointData();

  ncells = dsIn->GetNumberOfCells();
  npoints = dsIn->GetNumberOfPoints();

  if( (ncells < 1) && (npoints < 1) )
    {
    vtkErrorMacro(<<"Empty Data Set");
    return(1);
    }

  for(VectorIterator = this->rcfi->PutArrNames.begin();
    VectorIterator != this->rcfi->PutArrNames.end();
    VectorIterator++)
    {
    currentArray = PointinFD->GetArray(VectorIterator->VTKArrName.c_str());

    if(!currentArray)
      {
      currentArray = CellinFD->GetArray(VectorIterator->VTKArrName.c_str());
      }

    if(currentArray)
      {
      this->ri->AssignVTKDataArrayToRVariable(currentArray,
        VectorIterator->RarrName.c_str());
      }
    else
      {
      vtkErrorMacro(<<"Array Name not in Data Set " << VectorIterator->VTKArrName.c_str());
      return(1);
      }
    }
return (1);
}

//----------------------------------------------------------------------------
int vtkRCalculatorFilter::ProcessOutputDataSet(vtkDataSet* dsOut)
{
  vtkDataArray* currentArray = 0;
  vtkDataSetAttributes* CelloutFD = dsOut->GetCellData();
  vtkDataSetAttributes* PointoutFD = dsOut->GetPointData();

  int ncells = dsOut->GetNumberOfCells();
  int npoints = dsOut->GetNumberOfPoints();

  std::vector<ArrNames>::iterator VectorIterator;
  for(VectorIterator = this->rcfi->GetArrNames.begin();
    VectorIterator != this->rcfi->GetArrNames.end();
    VectorIterator++)
    {
    currentArray = this->ri->AssignRVariableToVTKDataArray(VectorIterator->RarrName.c_str());

    if(!currentArray)
      {
      vtkErrorMacro(<<"Failed to get array from R");
      return(1);
      }

    int ntuples = currentArray->GetNumberOfTuples();

    vtkDataSetAttributes* dsa;

    if(ntuples == ncells)
      {
      dsa = CelloutFD;
      }
    else if(ntuples == npoints)
      {
      dsa = PointoutFD;
      }
    else
      {
      vtkErrorMacro(<<"Array returned from R has wrong size");
      currentArray->Delete();
      return(1);
      }

    currentArray->SetName(VectorIterator->VTKArrName.c_str());

    if(dsa->HasArray(VectorIterator->VTKArrName.c_str()))
      {
      dsa->RemoveArray(VectorIterator->VTKArrName.c_str());
      }

    dsa->AddArray(currentArray);
    }

  return(1);
}


//----------------------------------------------------------------------------
int vtkRCalculatorFilter::ProcessInputGraph(vtkGraph* gIn)
{
  vtkDataSetAttributes* CellinFD = gIn->GetEdgeData();
  vtkDataSetAttributes* PointinFD = gIn->GetVertexData();

  int ncells = gIn->GetNumberOfEdges();
  int npoints = gIn->GetNumberOfVertices();

  if( (npoints < 1) && (ncells < 1) )
    {
    vtkErrorMacro(<<"Empty Data Set");
    return(1);
    }

  std::vector<ArrNames>::iterator VectorIterator;
  vtkDataArray* currentArray = 0;
  for(VectorIterator = this->rcfi->PutArrNames.begin();
    VectorIterator != this->rcfi->PutArrNames.end();
    VectorIterator++)
    {
    currentArray = PointinFD->GetArray(VectorIterator->VTKArrName.c_str());

    if(!currentArray)
      {
      currentArray = CellinFD->GetArray(VectorIterator->VTKArrName.c_str());
      }

    if(currentArray)
      {
      this->ri->AssignVTKDataArrayToRVariable(currentArray,
        VectorIterator->RarrName.c_str());
      }
    else
      {
      vtkErrorMacro(<<"Array Name not in Data Set " << VectorIterator->VTKArrName.c_str());
      return(1);
      }
    }
  return (1);
}


//----------------------------------------------------------------------------
int vtkRCalculatorFilter::ProcessOutputGraph(vtkGraph* gOut)
{
  vtkDataSetAttributes * CelloutFD = gOut->GetEdgeData();
  vtkDataSetAttributes *PointoutFD = gOut->GetVertexData();

  int ncells = gOut->GetNumberOfEdges();
  int npoints = gOut->GetNumberOfVertices();

  vtkDataArray* currentArray = 0;
  std::vector<ArrNames>::iterator VectorIterator;
  for(VectorIterator = this->rcfi->GetArrNames.begin();
    VectorIterator != this->rcfi->GetArrNames.end();
    VectorIterator++)
    {
    currentArray = this->ri->AssignRVariableToVTKDataArray(VectorIterator->RarrName.c_str());

    if(!currentArray)
      {
      vtkErrorMacro(<<"Failed to get array from R");
      return(1);
      }

    int ntuples = currentArray->GetNumberOfTuples();

    vtkDataSetAttributes* dsa;

    if(ntuples == ncells)
      {
      dsa = CelloutFD;
      }
    else if(ntuples == npoints)
      {
      dsa = PointoutFD;
      }
    else
      {
      vtkErrorMacro(<<"Array returned from R has wrong size");
      currentArray->Delete();
      return(1);
      }

    currentArray->SetName(VectorIterator->VTKArrName.c_str());

    if(dsa->HasArray(VectorIterator->VTKArrName.c_str()))
      {
      dsa->RemoveArray(VectorIterator->VTKArrName.c_str());
      }

    dsa->AddArray(currentArray);
    }

  return (1);
}


//----------------------------------------------------------------------------
int vtkRCalculatorFilter::ProcessInputArrayData(vtkArrayData * adIn)
{

  vtkArray* cArray = 0;
  std::vector<ArrNames>::iterator VectorIterator;
  for(VectorIterator = this->rcfi->PutArrNames.begin();
    VectorIterator != this->rcfi->PutArrNames.end();
    VectorIterator++)
    {
    int index = atoi(VectorIterator->VTKArrName.c_str());

    if( (index < 0) || (index >= adIn->GetNumberOfArrays()) )
      {
      vtkErrorMacro(<<"Array Index out of bounds " << index);
      return(1);
      }

    cArray = adIn->GetArray(index);

    this->ri->AssignVTKArrayToRVariable(cArray,  VectorIterator->RarrName.c_str());
    }
  return (1);
}


//----------------------------------------------------------------------------
int vtkRCalculatorFilter::ProcessOutputArrayData(vtkArrayData * adOut)
{
  vtkArray * cArray = 0;
  std::vector<ArrNames>::iterator VectorIterator;
  for(VectorIterator = this->rcfi->GetArrNames.begin();
    VectorIterator != this->rcfi->GetArrNames.end();
    VectorIterator++)
    {
    cArray = this->ri->AssignRVariableToVTKArray(VectorIterator->RarrName.c_str());

    if(!cArray)
      {
      vtkErrorMacro(<<"Failed to get array from R");
      return(1);
      }

    cArray->SetName(VectorIterator->VTKArrName.c_str());

    adOut->AddArray(cArray);
    }
  return (1);
}


//----------------------------------------------------------------------------
int vtkRCalculatorFilter::ProcessInputCompositeDataSet(vtkCompositeDataSet* cdsIn)
{
    vtkCompositeDataIterator* iter = cdsIn->NewIterator();

    if(this->BlockInfoOutput)
      {
      if(!this->BlockId)
        {
        this->BlockId = vtkDoubleArray::New();
        this->BlockId->SetNumberOfComponents(1);
        this->BlockId->SetNumberOfTuples(1);
        }

      if(!this->NumBlocks)
        {
        this->NumBlocks = vtkDoubleArray::New();
        this->NumBlocks->SetNumberOfComponents(1);
        this->NumBlocks->SetNumberOfTuples(1);
        }
      int nb = 0;
      for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
        {
        nb++;
        }
      this->NumBlocks->SetValue(0,nb);
      this->ri->AssignVTKDataArrayToRVariable(this->NumBlocks, "VTK_NUMBER_OF_BLOCKS");
      }


    int bid = 1;
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
      {
      if(this->BlockInfoOutput)
        {
        this->BlockId->SetValue(0,bid);
        this->ri->AssignVTKDataArrayToRVariable(this->BlockId, "VTK_BLOCK_ID");
        }
      vtkDataSet* inputDS = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
      this->ProcessInputDataSet(inputDS);
      bid++;
      }
    iter->Delete();
    return (1);
}


//----------------------------------------------------------------------------
int vtkRCalculatorFilter::ProcessOutputCompositeDataSet(vtkCompositeDataSet * cdsOut)
{

  vtkCompositeDataIterator* iter = cdsOut->NewIterator();
  iter->InitTraversal();

  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
    vtkDataSet* outputDS = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
    this->ProcessOutputDataSet(outputDS);
    }
  iter->Delete();
  return (1);
}


//----------------------------------------------------------------------------
int vtkRCalculatorFilter::ProcessInputTable(vtkTable* tIn)
{
  if(this->rcfi->PutTableName.size() > 0)
    {
    this->ri->AssignVTKTableToRVariable(tIn, this->rcfi->PutTableName.c_str());
    }
  return (1);

}


//----------------------------------------------------------------------------
int vtkRCalculatorFilter::ProcessOutputTable(vtkTable* tOut)
{

  if(this->rcfi->GetTableName.size() > 0)
    {
    tOut->ShallowCopy(this->ri->AssignRVariableToVTKTable(this->rcfi->GetTableName.c_str()));
    }

  return (1);

}


int vtkRCalculatorFilter::ProcessInputTree(vtkTree* tIn)
{

  if(this->rcfi->PutTreeName.size() > 0)
    {
    this->ri->AssignVTKTreeToRVariable(tIn, this->rcfi->PutTreeName.c_str());
    }
  return (1);

}


int vtkRCalculatorFilter::ProcessOutputTree(vtkTree* tOut)
{

  if(this->rcfi->GetTreeName.size() > 0)
    {
    tOut->ShallowCopy(this->ri->AssignRVariableToVTKTree(this->rcfi->GetTreeName.c_str()));
    }
  return (1);

}



int vtkRCalculatorFilter::SetRscriptFromFile(const char* fname)
{

  FILE *fp;
  long len;
  long rlen;

  if(fname && (strlen(fname) > 0) )
    {
    fp = fopen(fname,"rb");

    if(!fp)
      {
      vtkErrorMacro(<<"Can't open input file named " << fname);
      return(0);
      }

    fseek(fp,0,SEEK_END);
    len = ftell(fp);
    fseek(fp,0,SEEK_SET);

    if(this->RfileScript)
      {
      delete [] this->RfileScript;
      this->RfileScript = 0;
      }

    this->RfileScript = new char[len+1];
    rlen = static_cast<long>(fread(this->RfileScript,1,len,fp));
    this->RfileScript[len] = '\0';
    fclose(fp);

    if (rlen != len)
      {
      delete [] this->RfileScript;
      this->RfileScript = 0;
      vtkErrorMacro(<<"Error reading R script");
      return(0);
      }

    this->Modified();

    return(1);
    }
  else if(!fname)
    {
    vtkErrorMacro(<<"Input file name is NULL");
    return(0);
    }
  else
    {
    return(0);
    }

}


void vtkRCalculatorFilter::PutArray(const char* NameOfVTKArray, const char* NameOfMatVar)
{

  if( NameOfVTKArray && (strlen(NameOfVTKArray) > 0) && NameOfMatVar && (strlen(NameOfMatVar) > 0) )
    {
    rcfi->PutArrNames.push_back(ArrNames(NameOfVTKArray, NameOfMatVar));
    this->Modified();
    }

}


void vtkRCalculatorFilter::GetArray(const char* NameOfVTKArray, const char* NameOfMatVar)
{

  if( NameOfVTKArray && (strlen(NameOfVTKArray) > 0) && NameOfMatVar && (strlen(NameOfMatVar) > 0) )
    {
    rcfi->GetArrNames.push_back(ArrNames(NameOfVTKArray, NameOfMatVar));
    this->Modified();
    }

}


void vtkRCalculatorFilter::PutTable(const char* NameOfRvar)
{

  if( NameOfRvar && (strlen(NameOfRvar) > 0) )
    {
    rcfi->PutTableName = NameOfRvar;
    this->Modified();
    }

}


void vtkRCalculatorFilter::GetTable(const char* NameOfRvar)
{

  if( NameOfRvar && (strlen(NameOfRvar) > 0) )
    {
    rcfi->GetTableName = NameOfRvar;
    this->Modified();
    }

}


void vtkRCalculatorFilter::PutTree(const char* NameOfRvar)
{

  if( NameOfRvar && (strlen(NameOfRvar) > 0) )
    {
    rcfi->PutTreeName = NameOfRvar;
    this->Modified();
    }

}


void vtkRCalculatorFilter::GetTree(const char* NameOfRvar)
{

  if( NameOfRvar && (strlen(NameOfRvar) > 0) )
    {
    rcfi->GetTreeName = NameOfRvar;
    this->Modified();
    }

}


void vtkRCalculatorFilter::RemoveAllPutVariables()
{
  rcfi->PutArrNames.clear();
  this->Modified();
}


void vtkRCalculatorFilter::RemoveAllGetVariables()
{
  rcfi->GetArrNames.clear();
  this->Modified();
}

