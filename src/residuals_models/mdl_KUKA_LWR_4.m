% MDL_KUKA_LWR_4 Create model of Kuka LWR 4 robotic arm according to the
% standard Denavit-Hartenberg notation
%
%       mdl_KUKA_LWR_4
%
% Script creates the workspace variable "Kuka_LWR_4" which describes the 
% kinematic and dynamic characteristics of the KUKA LWR 4 robotic arm.
%
% Also defines the vector:
%   qz   corresponds to the zero joint angle configuration.
%
% Notes::
% - SI units of metres are used.

% MODEL: Kuka, LWR, 7DOF, standard_DH

clear Kuka_LWR_4 all_links d1 d2 qz

n_kuka_joint = 7;

d1 = 0.4; d2 = 0.39;

%All link lengths and offsets are measured in m
%            theta    d           a       alpha
all_links = [
            Revolute('d', 0,  'a', 0, 'alpha',  pi/2, 'standard')
            Revolute('d', 0,  'a', 0, 'alpha', -pi/2, 'standard');
            Revolute('d', d1, 'a', 0, 'alpha', -pi/2, 'standard');
            Revolute('d', 0,  'a', 0, 'alpha',  pi/2, 'standard');
            Revolute('d', d2, 'a', 0, 'alpha',  pi/2, 'standard');
            Revolute('d', 0,  'a', 0, 'alpha', -pi/2, 'standard');
            Revolute('d', 0,  'a', 0, 'alpha',  0,    'standard');
            ];

Kuka_LWR_4 = SerialLink(all_links, 'name', 'Kuka LWR 4');
% Enable by default fast RNE calculus
Kuka_LWR_4.fast = 1;

qz = [0 0 0 0 0 0 0];

clear all_links d1 d2

clear m_kuka 
m_kuka = [4.327412 6.990093 1.453501 2.495117 1.463685 0.535484 0.414342];

clear r_kuka c1 c2 c3 c4 c5 c6 c7
c1 = [-0.000237 0.027464 -0.000510];
c2 = [0.006341 -0.078281 0.181806];
c3 = [-0.001616 0.241064 0.065227];
c4 = [0.003828 -0.038670 0.242516];
c5 = [-0.002197 -0.136560 -0.026532];
c6 = [0.001000 0.011957 0.016761];
c7 = [-0.002105 -0.000751 0.055592];
r_kuka = [c1; c2; c3; c4; c5; c6; c7];
clear c1 c2 c3 c4 c5 c6 c7

clear Ic_kuka I1 I2 I3 I4 I5 I6 I7
I1 = [0.598562 -0.001049 0.004288; ...
     -0.001049 0.035545 -0.099545; ...
      0.004288 -0.099545 0.565422];
I2 = [0.041663 0.025914 -0.007890; ...
      0.025914 0.041160  0.010133; ...
     -0.007890 0.010133  0.054390];
I3 = [0.061423 -0.008671 -0.004271; ...
     -0.008671 0.025989 -0.019005; ...
     -0.004271 -0.019005 0.039490];
I4 = [0.012593 -0.002378 0.005175; ...
     -0.002378 0.013977 0.002782; ...
      0.005175 0.002782 0.007174];
I5 = [0.024067 -0.000901 -0.000411; ...
     -0.000901 0.006554 -0.010511; ...
     -0.000411 -0.010511 0.017884];
I6 = [0.007701 -0.001011 0.001449; ...
     -0.001011  0.003049 0.001540; ...
      0.001449  0.001540 0.006941];
I7 = [0.003495 -0.002318 0.000483; ...
     -0.002318  0.001854  0.000623; ...
      0.000483  0.000623 0.004954];
Ic_kuka(:,:,1) = I1; Ic_kuka(:,:,2) = I2; Ic_kuka(:,:,3) = I3;
Ic_kuka(:,:,4) = I4; Ic_kuka(:,:,5) = I5; Ic_kuka(:,:,6) = I6; Ic_kuka(:,:,7) = I7;

clear I1 I2 I3 I4 I5 I6 I7

for i = 1:n_kuka_joint
    % Link Mass
    Kuka_LWR_4.links(i).m = m_kuka(i);
    % Motor inertia (Unknown and neglected)
    Kuka_LWR_4.links(i).Jm = 0.0;
    % Gear ratio (Unknown and neglected)
    Kuka_LWR_4.links(i).G = 1;
    % Link Inertia Tensor
    %        Ixx     Iyy      Izz    Ixy     Iyz     Ixz
    Kuka_LWR_4.links(i).I = [Ic_kuka(1,1,i) Ic_kuka(2,2,i) Ic_kuka(3,3,i) ...
                             Ic_kuka(1,2,i) Ic_kuka(2,3,i) Ic_kuka(1,3,i) ];
    % Link Center of Mass
    %         rx      ry      rz
    Kuka_LWR_4.links(i).r = r_kuka(i,:); 
end

clear m_kuka Ic_kuka r_kuka n_kuka_joint

