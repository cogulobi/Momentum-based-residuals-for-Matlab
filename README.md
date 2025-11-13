## Robotics Toolbox add-on for Momentum-based residual calculus on MATLAB
The library is an addon for the Robotics Toolbox of Prof. P. Corke which makes it possible to compute the terms required for the computation of the residuals for an N link planar robot and for a a KUKA LWR4+ collaborative robot.

This repository contains the code and data supporting the paper:
**"Efficient computation of momentum-based residual for robot collision detection and isolation"**, G. Tonti et al., 2025.

## Contents
- `src/Sources_for_Residuals_NE_mex` – Source codes
- `src/residuals_models` – Additional robot models used in source codes

## Install from github:

You need to have a recent version of MATLAB, R2016b or later.

The Robotics Toolbox for MATLAB has dependency on two other GitHub repositories: [`spatial-math`](https://github.com/petercorke/spatial-math) and [`toolbox-common-matlab`](https://github.com/petercorke/toolbox-common-matlab).  

To install the Toolbox on your computer from github follow these simple instructions.

From the shell:

```shell
mkdir robotics_toolbox
cd robotics_toolbox
git clone https://github.com/petercorke/robotics-toolbox-matlab.git robot
git clone https://github.com/petercorke/spatial-math.git smtb
git clone https://github.com/petercorke/toolbox-common-matlab.git common

git clone https://github.com/cogulobi/Momentum-based-residuals-for-Matlab.git src
```

In order to maintain the folder structure of the Peter Corke's Robotics Toolbox:
- "Sources_for_Residuals_mex" folder must be copied entirely in "robotics_toolbox/robot/mex";
- "residuals_models" folder must be copied entirely in "robotics_toolbox/robot/models".

Then inside MATLAB add these folders to your path:

```matlab
>> addpath robot common smtb robot/models robot/models/residuals_models robot/mex robot/mex/Sources_for_Residuals_mex
```

This will work for just the current session. You can repeat this command every session, automate it by adding it to your MATLAB startup.m script, or use pathtool 
to save the current path configuration away for next time.

It is reccomended to add to the matlab path all subfolders within "robotics_toolbox". It can be performed as follows:
- right click on "robotics_toolbox" folder in the matlab file manager -> Add to Path -> Select Folder(s) and Subfolders .

## Online resources

* [Robotics toolbox page](https://petercorke.com/toolboxes/robotics-toolbox/)

## License

This toolbox is released under GNU GPL V3.0.

## Citation

If you use this code, please cite the associated paper and repository DOI: "10.xxxx/zenodo.xxxxxx"

