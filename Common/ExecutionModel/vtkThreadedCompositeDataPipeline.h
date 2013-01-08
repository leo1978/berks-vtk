/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkThreadedCompositeDataPipeline.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

  =========================================================================*/

// .NAME vtkThreadedCompositeDataPipeline - Executive ??
// .SECTION Description
// ??
//

// .SECTION See Also
// ??


#ifndef __vtkThreadedCompositeDataPipeline_h
#define __vtkThreadedCompositeDataPipeline_h

#include "vtkCommonExecutionModelModule.h" // For export macro
#include "vtkCompositeDataPipeline.h"

class vtkComputingResources;
class vtkExecutionScheduler;
class vtkExecutiveCollection;

class VTKCOMMONEXECUTIONMODEL_EXPORT vtkThreadedCompositeDataPipeline : public vtkCompositeDataPipeline
{
public:
  static vtkThreadedCompositeDataPipeline* New();
  vtkTypeMacro(vtkThreadedCompositeDataPipeline,vtkCompositeDataPipeline);

  void PrintSelf(ostream &os, vtkIndent indent);

  // Description
  // Turn on/off using TBB. For debugging and testing only
  static bool UseTBB;

 protected:
  vtkThreadedCompositeDataPipeline();
  ~vtkThreadedCompositeDataPipeline();
  virtual void ExecuteEach(vtkCompositeDataIterator* iter,
                           vtkInformationVector** inInfoVec,
                           vtkInformationVector* outInfoVec,
                           int compositePort,
                           int connection,
                           vtkInformation* request,
                           vtkCompositeDataSet* compositeOutput);
private:
  vtkThreadedCompositeDataPipeline(const vtkThreadedCompositeDataPipeline&);  // Not implemented.
  void operator=(const vtkThreadedCompositeDataPipeline&);  // Not implemented.
  friend class ProcessBlock;
};

#endif
