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

#include "xtensor-io/xio_blosc.hpp"
#include "xtensor-zarr/xzarr_hierarchy.hpp"
#include "xtensor-zarr/xzarr_file_system_store.hpp"
#include "xtensor-zarr/xzarr_compressor.hpp"

namespace xt
{
  template void xzarr_register_compressor<xzarr_file_system_store, xio_blosc_config>();
}

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

  xt::xzarr_register_compressor<xt::xzarr_file_system_store, xt::xio_blosc_config>();

  // this->Self::SetCompressor("");
  // this->Self::SetMaximumCompressionLevel(9);
  // this->Self::SetCompressionLevel(2);
}


void
OMEZarrNGFFImageIO::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent); // TODO: either add stuff here, or remove this override
}

bool
OMEZarrNGFFImageIO::CanReadFile(const char * filename)
{
  try
  {
    xt::xzarr_file_system_store store(filename);
    auto h = xt::get_zarr_hierarchy(store);

    return true;
  }
  catch (...) // file cannot be opened, access denied etc.
  {
    return false;
  }
}


void
OMEZarrNGFFImageIO::ReadImageInformation()
{
  if (this->m_FileName.empty())
  {
    itkExceptionMacro("FileName has not been set.");
  }

  xt::xzarr_file_system_store store(this->m_FileName);
  auto h = xt::get_zarr_hierarchy(store);
  //std::cout << "h.get_nodes " << h.get_nodes().dump();
  //std::cout << "h.get_children " << h.get_children("/").dump();
  std::cout << "h.get_array " << h.get_array("spectra");
}


void
OMEZarrNGFFImageIO::Read(void * buffer)
{
  const unsigned int nDims = this->GetNumberOfDimensions();

  // this will check to see if we are actually streaming
  // we initialize with the dimensions of the file, since if
  // largestRegion and ioRegion don't match, we'll use the streaming
  // path since the comparison will fail
  ImageIORegion largestRegion(nDims);

  for (unsigned int i = 0; i < nDims; ++i)
  {
    largestRegion.SetIndex(i, 0);
    largestRegion.SetSize(i, this->GetDimensions(i));
  }

  if (largestRegion != m_IORegion)
  {
    // Stream the data in chunks
  }
  else
  {
    // Read all at once
  }
}


bool
OMEZarrNGFFImageIO::CanWriteFile(const char* name)
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

  std::ofstream outFile;
  this->OpenFileForWriting(outFile, m_FileName);

  outFile << "Placeholder for real data. TODO";

  outFile.close();
}


void
OMEZarrNGFFImageIO::Write(const void * buffer)
{
  this->WriteImageInformation();

  std::ofstream outFile;
  this->OpenFileForWriting(outFile, m_FileName, false);
  outFile.seekp(32, std::ios::beg);

  outFile << "ImageSizeInBytes: " << this->GetImageSizeInBytes() << std::endl;
  outFile << "ImageSizeInComponents: " << this->GetImageSizeInComponents() << std::endl;
  outFile << "ComponentType: " << this->GetComponentTypeAsString(this->GetComponentType()) << std::endl;
  outFile << "m_IORegion: " << this->m_IORegion << std::endl;

  outFile.close();
}

} // end namespace itk
