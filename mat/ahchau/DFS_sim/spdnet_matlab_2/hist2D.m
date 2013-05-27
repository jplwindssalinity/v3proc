function [x,y,ct,yave,xave,ystd,xstd,miss]=hist2d(x0,y0,wx,wy)

    if(length(wx)==1)
         x=(min(x0):wx:max(x0))+0.5*wx;
    else
         x=wx;
         wx=x(2)-x(1);
    end
    if(length(wy)==1)
         y=(min(y0):wy:max(y0))+0.5*wy;
    else 
         y=wy;
         wy=y(2)-y(1);
    end

    idx=find(x0>=min(x) & x0<=max(x) & y0>=min(y) & y0<=max(y));
    miss=length(x0)-length(idx);
    x0=x0(idx);
    y0=y0(idx);
yave=zeros(1,length(x));
xave=zeros(1,length(y));
ystd=zeros(1,length(x));
xstd=zeros(1,length(y));
    sz=size(x0);
    sz=sz(1)*sz(2);
    szx=size(x,2);
    szy=size(y,2);
    ct(1:szx,1:szy)=0;
    mx=min(x);
    my=min(y);
    for c=1:sz
         i=floor((x0(c)-mx)/wx)+1;
        j=floor((y0(c)-my)/wy)+1;
        ct(i,j)=ct(i,j)+1;
        yave(i)=yave(i)+y0(c);
        xave(j)=xave(j)+x0(c);
ystd(i)=ystd(i)+y0(c)*y0(c);
xstd(j)=xstd(j)+x0(c)*x0(c);
    end
    x=x+0.5*wx;
    y=y+0.5*wy;

yave=yave./sum(ct');
xave=xave./sum(ct);
ystd=sqrt(ystd./sum(ct')-yave.*yave);
xstd=sqrt(xstd./sum(ct)-xave.*xave);
