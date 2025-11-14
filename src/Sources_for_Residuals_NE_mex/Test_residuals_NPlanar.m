
% Coded by Giammarco Tonti 2025

clc
clear all
close all

% Global variable useful to define the number of joints/links of a N-links planar robot
global n_joint

% It informs the "residuals_NE(...)" function that the robot passed as variable is a N-planar robot
% 0 -> Kuka LWR 4 robot
% 1 -> N-links planar robot
is_planar_robot = 1;

% Number of configurations (MUSTE BE EQUAL TO 1)
n = 1;
% Number of iterations
iterations = 10000;

%% Residual calculus

for l = 3:13

    clear q qd qdd qddd null_col_vec null_row_vec timings
    clear planar_robot_N

    disp("!!! RESIDUAL CALCULUS (ITERATION: " + (l-2) + " , NUM JOINTS: " + l + ") !!!")

    n_joint = l; 
    mdl_N_link
    
    % Create random points in state space
    q = rand(n, n_joint);
    qd = rand(n, n_joint);
    qdd = rand(n, n_joint);
    qddd = rand(n, n_joint);

    null_col_vec = zeros(n_joint,1);
    null_row_vec = zeros(1,n_joint);
    
    disp("-- Standard method for RESIDUALS calculus exploiting just General-Purpose NE routines (NE) --")
    res_calculus_mode = 0;
    for i = 1:iterations
        %%% Standard method for RESIDUALS calculus exploiting just NE routine
    
        [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(planar_robot_N, q,qd,qdd,qddd, ...
                                                                    [0;0;0;0;0;0], ...
                                                                    res_calculus_mode, is_planar_robot);
    
        outer_term_0 = tmp_outer_term';
        inner_term_0 = tmp_inner_term';
        timings(i,1) = tmp_timing;
    
    end
    
    disp("-- Novel method for RESIDUALS calculus exploiting both General-Purpose NE and NED routines (Ext. NE) --")
    res_calculus_mode = 1;
    for i = 1:iterations
    
        [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(planar_robot_N, q,qd,qdd,qddd, ...
                                                                    [0;0;0;0;0;0], ...
                                                                    res_calculus_mode, is_planar_robot);
    
        outer_term_1 = tmp_outer_term';
        inner_term_1 = tmp_inner_term';
        timings(i,2) = tmp_timing;
    
    end
    
    disp("-- Standard method for RESIDUALS calculus exploiting just Numerical NE routines (Cust. NE) --")
    res_calculus_mode = 2;
    for i = 1:iterations
    
        [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(planar_robot_N, q,qd,qdd,qddd, ...
                                                                    [0;0;0;0;0;0], ...
                                                                    res_calculus_mode, is_planar_robot);
    
        outer_term_2 = tmp_outer_term';
        inner_term_2 = tmp_inner_term';
        timings(i,3) = tmp_timing;
    
    end
    
    disp("-- Novel method for RESIDUALS calculus exploiting both Numerical NE and NED routines (Cust. Ext. NE) --")
    res_calculus_mode = 3;
    for i = 1:iterations
    
       [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(planar_robot_N, q,qd,qdd,qddd, ...
                                                                    [0;0;0;0;0;0], ...
                                                                    res_calculus_mode, is_planar_robot);
    
        outer_term_3 = tmp_outer_term';
        inner_term_3 = tmp_inner_term';
        timings(i,4) = tmp_timing;
    
    end
    
    disp("-- Classical RESIDUALS calculus exploiting Euler-Lagrange formulation (Cust. EL) --")
    res_calculus_mode = 4;
    for i = 1:iterations
    
        [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(planar_robot_N, q,qd,qdd,qddd, ...
                                                                    [0;0;0;0;0;0], ...
                                                                    res_calculus_mode, is_planar_robot);
    
        outer_term_4 = tmp_outer_term';
        inner_term_4 = tmp_inner_term';
        timings(i,5) = tmp_timing;
    
    end

    disp("-- Novel method for RESIDUALS calculus exploiting Numerical NE routines and smart dotM(q)*dotq product (paper [23]) --")
    res_calculus_mode = 5;
    for i = 1:iterations
    
        [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(planar_robot_N, q,qd,qdd,qddd, ...
                                                                    [0;0;0;0;0;0], ...
                                                                    res_calculus_mode, is_planar_robot);
    
        outer_term_5 = tmp_outer_term';
        inner_term_5 = tmp_inner_term';
        timings(i,6) = tmp_timing;
    
    end
    
    %%% Calculus of all computational timings
    
    for i = 1:6
        mean_time_complete(i) = mean(timings(:,i));
    
        % Non filtered
        % filtered_timings = timings(:,i);
        % Filtered
        filtered_timings = timings(timings(:,i) < mean_time_complete(i),i);
        
        if isempty(filtered_timings)
            min_time_filtered(i) = 0;
            mean_time_filtered(i) = 0;
            std_time_filtered(i) = 0;
        else 
            min_time_filtered(i) = min(filtered_timings);
            mean_time_filtered(i) = mean(filtered_timings);
            std_time_filtered(i) = std(filtered_timings);
        end
    end
    % Saving of all timings
    tmp_idx = l-2;
    overall_metrics(:,:,tmp_idx) = [min_time_filtered', mean_time_filtered', std_time_filtered'];

    disp("Min timings (usec): NE = " + min_time_filtered(1) + " , Ext. NE = " + min_time_filtered(2) + ...
                          " , Cust. NE = " + min_time_filtered(3) + " , Cust. Ext. NE = " + min_time_filtered(4) + ...
                          " , Cust. EL = " + min_time_filtered(5) + " , Smart Ext. NE = " + min_time_filtered(6) )
    disp("Mean timings (usec): NE = " + mean_time_filtered(1) + " , Ext. NE = " + mean_time_filtered(2) + ...
                          " , Cust. NE = " + mean_time_filtered(3) + " , Cust. Ext. NE = " + mean_time_filtered(4) + ...
                          " , Cust. EL = " + mean_time_filtered(5) + " , Smart Ext. NE = " + mean_time_filtered(6) )
    disp("StdDev timings (usec): NE = " + std_time_filtered(1) + " , Ext. NE = " + std_time_filtered(2) + ...
                          " , Cust. NE = " + std_time_filtered(3) + " , Cust. Ext. NE = " + std_time_filtered(4) + ...
                          " , Cust. EL = " + std_time_filtered(5) + " , Smart Ext. NE = " + std_time_filtered(6) )
    disp(newline)

end

%% Overall Min and Mean timings plotting

% Custom colors for each method
customColors = [
    1, 0, 0; % Red for NE
    0, 0, 1; % Blue for Ext. NE
    1, 0.85, 0; % Yellow for Cust. NE
    0, 1, 1; % Cyan for Cust. Ext. NE
    0, 1, 0; % Green for Cust. EL
    1, 0, 1; % Magenta for Smart Ext NNE
];
vec_of_joints = 3:13;

figure
hold on
title("Minimum computational times")
xlabel("Number of Joints")
ylabel("Time [{\mu}ec]")
ax = gca;
ax.XAxis.FontName = 'Times New Roman';
ax.XAxis.FontSize = 11.5;
ax.YAxis.FontName = 'Times New Roman';
ax.YAxis.FontSize = 11.5;
for k = 1:6
    for j = 1:13-2
        tmp_timings_vec(j) = overall_metrics(k,1,j);
    end 
    plot(vec_of_joints, tmp_timings_vec, "Color", customColors(k,:), 'LineWidth', 1.2, LineJoin="chamfer")
end
legend('Newton-Euler', 'Ext. Newton-Euler', 'Cust. Newton-Euler', ...
       'Cust. Ext. Newton-Euler', 'Cust. Euler-Lagrange', 'Smart Ext. Newton-Euler',  ...
       'FontName', 'Times New Roman', 'FontSize', 11.5)

figure
hold on
title("Mean computational times")
xlabel("Number of Joints")
ylabel("Time [{\mu}ec]")
ax = gca;
ax.XAxis.FontName = 'Times New Roman';
ax.XAxis.FontSize = 11.5;
ax.YAxis.FontName = 'Times New Roman';
ax.YAxis.FontSize = 11.5;
for k = 1:6
    for j = 1:13-2
        tmp_timings_vec(j) = overall_metrics(k,2,j);
    end 
    plot(vec_of_joints, tmp_timings_vec, "Color", customColors(k,:), 'LineWidth', 1.2, LineJoin="chamfer")
end
legend('Newton-Euler', 'Ext. Newton-Euler', 'Cust. Newton-Euler', ...
       'Cust. Ext. Newton-Euler', 'Cust. Euler-Lagrange', 'Smart Ext. Newton-Euler', ...
       'FontName', 'Times New Roman', 'FontSize', 11.5)

%% Overall SPLITTED Mean timings plotting

% Custom colors for each method
customColors = [
    1, 0, 0; % Red for NE
    0, 0, 1; % Blue for Ext. NE
    1, 0.85, 0; % Yellow for Cust. NE
    0, 1, 1; % Cyan for Cust. Ext. NE
    0, 1, 0; % Green for Cust. EL
    1, 0, 1; % Magenta for Smart Ext NNE
];
vec_of_joints = 3:13;

figure
hold on
title("Mean computational times")
xlabel("Number of Joints")
ylabel("Time [{\mu}ec]")
ax = gca;
ax.XAxis.FontName = 'Times New Roman';
ax.XAxis.FontSize = 11.5;
ax.YAxis.FontName = 'Times New Roman';
ax.YAxis.FontSize = 11.5;
for k = 1:2
    for j = 1:13-2
        tmp_timings_vec(j) = overall_metrics(k,2,j);
    end 
    plot(vec_of_joints, tmp_timings_vec, "Color", customColors(k,:), 'LineWidth', 1.2, LineJoin="chamfer")
end
legend('Newton-Euler', 'Ext. Newton-Euler', ...
       'FontName', 'Times New Roman', 'FontSize', 11.5)

figure
hold on
title("Mean computational times")
xlabel("Number of Joints")
ylabel("Time [{\mu}ec]")
ax = gca;
ax.XAxis.FontName = 'Times New Roman';
ax.XAxis.FontSize = 11.5;
ax.YAxis.FontName = 'Times New Roman';
ax.YAxis.FontSize = 11.5;
for k = 3:6
    for j = 1:13-2
        tmp_timings_vec(j) = overall_metrics(k,2,j);
    end 
    plot(vec_of_joints, tmp_timings_vec, "Color", customColors(k,:), 'LineWidth', 1.2, LineJoin="chamfer")
end
legend('Cust. Newton-Euler', 'Cust. Ext. Newton-Euler', 'Cust. Euler-Lagrange', 'Smart Ext. Newton-Euler', ...
       'FontName', 'Times New Roman', 'FontSize', 11.5)
