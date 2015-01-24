# -*- coding: utf-8 -*-
"""
Created on Wed Mar 27 21:29:10 2013

@author: soupaman
"""

from pylab import *
from math import *
from random import *
from numpy import *

Px = 0.0
Py = 0.0
Pz = 0.0
Vx = 0.0
Vy = 0.0
Vz = 0.0
q0 = 1.0
q1 = 0.0
q2 = 0.0
q3 = 0.0
b_wx = -0.2
b_wy = 0.4
b_wz = -0.3
g = 9.8     #m/s^2

sigma2_wx = 50e-8
sigma2_wy = 50e-8
sigma2_wz = 50e-8
sigma2_ax = 0.01
sigma2_ay = 0.01
sigma2_az = 0.01
sigma2_bx = 2e-9
sigma2_by = 2e-9
sigma2_bz = 2e-9

#Q = diag([sigma2_wx, sigma2_wy, sigma2_wz, sigma2_ax, sigma2_ay, sigma2_az, sigma2_bx, sigma2_by, sigma2_bz])

Q = matrix([[sigma2_wx,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0],
            [      0.0, sigma2_wy,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0],
            [      0.0,       0.0, sigma2_wz,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0],
            [      0.0,       0.0,       0.0, sigma2_ax,       0.0,       0.0,       0.0,       0.0,       0.0],
            [      0.0,       0.0,       0.0,       0.0, sigma2_ay,       0.0,       0.0,       0.0,       0.0],
            [      0.0,       0.0,       0.0,       0.0,       0.0, sigma2_az,       0.0,       0.0,       0.0],
            [      0.0,       0.0,       0.0,       0.0,       0.0,       0.0, sigma2_bx,       0.0,       0.0],
            [      0.0,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0, sigma2_by,       0.0],
            [      0.0,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0,       0.0, sigma2_bz]])



sigma2_Px = 0.004
sigma2_Py = 0.004
sigma2_Pz = 0.036
sigma2_Vx = 0.004
sigma2_Vy = 0.004
sigma2_Vz = 100
sigma2_Bx = 0.005
sigma2_By = 0.005
sigma2_Bz = 0.005
sigma2_Alt = 0.05

R = diag([sigma2_Px, sigma2_Py, sigma2_Pz, sigma2_Vx, sigma2_Vy, sigma2_Vz, sigma2_Bx, sigma2_By, sigma2_Bz, sigma2_Alt])

P0_Px = 25
P0_Py = 25
P0_Pz = 25
P0_Vx = 5
P0_Vy = 5
P0_Vz = 5
P0_q0 = 1e-5
P0_q1 = 1e-5
P0_q2 = 1e-5
P0_q3 = 1e-5
P0_bwx = 1e-5
P0_bwy = 1e-5
P0_bwz = 1e-5

P0 = diag([P0_Px, P0_Py, P0_Pz, P0_Vx, P0_Vy, P0_Vz, P0_q0, P0_q1, P0_q2, P0_q3, P0_bwx, P0_bwy, P0_bwz])

w_mx = 0.0
w_my = 0.0
w_mz = 0.0   
a_mx = 0.0
a_my = 0.0
a_mz = -g       # acceleration due to gravity in NED frame
B_ex = 0.365
B_ey = -0.097
B_ez = 0.9258

B_e = transpose(matrix([[B_ex, B_ey, B_ez]]))

B_exr = 1.0
B_eyr = 0.0
B_ezr = 0.0

B_er = transpose(matrix([[B_exr, B_eyr, B_ezr]]))