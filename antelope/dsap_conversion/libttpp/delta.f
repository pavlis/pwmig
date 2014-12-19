c
c*******************************************************************************
c
c    FUNCTION DELTA
c
c*******************************************************************************
c
      real*8 function delta (xlat1, xlong1, xlat2, xlong2)
c
      real*8                 xlat1
      real*8                        xlong1
      real*8                                xlat2
      real*8                                       xlong2
c
c    Function delta will return a great circle angular distance in radians
c    between two points on a sphere specified by latitude-longitude
c    coordinates also in radians.
c
      real*8 clat, slat, s
      real*8 x1, y1, z1, x2, y2, z2
c
      slat = dsin(xlat1)
      clat = dcos(xlat1)
      x1 = clat*dcos(xlong1)
      y1 = clat*dsin(xlong1)
      z1 = slat
c
      slat = dsin(xlat2)
      clat = dcos(xlat2)
      x2 = clat*dcos(xlong2)
      y2 = clat*dsin(xlong2)
      z2 = slat
c
      s = dsqrt ( (x1-x2)**2 + (y1-y2)**2 + (z1-z2)**2 )
      delta = 2.0d0 * dasin ( 0.5d0*s )
c
      return
      end

c $Id: delta.f,v 1.1.1.1 1997/04/12 04:18:47 danq Exp $ 
