#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"

#include "netcdf.h"
#define BAIL(e)                                                                                 \
  do                                                                                            \
  {                                                                                             \
    printf("Bailing out in file %s, line %d, error:%s.\n", __FILE__, __LINE__, nc_strerror(e)); \
    return EXIT_FAILURE;                                                                        \
  } while (0)

// Make netCDF call, and error check it.
// Requires variable int r; to be defined.
#define netCDF_call(call)                                  \
  r = call;                                                \
  if (r) /* error */                                       \
  {                                                        \
    nc_close(m_NCID); /* clean up a little */              \
    itkExceptionMacro("netCDF error: " << nc_strerror(r)); \
  }

int
itkOMEZarrNGFFImageIOTestBug(int argc, char * argv[])
{
  const unsigned Dim = 3;
  using ImageType = itk::Image<short, Dim>;
  auto image = itk::ReadImage<ImageType>(argv[1]);

  ImageType::SizeType size = image->GetLargestPossibleRegion().GetSize(); // 667 667 433

  int ncid, temp_varid, dimids[Dim];
  int res;

  std::string ncFilename = "file://" + std::string(argv[2]) + "#mode=nczarr,noxarray,file";

  /* Create the netCDF file. */
  if ((res = nc_create(ncFilename.c_str(), NC_CLOBBER, &ncid)))
    BAIL(res);

  const std::vector<std::string> dimensionNames = { "i", "j", "k", "t" };

  const std::string unitName = "HU"; // Hounsfield Units

  /* Define dimensions. */
  for (unsigned d = 0; d < Dim; ++d)
  {
    if ((res = nc_def_dim(ncid, dimensionNames[Dim - 1 - d].c_str(), size[Dim - 1 - d], &dimids[d])))
      BAIL(res);
  }

  /* Define the variable. */
  if ((res = nc_def_var(ncid, "CT_number", NC_SHORT, Dim, dimids, &temp_varid)))
    BAIL(res);

  /* We'll store the units. */
  if ((res = nc_put_att_text(ncid, temp_varid, "units", unitName.size(), unitName.c_str())))
    BAIL(res);

  /* We're finished defining metadata. */
  if ((res = nc_enddef(ncid)))
    BAIL(res);

  if ((res = nc_put_var_short(ncid, temp_varid, image->GetBufferPointer())))
    BAIL(res);

  /* We're done! */
  if ((res = nc_close(ncid)))
    BAIL(res);

  
  
  // Now read the file for sanity checking
  int m_NCID;
  
  // from OMEZarrNGFFImageIO::ReadImageInformation()
  int r;
  netCDF_call(nc_open(ncFilename.c_str(), NC_NOWRITE, &m_NCID));

  int nDims;
  int nVars;
  int nAttr; // we will ignore attributes for now
  netCDF_call(nc_inq(m_NCID, &nDims, &nVars, &nAttr, nullptr));

  if (nVars == 0)
  {
    itkWarningMacro("The file '" << ncFilename.c_str() << "' contains no arrays.");
    return;
  }

  char    name[NC_MAX_NAME + 1];
  nc_type ncType;
  int     dimIDs[NC_MAX_VAR_DIMS];
  netCDF_call(nc_inq_var(m_GroupID, 0, name, &ncType, &nDims, dimIDs, nullptr));

  if (nVars > 1)
  {
    itkWarningMacro("The file '" << ncFilename.c_str() << "' contains more than one array. The first one (" << name
                                 << ") will be read. The others will be ignored. ");
  }

  int    cDim = 0;
  size_t len;
  netCDF_call(nc_inq_dim(m_GroupID, dimIDs[nDims - 1], name, &len));
  if (std::string(name) == "c") // last dimension is component
  {
    cDim = 1;
    this->SetNumberOfComponents(len);
    this->SetPixelType(CommonEnums::IOPixel::VECTOR); // TODO: RGB/A, Tensor etc.
  }

  this->SetNumberOfDimensions(nDims - cDim);
  for (unsigned d = 0; d < nDims - cDim; ++d)
  {
    netCDF_call(nc_inq_dim(m_GroupID, dimIDs[d], name, &len));
    this->SetDimensions(nDims - cDim - d - 1, len);
  }

  this->SetComponentType(netCDFToITKComponentType(ncType));
  
  // from OMEZarrNGFFImageIO::Read(void * buffer)
  
  int varID = 0; // always zero for now
  netCDF_call(nc_get_var_short(m_GroupID, varID, static_cast<short *>(buffer)));
  netCDF_call(nc_close(m_NCID));

  return EXIT_SUCCESS;
}
