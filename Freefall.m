% Beregning af det frie fald, med og uden luftmodstand

V0=0;               % initial speed
h0=0.5;             % initial height
m=0.197;            % mass in kg

g=9.82;             % gravity acceleration kg/m3
rho=1.225;          % Air density
A_w=0.00064;          % Object area (small cross-section)
A_b=0.01201;          % Object area (large cross-section)
cw=1.05;            % Numerical drag coefficient
k_w=0.5*cw*rho*A_w; % Coefficient
k_b=0.5*cw*rho*A_b; % Coefficient

N=5100;             % Time step

%(worst case - least amount of drag)
V_w=zeros(1,N);     % Speed 
h_w=zeros(1,N);     % height at a given point
V_w(1)=V0;          % assign initial velocity
h_w(1)=h0;          % assign initial height

%(Best case - most amount of drag)
V_b=zeros(1,N);     % Speed
h_b=zeros(1,N);     % height at a given point
V_b(1)=V0;          % assign initial velocity
h_b(1)=h0;          % assign initial height

% fall with no drag accounted
h_fald=zeros(1,N);  % height at a given point with drag neglected

deltat=0.0001;      % incremental timestep
t=(0:N-1)*deltat;   % Time span intended to be plottet
u=t-t;              % line at y=0
%vterminal=sqrt(g*m/k_w); % Terminal velocity


for i=1:N-1
V_w(i+1)=V_w(i)+deltat*(g-(k_w*V_w(i)^2)/m);

h_w(i+1)=h_w(i)-V_w(i)*deltat;

V_b(i+1)=V_b(i)+deltat*(g-(k_b*V_b(i)^2)/m);

h_b(i+1)=h_b(i)-V_b(i)*deltat;

h_fald=-(1/2)*g*t.^2+h0;
end

plot(t,h_w,t,h_b,t,h_fald,t,u);

    set(gca,'xlim',[0,0.51],'ylim',[-0.01,0.51]);
    xlabel('time in s');
    ylabel('height in m');
    legend ('Euler Method (worst case)','Euler Method (best case)','Analytical velocity','location','south');
    