.. _raster.gpkg:

================================================================================
GPKG -- GeoPackage raster
================================================================================

.. shortname:: GPKG

.. build_dependencies:: libsqlite3 (and any or all of PNG, JPEG, WEBP drivers)

This driver implements full read/creation/update
of tables containing raster tiles in the `OGC GeoPackage format
standard <http://www.geopackage.org/spec/>`__. The GeoPackage standard
uses a SQLite database file as a generic container, and the standard
defines:

-  Expected metadata tables (``gpkg_contents``,
   ``gpkg_spatial_ref_sys``, ``gpkg_tile_matrix``,
   ``gpkg_tile_matrix_set``, ...)
-  Tile format encoding (PNG and JPEG for base specification, WebP as
   extension) and tiling conventions
-  Naming and conventions for extensions

This driver reads and writes SQLite files from the file system, so it
must be run by a user with read/write access to the files it is working
with.

The driver can also handle GeoPackage vectors. See :ref:`GeoPackage
vector <vector.gpkg>` documentation page

Various kind of input datasets can be converted to GeoPackage raster :

-  Single band grey level
-  Single band with R,G,B or R,G,B,A color table
-  Two bands: first band with grey level, second band with alpha channel
-  Three bands: Red, Green, Blue
-  Four band: Red, Green, Blue, Alpha

GeoPackage rasters only support Byte data type.

All raster extensions standardized by the GeoPackage specification are
supported in read and creation :

-  *gpkg_webp*: when storing WebP tiles, provided that GDAL is compiled
   against libwebp.
-  *gpkg_zoom_other*: when resolution of consecutive zoom levels does
   not vary with a factor of 2.

Driver capabilities
-------------------

.. supports_createcopy::

.. supports_create::

.. supports_georeferencing::

.. supports_virtualio::

Opening options
---------------

By default, the driver will expose a GeoPackage dataset as a four band
(Red,Green, Blue,Alpha) dataset, which gives the maximum compatibility
with the various encodings of tiles that can be stored. It is possible
to specify an explicit number of bands with the BAND_COUNT opening
option.
Starting with GDAL 3.6, a special metadata item is written when creating
a GeoPackage raster with GDAL (when using the default CUSTOM tiling scheme),
and it is used on reading as the default number of bands.

The driver will use the geographic/projected extent indicated in the
`gpkg_contents <http://www.geopackage.org/spec/#_contents>`__ table, and
do necessary clipping, if needed, to respect that extent. However that
information being optional, if omitted, the driver will use the extent
provided by the
`gpkg_tile_matrix_set <http://www.geopackage.org/spec/#_tile_matrix_set>`__,
which covers the extent at all zoom levels. The user can also specify
the USE_TILE_EXTENT=YES open option to use the actual extent of tiles at
the maximum zoom level. Or it can specify any of MINX/MINY/MAXX/MAXY to
have a custom extent.

|about-open-options|
The following open options are available:

-  .. oo:: TABLE
      :choices: <table_name>

      Name of the table containing the tiles
      (called `"Tile Pyramid User Data
      Table" <http://www.geopackage.org/spec/#tiles_user_tables>`__ in the
      GeoPackage specification language). If the GeoPackage dataset only
      contains one table, this option is not necessary. Otherwise, it is
      required.

-  .. oo:: ZOOM_LEVEL

      Integer value between 0 and the maximum
      filled in the *gpkg_tile_matrix* table. By default, the driver will
      select the maximum zoom level, such as at least one tile at that zoom
      level is found in the raster table.

-  .. oo:: BAND_COUNT
      :choices: AUTO, 1, 2, 3, 4

      Number of bands of the dataset exposed
      after opening. Only used for Byte data type.
      Some conversions will be done when possible and
      implemented, but this might fail in some cases, depending on the
      BAND_COUNT value and the number of bands of the tile.
      Before GDAL 3.6, the default value is 4 (which is the always safe value).
      Starting with GDAL 3.6, when the metadata of the file contains an hint
      of the number of bands, this one is used in AUTO mode (default value), or
      fallback to 4 when it is not present.

-  .. oo:: MINX

      Minimum longitude/easting of the area of interest.

-  .. oo:: MINY

      Minimum latitude/northing of the area of interest.

-  .. oo:: MAXX

      Maximum longitude/easting of the area of interest.

-  .. oo:: MAXY

      Maximum latitude/northing of the area of interest.

-  .. oo:: USE_TILE_EXTENT
      :choices: YES, NO
      :default: NO

      Whether to use the extent of actual
      existing tiles at the zoom level of the full resolution dataset.

-  .. oo:: TILE_FORMAT
      :choices: PNG_JPEG, PNG, PNG8, JPEG, WEBP

      Format used to store
      tiles. See :ref:`raster.gpkg.tile_formats`. Only used in
      update mode and for Byte data type.
      Defaults to PNG_JPEG, unless, starting with GDAL 3.6, if the
      raster has one band, in which case PNG is used.

-  .. oo:: QUALITY
      :choices: 1-100
      :default: 75

      Quality setting for JPEG and WEBP compression.
      Only used in update mode.

-  .. oo:: ZLEVEL
      :choices: 1-9
      :default: 6

      DEFLATE compression level for PNG tiles. Only used
      in update mode.

-  .. oo:: DITHER
      :choices: YES, NO
      :default: NO

      Whether to use Floyd-Steinberg dithering (for
      :co:`TILE_FORMAT=PNG8`). Only used in update mode.

Note: open options are typically specified with "-oo name=value" syntax
in most GDAL utilities, or with the GDALOpenEx() API call.

Creation issues
---------------

Depending of the number of bands of the input dataset and the tile
format selected, the driver will do the necessary conversions to be
compatible with the tile format.

To add several tile tables to a GeoPackage dataset (seen as GDAL
subdatasets), or to add a tile table to an existing vector-only
GeoPackage, the generic APPEND_SUBDATASET=YES creation option must be
provided.

Fully transparent tiles will not be written to the database, as allowed
by the format.

The driver implements the Create() and IWriteBlock() methods, so that
arbitrary writing of raster blocks is possible, enabling the direct use
of GeoPackage as the output dataset of utilities such as gdalwarp.

On creation, raster blocks can be written only if the geotransformation
matrix has been set with SetGeoTransform() This is effectively needed to
determine the zoom level of the full resolution dataset based on the
pixel resolution, dataset and tile dimensions.

Technical/implementation note: when a dataset is opened with a
non-default area of interest (i.e. use of MINX,MINY,MAXX,MAXY or
USE_TILE_EXTENT open option), or when creating/ opening a dataset with a
non-custom tiling scheme, it is possible that GDAL blocks do not exactly
match a single GeoPackage tile. In which case, each GDAL block will
overlap four GeoPackage tiles. This is easily handled on the read side,
but on creation/update side, such configuration could cause numerous
decompression/ recompression of tiles to be done, which might cause
unnecessary quality loss when using lossy compression (JPEG, WebP). To
avoid that, the driver will create a temporary database next to the main
GeoPackage file to store partial GeoPackage tiles in a lossless (and
uncompressed) way. Once a tile has received data for its four quadrants
and for all the bands (or the dataset is closed or explicitly flushed
with FlushCache()), those uncompressed tiles are definitely transferred
to the GeoPackage file with the appropriate compression. All of this is
transparent to the user of GDAL API/utilities

The driver updates the GeoPackage ``last_change`` timestamp when the file is
created or modified. If consistent binary output is required for
reproducibility, the timestamp can be forced to a specific value by setting the
:config:`OGR_CURRENT_DATE` global configuration option.
When setting the option, take care to meet the specific time format
requirement of the GeoPackage standard,
e.g. `for version 1.2 <https://www.geopackage.org/spec120/#r15>`__.


.. _raster.gpkg.tile_formats:

Tile formats
~~~~~~~~~~~~

Tiled rasters
^^^^^^^^^^^^^

This section only applies for raster of Byte data type.

GeoPackage can store tiles in different formats, PNG and/or JPEG for the
baseline specification, and WebP for extended GeoPackage. Support for
those tile formats depend if the underlying drivers are available in
GDAL, which is generally the case for PNG and JPEG, but not necessarily
for WebP since it requires GDAL to be compiled against the optional
libwebp.

By default, GDAL will use a mix of PNG and JPEG tiles (PNG_JPEG tile
format, or AUTO). PNG tiles will be used to store tiles that are not
completely opaque, either because input dataset has an alpha channel
with non fully opaque content, or because tiles are partial due to
clipping at the right or bottom edges of the raster, or when a dataset
is opened with a non-default area of interest, or with a non-custom
tiling scheme. On the contrary, for fully opaque tiles, JPEG format will
be used.
Starting with GDAL 3.6, if the raster has only one band, the default is PNG.

It is possible to select one unique tile format by setting the
creation/open option TILE_FORMAT to one of PNG, JPEG or WEBP. When using
JPEG, the alpha channel will not be stored. When using WebP, the
`gpkg_webp <http://www.geopackage.org/spec/#extension_tiles_webp>`__
extension will be registered. The lossy compression of WebP is used.
Note that a recent enough libwebp (>=0.1.4) must be used to support
alpha channel in WebP tiles.

PNG8 can be selected to use 8-bit PNG with a color table up to 256
colors. On creation, an optimized color table is computed for each tile.
The DITHER option can be set to YES to use Floyd/Steinberg dithering
algorithm, which spreads the quantization error on neighbouring pixels
for better rendering (note however than when zooming in, this can cause
non desirable visual artifacts). Setting it to YES will generally cause
less effective compression. Note that at that time, such an 8-bit PNG
formulation is only used for fully opaque tiles, as the median-cut
algorithm currently implemented to compute the optimal color table does
not support alpha channel (even if PNG8 format would potentially allow
color table with transparency). So when selecting PNG8, non fully opaque
tiles will be stored as 32-bit PNG.

Tiled gridded coverage data
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Since GDAL 2.3, `tiled gridded coverage
data <http://docs.opengeospatial.org/is/17-066r1/17-066r1.html#27>`__
can be stored using PNG unsigned 16bit tiles (with potential offset and
scaling so as to be able to represent floating point data) or TIFF
32-bit floating-point LZW compressed tiles.

When converting a GDAL Int16 or UInt16 dataset, PNG tiles will be used.
When converting a GDAL Float32 dataset, TIFF tiles will be used by
default, unless PNG is explicitly selected, in which case scaling and
offsetting will be automatically computed for each tile.

.. warning::

    The `tiled gridded
    extension <http://www.geopackage.org/spec/#extension_tiled_gridded_elevation_data>`__
    initially implemented in GDAL 2.2 was not officially adopted and had
    been later reworked by OGC. The adopted `tiled gridded coverage
    data <http://docs.opengeospatial.org/is/17-066r1/17-066r1.html#27>`__
    has a few differences that will make GDAL 2.2 datasets not be compliant
    with the final extension. GDAL 2.3 can open those GDAL 2.2-generated
    files.

.. _raster.gpkg.tiling_schemes:

Tiling schemes
~~~~~~~~~~~~~~

By default, conversion to GeoPackage will create a custom tiling scheme,
such that the input dataset can be losslessly converted, both at the
pixel and georeferencing level (if using a lossless tile format such as
PNG). That tiling scheme is such that its origin (*min_x*, *max_y*) in
the
`gpkg_tile_matrix_set <http://www.geopackage.org/spec/#_tile_matrix_set>`__
table perfectly matches the top left corner of the dataset, and the
selected resolution (*pixel_x_size*, *pixel_y_size*) at the computed
maximum zoom_level of the
`gpkg_tile_matrix <http://www.geopackage.org/spec/#_tile_matrix>`__
table will match the pixel width and height of the raster.

However to ease interoperability with other implementations, and enable
use of GeoPackage with tile servicing software, it is possible to select
a predefined tiling scheme that has world coverage. The available tiling
schemes are :

-  *GoogleMapsCompatible*, as described in WMTS 1.0 specification, Annex
   E.4. That tiling schemes consists of a single 256x256 tile at its
   zoom level 0, in EPSG:3857 CRS, with extent in easting and northing
   in the range [-20037508.34,20037508.34].
-  *InspireCRS84Quad*, as described in `Inspire View
   Services <http://inspire.ec.europa.eu/documents/Network_Services/TechnicalGuidance_ViewServices_v3.0.pdf>`__.
   That tiling schemes consists of two 256x256 tiles at its zoom level
   0, in EPSG:4326 CRS, with extent in longitude in the range [-180,180]
   and in latitude in the range [-90,90].
-  *PseudoTMS_GlobalGeodetic*, based on the
   `global-geodetic <http://wiki.osgeo.org/wiki/Tile_Map_Service_Specification#global-geodetic>`__
   profile of OSGeo TMS (Tile Map Service) specification. This has
   exactly the same definition as *InspireCRS84Quad* tiling scheme. Note
   however that full interoperability with TMS is not possible due to
   the origin of numbering of tiles being the top left corner in
   GeoPackage (consistently with WMTS convention), whereas TMS uses the
   bottom left corner as origin.
-  *PseudoTMS_GlobalMercator*, based on the
   `global-mercator <http://wiki.osgeo.org/wiki/Tile_Map_Service_Specification#global-mercator>`__
   profile of OSGeo TMS (Tile Map Service) specification. That tiling
   schemes consists of four 256x256 tiles at its zoom level 0, in
   EPSG:3857 CRS, with extent extent in easting and northing in the
   range [-20037508.34,20037508.34]. The same remark as with
   PseudoTMS_GlobalGeodetic applies regarding interoperability with TMS.
-  *GoogleCRS84Quad*, as described in `OGC 07-057r7 WMTS
   1.0 <http://portal.opengeospatial.org/files/?artifact_id=35326>`__
   specification, Annex E.3. That tiling schemes consists of a single
   256x256 tile at its zoom level 0, in EPSG:4326 CRS, with extent in
   longitude and latitude in the range [-180,180]. Consequently, at zoom
   level 0, 64 lines are unused at the top and bottom of that tile. This
   may cause issues with some implementations of the specification, and
   there are some ambiguities about the exact definition of this tiling
   scheme. Using InspireCRS84Quad/PseudoTMS_GlobalGeodetic instead is
   therefore recommended.
   NOTE: `OGC WMTS Simple Profile
   13-082r2 <http://docs.opengeospatial.org/is/13-082r2/13-082r2.html#30>`__
   changed the definition of GoogleCRS84Quad (so not implemented by the
   driver). The new definition includes a -1 level (that cannot be
   modeled in GeoPackage given constraints on zoom_level being positive
   or 0), with a single tile at origin -180,90 and whose bottom 128
   lines are empty. Levels 0 or greater are identical to the
   InspireCRS84Quad tiling scheme. So for practical purposes,
   InspireCRS84Quad in GeoPackage is conformant to the new
   GoogleCRS84Quad definition.

In all the above tiling schemes, consecutive zoom levels defer by a
resolution of a factor of two.

Starting with GDAL 3.2, it is also possible to use a Tile Matrix Set definition,
encoded as a JSON file, according to the `OGC Two Dimensional Tile Matrix Set standard`_
Examples of such files can be found at http://schemas.opengis.net/tms/1.0/json/examples/
The GDAL data directory also contains files prefixed with ``tms_`` and with a ``.json``
extension. If there is a ``tms_FOO.json`` file, then ``FOO`` can be used as the
value of the TILING_SCHEME creation option. There are restrictions on the types
of tile matrix set supported:

* all zoom levels must have the same origin
* consecutive zoom levels defer by a resolution of a factor of two.
* all zoom levels must have the same tile dimension
* variable matrix width tile set are not supported.

.. _`OGC Two Dimensional Tile Matrix Set standard`: http://docs.opengeospatial.org/is/17-083r2/17-083r2.html

Nodata value
~~~~~~~~~~~~

The concept of the nodata value is only supported for tiled gridded
elevation datasets. For regular tiled rasters, the alpha band must
rather be used.

For Float32 datasets with TIFF tiles, the concepts of nodata in GDAL and
null_value in the GeoPackage internals perfectly match.

For Int16, UInt16 or Float32 with PNG tiles, GDAL will generally remap
the input nodata value to another value.

On writing, for PNG tiles, the behavior is the following one:

============== =================================================== =====================================================
GDAL data type Input GDAL nodata value                             null_value in GPKG gpkg_2d_gridded_coverage_ancillary
Int16          Any                                                 65535
UInt16         X (if coverage offset == 0 and coverage scale == 1) X
Float32        Any                                                 65535
============== =================================================== =====================================================

On reading, for PNG tiles, the behavior is the following one:

============== ===================================================== =========================
GDAL data type null_value in GPKG gpkg_2d_gridded_coverage_ancillary Exposed GDAL nodata value
Int16          >= 32768                                              -32768
Int16          X <= 32767                                            X
UInt16         X                                                     X
Float32        X                                                     X
============== ===================================================== =========================

Thus, perfect roundtripping is achieved in the following cases:

============== =================================================== =====================================================
GDAL data type GDAL nodata value                                   null_value in GPKG gpkg_2d_gridded_coverage_ancillary
Int16          -32768                                              65535
UInt16         X (if coverage offset == 0 and coverage scale == 1) X
Float32        65535                                               65535
============== =================================================== =====================================================

Creation options
~~~~~~~~~~~~~~~~

|about-creation-options|
The following creation options are available:

-  .. co:: RASTER_TABLE

      Name of tile user table. By default, based
      on the filename (i.e. if filename is foo.gpkg, the table will be
      called "foo").

-  .. co:: APPEND_SUBDATASET
      :choices: YES, NO
      :default: NO

      If set to YES, an existing GeoPackage
      will not be priorly destroyed, such as to be able to add new content
      to it.

-  .. co:: RASTER_IDENTIFIER

      Human-readable identifier (e.g. short
      name), put in the *identifier* column of the *gpkg_contents* table.

-  .. co:: RASTER_DESCRIPTION

      Human-readable description, put in
      the *description* column of the *gpkg_contents* table.

-  .. co:: BLOCKSIZE
      :choices: <integer>
      :default: 256

      Block size in width and height in pixels.
      Maximum supported is 4096. Should not be set when
      using a non-custom :co:`TILING_SCHEME`.

-  .. co:: BLOCKXSIZE
      :choices: <integer>
      :default: 256

      Block width in pixels. Maximum supported is 4096.

-  .. co:: BLOCKYSIZE
      :choices: <integer>
      :default: 256

      Block height in pixels. Maximum supported is 4096.

-  .. co:: TILE_FORMAT
      :choices: PNG_JPEG, PNG, PNG8, JPEG, WEBP, TIFF, AUTO
      :default: AUTO

      Format used to store tiles. See :ref:`raster.gpkg.tile_formats`.

-  .. co:: QUALITY
      :choices: 1-100
      :default: 75

      Quality setting for JPEG and WEBP compression.

-  .. co:: ZLEVEL
      :choices: 1-9
      :default: 6

      DEFLATE compression level for PNG tiles.

-  .. co:: DITHER
      :choices: YES, NO
      :default: NO

      Whether to use Floyd-Steinberg dithering (for
      :co:`TILE_FORMAT=PNG8`).

-  .. co:: TILING_SCHEME
      :choices: CUSTOM, GoogleCRS84Quad, GoogleMapsCompatible, InspireCRS84Quad, PseudoTMS_GlobalGeodetic, PseudoTMS_GlobalMercator, other
      :default: CUSTOM

      See :ref:`raster.gpkg.tiling_schemes`.

      Starting with GDAL 3.2, the value of TILING_SCHEME can also be the filename
      of a JSON file according to the `OGC Two Dimensional Tile Matrix Set standard`_,
      a URL to such file, the radical of a definition file in the GDAL data directory
      (e.g. ``FOO`` for a file named ``tms_FOO.json``) or the inline JSON definition.
      Note: the TILING_SCHEME option with a non-CUSTOM value is best used
      with the gdal_translate utility / CreateCopy() API operation. If used
      with gdalwarp, it requires setting the -tr switch to the exact value
      expected by one zoom level of the tiling scheme.

      In non-CUSTOM mode, the actual resolution can be controlled with the
      :co:`ZOOM_LEVEL` or :co:`ZOOM_LEVEL_STRATEGY` options.

- .. co:: ZOOM_LEVEL
     :choices: <integer>
     :since: 3.9

     Zoom level number (starting at 0 for
     coarsest zoom level). Only used for :co:`TILING_SCHEME` different from CUSTOM.
     If this option is specified, :co:`ZOOM_LEVEL_STRATEGY` is ignored.

-  .. co:: ZOOM_LEVEL_STRATEGY
      :choices: AUTO, LOWER, UPPER
      :default: AUTO

      Strategy to determine
      zoom level. Only used by CreateCopy() for :co:`TILING_SCHEME` different
      from CUSTOM. LOWER will select the zoom level immediately below the
      theoretical computed non-integral zoom level, leading to subsampling.
      On the contrary, UPPER will select the immediately above zoom level,
      leading to oversampling. Defaults to AUTO which selects the closest
      zoom level.

-  .. co:: RESAMPLING
      :choices: NEAREST, BILINEAR, CUBIC, CUBICSPLINE, LANCZOS, MODE, AVERAGE
      :default: BILINEAR

      Resampling algorithm. Only used by CreateCopy() for TILING_SCHEME
      different from CUSTOM. Defaults to BILINEAR.

-  .. co:: PRECISION
      :choices: <floating_point_value_in_vertical_units>
      :default: 1.0

      Smallest
      significant value. Only used for tile gridded coverage datasets.

-  .. co:: UOM
      :since: 2.3

      Unit of Measurement. Only used for
      tiled gridded coverage datasets. Also set through SetUnitType()

-  .. co:: FIELD_NAME
      :default: Height
      :since: 2.3

      Field name. Only used for tiled gridded coverage datasets.

-  .. co:: QUANTITY_DEFINITION
      :default: Height
      :since: 2.3

      Description of the
      field. Only used for tiled gridded coverage datasets.

-  .. co:: GRID_CELL_ENCODING
      :choices: grid-value-is-center, grid-value-is-area, grid-value-is-corner
      :since: 2.3

      Grid cell encoding. Only used for
      tiled gridded coverage datasets. Defaults to grid-value-is-center,
      when AREA_OR_POINT metadata item is not set.

-  .. co:: VERSION
      :choices: AUTO, 1.0, 1.1, 1.2, 1.3, 1.4
      :since: 2.2

      Set GeoPackage version
      (for application_id and user_version fields). In AUTO mode, this will
      be equivalent to 1.4 starting with GDAL 3.11 (1.2 in prior versions)
      1.3 is available starting with GDAL 3.3
      1.4 is available starting with GDAL 3.7.1

-  .. co:: ADD_GPKG_OGR_CONTENTS
      :choices: YES, NO
      :default: YES
      :since: 2.2

      Defines whether to
      add a gpkg_ogr_contents table to keep feature count for vector
      layers, and associated triggers.

-  .. co:: CRS_WKT_EXTENSION
      :choices: YES, NO
      :default: NO
      :since: 3.8

      Defines whether to add the ``definition_12_063`` column to the
      ``gpkg_spatial_ref_sys`` system table, according to
      http://www.geopackage.org/spec/#extension_crs_wkt . The default is NO,
      unless the tile gridded coverage extension is used.
      With VERSION >= 1.4, a ``epoch`` column is also added.
      WKT strings in ``definition_12_063`` will follow the
      `WKT2:2015 standard <https://docs.ogc.org/is/12-063r5/12-063r5.html>`__
      when possible, but may use the
      `WKT2:2019 standard <https://docs.ogc.org/is/18-010r7/18-010r7.html>`__
      for specific cases (dynamic CRS with coordinate epoch).
      This option generally does not need to be specified, as the driver will
      automatically update the ``gpkg_spatial_ref_sys`` table when needed, but
      it may be useful to create GeoPackage datasets matching the exceptions of
      other software or profiles (such as the DGIWG-GPKG profile).

-  .. co:: METADATA_TABLES
      :choices: YES, NO
      :since: 3.8

      Defines whether to add the metadata system tables.
      By default, they are created on demand.
      If NO is specified, they are not created, even if metadata is set.
      If YES is specified, they are always created.

Overviews
---------

gdaladdo / BuildOverviews() can be used to compute overviews.
Power-of-two overview factors (2,4,8,16,...) should be favored to be
conformant with the baseline GeoPackage specification. Use of other
overview factors will work with the GDAL driver, and cause the
`gpkg_zoom_other <http://www.geopackage.org/spec/#extension_zoom_other_intervals>`__
extension to be registered, but that could potentially cause
interoperability problems with other implementations that do not support
that extension.

Overviews can also be cleared with the -clean option of gdaladdo (or
BuildOverviews() with nOverviews=0)

Metadata
--------

GDAL uses the standardized
`gpkg_metadata <http://www.geopackage.org/spec/#_metadata_table>`__
and
`gpkg_metadata_reference <http://www.geopackage.org/spec/#_metadata_reference_table>`__
tables to read and write metadata.

GDAL metadata, from the default metadata domain and possibly other
metadata domains, is serialized in a single XML document, conformant
with the format used in GDAL PAM (Persistent Auxiliary Metadata)
.aux.xml files, and registered with md_scope=dataset and
md_standard_uri=http://gdal.org in gpkg_metadata. In
gpkg_metadata_reference, this entry is referenced with a
reference_scope=table and table_name={name of the raster table}

It is possible to read and write metadata that applies to the global
GeoPackage, and not only to the raster table, by using the *GEOPACKAGE*
metadata domain.

Metadata not originating from GDAL can be read by the driver and will be
exposed as metadata items with keys of the form GPKG_METADATA_ITEM_XXX
and values the content of the *metadata* columns of the gpkg_metadata
table. Update of such metadata is not currently supported through GDAL
interfaces ( although it can be through direct SQL commands).

The specific DESCRIPTION and IDENTIFIER metadata item of the default
metadata domain can be used in read/write to read from/update the
corresponding columns of the gpkg_contents table.

You can set the CREATE_METADATA_TABLES configuration option to NO to
avoid creating and filling the metadata tables.

IMAGE_STRUCTURE metadata item
-----------------------------

.. note::

    Implementation details, normally transparent to GDAL users, but useful
    for other implementations.

Starting with GDAL 3.6.1, the following optional metadata items can be read and
write into the ``IMAGE_STRUCTURE`` metadata domain, in the
``<GDALMultiDomainMetadata>`` XML element:

- BAND_COUNT=1, 2, 3 or 4. Applies only for Byte data. Set when creating a
  dataset so that GDAL knows the number of bands when reopening it.

- COLOR_TABLE={{r0,g0,b0,a0},...{r255,g255,b255,a255}}.
  Applies only for Byte data and a single band dataset. Set when creating a
  dataset from a source dataset that has a color table.

- TILE_FORMAT=PNG/PNG8/PNG_JPEG/JPEG/WEBP. Set when creating a
  dataset so that GDAL knows the tile format when reopening it, for updates.

- NODATA_VALUE=integer between 0 and 255. Applies only for Byte data.


Example:

.. code-block:: sql

    INSERT INTO gpkg_metadata VALUES(
        1,
        'dataset',
        'http://gdal.org',
        'text/xml',
        '<GDALMultiDomainMetadata><Metadata domain="IMAGE_STRUCTURE"><MDI key="BAND_COUNT">1</MDI><MDI key="NODATA_VALUE">255</MDI></Metadata></GDALMultiDomainMetadata>')
    );
    INSERT INTO gpkg_metadata_reference VALUES(
        'table','my_raster_table',NULL,NULL,'2022-11-09T18:44:59.723Z',1,NULL);


Level of support of GeoPackage Extensions
-----------------------------------------

(Restricted to those have a raster scope)

.. list-table:: Extensions
   :header-rows: 1

   * - Extension name
     - OGC adopted extension ?
     - Supported by GDAL?
   * - `Zoom Other intervals <http://www.geopackage.org/guidance/extensions/zoom_other_intervals.html>`__
     - Yes
     - Yes
   * - `Tiles Encoding WebP <http://www.geopackage.org/guidance/extensions/tiles_encoding_webp.html>`__
     - Yes
     - Yes
   * - `Metadata <http://www.geopackage.org/guidance/extensions/metadata.html>`__
     - Yes
     - Yes
   * - `WKT for Coordinate Reference Systems <http://www.geopackage.org/guidance/extensions/wkt_for_crs.md>`__ (WKT v2)
     - Yes
     - Partially, since GDAL 2.2. GDAL can read databases using this extension. GDAL 3.0 brings support for the WKT v2 entry.
   * - `Tiled Gridded Coverage Data <http://www.geopackage.org/guidance/extensions/tiled_gridded_coverage_data.html>`__
     - Yes
     - Yes, since GDAL 2.3 (GDAL 2.2 supported a preliminary version of this extension)

Examples
--------

-  Simple translation of a GeoTIFF into GeoPackage. The table 'byte'
   will be created with the tiles.

   ::

      gdal_translate -of GPKG byte.tif byte.gpkg

-  Translation of a GeoTIFF into GeoPackage using WebP tiles

   ::

      gdal_translate -of GPKG byte.tif byte.gpkg -co TILE_FORMAT=WEBP

-  Translation of a GeoTIFF into GeoPackage using GoogleMapsCompatible
   tiling scheme (with reprojection and resampling if needed)

   ::

      gdal_translate -of GPKG byte.tif byte.gpkg -co TILING_SCHEME=GoogleMapsCompatible

-  Building of overviews of an existing GeoPackage, and forcing JPEG
   tiles

   ::

      gdaladdo -r cubic -oo TILE_FORMAT=JPEG my.gpkg 2 4 8 16 32 64

-  Addition of a new subdataset to an existing GeoPackage, and choose a
   non default name for the raster table.

   ::

      gdal_translate -of GPKG new.tif existing.gpkg -co APPEND_SUBDATASET=YES -co RASTER_TABLE=new_table

-  Reprojection of an input dataset to GeoPackage

   ::

      gdalwarp -of GPKG in.tif out.gpkg -t_srs EPSG:3857

-  Open a specific raster table in a GeoPackage

   ::

      gdalinfo my.gpkg -oo TABLE=a_table

.. _raster.gpkg.raster:

Raster SQL functions
~~~~~~~~~~~~~~~~~~~~

The raster SQL functions mentioned at :ref:`sql_sqlite_dialect_raster_functions`
are also available.

The ``gdal_get_layer_pixel_value()`` function (added in GDAL 3.7), variant of the
generic ``gdal_get_pixel_value()``, can be used to extract the value of a pixel
in a raster layer of the current dataset.

It takes 5 or 6 arguments:

* a string with the layer/table name
* a band number (numbering starting at 1)
* a string being "georef" to indicate that subsequent values will be georeferenced
  coordinates, or "pixel" to indicate that subsequent values will be in column, line
  pixel space
* georeferenced X value or column number
* georeferenced Y value or line number
* resampling method among ``nearest`` (default), ``bilinear``, ``cubic``, ``cubicspline``. Optional, added in GDAL 3.10

.. code-block::

    SELECT gdal_get_layer_pixel_value('my_raster_table', 1, 'georef', 440720, 3751320)
    SELECT gdal_get_layer_pixel_value('my_raster_table', 1, 'pixel', 0, 0)
    SELECT gdal_get_pixel_value('my_raster_table', 1, 'pixel', 0.5, 0.5, 'bilinear')  -- GDAL >= 3.10

See Also
--------

-  :ref:`GeoPackage vector <vector.gpkg>` documentation page
-  `Getting Started With
   GeoPackage <http://www.geopackage.org/guidance/getting-started.html>`__
-  `OGC GeoPackage format standard <http://www.geopackage.org/spec/>`__
   specification, HTML format (current/development version of the
   standard)
-  `OGC GeoPackage Encoding
   Standard <http://www.opengeospatial.org/standards/geopackage>`__ page
-  `SQLite <http://sqlite.org/>`__
-  :ref:`PNG driver <raster.png>` documentation page
-  :ref:`JPEG driver <raster.jpeg>` documentation page
-  :ref:`WEBP driver <raster.webp>` documentation page
-  `OGC 07-057r7 WMTS
   1.0 <http://portal.opengeospatial.org/files/?artifact_id=35326>`__
   specification
-  `OSGeo TMS (Tile Map
   Service) <http://wiki.osgeo.org/wiki/Tile_Map_Service_Specification>`__
   specification

Other notes
-----------

Development of raster support in the GeoPackage driver was financially
supported by `Safe Software <http://www.safe.com>`__.
