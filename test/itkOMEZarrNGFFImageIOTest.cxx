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
#include "xtensor-io/xio_gzip.hpp"
#include "xtensor-zarr/xzarr_compressor.hpp"

#define SPECIFIC_IMAGEIO_MODULE_TEST


int
itkOMEZarrNGFFImageIOTest(int argc, char * argv[])
{
  // xt::xzarr_register_compressor<xt::xzarr_file_system_store, xt::xio_binary_config>();
  // xt::xzarr_register_compressor<xt::xzarr_file_system_store, xt::xio_gzip_config>();
  xt::xzarr_register_compressor<xt::xzarr_file_system_store, xt::xio_blosc_config>();

  {
    // open a hierarchy
    xt::xzarr_file_system_store store("test.zr3");
    auto                        h = xt::create_zarr_hierarchy(store);
    // create an array in the hierarchy
    nlohmann::json      attrs = { { "question", "life" }, { "answer", 42 } };
    std::vector<size_t> shape = { 4, 4 };
    std::vector<size_t> chunk_shape = { 2, 2 };
    // specify options
    xt::xzarr_create_array_options<xt::xio_blosc_config> o;
    o.chunk_memory_layout = 'C';
    o.chunk_separator = '/';
    o.attrs = attrs;
    o.chunk_pool_size = 2;
    o.fill_value = 1.2;

    xt::zarray z = h.create_array("/arthur/dent", // path to the array in the store
                                  shape,          // array shape
                                  chunk_shape,    // chunk shape
                                  "<f8",          // data type, here little-endian 64-bit floating point
                                  o               // options
    );

    // write array data
    auto a = z.get_array<double>();
    a(2, 1) = 3.;

    // read array data
    std::cout << a(2, 1) << std::endl;
    // prints `3.`
    std::cout << a(2, 2) << std::endl;
    // prints `1.2` (fill value)

    // the below properly works only with version 3 of the format

    // create a group
    auto g = h.create_group("/tricia/mcmillan", attrs);
    std::cout << g.attrs() << std::endl;
    // prints `{"answer":42,"question":"life"}`

    // explore the hierarchy
    // get children at a point in the hierarchy
    std::string children = h.get_children("/").dump();
    std::cout << children << std::endl;
    // prints `{"arthur":"implicit_group","tricia":"implicit_group"}`
    // view the whole hierarchy
    std::string nodes = h.get_nodes().dump();
    std::cout << nodes << std::endl;
    // prints `{"arthur\\dent":"array","tricia\\mcmillan":"explicit_group"}`
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
