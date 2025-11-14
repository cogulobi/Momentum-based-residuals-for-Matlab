# How it works

The proposed Momentum-based residual calculations rely also on multiple calls to the function `rne()` of the robotics toolbox. 
Since `rne()` is an M-file, the code can run rather slowly.
As done for `rne()` in the native robotics toolbox, a MEX file is necessary to speed up the residuals calculus (typically an order of magnitude faster than the M-file).
This directory contains source codes for the MEX file `residuals_NE.mex`.

# Building the MEX file

The mex file compries multiples files written in C with no other library dependencies (apart from libm):


| File                                          | Purpose                                                                                |
|-----------------------------------------------|----------------------------------------------------------------------------------------|
| residuals_NE.c                                | MEX function main for the evaluation of RESIDUALS TERMS for a Kuka LWR IV or a N-links |    
|                                               | planar manipulator                                                                     |
| ne.c                                          | The recursive Newton-Euler code for standard and modified DH parameters                |
| ne_ext.c                                      | The recursive Extended Newton-Euler code for standard and modified DH parameters       |
| vmath_ext.c                                   | A very simple vector and matrix (linalg) library                                       |
| some_robot_C_stuff/dotMdotq_KUKAlwr4_num.c    | Numerical calculus of the term dotM*dotq for a Kuka LWR IV Robot                       |
| some_robot_C_stuff/dotMdotq_PlanarN_num.c     | Numerical calculus of the term dotM*dotq for a N-link Planar Robot                     |
| some_robot_C_stuff/frne_ext_KUKAlwr4_num.c    | Numerical calculus of the Extended Newton-Euler algorithm for a Kuka LWR IV Robot      |
| some_robot_C_stuff/frne_ext_PlanarN_num.c     | Numerical calculus of the Extended Newton-Euler algorithm for a N-link Planar Robot    |
| some_robot_C_stuff/frne_KUKAlwr4_num.c        | Numerical calculus of the Newton-Euler algorithm for a Kuka LWR IV Robot               |
| some_robot_C_stuff/frne_PlanarN_num.c         | Numerical calculus of the Newton-Euler algorithm for a N-link Planar Robot             |
| some_robot_C_stuff/get_Kukagnum.c             | Numerical function to compute the gravity vector for KUKA LWR IV robot                 |
| some_robot_C_stuff/get_KukaMnum.c             | Numerical computation of the Inertia matrix M for KUKA LWR IV robot                    |
| some_robot_C_stuff/get_KukaSnum.c             | Numerical computation of the Coriolis matrix S for KUKA LWR IV robot                   |
| some_robot_C_stuff/get_PlanarN_3_Mnum.c       | Numerical computation of the Inertia matrix M for a 3-DOF Planar robot                 |
| some_robot_C_stuff/get_PlanarN_3_Snum.c       | Numerical computation of the Coriolis matrix S for a 3-DOF Planar robot                |
| some_robot_C_stuff/get_PlanarN_4_Mnum.c       | Numerical computation of the Inertia matrix M for a 4-DOF Planar robot                 |
| some_robot_C_stuff/get_PlanarN_4_Snum.c       | Numerical computation of the Coriolis matrix S for a 4-DOF Planar robot                |
| some_robot_C_stuff/get_PlanarN_5_Mnum.c       | Numerical computation of the Inertia matrix M for a 5-DOF Planar robot                 |
| some_robot_C_stuff/get_PlanarN_5_Snum.c       | Numerical computation of the Coriolis matrix S for a 5-DOF Planar robot                |
| some_robot_C_stuff/get_PlanarN_6_Mnum.c       | Numerical computation of the Inertia matrix M for a 6-DOF Planar robot                 |
| some_robot_C_stuff/get_PlanarN_6_Snum.c       | Numerical computation of the Coriolis matrix S for a 6-DOF Planar robot                |
| some_robot_C_stuff/get_PlanarN_7_Mnum.c       | Numerical computation of the Inertia matrix M for a 7-DOF Planar robot                 |
| some_robot_C_stuff/get_PlanarN_7_Snum.c       | Numerical computation of the Coriolis matrix S for a 7-DOF Planar robot                |
| some_robot_C_stuff/get_PlanarN_8_Mnum.c       | Numerical computation of the Inertia matrix M for a 8-DOF Planar robot                 |
| some_robot_C_stuff/get_PlanarN_8_Snum.c       | Numerical computation of the Coriolis matrix S for a 8-DOF Planar robot                |
| some_robot_C_stuff/get_PlanarN_9_Mnum.c       | Numerical computation of the Inertia matrix M for a 9-DOF Planar robot                 |
| some_robot_C_stuff/get_PlanarN_9_Snum.c       | Numerical computation of the Coriolis matrix S for a 9-DOF Planar robot                |
| some_robot_C_stuff/get_PlanarN_10_Mnum.c      | Numerical computation of the Inertia matrix M for a 10-DOF Planar robot                |
| some_robot_C_stuff/get_PlanarN_10_Snum.c      | Numerical computation of the Coriolis matrix S for a 10-DOF Planar robot               |
| some_robot_C_stuff/get_PlanarN_11_Mnum.c      | Numerical computation of the Inertia matrix M for a 11-DOF Planar robot                |
| some_robot_C_stuff/get_PlanarN_11_Snum.c      | Numerical computation of the Coriolis matrix S for a 11-DOF Planar robot               |
| some_robot_C_stuff/get_PlanarN_12_Mnum.c      | Numerical computation of the Inertia matrix M for a 12-DOF Planar robot                |
| some_robot_C_stuff/get_PlanarN_12_Snum.c      | Numerical computation of the Coriolis matrix S for a 12-DOF Planar robot               |
| some_robot_C_stuff/get_PlanarN_13_Mnum.c      | Numerical computation of the Inertia matrix M for a 13-DOF Planar robot                |
| some_robot_C_stuff/get_PlanarN_13_Snum.c      | Numerical computation of the Coriolis matrix S for a 13-DOF Planar robot               |


These are compiled and linked together, resulting in a MEX file called `residuals_NE`.  
The extension of the file depends on the operating system and architecture.

## For Linux or MacOS users:
```matlab
>> make_residuals
```

which assumes that the `mex` utility is in your current path.  
This is typically found in the `bin` subdirectory of your installed MATLAB. You, of course, also need a C compiler (gcc).

## Additional Informations For Windows users:

You will need to have a C compiler installed and configured within MATLAB.

MathWorks provide a free C compiler for Windows called [`MinGW`](http://www.mingw.org) (minimalist GNU compiler). 
You can install this through the AddOns manager from MATLAB desktop.

By default, the MEX file `residuals_NE.mex` is configured to run on a LINUX MACHINE.
In order to compile and use the same MEX file on a WINDOWS MACHINE, it must be commented the following line within `residuals_NE.c`:

```C
// #define LINUX_MACHINE 
```

## Usage

Once the `residuals_NE` MEX file is compiled without errors, it can be called in a M-file.
Below there is a brief description of the mandatory INPUTS and returned OUTPUS of the early compiled MEX file:

```
[OUTER_TERM, INNER_TERM, COMP_TIMING] = RESIDUALS_NE(ROBOT, Q, QD, QDD, QDDD, FEXT, RES_CALCULUS_MODE, IS_PLANAR_ROBOT)

where:
-	Q, QD, QDD, and QDDD are row vectors of the manipulator state: pos, vel, accel, and jerk;
- 	FEXT is the external force/moment acting on the end of the manipulator. It may also be specified by a 6-element vector [Fx Fy Fz Mx My Mz];
-	RES_CALCULUS_MODE determines how the residuals are going to be evaluated:
 	    0 -> just General-Purpose NE function calls will be used (NE);
 		1 -> both General-Purpose NE and extended NE function calls will be used (Ext. NE);
 		2 -> just Numerical NE function calls will be used (Cust. NE);
 		3 -> both Numerical NE and extended RNE function calls will be used (Cust. Ext. NE);
 		4 -> classical Euler-Lagrange method. M, S and g are computed numerically (Cust. EL);
 		5 -> dotMotq in addiction to Numerical NE function calls (Paper [23] of the article).
-   IS_PLANAR_ROBOT determines which robot is passed as variable to "residuals_NE(...)" function:
 		0 -> KUKA LWR IV robot;
 		1 -> N-LINK PLANAR robot.

Returns 2 row vectors, 1 by N (N is the number of robot joints), which correspond to the terms that charaterize the MOMENTUM-BASED RESIDUALS computation,
and 1 value which represents how much time "residuals_NE(...)" takes to compute it:
	OUTER_TERM:  robot generalized momentum -> M(Q)*QD;
 	INNER_TERM:  summation of all the terms within the integral (without friction terms);
 	COMP_TIMING: how much time the residual calculus takes.
```

The COMP_TIMING value returned may vary depending on whether the MEX file is compiled and executed on a LINUX or WINDOWS-based machine.

## Extra MATLAB codes

This directory also contains 2 extra M-files that exploit the early compiled MEX file:
- Test_residuals_KukaLWR4.m;
- Test_residuals_NPlanar.m.

Once they run, some figures, like the ones in **"Efficient computation of momentum-based residual for robot collision detection and isolation"**, will appear.
 
## Limitations

- The MEX file does not support multiple robot configurations (just a single set of Q, QD, QDD, QDDD and FEXT vectors).
