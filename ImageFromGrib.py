from mpl_toolkits.basemap import Basemap, shiftgrid
import gdal
import matplotlib.pyplot as plt
import numpy as np
import struct

class Data:

    def __init__( self, link ):

        self.source = link
        self.x = self.opener()[0]
        self.y = self.opener()[1]
        self.band = self.opener()[2]
        self.data = self.opener()[3]

    def opener( self ):

        ds = gdal.Open( self.source, gdal.GA_ReadOnly )

        data = ds.ReadAsArray()

        numOfBands = ds.RasterCount

        band = []

        for i in range( 1, numOfBands + 1 ):

            band.insert( i, ds.GetRasterBand( i ).GetMetadata().get('GRIB_COMMENT') )

        gt = ds.GetGeoTransform()
        proj = ds.GetProjection()

        xres = gt[1]
        yres = gt[5]

        xsize = ds.RasterXSize
        ysize = ds.RasterYSize

        ds = None

        # get the edge coordinates and add half the resolution 
        # to go to center coordinates
        xmin = gt[0] + xres * 0.5
        xmax = gt[0] + (xres * xsize) - xres * 0.5
        ymin = gt[3] + (yres * ysize) + yres * 0.5
        ymax = gt[3] - yres * 0.5

        # Create the latitudes and longitudes according to our minimum and maximum values,
        # also according to the step or resolution of the grid
        xx = np.arange( xmin, xmax + xres, xres )
        yy = np.arange( ymax + yres, ymin, yres )

        data, xx = shiftgrid( 180.0, data, xx, start = False )

        x, y = np.meshgrid( xx, yy )

        return x, y, band, data;


# Note that the links change to the location in your own computer where you store the grib2 files.
# grib2 file.
linkOne = "D:\Downloads\gfs.grb2"
dataOne = Data( linkOne )
# Latitude and Longitude.
lat = dataOne.x
lon = dataOne.y
meta = dataOne.band
data = dataOne.data


counter = 1
for i in meta:
    print( 'Index ' + str( counter ) + ': ' + i )
    counter += 1

index = int( raw_input( "Insert index to get data from: " ) )

# Construct a map from the data we already have using Basemap.
# Mercator
#m = Basemap(projection='merc',llcrnrlat=-85,urcrnrlat=85,\
#            llcrnrlon=-180,urcrnrlon=180,lat_ts=0,resolution='c')
# Robinson
#m = Basemap(projection='robin', lon_0=0, resolution='c')
# Cylindrical
m = Basemap(llcrnrlon=-180.0,llcrnrlat=-85.0,urcrnrlon=180.0,urcrnrlat=85.0,
            projection='cyl',lon_0=0.0,lat_0=0.0)
# plot the data (first layer) data[0,:,:].T
im1 = m.pcolormesh( lat, lon, data[index,:,:], shading = "flat", cmap=plt.cm.jet )
# annotate
#m.drawcountries()
#m.drawcoastlines(linewidth=.5)
plt.show()
