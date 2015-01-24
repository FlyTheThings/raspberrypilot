# -*- coding: utf-8 -*-
"""
Created on Sun Mar 10 14:57:11 2013

@author: soupaman
"""

from pylab import *
from math import *
from random import *
from numpy import *
from insgps_init import *

#-------------------------------------------------------------------

def state_equation(x,u):

    xdot = zeros((13,1))
    
    q0 = x[6,0]
    q1 = x[7,0]
    q2 = x[8,0]
    q3 = x[9,0]
    b_wx = x[10,0]
    b_wy = x[11,0]
    b_wz = x[12,0]
    
    w_mx = u[0,0]
    w_my = u[1,0]
    w_mz = u[2,0]
    a_mx = u[3,0]
    a_my = u[4,0]
    a_mz = u[5,0]    
    
    w_x = w_mx - b_wx
    w_y = w_my - b_wy   
    w_z = w_mz - b_wz    
    
    R_11 = q0**2 + q1**2 - q2**2 - q3**2
    R_12 = 2*(q1*q2 + q0*q3)
    R_13 = 2*(q1*q3 - q0*q2)
    R_21 = 2*(q1*q2 - q0*q3)
    R_22 = q0**2 - q1**2 + q2**2 - q3**2 
    R_23 = 2*(q2*q3 + q0*q1)
    R_31 = 2*(q1*q3 + q0*q2)
    R_32 = 2*(q2*q3 - q0*q1)
    R_33 = q0**2 - q1**2 - q2**2 + q3**2 
    
    xdot[0] = x[3,0]                                    # dPx/dt
    xdot[1] = x[4,0]                                    # dPy/dt
    xdot[2] = x[5,0]                                    # dPz/dt
    xdot[3] = R_11*a_mx + R_21*a_my + R_31*a_mz         # dVx/dt
    xdot[4] = R_12*a_mx + R_22*a_my + R_32*a_mz         # dVy/dt
    xdot[5] = R_13*a_mx + R_23*a_my + R_33*a_mz + g     # dVz/dt
    xdot[6] = -0.5* (q1*w_x + q2*w_y + q3*w_z)          # dq0/dt
    xdot[7] =  0.5* (q0*w_x - q3*w_y + q2*w_z)          # dq1/dt
    xdot[8] =  0.5* (q3*w_x + q0*w_y - q1*w_z)          # dq2/dt
    xdot[9] =  0.5*(-q2*w_x + q1*w_y + q0*w_z)          # dq3/dt
    xdot[10] = 0.0                                      # dbwx/dt
    xdot[11] = 0.0                                      # dbwy/dt
    xdot[12] = 0.0                                      # dbwz/dt
    
    return xdot

#--------------------------------------------------------------------

def runge_kutta(x,u,dt):
    
    k1 = state_equation(x,u)
    k2 = state_equation(x+k1*dt/2,u)    
    k3 = state_equation(x+k2*dt/2,u)
    k4 = state_equation(x+k3*dt/2,u)
    
    xnew = x + dt*(k1 + 2*k2 + 2*k3 + k4)/6
    
    return xnew

#----------------------------------------------------------------------

def linearize_state(x,u):
    
    q0 = x[6,0]
    q1 = x[7,0]
    q2 = x[8,0]
    q3 = x[9,0]
    b_wx = x[10,0]
    b_wy = x[11,0]
    b_wz = x[12,0]
    
    w_mx = u[0,0]
    w_my = u[1,0]
    w_mz = u[2,0]
    a_mx = u[3,0]
    a_my = u[4,0]
    a_mz = u[5,0]

    # Initialize state transition matrix    
    F = zeros((13,13))

    # Pdot = V    
    F[0,3] = 1.0
    F[1,4] = 1.0
    F[2,5] = 1.0
    
    # dVdot/dq
    F_vq0 = 2*( q0*a_mx - q3*a_my + q2*a_mz)
    F_vq1 = 2*( q1*a_mx + q2*a_my + q3*a_mz)
    F_vq2 = 2*(-q2*a_mx + q1*a_my + q0*a_mz)
    F_vq3 = 2*(-q3*a_mx - q0*a_my + q1*a_mz)
    
    F[3,6] =  F_vq0
    F[3,7] =  F_vq1
    F[3,8] =  F_vq2
    F[3,9] =  F_vq3
    F[4,6] = -F_vq3
    F[4,7] = -F_vq2
    F[4,8] =  F_vq1
    F[4,9] =  F_vq0
    F[5,6] =  F_vq2
    F[5,7] = -F_vq3
    F[5,8] = -F_vq0
    F[5,9] =  F_vq1
    
    #dqdot/dq
    w_x = w_mx - b_wx
    w_y = w_my - b_wy
    w_z = w_mz - b_wz
    
    F[6,7] = -w_x/2
    F[6,8] = -w_y/2
    F[6,9] = -w_z/2
    F[7,6] =  w_x/2
    F[7,8] =  w_z/2 
    F[7,9] = -w_y/2
    F[8,6] =  w_y/2 
    F[8,7] = -w_z/2
    F[8,9] =  w_x/2
    F[9,6] =  w_z/2
    F[9,7] =  w_y/2
    F[9,8] = -w_x/2
   
    # dqdot/dwbias   
    F[6,10] =  q1/2
    F[6,11] =  q2/2
    F[6,12] =  q3/2
    F[7,10] = -q0/2
    F[7,11] =  q3/2
    F[7,12] = -q2/2
    F[8,10] = -q3/2
    F[8,11] = -q0/2
    F[8,12] =  q1/2
    F[9,10] =  q2/2
    F[9,11] = -q1/2 
    F[9,12] = -q0/2
    
    # Initialize noise transition matrix
    G = zeros((13,9))

    # dVdot/dna
    R_11 = q0**2 + q1**2 - q2**2 - q3**2
    R_12 = 2*(q1*q2 + q0*q3)
    R_13 = 2*(q1*q3 - q0*q2)
    R_21 = 2*(q1*q2 - q0*q3)
    R_22 = q0**2 - q1**2 + q2**2 - q3**2 
    R_23 = 2*(q2*q3 + q0*q1)
    R_31 = 2*(q1*q3 + q0*q2)
    R_32 = 2*(q2*q3 - q0*q1)
    R_33 = q0**2 - q1**2 - q2**2 + q3**2 

    G[3,3] = R_11
    G[3,4] = R_21
    G[3,5] = R_31
    G[4,3] = R_12
    G[4,4] = R_22
    G[4,5] = R_32
    G[5,3] = R_13
    G[5,3] = R_23
    G[5,5] = R_33

    # dqdot/dnw
    G[6,0] = -q1/2
    G[6,1] = -q2/2
    G[6,2] = -q3/2
    G[7,0] =  q0/2
    G[7,1] = -q3/2
    G[7,2] =  q2/2
    G[8,0] =  q3/2
    G[8,1] =  q0/2
    G[8,2] = -q1/2
    G[9,0] = -q2/2
    G[9,1] =  q1/2 
    G[9,2] =  q0/2

    # dwbias/dnw
    G[10,6] = 1.0
    G[11,7] = 1.0
    G[12,8] = 1.0
    
    return F, G

#-------------------------------------------------------------------------

def measurement_equation(x,B_e):
    
    q0 = x[6,0]
    q1 = x[7,0]
    q2 = x[8,0]
    q3 = x[9,0]
    
    B_ex = B_e[0,0]
    B_ey = B_e[1,0]
    B_ez = B_e[2,0]
    
    y0 = x[0,0]     # P_x
    y1 = x[1,0]     # P_y
    y2 = x[2,0]     # P_z
    y3 = x[3,0]     # V_x
    y4 = x[4,0]     # V_y
    y5 = x[5,0]     # V_z
    
    
    R_11 = q0**2 + q1**2 - q2**2 - q3**2
    R_12 = 2*(q1*q2 + q0*q3)
    R_13 = 2*(q1*q3 - q0*q2)
    R_21 = 2*(q1*q2 - q0*q3)
    R_22 = q0**2 - q1**2 + q2**2 - q3**2 
    R_23 = 2*(q2*q3 + q0*q1)
    R_31 = 2*(q1*q3 + q0*q2)
    R_32 = 2*(q2*q3 - q0*q1)
    R_33 = q0**2 - q1**2 - q2**2 + q3**2 
      
    y6 = R_11*B_ex + R_12*B_ey + R_13*B_ez    # B_bx
    y7 = R_21*B_ex + R_22*B_ey + R_23*B_ez    # B_by
    y8 = R_31*B_ex + R_32*B_ey + R_33*B_ez    # B_bz
    
    y9 = -x[2,0]    # Altitude = -Pz
    
    y = transpose(matrix([[y0, y1, y2, y3, y4, y5, y6, y7, y8, y9]]))    
    
    return y

#------------------------------------------------------------------------

def linearize_measurement(x,B_e):

    q0 = x[6,0]
    q1 = x[7,0]
    q2 = x[8,0]
    q3 = x[9,0]
    B_ex = B_e[0,0]
    B_ey = B_e[1,0]
    B_ez = B_e[2,0]
            
    
    # Initialize measurement matrix    
    H = zeros((10,13))
    
    # dPdot/dP = I
    H[0,0] = 1.0
    H[1,1] = 1.0
    H[2,2] = 1.0
    
    # dVdot/dV = I
    H[3,3] = 1.0
    H[4,4] = 1.0
    H[5,5] = 1.0
    
    # dBb/dq
    H_bq0 = 2*( q0*B_ex + q3*B_ey - q2*B_ez)
    H_bq1 = 2*( q1*B_ex + q2*B_ey + q3*B_ez)
    H_bq2 = 2*(-q2*B_ex + q1*B_ey - q0*B_ez)
    H_bq3 = 2*(-q3*B_ex + q0*B_ey + q1*B_ez)
    
    H[6,6] =  H_bq0
    H[6,7] =  H_bq1
    H[6,8] =  H_bq2
    H[6,9] =  H_bq3
    H[7,6] =  H_bq3
    H[7,7] = -H_bq2
    H[7,8] =  H_bq1
    H[7,9] = -H_bq0
    H[8,6] = -H_bq2
    H[8,7] = -H_bq3
    H[8,8] =  H_bq0
    H[8,9] =  H_bq1

    #dAltitude/dPz = -1
    H[9,2] = -1.0    
    
    return H

#---------------------------------------------------------------------

def normalize_quart(q):
  
    q0 = q[0]
    q1 = q[1]
    q2 = q[2]
    q3 = q[3]    
    
    qmag = (q0**2 + q1**2 + q2**2 + q3**2)**0.5  
        
    q0 = q0/qmag
    q1 = q1/qmag
    q2 = q2/qmag
    q3 = q3/qmag
    
    q[0] = q0
    q[1] = q1
    q[2] = q2
    q[3] = q3
    
    return q

#----------------------------------------------------------------------

def earth_to_body(q):
    
    q0 = q[0]
    q1 = q[1]
    q2 = q[2]
    q3 = q[3]    
    
    R_11 = q0**2 + q1**2 - q2**2 - q3**2
    R_12 = 2*(q1*q2 + q0*q3)
    R_13 = 2*(q1*q3 - q0*q2)
    R_21 = 2*(q1*q2 - q0*q3)
    R_22 = q0**2 - q1**2 + q2**2 - q3**2 
    R_23 = 2*(q2*q3 + q0*q1)
    R_31 = 2*(q1*q3 + q0*q2)
    R_32 = 2*(q2*q3 - q0*q1)
    R_33 = q0**2 - q1**2 - q2**2 + q3**2 

    R_be = matrix([[R_11,R_12,R_13],[R_21,R_22,R_23],[R_31,R_32,R_33]]) # rotate vector from earth frame to body frame

    return R_be

