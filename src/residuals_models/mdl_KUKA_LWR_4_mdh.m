% MDL_KUKA_LWR_4_MDH Create model of Kuka LWR 4 robotic arm according to the
% modified Denavit-Hartenberg notation
%
%       mdl_KUKA_LWR_4_mdh
%
% Script creates the workspace variable "Kuka_LWR_4" which describes the 
% kinematic and dynamic characteristics of the KUKA LWR 4 robotic arm.
%
% Also defines the vector:
%   qz   corresponds to the zero joint angle configuration.
%
% Notes::
% - SI units of metres are used.

% MODEL: Kuka, LWR, 7DOF, modified_DH

clear Kuka_LWR_4_mdh all_links d1 d2 qz

n_kuka_joint = 7;

d1 = 0.4; d2 = 0.39;

%All link lengths and offsets are measured in m
%            theta    d           a       alpha
all_links = [
            Revolute('d', 0,  'a', 0, 'alpha',  0,    'modified')
            Revolute('d', 0,  'a', 0, 'alpha',  pi/2, 'modified');
            Revolute('d', d1, 'a', 0, 'alpha', -pi/2, 'modified');
            Revolute('d', 0,  'a', 0, 'alpha', -pi/2, 'modified');
            Revolute('d', d2, 'a', 0, 'alpha',  pi/2, 'modified');
            Revolute('d', 0,  'a', 0, 'alpha',  pi/2, 'modified');
            Revolute('d', 0,  'a', 0, 'alpha', -pi/2, 'modified');
            ];

Kuka_LWR_4_mdh = SerialLink(all_links, 'name', 'Kuka LWR 4');
% Enable by default fast RNE calculus
Kuka_LWR_4_mdh.fast = 1;

qz = [0 0 0 0 0 0 0];

clear all_links d1 d2

clear m_kuka 
m_kuka = [4.327412 6.990093 1.453501 2.495117 1.463685 0.535484 0.414342];

clear r_kuka c1 c2 c3 c4 c5 c6 c7 
clear c1_std c2_std c3_std c4_std c5_std c6_std c7_std
c1_std = [-0.000237 0.027464 -0.000510]';
c2_std = [0.006341 -0.078281 0.181806]';
c3_std = [-0.001616 0.241064 0.065227]';
c4_std = [0.003828 -0.038670 0.242516]';
c5_std = [-0.002197 -0.136560 -0.026532]';
c6_std = [0.001000 0.011957 0.016761]';
c7_std = [-0.002105 -0.000751 0.055592]';
c1 = ( Rx(pi/2)  * c1_std )';
c2 = ( Rx(-pi/2) * c2_std )';
c3 = ( Rx(-pi/2) * c3_std )';
c4 = ( Rx(pi/2)  * c4_std )';
c5 = ( Rx(pi/2)  * c5_std )';
c6 = ( Rx(-pi/2) * c6_std )';
c7 = ( Rx(0)     * c7_std )';
r_kuka = [c1; c2; c3; c4; c5; c6; c7];
clear c1 c2 c3 c4 c5 c6 c7
clear c1_std c2_std c3_std c4_std c5_std c6_std c7_std

clear Ic_kuka 
clear I1_std I2_std I3_std I4_std I5_std I6_std I7_std 
I1_std = [0.598562 -0.001049 0.004288; ...
         -0.001049 0.035545 -0.099545; ...
          0.004288 -0.099545 0.565422];
I2_std = [0.041663 0.025914 -0.007890; ...
          0.025914 0.041160  0.010133; ...
         -0.007890 0.010133  0.054390];
I3_std = [0.061423 -0.008671 -0.004271; ...
         -0.008671 0.025989 -0.019005; ...
         -0.004271 -0.019005 0.039490];
I4_std = [0.012593 -0.002378 0.005175; ...
         -0.002378 0.013977 0.002782; ...
          0.005175 0.002782 0.007174];
I5_std = [0.024067 -0.000901 -0.000411; ...
         -0.000901 0.006554 -0.010511; ...
         -0.000411 -0.010511 0.017884];
I6_std = [0.007701 -0.001011 0.001449; ...
         -0.001011  0.003049 0.001540; ...
          0.001449  0.001540 0.006941];
I7_std = [0.003495 -0.002318 0.000483; ...
         -0.002318  0.001854  0.000623; ...
          0.000483  0.000623 0.004954];
Ic_kuka(:,:,1) = Rx(pi/2)  * I1_std * Rx(pi/2)'; 
Ic_kuka(:,:,2) = Rx(-pi/2) * I2_std * Rx(-pi/2)'; 
Ic_kuka(:,:,3) = Rx(-pi/2) * I3_std * Rx(-pi/2)';
Ic_kuka(:,:,4) = Rx(pi/2)  * I4_std * Rx(pi/2)'; 
Ic_kuka(:,:,5) = Rx(pi/2)  * I5_std * Rx(pi/2)'; 
Ic_kuka(:,:,6) = Rx(-pi/2) * I6_std * Rx(-pi/2)'; 
Ic_kuka(:,:,7) = I7_std;

clear I1_std I2_std I3_std I4_std I5_std I6_std I7_std 

for i = 1:n_kuka_joint
    % Link Mass
    Kuka_LWR_4_mdh.links(i).m = m_kuka(i);
    % Motor inertia (Unknown and neglected)
    Kuka_LWR_4_mdh.links(i).Jm = 0.0;
    % Gear ratio (Unknown and neglected)
    Kuka_LWR_4_mdh.links(i).G = 1;
    % Link Inertia Tensor
    %        Ixx     Iyy      Izz    Ixy     Iyz     Ixz
    Kuka_LWR_4_mdh.links(i).I = [Ic_kuka(1,1,i) Ic_kuka(2,2,i) Ic_kuka(3,3,i) ...
                             Ic_kuka(1,2,i) Ic_kuka(2,3,i) Ic_kuka(1,3,i) ];
    % Link Center of Mass
    %         rx      ry      rz
    Kuka_LWR_4_mdh.links(i).r = r_kuka(i,:); 
end

clear m_kuka Ic_kuka r_kuka n_kuka_joint

%%
function Rout = Rx(value)
    Rout = [1  0           0; ...
            0  cos(value) -sin(value); ...
            0  sin(value)  cos(value) ];
end


