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

#include "xtensor-zarr/xzarr_hierarchy.hpp"
#include "xtensor-zarr/xzarr_file_system_store.hpp"

#include "xtensor-zarr/xzarr_compressor.hpp"
#include "xtensor-io/xio_binary.hpp"
#include "xtensor-io/xio_blosc.hpp"
#include "xtensor-io/xio_gzip.hpp"

namespace xt
{
template void
xzarr_register_compressor<xzarr_file_system_store, xio_blosc_config>();
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
    auto                        h = xt::get_zarr_hierarchy(store);

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
  auto                        h = xt::get_zarr_hierarchy(store);
  // std::cout << "h.get_nodes " << h.get_nodes().dump();
  // std::cout << "h.get_children " << h.get_children("/").dump();
  std::cout << "h.get_array " << h.get_array("image");
}


void
OMEZarrNGFFImageIO::Read(void * buffer)
{
  // this will check to see if we are actually streaming
  // we initialize with the dimensions of the file, since if
  // largestRegion and ioRegion don't match, we'll use the streaming
  // path since the comparison will fail
  if (this->GetLargestRegion() != m_IORegion)
  {
    // Stream the data in chunks
  }
  else
  {
    // Read all at once
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
  xt::xzarr_file_system_store fsStore(this->m_FileName);

  auto zarrHierarchy = xt::create_zarr_hierarchy(fsStore);

  nlohmann::json attributes = { "multiscales",
                                { { "name",
                                    "image",
                                    "version",
                                    "0.3",
                                    "datasets",
                                    //{ { "path", "0" }, { "path", "1" }, { "path", "2" } },
                                    { { "path", "0" } },
                                    "axes",
                                    //{ "t", "z", "y", "x", "c" },
                                    { "z", "y", "x", "c" },
                                    "type",
                                    "gaussian",
                                    "metadata",
                                    { "method", "not_implemented" } } } };

  zarrHierarchy.create_group("/image", attributes);

  if (this->GetLargestRegion() != m_IORegion)
  {
    // Stream the data in chunks
  }
  else
  {
    // Write all at once
  }

  const unsigned int  nDims = this->GetNumberOfDimensions();
  std::vector<size_t> shape;
  std::vector<size_t> chunk_shape;
  for (unsigned int i = nDims - 1; i < nDims; --i)
  {
    shape.push_back(this->GetDimensions(i));
    SizeValueType chunkSize = std::max(256ull, this->GetDimensions(i) / 256ull);
    chunkSize = std::min(chunkSize, this->GetDimensions(i));
    chunk_shape.push_back(chunkSize);
  }
  shape.push_back(this->GetNumberOfComponents());
  chunk_shape.push_back(this->GetNumberOfComponents());

  // specify options
  xt::xzarr_create_array_options<xt::xio_blosc_config> o;
  o.chunk_memory_layout = 'C';
  o.chunk_separator = '/';
  // o.attrs = attributes;
  // o.chunk_pool_size = 2;
  o.fill_value = 0.0;

  xt::zarray z = zarrHierarchy.create_array("/image/0",  // path to the array in the store
                                            shape,       // array shape
                                            chunk_shape, // chunk shape
                                            "f4",        // data type, here 32-bit floating point
                                            o            // options
  );

  void *  writableBuffer = const_cast<void *>(buffer);
  float * data = static_cast<float *>(writableBuffer);

  size_t size = m_IORegion.GetNumberOfPixels() * this->GetNumberOfComponents();

  auto       dArray = xt::adapt(data, size, xt::no_ownership(), shape);
  z.assign(dArray);
}

} // end namespace itk
