#!/bin/sh

resolution="$(xdpyinfo | awk '/dimensions:/ {print $2}')"

# create folders needed
mkdir ~/.dontblink ~/.dontblink/anim ~/.dontblink/static

echo "-> copying images and splitting gifs into frames"
for i in $(seq 0 5)
do
	echo "   -> $(($i+1)) of 6"

	# static images
	mkdir ~/.dontblink/static/${i}
	cp static/${i}.png ~/.dontblink/static/${i}.png

	# frames for animated wallpaper
	mkdir ~/.dontblink/anim/${i}
	convert anim/${i}.gif -coalesce -resize $resolution ~/.dontblink/anim/${i}/%05d.gif
done

# build binary
echo "-> building dontblink.c"
make
