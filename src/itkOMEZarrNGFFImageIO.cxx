/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkOMEZarrNGFFImageIO.h"
#include "itkIOCommon.h"
#include "itkIntTypes.h"

#include "netcdf.h"

namespace itk
{

OMEZarrNGFFImageIO::OMEZarrNGFFImageIO()

{
  this->AddSupportedWriteExtension(".zarr");
  this->AddSupportedWriteExtension(".zr3");
  this->AddSupportedWriteExtension(".zip");

  this->AddSupportedReadExtension(".zarr");
  this->AddSupportedReadExtension(".zr3");
  this->AddSupportedReadExtension(".zip");

  this->Self::SetCompressor("");
  this->Self::SetMaximumCompressionLevel(9);
  this->Self::SetCompressionLevel(2);
}


void
OMEZarrNGFFImageIO::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent); // TODO: either add stuff here, or remove this override
}

// IOComponentEnum
// netCDFToITKComponentType(const int netCDFComponentType) const
//{
//  switch (nrrdComponentType)
//  {
//    case nrrdTypeUnknown:
//    case nrrdTypeBlock:
//      return IOComponentEnum::UNKNOWNCOMPONENTTYPE;
//
//    case nrrdTypeChar:
//      return IOComponentEnum::CHAR;
//
//    case nrrdTypeUChar:
//      return IOComponentEnum::UCHAR;
//
//    case nrrdTypeShort:
//      return IOComponentEnum::SHORT;
//
//    case nrrdTypeUShort:
//      return IOComponentEnum::USHORT;
//
//    case nrrdTypeInt:
//      return IOComponentEnum::INT;
//
//    case nrrdTypeUInt:
//      return IOComponentEnum::UINT;
//
//    case nrrdTypeLLong:
//      return IOComponentEnum::LONGLONG;
//
//    case nrrdTypeULLong:
//      return IOComponentEnum::ULONGLONG;
//
//    case nrrdTypeFloat:
//      return IOComponentEnum::FLOAT;
//
//    case nrrdTypeDouble:
//      return IOComponentEnum::DOUBLE;
//  }
//  // Strictly to avoid compiler warning regarding "control may reach end of
//  // non-void function":
//  return IOComponentEnum::UNKNOWNCOMPONENTTYPE;
//}

int
itkToNetCDFComponentType(const IOComponentEnum itkComponentType)
{
  switch (itkComponentType)
  {
    case IOComponentEnum::UNKNOWNCOMPONENTTYPE:
      return NC_NAT;

    case IOComponentEnum::CHAR:
      return NC_BYTE;

    case IOComponentEnum::UCHAR:
      return NC_UBYTE;

    case IOComponentEnum::SHORT:
      return NC_SHORT;

    case IOComponentEnum::USHORT:
      return NC_USHORT;

    // "long" is a silly type because it basically guaranteed not to be
    // cross-platform across 32-vs-64 bit machines, but we can figure out
    // a cross-platform way of storing the information.
    case IOComponentEnum::LONG:
      return (4 == sizeof(long)) ? NC_INT : NC_INT64;

    case IOComponentEnum::ULONG:
      return (4 == sizeof(long)) ? NC_UINT : NC_UINT64;

    case IOComponentEnum::INT:
      return NC_INT;

    case IOComponentEnum::UINT:
      return NC_UINT;

    case IOComponentEnum::LONGLONG:
      return NC_INT64;

    case IOComponentEnum::ULONGLONG:
      return NC_UINT64;

    case IOComponentEnum::FLOAT:
      return NC_FLOAT;

    case IOComponentEnum::DOUBLE:
      return NC_DOUBLE;
    case IOComponentEnum::LDOUBLE:
      return NC_NAT; // Long double not supported by netCDF
  }
  // Strictly to avoid compiler warning regarding "control may reach end of
  // non-void function":
  return NC_NAT;
}


bool
OMEZarrNGFFImageIO::CanReadFile(const char * filename)
{
  if (!this->HasSupportedWriteExtension(filename, true))
  {
    return false;
  }

  try
  {
    int result = nc_open(getNCFilename(filename), NC_NOWRITE, &m_NCID);
    std::cout << "netCDF error: " << nc_strerror(result) << std::endl;
    if (!result) // if it was opened correctly, we should close it
    {
      nc_close(m_NCID);
      return true;
    }
    return false;
  }
  catch (...) // file cannot be opened, access denied etc.
  {
    return false;
  }
}

// Make netCDF call, and error check it.
// Requires variable int r; to be defined.
#define netCDF_call(call)                                  \
  r = call;                                                \
  if (r) /* error */                                       \
  {                                                        \
    nc_close(m_NCID); /* clean up a little */              \
    itkExceptionMacro("netCDF error: " << nc_strerror(r)); \
  }

void
OMEZarrNGFFImageIO::ReadImageInformation()
{
  if (this->m_FileName.empty())
  {
    itkExceptionMacro("FileName has not been set.");
  }

  int r;
  netCDF_call(nc_open(getNCFilename(this->m_FileName), NC_NOWRITE, &m_NCID));

  int              m_NCvarID = 0;
  int              ntypes;
  std::vector<int> typeids;

  int m_NCdimIDs[MaximumDimension] = { 0 };

  netCDF_call(nc_inq_typeids(m_NCID, &ntypes, nullptr));
  typeids.resize(ntypes);

  IOComponentEnum cmpType = IOComponentEnum::UNKNOWNCOMPONENTTYPE;
  this->SetComponentType(cmpType);

  // xt::xzarr_file_system_store store(this->m_FileName);
  //
  // auto        h = xt::get_zarr_hierarchy(store, "");
  // auto        node = h["/"];
  // std::string nodes = h.get_nodes("/").dump();
  // std::cout << nodes << std::endl;
  // std::string children = h.get_children("/").dump();
  // std::cout << children << std::endl;
  //
  // xt::zarray         z = h.get_array("/image/0"); // TODO: do not hard-code a prefix.
  // const unsigned int nDims = z.dimension();
  // this->SetNumberOfDimensions(nDims - 1);
  //
  // std::vector<size_t> shape(z.shape().crbegin(), z.shape().crend()); // construct in reverse via iterators
  // this->SetComponentType(IOComponentEnum::FLOAT);                    // TODO: determine this programatically
  // this->SetNumberOfComponents(shape[0]);
  //
  // for (unsigned int i = 0; i < nDims - 1; ++i)
  // {
  //   this->SetDimensions(i, shape[i + 1]);
  // }
}


void
OMEZarrNGFFImageIO::Read(void * buffer)
{
  if (this->GetLargestRegion() != m_IORegion)
  {
    // Stream the data in chunks
  }
  else
  {
    // xt::xzarr_file_system_store store(this->m_FileName);
    // auto                        h = xt::get_zarr_hierarchy(store);
    // xt::zarray                  z = h.get_array("/image/0");
    //
    // float * data = static_cast<float *>(buffer);
    //
    // size_t size = m_IORegion.GetNumberOfPixels() * this->GetNumberOfComponents();
    // auto   dArray = xt::adapt(data, size, xt::no_ownership(), z.shape());
    // dArray.assign(z.get_array<float>());
  }
}


bool
OMEZarrNGFFImageIO::CanWriteFile(const char * name)
{
  const std::string filename = name;

  if (filename.empty())
  {
    return false;
  }

  return this->HasSupportedWriteExtension(name, true);
}


void
OMEZarrNGFFImageIO::WriteImageInformation()
{
  if (this->m_FileName.empty())
  {
    itkExceptionMacro("FileName has not been set.");
  }

  // we will do everything in Write() method
}


void
OMEZarrNGFFImageIO::Write(const void * buffer)
{
  int r;
  netCDF_call(nc_create(getNCFilename(this->m_FileName), NC_CLOBBER, &m_NCID));

  const std::vector<std::string> dimensionNames = { "c", "i", "j", "k", "t" };

  unsigned nDims = this->GetNumberOfDimensions();

  // handle component dimension
  unsigned cDims = 0;
  if (this->GetNumberOfComponents() > 1)
  {
    cDims = 1;
  }

  int dimIDs[MaximumDimension];
  netCDF_call(nc_def_dim(m_NCID, dimensionNames[0].c_str(), this->GetNumberOfComponents(), &dimIDs[nDims]));

  // handle other dimensions
  for (unsigned d = 0; d < nDims; ++d)
  {
    unsigned dSize = this->GetDimensions(nDims - 1 - d);
    netCDF_call(nc_def_dim(m_NCID, dimensionNames[nDims - d].c_str(), dSize, &dimIDs[d]));
  }

  int netCDF_type = itkToNetCDFComponentType(this->m_ComponentType);
  int varID;
  netCDF_call(nc_def_var(m_NCID, "image", netCDF_type, nDims + cDims, dimIDs, &varID));
  netCDF_call(nc_enddef(m_NCID));

  switch (netCDF_type)
  {
    case NC_BYTE:
      netCDF_call(nc_put_var_text(m_NCID, varID, static_cast<const char *>(buffer)));
      break;
    case NC_UBYTE:
      netCDF_call(nc_put_var_ubyte(m_NCID, varID, static_cast<const unsigned char *>(buffer)));
      break;
    case NC_SHORT:
      netCDF_call(nc_put_var_short(m_NCID, varID, static_cast<const short *>(buffer)));
      break;
    case NC_USHORT:
      netCDF_call(nc_put_var_ushort(m_NCID, varID, static_cast<const unsigned short *>(buffer)));
      break;
    case NC_INT:
      netCDF_call(nc_put_var_int(m_NCID, varID, static_cast<const int *>(buffer)));
      break;
    case NC_UINT:
      netCDF_call(nc_put_var_uint(m_NCID, varID, static_cast<const unsigned int *>(buffer)));
      break;
    case NC_INT64:
      netCDF_call(nc_put_var_longlong(m_NCID, varID, static_cast<const long long *>(buffer)));
      break;
    case NC_UINT64:
      netCDF_call(nc_put_var_ulonglong(m_NCID, varID, static_cast<const unsigned long long *>(buffer)));
      break;
    case NC_FLOAT:
      netCDF_call(nc_put_var_float(m_NCID, varID, static_cast<const float *>(buffer)));
      break;
    case NC_DOUBLE:
      netCDF_call(nc_put_var_double(m_NCID, varID, static_cast<const double *>(buffer)));
      break;
    default:
      itkExceptionMacro("Unsupported component type: " << this->GetComponentTypeAsString(this->m_ComponentType));
      break;
  }

  netCDF_call(nc_close(m_NCID));
}

} // end namespace itk
