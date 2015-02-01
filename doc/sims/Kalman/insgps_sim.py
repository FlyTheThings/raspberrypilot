# -*- coding: utf-8 -*-
"""
Created on Sun Mar 10 15:17:27 2013

@author: soupaman
"""

from pylab import *
from math import *
from random import *
from numpy import *
from insgps_lib import *
from insgps_init import *
import time


x = transpose(matrix([[Px, Py, Pz, Vx, Vy, Vz, q0, q1, q2, q3, 0.0, 0.0, 0.0]]))

Pos_x = [x[0,0]]
Pos_y = [x[1,0]]
Pos_z = [x[2,0]]
Vel_x = [x[3,0]]
Vel_y = [x[4,0]]
Vel_z = [x[5,0]]
q_0 = [x[6,0]]
q_1 = [x[7,0]]
q_2 = [x[8,0]]
q_3 = [x[9,0]]
bias_wx = [b_wx]
bias_wy = [b_wy]
bias_wz = [b_wz]

R_be = earth_to_body([x[6,0],x[7,0],x[8,0],x[9,0]]) 
B_b = R_be * B_e

B_bx = [B_b[0,0]]
B_by = [B_b[1,0]]
B_bz = [B_b[2,0]]

phi = atan2(2*(x[6,0]*x[7,0]+x[8,0]*x[9,0]),1-2*(x[7,0]**2 + x[8,0]**2))
theta = asin(2*(x[6,0]*x[8,0] - x[7,0]*x[9,0]))
psi = atan2(2*(x[6,0]*x[9,0]+x[7,0]*x[8,0]),1-2*(x[8,0]**2 + x[9,0]**2))

roll = [phi]
pitch = [theta]
yaw = [psi]

x_m = transpose(matrix([[Px, Py, Pz, Vx, Vy, Vz, q0, q1, q2, q3, 0.0, 0.0, 120*pi/180.0]])) # changed gyro starting value to 120 d/s
X_m = transpose(matrix([[Px, Py, Pz, Vx, Vy, Vz, q0, q1, q2, q3, 0.0, 0.0, 120*pi/180.0]])) # changed gyro starting value to 120 d/s

#x_m = transpose(matrix([[1.0, Py, -1.0, Vx, Vy, Vz, 0.707, 0.408, -0.408, 0.408, 0.0, 0.0, 0.0]]))
#X_m = transpose(matrix([[1.0, Py, -1.0, Vx, Vy, Vz, 0.707, 0.408, -0.408, 0.408, 0.0, 0.0, 0.0]]))

Pos_xm = [x_m[0,0]]
Pos_ym = [x_m[1,0]]
Pos_zm = [x_m[2,0]]
Vel_xm = [x_m[3,0]]
Vel_ym = [x_m[4,0]]
Vel_zm = [x_m[5,0]]
q_0m = [x_m[6,0]]
q_1m = [x_m[7,0]]
q_2m = [x_m[8,0]]
q_3m = [x_m[9,0]]
bias_wxm = [x_m[10,0]]
bias_wym = [x_m[11,0]]
bias_wzm = [x_m[12,0]]

R_bem = earth_to_body([x_m[6,0],x_m[7,0],x_m[8,0],x_m[9,0]]) 
B_bm = R_bem * B_er

B_bxm = [B_bm[0,0]]
B_bym = [B_bm[1,0]]
B_bzm = [B_bm[2,0]]

u = transpose(matrix([[w_mx, w_my, w_mz, a_mx, a_my, a_mz]]))

Wx = [u[0,0]]
Wy = [u[1,0]]
Wz = [u[2,0]]
Ax = [u[3,0]]
Ay = [u[4,0]]
Az = [u[5,0]]

wx_m = gauss(u[0,0],sqrt(sigma2_wx)) + b_wx
wy_m = gauss(u[1,0],sqrt(sigma2_wy)) + b_wy
wz_m = gauss(u[2,0],sqrt(sigma2_wz)) + b_wz
ax_m = gauss(u[3,0],sqrt(sigma2_ax))
ay_m = gauss(u[4,0],sqrt(sigma2_ay))
az_m = gauss(u[5,0],sqrt(sigma2_az))

u_m = transpose(matrix([[wx_m, wy_m, wz_m, ax_m, ay_m, az_m]]))

Wxm = [u_m[0,0]]
Wym = [u_m[1,0]]
Wzm = [u_m[2,0]]
Axm = [u_m[3,0]]
Aym = [u_m[4,0]]
Azm = [u_m[5,0]]

z_Px = gauss(x[0,0],sqrt(sigma2_Px))
z_Py = gauss(x[1,0],sqrt(sigma2_Py))
z_Pz = gauss(x[2,0],sqrt(sigma2_Pz))
z_Vx = gauss(x[3,0],sqrt(sigma2_Vx))
z_Vy = gauss(x[4,0],sqrt(sigma2_Vy))
z_Vz = gauss(x[5,0],sqrt(sigma2_Vz))
z_Bx = gauss(B_b[0,0],sqrt(sigma2_Bx))
z_By = gauss(B_b[1,0],sqrt(sigma2_By))
z_Bz = gauss(B_b[2,0],sqrt(sigma2_Bz))
z_Alt = gauss(-x[2,0],sqrt(sigma2_Px))

Z = transpose(matrix([[z_Px, z_Py, z_Pz, z_Vx, z_Vy, z_Vz, z_Bx, z_By, z_Bz, z_Alt]]))

GPS_Px = [Z[0,0]]
GPS_Py = [Z[1,0]]
GPS_Pz = [Z[2,0]]
GPS_Vx = [Z[3,0]]
GPS_Vy = [Z[4,0]]
GPS_Vz = [Z[5,0]]
Mag_Bx = [Z[6,0]]
Mag_By = [Z[7,0]]
Mag_Bz = [Z[8,0]]
Baro_Alt = [Z[9,0]]

Pos_xk = [X_m[0,0]]
Pos_yk = [X_m[1,0]]
Pos_zk = [X_m[2,0]]
Vel_xk = [X_m[3,0]]
Vel_yk = [X_m[4,0]]
Vel_zk = [X_m[5,0]]
q_0k = [X_m[6,0]]
q_1k = [X_m[7,0]]
q_2k = [X_m[8,0]]
q_3k = [X_m[9,0]]
bias_wxk = [X_m[10,0]]
bias_wyk = [X_m[11,0]]
bias_wzk = [X_m[12,0]]

phi_k = atan2(2*(X_m[6,0]*X_m[7,0]+X_m[8,0]*X_m[9,0]),1-2*(X_m[7,0]**2 + X_m[8,0]**2))
theta_k = asin(2*(X_m[6,0]*X_m[8,0] - X_m[7,0]*X_m[9,0]))
psi_k = atan2(2*(X_m[6,0]*X_m[9,0]+X_m[7,0]*X_m[8,0]),1-2*(X_m[8,0]**2 + X_m[9,0]**2))

roll_k = [phi_k]
pitch_k = [theta_k]
yaw_k = [psi_k]

time = [0]

iterations = 5000
dt = 0.01
I = eye(13)
P_m = P0
print "\n\nRunning {} second simulation at {} second step size, please stand by....".format(iterations * dt, dt)

for i in range(iterations):
    print "\rIteration {} of {}...".format(i, iterations),
    sys.stdout.flush()
    time.append(i*dt)

    # Ideal Simulation
    Xnew = runge_kutta(x,u,dt)
    x = Xnew    
    Pos_x.append(x[0,0])
    Pos_y.append(x[1,0])    
    Pos_z.append(x[2,0])
    Vel_x.append(x[3,0])
    Vel_y.append(x[4,0])
    Vel_z.append(x[5,0])
    q_0.append(x[6,0])
    q_1.append(x[7,0])
    q_2.append(x[8,0])
    q_3.append(x[9,0])
    bias_wx.append(b_wx)
    bias_wy.append(b_wy)
    bias_wz.append(b_wz)
    Wx.append(u[0,0])
    Wy.append(u[1,0])
    Wz.append(u[2,0])
    Ax.append(u[3,0])
    Ay.append(u[4,0])
    Az.append(u[5,0])
    
    R_be = earth_to_body([x[6,0],x[7,0],x[8,0],x[9,0]]) 
    B_b = R_be * B_e

    B_bx.append(B_b[0,0])
    B_by.append(B_b[1,0])
    B_bz.append(B_b[2,0])   

    phi = atan2(2*(x[6,0]*x[7,0]+x[8,0]*x[9,0]),1-2*(x[7,0]**2 + x[8,0]**2))
    theta = asin(2*(x[6,0]*x[8,0] - x[7,0]*x[9,0]))
    psi = atan2(2*(x[6,0]*x[9,0]+x[7,0]*x[8,0]),1-2*(x[8,0]**2 + x[9,0]**2))    

    roll.append(phi)
    pitch.append(theta)
    yaw.append(psi)
    
    # Simulation with real accelerometer and gyroscopes
    x_m = runge_kutta(X_m,u_m,dt)
    [x_m[6,0],x_m[7,0],x_m[8,0],x_m[9,0]] = normalize_quart([x_m[6,0],x_m[7,0],x_m[8,0],x_m[9,0]])
      
    [F, G] = linearize_state(x_m,u_m)
    A = I + (F*dt)
    P_est = A*P_m*transpose(A) + G*Q*transpose(G)*(dt**2) # a priori covariance estimate

    Y = measurement_equation(x_m,B_e)
    H = linearize_measurement(x_m,B_e)

    K = 1.0*(P_est*transpose(H) * inv(H*P_est*transpose(H) + R))  # Kalman gain

    Pos_xm.append(x_m[0,0])
    Pos_ym.append(x_m[1,0])    
    Pos_zm.append(x_m[2,0])
    Vel_xm.append(x_m[3,0])
    Vel_ym.append(x_m[4,0])
    Vel_zm.append(x_m[5,0])
    q_0m.append(x_m[6,0])
    q_1m.append(x_m[7,0])
    q_2m.append(x_m[8,0])
    q_3m.append(x_m[9,0])
    bias_wxm.append(x_m[10,0])
    bias_wym.append(x_m[11,0])
    bias_wzm.append(x_m[12,0])
    Wxm.append(u_m[0,0])
    Wym.append(u_m[1,0])
    Wzm.append(u_m[2,0])
    Axm.append(u_m[3,0]) 
    Aym.append(u_m[4,0])
    Azm.append(u_m[5,0])

    R_bem = earth_to_body([x_m[6,0],x_m[7,0],x_m[8,0],x_m[9,0]]) 
    B_bm = R_bem * B_er

    B_bxm.append(B_bm[0,0])
    B_bym.append(B_bm[1,0])
    B_bzm.append(B_bm[2,0])
    
    wx_m = gauss(u[0,0],sqrt(sigma2_wx)) + b_wx
    wy_m = gauss(u[1,0],sqrt(sigma2_wy)) + b_wy
    wz_m = gauss(u[2,0],sqrt(sigma2_wz)) + b_wz
    ax_m = gauss(u[3,0],sqrt(sigma2_ax))
    ay_m = gauss(u[4,0],sqrt(sigma2_ay))
    az_m = gauss(u[5,0],sqrt(sigma2_az))
    u_m = transpose(matrix([[wx_m, wy_m, wz_m, ax_m, ay_m, az_m]]))

    # GPS, Magnetometer and Altimeter data
    z_Px = gauss(x[0,0],sqrt(sigma2_Px))
    z_Py = gauss(x[1,0],sqrt(sigma2_Py))
    z_Pz = gauss(x[2,0],sqrt(sigma2_Pz))
    z_Vx = gauss(x[3,0],sqrt(sigma2_Vx))
    z_Vy = gauss(x[4,0],sqrt(sigma2_Vy))
    z_Vz = gauss(x[5,0],sqrt(sigma2_Vz))
    z_Bx = gauss(B_b[0,0],sqrt(sigma2_Bx))
    z_By = gauss(B_b[1,0],sqrt(sigma2_By))
    z_Bz = gauss(B_b[2,0],sqrt(sigma2_Bz))
    z_Alt = gauss(-x[2,0],sqrt(sigma2_Px))

    Z = transpose(matrix([[z_Px, z_Py, z_Pz, z_Vx, z_Vy, z_Vz, z_Bx, z_By, z_Bz, z_Alt]]))

    GPS_Px.append(Z[0,0])
    GPS_Py.append(Z[1,0])
    GPS_Pz.append(Z[2,0])
    GPS_Vx.append(Z[3,0])
    GPS_Vy.append(Z[4,0])
    GPS_Vz.append(Z[5,0])
    Mag_Bx.append(Z[6,0])
    Mag_By.append(Z[7,0])
    Mag_Bz.append(Z[8,0])
    Baro_Alt.append(Z[9,0])


    X_m = x_m + K*(Z - Y)
    P_m = (I - K*H)*P_est

#    X_m[10,0] = x_m[10,0]
#    X_m[11,0] = x_m[11,0]   
#    X_m[12,0] = x_m[12,0]
    
    Pos_xk.append(X_m[0,0])
    Pos_yk.append(X_m[1,0])    
    Pos_zk.append(X_m[2,0])
    Vel_xk.append(X_m[3,0])
    Vel_yk.append(X_m[4,0])
    Vel_zk.append(X_m[5,0])
    q_0k.append(X_m[6,0])
    q_1k.append(X_m[7,0])
    q_2k.append(X_m[8,0])
    q_3k.append(X_m[9,0])
    bias_wxk.append(X_m[10,0])
    bias_wyk.append(X_m[11,0])
    bias_wzk.append(X_m[12,0])

    phi_k = atan2(2*(X_m[6,0]*X_m[7,0]+X_m[8,0]*X_m[9,0]),1-2*(X_m[7,0]**2 + X_m[8,0]**2))
    theta_k = asin(2*(X_m[6,0]*X_m[8,0] - X_m[7,0]*X_m[9,0]))
    psi_k = atan2(2*(X_m[6,0]*X_m[9,0]+X_m[7,0]*X_m[8,0]),1-2*(X_m[8,0]**2 + X_m[9,0]**2))

    roll_k.append(phi_k)
    pitch_k.append(theta_k)
    yaw_k.append(psi_k)


#figure(1)
#plot(time,Pos_x,'-',time,Pos_y,'-',time,Pos_z,'-',time,Pos_xm,'b.',time,Pos_ym,'g.',time,Pos_zm,'r.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
#xlabel('time')
#ylabel('Position')
#title('Position vs. Time')
#legend(('P_x','P_y','P_z','P_xm','P_ym','P_zm'))
#grid(axis='both', color='k', linestyle='--', linewidth=1)
#
#figure(2)
#plot(time,Vel_x,'-',time,Vel_y,'-',time,Vel_z,'-',time,Vel_xm,'b.',time,Vel_ym,'g.',time,Vel_zm,'r.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
#xlabel('time')
#ylabel('Velocity')
#title('Velocity vs. Time')
#legend(('V_x','V_y','V_z','V_xm','V_ym','V_zm'))
#grid(axis='both', color='k', linestyle='--', linewidth=1)
#
#figure(3)
#plot(time,q_0,'-',time,q_1,'-',time,q_2,'-',time,q_3,'-',time,q_0m,'b.',time,q_1m,'g.',time,q_2m,'r.',time,q_3m,'c.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
#xlabel('time')
#ylabel('Quarternion')
#title('Attitude vs. Time')
#legend(('q_0','q_1','q_2','q_3','q_0m','q_1m','q_2m','q_3m'))
#grid(axis='both', color='k', linestyle='--', linewidth=1)
#
# figure(4)
# plot(time,bias_wx,'-',time,bias_wy,'-',time,bias_wz,'-',time,bias_wxm,'b.',time,bias_wym,'g.',time,bias_wzm,'r.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
# xlabel('time')
# ylabel('Bias')
# title('Gyro Bias vs. Time')
# legend(('b_wx','b_wy','b_wz','b_wxm','b_wym','b_wzm'))
# grid(axis='both', color='k', linestyle='--', linewidth=1)
#
#figure(5)
#plot(time,Wx,'-',time,Wy,'-',time,Wz,'-',time,Wxm,'b.',time,Wym,'g.',time,Wzm,'r.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
#xlabel('time')
#ylabel('Angular velocity')
#title('Gyro output vs. Time')
#legend(('Wx','Wy','Wz','Wxm','Wym','Wzm'))
#grid(axis='both', color='k', linestyle='--', linewidth=1)
#
#figure(6)
#plot(time,Ax,'-',time,Ay,'-',time,Az,'-',time,Axm,'b.',time,Aym,'g.',time,Azm,'r.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
#xlabel('time')
#ylabel('Acceleration')
#title('Accelerometer output vs. Time')
#legend(('Ax','Ay','Az','Axm','Aym','Azm'))
#grid(axis='both', color='k', linestyle='--', linewidth=1)
#
# figure(7)
# plot(time,B_bx,'-',time,B_by,'-',time,B_bz,'-',time,B_bxm,'b-.',time,B_bym,'g-.',time,B_bzm,'r-.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
# xlabel('time')
# ylabel('Magnetic Field')
# title('Magnetic field estimate vs. Time')
# legend(('B_bx','B_by','B_bz','B_bxm','B_bym','B_bzm'))
# grid(axis='both', color='k', linestyle='--', linewidth=1)
#
#
#figure(8)
#plot(time,Pos_x,'-',time,Pos_y,'-',time,Pos_z,'-',time,GPS_Px,'b.',time, GPS_Py,'g.',time,GPS_Pz,'r.',-1*Baro_Alt,'c.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
#xlabel('time')
#ylabel('Position')
#title('GPS Position and Altimeter output vs. Time')
#legend(('P_x','P_y','P_z','GPS_Px','GPS_Py','GPS_Pz','Altimeter'))
#grid(axis='both', color='k', linestyle='--', linewidth=1)
#
#figure(9)
#plot(time,Vel_x,'-',time,Vel_y,'-',time,Vel_z,'-',time,GPS_Vx,'b.',time,GPS_Vy,'g.',time,GPS_Vz,'r.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
#xlabel('time')
#ylabel('Velocity')
#title('Velocity vs. Time')
#legend(('V_x','V_y','V_z','GPS_Vx','GPS_Vy','GPS_Vz'))
#grid(axis='both', color='k', linestyle='--', linewidth=1)
#
# figure(10)
# plot(time,B_bx,'-',time,B_by,'-',time,B_bz,'-',time,Mag_Bx,'b.',time,Mag_By,'g.',time,Mag_Bz,'r.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
# xlabel('time')
# ylabel('Magnetic Field')
# title('Magnetometer output vs. Time')
# legend(('B_bx','B_by','B_bz','Mag_Bx','Mag_By','Mag_Bz'))
# grid(axis='both', color='k', linestyle='--', linewidth=1)

rad2deg = float(180.0)/pi

bias_wx = [x*rad2deg for x in bias_wx]
bias_wy = [x*rad2deg for x in bias_wy]
bias_wz = [x*rad2deg for x in bias_wz]
bias_wxk = [x*rad2deg for x in bias_wxk]
bias_wyk = [x*rad2deg for x in bias_wyk]
bias_wzk = [x*rad2deg for x in bias_wzk]

figure(11)
plot(time,bias_wx,'-',time,bias_wy,'-',time,bias_wz,'-',time,bias_wxk,'b-.',time,bias_wyk,'g-.',time,bias_wzk,'r-.',linewidth=2)
#axis([0, iterations*dt, -1, 2])
xlabel('time')
ylabel('Bias')
title('Gyro Bias vs. Time')
legend(('b_wx','b_wy','b_wz','b_wxk','b_wyk','b_wzk'))
grid(axis='both', color='k', linestyle='-', linewidth=1)


#figure(12)
#plot(time,Pos_x,'-',time,GPS_Px,'.',time,Pos_xk,'-.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
#xlabel('time')
#ylabel('Position')
#title('Position vs. Time')
#legend(('P_x','P_xgps','P_xk'))
#grid(axis='both', color='k', linestyle='--', linewidth=1)
#
#
#figure(13)
#plot(time,Pos_z,'-',time,GPS_Pz,'.',time,Pos_zk,'-.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
#xlabel('time')
#ylabel('Position')
#title('Position vs. Time')
#legend(('P_z','P_zgps','P_zk'))
#grid(axis='both', color='k', linestyle='--', linewidth=1)
#
#
#figure(14)
#plot(time,q_0,'-',time,q_1,'-',time,q_2,'-',time,q_3,'-',time,q_0k,'b-.',time,q_1k,'g-.',time,q_2k,'r-.',time,q_3k,'c-.',linewidth=2)
##axis([0, iterations*dt, -1, 2])
#xlabel('time')
#ylabel('Quarternion')
#title('Attitude vs. Time')
#legend(('q_0','q_1','q_2','q_3','q_0m','q_1m','q_2m','q_3m'))
#grid(axis='both', color='k', linestyle='--', linewidth=1)

roll = [x*rad2deg for x in roll]
pitch = [x*rad2deg for x in pitch]
yaw = [x*rad2deg for x in yaw]
roll_k = [x*rad2deg for x in roll_k]
pitch_k = [x*rad2deg for x in pitch_k]
yaw_k = [x*rad2deg for x in yaw_k]

figure(15)
plot(time,roll,'-',time,pitch,'-',time,yaw,'-',time,roll_k,'b-.',time,pitch_k,'g-.',time,yaw_k,'r-.',linewidth=2)
#axis([0, iterations*dt, -1, 2])
xlabel('time')
ylabel('Angle')
title('Attitude vs. Time')
legend(('roll','pitch','yaw','roll_k','pitch_k','yaw_k'))
grid(axis='both', color='k', linestyle='--', linewidth=1)

show()
