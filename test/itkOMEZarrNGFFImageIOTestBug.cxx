#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"

#include "netcdf.h"
#define BAIL(e)                                                                                 \
  do                                                                                            \
  {                                                                                             \
    printf("Bailing out in file %s, line %d, error:%s.\n", __FILE__, __LINE__, nc_strerror(e)); \
    return EXIT_FAILURE;                                                                        \
  } while (0)

int
main()
{
  const unsigned Dim = 3;
  using ImageType = itk::Image<short, Dim>;
  auto image = itk::ReadImage<ImageType>("CBCT.nrrd");

  ImageType::SizeType size = image->GetLargestPossibleRegion().GetSize(); // 667 667 433

  int ncid, temp_varid, dimids[Dim];
  int res;

  /* Create the netCDF file. */
  // if ((res = nc_create("file://./CBCT.zip#mode=nczarr,zip", NC_CLOBBER, &ncid))) // zip not in Ubuntu 22.04 apt package
  if ((res = nc_create("file://./CBCT.zarr#mode=nczarr,file", NC_CLOBBER, &ncid)))
    // if ((res = nc_create("CBCT.nc", NC_CLOBBER | NC_NETCDF4, &ncid)))
    // if ((res = nc_create("CBCT.nc", NC_CLOBBER, &ncid)))
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



  return EXIT_SUCCESS;
}
