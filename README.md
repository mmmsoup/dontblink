# dontblink

Animated wallpaper functionality based on the awesome [asetroot](https://github.com/Wilnath/asetroot) by [@Wilnath](https://github.com/Wilnath)

## Installation
Install dependencies:
```
pacman -S imagemagick imlib2
```
(or whatever your package manager is)

Clone the repo and setup:
```
git clone https://github.com/mmmsoup/dontblink
cd dontblink
./install.sh
```

## To-Do
- make checking whether desktop is obscured better and more comprehensive
- add option to use stills instead of animations
- restore previous wallpaper after program is terminated
- resize static images based on target resolution during install process
- optimise loading of images so wait at beginning is not so long
