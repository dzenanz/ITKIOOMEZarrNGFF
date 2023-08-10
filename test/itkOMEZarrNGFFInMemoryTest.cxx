/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include <fstream>
#include <string>
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkOMEZarrNGFFImageIO.h"
#include "itkOMEZarrNGFFImageIOFactory.h"
#include "itkTestingMacros.h"
#include "itkImageIOBase.h"
#include "itkTestingComparisonImageFilter.h"


namespace
{
std::string
makeMemoryFileName(const itk::OMEZarrNGFFImageIO::BufferInfo & bufferInfo)
{
  size_t bufferInfoAddress = reinterpret_cast<size_t>(&bufferInfo);
  return std::to_string(bufferInfoAddress) + ".memory";
}

template <typename PixelType, unsigned Dimension>
int
doTest(const char * inputFileName, const char * outputFileName)
{
  using ImageType = itk::Image<PixelType, Dimension>;

  //// Read the image from .zip as a baseline
  using ReaderType = itk::ImageFileReader<ImageType>;
  typename ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputFileName);

  itk::OMEZarrNGFFImageIO::Pointer zarrIO = itk::OMEZarrNGFFImageIO::New();
  reader->SetImageIO(zarrIO);
  ITK_TRY_EXPECT_NO_EXCEPTION(reader->Update());
  typename ImageType::Pointer image = reader->GetOutput();
  image->DisconnectPipeline(); // this is our baseline image

  //// Copy zip data into stack memory and then read from the raw memory buffer

  // Copy zip into stack memory
  // Adapted from https://stackoverflow.com/a/18816228/276168
  std::ifstream   file(inputFileName, std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<char> buffer(size);
  if (!file.read(buffer.data(), size))
  {
    itkGenericExceptionMacro(<< "Could not read the input file directly: " << inputFileName);
  }
  file.close();

  // Read the zip-compressed bitstream from stack memory into an ITK image
  itk::OMEZarrNGFFImageIO::BufferInfo bufferInfo{ buffer.data(), buffer.size() };
  const auto initBufferInfoPtr = &bufferInfo;
  std::string                         memAddress = makeMemoryFileName(bufferInfo);

  typename ReaderType::Pointer memReader = ReaderType::New();
  memReader->SetFileName(memAddress);
  memReader->SetImageIO(itk::OMEZarrNGFFImageIO::New());
  ITK_TRY_EXPECT_NO_EXCEPTION(memReader->Update());
  typename ImageType::Pointer memImage = memReader->GetOutput();

  // Validate both read methods returned the same image
  using CompareType = itk::Testing::ComparisonImageFilter<ImageType, ImageType>;
  typename CompareType::Pointer comparer = CompareType::New();
  comparer->SetValidInput(image);
  comparer->SetTestInput(memImage);
  comparer->Update();
  if (comparer->GetNumberOfPixelsWithDifferences() > 0)
  {
    itkGenericExceptionMacro("The image read through memory is different from the one read through file");
  }

  // Verify a local copy of the buffer is maintained in the itk::Image
  memImage->DisconnectPipeline(); // keep our local copy of the memory buffer
  std::fill(buffer.begin(), buffer.end(), 0);   // reset the memory buffer in place
  ITK_TRY_EXPECT_NO_EXCEPTION(comparer->Update()); // should not reflect buffer update
  if (comparer->GetNumberOfPixelsWithDifferences() > 0)
  {
    itkGenericExceptionMacro("After deallocating the memory buffer, the image read through memory is different from "
                             "the one read through file");
  }

  // Verify our memory buffer is still valid
  std::cout << static_cast<void *>(bufferInfo.pointer) << std::endl;
  std::cout << bufferInfo.size << std::endl;
  ITK_TEST_EXPECT_EQUAL(bufferInfo.pointer, buffer.data());
  ITK_TEST_EXPECT_EQUAL(bufferInfo.size, buffer.size());

  // Clear members of "BufferInfo" at "memAddress"
  /*bufferInfo = itk::OMEZarrNGFFImageIO::BufferInfo{};
  ITK_TEST_EXPECT_EQUAL(bufferInfo.pointer, nullptr);
  ITK_TEST_EXPECT_EQUAL(bufferInfo.size, 0);
  ITK_TEST_EXPECT_EQUAL(&bufferInfo, initBufferInfoPtr);*/

  // Write image back to in-memory buffer as a zip compressed bitstream
  using WriterType = itk::ImageFileWriter<ImageType>;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetInput(image);
  writer->SetFileName(memAddress);
  writer->SetImageIO(zarrIO);
  ITK_TRY_EXPECT_NO_EXCEPTION(writer->Update());

  std::cout << static_cast<void *>(bufferInfo.pointer) << std::endl;
  std::cout << bufferInfo.size << std::endl;
  //ITK_TEST_EXPECT_EQUAL(bufferInfo.pointer, buffer.data()); // TESTME
  //ITK_TEST_EXPECT_EQUAL(bufferInfo.size, buffer.size());
  // verify we can access the memory at the end of the data buffer

  // Write zip bitstream to disk
  std::ofstream oFile(outputFileName, std::ios::binary);
  oFile.write(bufferInfo.pointer, bufferInfo.size);
  oFile.close();
  free(bufferInfo.pointer);

  // Validate output file was written and is available for read
  std::ifstream oFileResult(outputFileName);
  ITK_TEST_EXPECT_TRUE(oFileResult.good());
  std::cout << "Output image written to " << std::string(outputFileName) << std::endl;

  // Validate output file can be read back in
  auto outputImage = itk::ReadImage<ImageType>(outputFileName);
  outputImage->Print(std::cout);

  std::cout << "Test finished" << std::endl;

  return EXIT_SUCCESS;
}

template <typename PixelType>
int
doTest(const char * inputFileName, const char * outputFileName, unsigned dimension)
{
  switch (dimension)
  {
    case 2:
      return doTest<PixelType, 2>(inputFileName, outputFileName);
    case 3:
      return doTest<PixelType, 3>(inputFileName, outputFileName);
    case 4:
      return doTest<PixelType, 4>(inputFileName, outputFileName);
    case 5:
      return doTest<PixelType, 5>(inputFileName, outputFileName);
    default:
      itkGenericExceptionMacro("Unsupported image dimension: " << dimension);
      break;
  }
}
} // namespace

int
itkOMEZarrNGFFInMemoryTest(int argc, char * argv[])
{
  if (argc < 3)
  {
    std::cerr << "Missing parameters." << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << itkNameOfTestExecutableMacro(argv) << " Input.zip Output.zip" << std::endl;
    return EXIT_FAILURE;
  }
  const char * inputFileName = argv[1];
  const char * outputFileName = argv[2];

  itk::OMEZarrNGFFImageIOFactory::RegisterOneFactory();

  using ImageType = itk::Image<unsigned char, 3>;
  auto imageReader = itk::ImageFileReader<ImageType>::New();
  imageReader->SetFileName(inputFileName);
  imageReader->SetImageIO(itk::OMEZarrNGFFImageIO::New()); // explicitly request zarr IO
  imageReader->UpdateOutputInformation();

  unsigned dim = imageReader->GetImageIO()->GetNumberOfDimensions();
  auto     componentType = imageReader->GetImageIO()->GetComponentType();

  switch (componentType)
  {
    case itk::ImageIOBase::IOComponentEnum::UCHAR:
      return doTest<unsigned char>(inputFileName, outputFileName, dim);
      break;
    case itk::ImageIOBase::IOComponentEnum::CHAR:
      return doTest<char>(inputFileName, outputFileName, dim);
      break;
    case itk::ImageIOBase::IOComponentEnum::USHORT:
      return doTest<unsigned short>(inputFileName, outputFileName, dim);
      break;
    case itk::ImageIOBase::IOComponentEnum::SHORT:
      return doTest<short>(inputFileName, outputFileName, dim);
      break;
    case itk::ImageIOBase::IOComponentEnum::FLOAT:
      return doTest<float>(inputFileName, outputFileName, dim);
      break;
    case itk::ImageIOBase::IOComponentEnum::DOUBLE:
      return doTest<double>(inputFileName, outputFileName, dim);
      break;
    default:
      std::cerr << "Unsupported input image pixel component type: ";
      std::cerr << itk::ImageIOBase::GetComponentTypeAsString(componentType);
      std::cerr << std::endl;
      return EXIT_FAILURE;
      break;
  }
}
