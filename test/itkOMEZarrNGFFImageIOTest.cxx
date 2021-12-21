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

#include <fstream>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkOMEZarrNGFFImageIO.h"
#include "itkTestingMacros.h"

#include <vector>
#include "xtensor-zarr/xzarr_hierarchy.hpp"
#include "xtensor-zarr/xzarr_file_system_store.hpp"
#include "xtensor-io/xio_binary.hpp"

#include "xtensor-io/xio_blosc.hpp"
#include "xtensor-zarr/xzarr_compressor.hpp"

#define SPECIFIC_IMAGEIO_MODULE_TEST


int
itkOMEZarrNGFFImageIOTest(int argc, char * argv[])
{
  //xt::xzarr_register_compressor<xt::xzarr_file_system_store, xt::xio_binary_config>();
  //xt::xzarr_register_compressor<xt::xzarr_file_system_store, xt::xio_blosc_config>();

  {
    // create a hierarchy on the local file system
    xt::xzarr_file_system_store store("test.zr3");
    auto h = xt::get_zarr_hierarchy(store);

    // create an array in the hierarchy
    nlohmann::json attrs = { {"question", "life"}, {"answer", 42} };
    using S = std::vector<std::size_t>;
    S shape = { 4, 4 };
    S chunk_shape = { 2, 2 };

    xt::zarray a = h.create_array(
      "/arthur/dent",  // path to the array in the store
      shape,  // array shape
      chunk_shape,  // chunk shape
      "<f8"  // data type, here little-endian 64-bit floating point
    );

    //// write array data
    //a(2, 1) = 3.;

    //// read array data
    //std::cout << a(2, 1) << std::endl;
    //// prints `3.`
    //std::cout << a(2, 2) << std::endl;
    //// prints `0.` (fill value)
    //std::cout << a.attrs() << std::endl;
    //// prints `{"answer":42,"question":"life"}`

    // create a group
    auto g = h.create_group("/tricia/mcmillan", attrs);

    // explore the hierarchy
    // get children at a point in the hierarchy
    std::string children = h.get_children("/").dump();
    std::cout << children << std::endl;
    // prints `{"arthur":"implicit_group","marvin":"explicit_group","tricia":"implicit_group"}`
    // view the whole hierarchy
    std::string nodes = h.get_nodes().dump();
    std::cout << nodes << std::endl;
    // prints `{"arthur":"implicit_group","arthur/dent":"array","tricia":"implicit_group","tricia/mcmillan":"explicit_group"}`
  }

  if (argc < 3)
  {
    std::cerr << "Missing parameters." << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << itkNameOfTestExecutableMacro(argv) << " Input Output" << std::endl;
    return EXIT_FAILURE;
  }
  const char * inputFileName = argv[1];
  const char * outputFileName = argv[2];

  constexpr unsigned int Dimension = 4;
  using PixelType = float;
  using ImageType = itk::Image<PixelType, Dimension>;

  using ReaderType = itk::ImageFileReader<ImageType>;
  ReaderType::Pointer reader = ReaderType::New();

  // we will need to force use of zarrIO for either reading or writing
  itk::OMEZarrNGFFImageIO::Pointer zarrIO = itk::OMEZarrNGFFImageIO::New();

  ITK_EXERCISE_BASIC_OBJECT_METHODS(zarrIO, OMEZarrNGFFImageIO, ImageIOBase);

  // check usability of dimension (for coverage)
  if (!zarrIO->SupportsDimension(3))
  {
    std::cerr << "Did not support dimension 3" << std::endl;
    return EXIT_FAILURE;
  }

  if (zarrIO->CanReadFile(inputFileName))
  {
    reader->SetImageIO(zarrIO);
  }

  reader->SetFileName(inputFileName);
  try
  {
    reader->Update();
  }
  catch (itk::ExceptionObject & error)
  {
    std::cerr << "Exception in the file reader " << std::endl;
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
  }

  ImageType::Pointer image = reader->GetOutput();
  image->Print(std::cout);


  using WriterType = itk::ImageFileWriter<ImageType>;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(reader->GetOutput());
  writer->SetFileName(outputFileName);

  if (zarrIO->CanWriteFile(outputFileName))
  {
    writer->SetImageIO(zarrIO);
  }

  ITK_TRY_EXPECT_NO_EXCEPTION(writer->Update());

  std::cout << "Test finished" << std::endl;
  return EXIT_SUCCESS;
}
