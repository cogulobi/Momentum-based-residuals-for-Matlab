
% Coded by Giammarco Tonti 2025

clc
clear all
close all

mdl_KUKA_LWR_4

% It informs the "residuals_NE(...)" function that the robot passed as variable is a Kuka LWR 4
% 0 -> Kuka LWR 4 robot 
% 1 -> N-links planar robot
is_planar_robot = 0;

% Number of configurations (MUSTE BE EQUAL TO 1)
n = 1;
% Number of iterations
iterations = 10000;

n_joint = Kuka_LWR_4.n;

% Create random points in state space
q = rand(n, n_joint);
qd = rand(n, n_joint);
qdd = rand(n, n_joint);
qddd = rand(n, n_joint);

%% Residual calculus

null_col_vec = zeros(n_joint,1);
null_row_vec = zeros(1,n_joint);

disp("-- Standard method for RESIDUALS calculus exploiting just General-Purpose NE routines (NE) --")
res_calculus_mode = 0;
for i = 1:iterations

    [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(Kuka_LWR_4, q,qd,qdd,qddd, ...
                                                                [0;0;0;0;0;0], ...
                                                                res_calculus_mode, is_planar_robot);

    outer_term_0 = tmp_outer_term';
    inner_term_0 = tmp_inner_term';
    timings(i,1) = tmp_timing;

end

disp("-- Novel method for RESIDUALS calculus exploiting both General-Purpose NE and NED routines (Ext. NE) --")
res_calculus_mode = 1;
for i = 1:iterations

    [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(Kuka_LWR_4, q,qd,qdd,qddd, ...
                                                                [0;0;0;0;0;0], ...
                                                                res_calculus_mode, is_planar_robot);

    outer_term_1 = tmp_outer_term';
    inner_term_1 = tmp_inner_term';
    timings(i,2) = tmp_timing;

end

disp("-- Standard method for RESIDUALS calculus exploiting just Numerical NE routines (Cust. NE) --")
res_calculus_mode = 2;
for i = 1:iterations

    [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(Kuka_LWR_4, q,qd,qdd,qddd, ...
                                                                [0;0;0;0;0;0], ...
                                                                res_calculus_mode, is_planar_robot);

    outer_term_2 = tmp_outer_term';
    inner_term_2 = tmp_inner_term';
    timings(i,3) = tmp_timing;

end

disp("-- Novel method for RESIDUALS calculus exploiting both Numerical NE and NED routines (Cust. Ext. NE) --")
res_calculus_mode = 3;
for i = 1:iterations

    [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(Kuka_LWR_4, q,qd,qdd,qddd, ...
                                                                [0;0;0;0;0;0], ...
                                                                res_calculus_mode, is_planar_robot);

    outer_term_3 = tmp_outer_term';
    inner_term_3 = tmp_inner_term';
    timings(i,4) = tmp_timing;

end

disp("-- Classical RESIDUALS calculus exploiting Euler-Lagrange formulation (Cust. EL) --")
res_calculus_mode = 4;
for i = 1:iterations

    [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(Kuka_LWR_4, q,qd,qdd,qddd, ...
                                                                [0;0;0;0;0;0], ...
                                                                res_calculus_mode, is_planar_robot);

    outer_term_4 = tmp_outer_term';
    inner_term_4 = tmp_inner_term';
    timings(i,5) = tmp_timing;

end

disp("-- Novel method for RESIDUALS calculus exploiting Numerical NE routines and smart dotM(q)*dotq product (paper [23]) --")
res_calculus_mode = 5;
for i = 1:iterations

    [tmp_outer_term, tmp_inner_term, tmp_timing] = residuals_NE(Kuka_LWR_4, q,qd,qdd,qddd, ...
                                                                [0;0;0;0;0;0], ...
                                                                res_calculus_mode, is_planar_robot);

    outer_term_5 = tmp_outer_term';
    inner_term_5 = tmp_inner_term';
    timings(i,6) = tmp_timing;

end

%% Calculus of all computational timings

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

disp("Min timings (usec): NE = " + min_time_filtered(1) + " , Ext. NE = " + min_time_filtered(2) + ...
                      " , Cust. NE = " + min_time_filtered(3) + " , Cust. Ext. NE = " + min_time_filtered(4) + ...
                      " , Cust. EL = " + min_time_filtered(5) + " , Smart Ext. NNE = " + min_time_filtered(6) )
disp("Mean timings (usec): NE = " + mean_time_filtered(1) + " , Ext RNE = " + mean_time_filtered(2) + ...
                      " , Cust. NE = " + mean_time_filtered(3) + " , Cust. Ext. NE = " + mean_time_filtered(4) + ...
                      " , Cust. EL = " + mean_time_filtered(5) + " , Smart Ext. NE = " + mean_time_filtered(6) )
disp("StdDev timings (usec): NE = " + std_time_filtered(1) + " , Ext. NE = " + std_time_filtered(2) + ...
                      " , Cust. NE = " + std_time_filtered(3) + " , Cust. Ext. NE = " + std_time_filtered(4) + ...
                      " , Cust. EL = " + std_time_filtered(5) + " , Smart Ext. NNE = " + std_time_filtered(6) )

%% Histogram 2D plots

% Hystogram Data
data_tmp = [
    min_time_filtered; ...
    mean_time_filtered; ...
    std_time_filtered
];
tmp_RNE     = data_tmp(:,1);
tmp_ext_RNE = data_tmp(:,2);
tmp_NNE     = data_tmp(:,3);
tmp_ext_NNE = data_tmp(:,4);
tmp_NEL     = data_tmp(:,5);
% tmp_SMART_NE = data_tmp(:,6);
data = [tmp_RNE, tmp_ext_RNE, tmp_NEL, tmp_NNE, tmp_ext_NNE];

% Custom colors for each method
customColors = [
    1, 0, 0; % Red for NE
    0, 0, 1; % Blue for Ext. NE
    0, 1, 0; % Green for Cust. EL
    1, 1, 0; % Yellow for Cust. NE
    0, 1, 1; % Cyan for Cust. Ext. NE
];
str_metrics = ['Best', 'Mean', 'StdDev'];
for i = 1:3
    figure
    b = bar(data(i, :), 0.6, 'FaceColor', 'flat'); % Istogramma 2D

    %  Assignment of custom colors 
    for j = 1:length(data(i, :))
        b.CData(j, :) = customColors(j, :); % Color for each bar
    end
    % Adding numerical values over each bar of the hystogram
    for j = 1:length(data(i, :))
        if i == 1 
            text(j, data(i, j) + 0.1, num2str(data(i, j), '%.1f'), ...
                'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom', ...
                'FontSize', 11.5, 'Color', 'k', 'FontName', 'Times New Roman'); % Over each bar
        elseif i == 2
            text(j, data(i, j) + 0.1, num2str(data(i, j), '%.3f'), ...
                'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom', ...
                'FontSize', 11.5, 'Color', 'k', 'FontName', 'Times New Roman'); % Over each bar
        else
            text(j, data(i, j) + 0.005, num2str(data(i, j), '%.3f'), ...
                'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom', ...
                'FontSize', 11.5, 'Color', 'k', 'FontName', 'Times New Roman'); % Over each bar
        end
    end
    
    % Panel customization
    if i == 1
        title('Best computational time'); % Title
        ylim([0 10])
    elseif i == 2
        title('Mean computational time'); % Title
        ylim([0 10])
    else
        title('StdDev computational time'); % Title
        ylim([0 max(data(i,:))+0.05])
    end
    % xlabel(str_metrics(i)); % Label X axis
    ylabel('Time [{\mu}sec]'); % Label Y axis
    xticks(1:5);
    xticklabels({'NE', 'Ext. NE', 'Cust. EL', 'Cust. NE', 'Cust. Ext. NE'});

    ax = gca;
    ax.XAxis.FontName = 'Times New Roman';
    ax.XAxis.FontSize = 12.5;
    ax.YAxis.FontName = 'Times New Roman';
    ax.YAxis.FontSize = 12.5;
end


