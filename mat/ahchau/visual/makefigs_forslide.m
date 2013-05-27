function figsize_pixels = makefigs_forslide(fignum);
% function figsize_pixels = makefigs_forslide(fignum);
% should ulimately make this calculate position on screen. for now, just
% change sizes appropriately
% AHChau 12/3/12


figsize = [9 6.07];  % my standard slide size, in inches

hf = figure(fignum);
set(hf,'papersize',figsize);
set(hf,'paperposition',[0 0 figsize]);

set(hf,'units','inches');
currpos = get(hf,'position');
set(hf,'position',[currpos(1:2) figsize]);
set(hf,'units','pixels');
newpos = get(hf,'position');
figsize_pixels = newpos(3:4);


