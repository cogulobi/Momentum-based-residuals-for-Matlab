% MDL_N_LINK Create model of a simple N-link planar arm according to the
% standard Denavit-Hartenberg notation
%
%       mdl_N_link
%
% Script creates the workspace variable "planar_robot_N" which describes the 
% kinematic and dynamic characteristics of the N-DOF planar arm.
%
% Also defines the vector:
%   qz   corresponds to the zero joint angle configuration.
%
% Notes::
% - Moves in the XY plane WITHOUT gravity.

% MODEL: generic, planar, N-DOF, standard_DH

clear planar_robot_N

global n_joint
a = 0.22; 

% n_joint = 7;

for i = 1:n_joint
    all_joints(i) = Revolute('d', 0, 'a', a, 'alpha', 0, 'standard');
end
planar_robot_N = SerialLink( ...
    all_joints, ...
    'name', 'planar N link');

for i = 1:n_joint
    % Link Mass
    planar_robot_N.links(i).m = 3.458;
    % Motor inertia
    planar_robot_N.links(i).Jm = 0; %0.006;
    % Gear ratio
    planar_robot_N.links(i).G = 1; %5;
    % Link Inertia Tensor
    %                            Ixx       Iyy       Izz      Ixy        Iyz         Ixz
    planar_robot_N.links(i).I = [0.00457   0.03294   0.3287   0.0   -0.001691   -0.0];
    %planar_robot_N.links(i).I = [0.00457   0.03294   0.3287   0.00024   -0.001691   -0.00013];
    % Link Center of Mass
    planar_robot_N.links(i).r = [-0.068851   0.0   0]; 
    %planar_robot_N.links(i).r = [-0.068851   0.002107   0]; 
end

qz = zeros(1,n_joint); %[0 0 0 0 0];
value = pi/6;
qn = zeros(1,n_joint);
for i = 1:n_joint
    if mod(i,2)
        qn(i) = value;
    else
        qn(i) = -value;
    end
end

clear a

% the robot defined above moves in the XY plane and is not influenced by
% the default gravity vector, acting in the Z-direction.  We choose to
% rotate the robot so that it moves in the XZ plane.
% robot_N.base = trotx(pi/2);



