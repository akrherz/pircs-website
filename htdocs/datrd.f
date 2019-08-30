       SUBROUTINE DATRD (PST, U, V, THETA, Q,
     1   NX, NY, NZ, KNX, KNY, KNZ, 
     1   IYEAR, IMO, IDAY, IHOUR, SWLON, SWLAT, DELON, DELAT, NIN)
       DIMENSION PST(NX,NY)
       DIMENSION U(NX,NY,NZ)
       DIMENSION V(NX,NY,NZ)
       DIMENSION THETA(NX,NY,NZ)
       DIMENSION Q(NX,NY,NZ)

C-------- Subroutine to read the Hadley Centre monthly data.

C-------- On entry:
C            NX, NY, NZ = dimensions of the data arrays.  These
C               must be greater than or equal to the actual number
C               of values along each coordinate.
C            NIN = Fortran logical unit number for the data file.
C            
C-------- On return:
C            PST = array of p-star (scaling pressure), in Pascals
C            U, V = arrays of U and V wind components, in m/s
C            THETA = array of potential temperature, in Kelvins
C            Q = array of specific humidity.  Dimensionless; may be
C               interpreted as kg/kg
C            KNX, KNY, KNZ = actual number of data values along each
C               coordinate (KNZ applies only to 3D arrays)
C            IYEAR, IMO, IDAY = year, month and day of this data record
C            IHOUR = hour (0-23) of this data record.         
C            SWLON, SWLAT = longitude and latitude of the grid point
C               at the southwest corner of the thermodynamic grid.
C               The velocity points are staggered 1/2 grid point
C               south and east of the thermodynamic points.
C            DELON, DELAT = grid spacing (degrees) for longitude
C               and latitude. 

           
C-------- Read and echo the number of grid points in this
C-------- data set for each coordinate.

       READ (NIN)  KNX, KNY, KNZ

C-------- Check if the data arrays are large enough.

       IF (KNX .GT. NX .OR. KNY .GT. NY .OR. KNZ .GT. NZ)  THEN
          WRITE (6,6001)  NX, NY, NZ, KNX, KNY, KNZ
          STOP
       ELSE
       END IF

6001   FORMAT (///20X, 'STOPPING EXECUTION:'/
     1   25X, 'Data array in DATRD is too small.'/
     1   25X, 'Array dimensions are ', 3 I5/
     1   25X, 'Data dimensions are  ', 3 I5)


C-------- Read and echo the date and time.

       READ (NIN)  IYEAR, IMO, IDAY, IHOUR

       WRITE (6,*)  KNX, KNY, KNZ
       WRITE (6,*)  IYEAR, IMO, IDAY, IHOUR

C-------- Grid information: southwest corner and delta lat/lon

       READ (NIN)  SWLON, SWLAT, DELON, DELAT

C-------- Read the data fields into their respective arrays:

C-------- p*
       READ (NIN) ((PST(I,J),I=1,KNX),J=1,KNY)
C-------- u component
       READ (NIN) (((U(I,J,K),I=1,KNX),J=1,KNY),K=1,KNZ)
C-------- v component
       READ (NIN) (((V(I,J,K),I=1,KNX),J=1,KNY),K=1,KNZ)
C-------- potential temperature
       READ (NIN) (((THETA(I,J,K),I=1,KNX),J=1,KNY),K=1,KNZ)
C-------- specific humidity
       READ (NIN) (((Q(I,J,K),I=1,KNX),J=1,KNY),K=1,KNZ)


       RETURN
       END
