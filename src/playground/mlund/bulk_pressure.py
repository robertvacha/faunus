#! /usr/bin/env python
from math import pi,sinh,sqrt,exp
import numpy as np

N=10.    # num. particles
Q=5.
L=80.
R=10.

Vf = L**3 - N*4*pi*R**3/3.

Cp = N/L**3*1660.58 # protein concentration [mol/l]
Cc = Cp*abs(Q)      # counter ion conc [mol/l]
I = 0.5*Cc          # ionic strength [mol/l]
Debye=3.04/sqrt(I)  # Debye length (AA)
k=1/Debye           # inv. debye len

print "particle charge    = ", Q
print "num. particles     = ", N
print "box length         = ", L
print "Conc. (M)          = ", Cp
print "Counter conc (M)   = ", Cc
print "Counter conc,free  = ", Cc*L**3/Vf
print "Ionic strength     = ", I
print "Ionic strength     = ", I*L**3/Vf
print "Debye length       = ", Debye
print "Kappa              = ", 1/Debye

Pid = Cc*1e3
Pex = -(1/Debye)**3/24/pi*1660.58*1e3

_Pid = 48
_Pex = 17

print "P ideal  = ", Pid, Pid+_Pid
print "P excess = ", Pex, Pex+_Pex
print "P total  = ", Pid + Pex, Pid+_Pid+Pex+_Pex

print "osm coeff = ",(Pid+_Pid+Pex+_Pex) / (Pid+_Pid)

