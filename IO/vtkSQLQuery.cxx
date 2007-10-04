/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSQLQuery.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/
#include "vtkSQLQuery.h"

#include "vtkObjectFactory.h"
#include "vtkSQLDatabase.h"
#include "vtkVariantArray.h"

vtkCxxRevisionMacro(vtkSQLQuery, "1.2");

vtkSQLQuery::vtkSQLQuery()
{
  this->Query = 0;
  this->Database = 0;
  this->Active = false;
}

vtkSQLQuery::~vtkSQLQuery()
{
  this->SetQuery(0);
  if (this->Database)
    {
    this->Database->Delete();
    this->Database = NULL;
    }
}

vtkCxxSetObjectMacro(vtkSQLQuery, Database, vtkSQLDatabase);

void vtkSQLQuery::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Query: " << (this->Query ? this->Query : "NULL") << endl;
  os << indent << "Database: " << (this->Database ? "" : "NULL") << endl;
  if (this->Database)
    {
    this->Database->PrintSelf(os, indent.GetNextIndent());
    }
}

