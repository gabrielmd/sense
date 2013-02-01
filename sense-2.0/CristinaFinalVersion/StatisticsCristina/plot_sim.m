function [] = plot_sim( Statistics )

color1 = 'blue';

% x = Statistics(:,1)
% 
% figure(1);
% axes('box', 'on','Ygrid','on', 'Xgrid', 'on');
% hold on;
% set(gcf,'Color', 'w');
% xlabel('Change Cell Probability');
% ylabel('Time (s)');
% plot (x, Statistics(:,2),'-ko','Color',color1);
% legend('All Stations Are Out the Cell', 2);
% hold off;

x = Statistics(:,1);
x2 = [0:18];

figure(1);
axes('box', 'on','Ygrid','on', 'Xgrid', 'on');
hold on;
set(gcf,'Color', 'w');
xlabel('Node');
ylabel('Number of stations');
for i = 1:13
plot (x2, Statistics(i,2:20),'-k','Color',color1);
end
legend('Time 100000s', 2);
hold off;

end