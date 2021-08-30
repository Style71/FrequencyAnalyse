function createfigure(X1, Y1)
%CREATEFIGURE(X1, Y1)
%  X1:  x 数据的矢量
%  Y1:  y 数据的矢量

%  由 MATLAB 于 20-Aug-2021 09:45:42 自动生成

% 创建 figure
figure1 = figure;

% 创建 axes
axes1 = axes('Parent',figure1);
hold(axes1,'on');

% 创建 semilogx
semilogx(X1,Y1);

% 创建 xlabel
xlabel('$f/\mathrm{Hz}$','FontSize',12,'Interpreter','latex');

% 创建 ylabel
ylabel('$|Z|/\Omega$','FontSize',12,'Interpreter','latex');

box(axes1,'on');
% 设置其余坐标轴属性
set(axes1,'XGrid','on','XMinorTick','on','XScale','log','YGrid','on');
