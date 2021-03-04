#!/bin/sh

resolution="$(xdpyinfo | awk '/dimensions:/ {print $2}')"

mkdir ~/.dontblink

echo "-> copying images and splitting gifs into frames"
for i in $(seq 0 5)
do
	echo "   -> $(($i+1)) of 6"

	mkdir ~/.dontblink/${i}

	# static images
	cp static/${i}.png ~/.dontblink/${i}/00000.png

	# frames for animated wallpaper
	mkdir ~/.dontblink/${i}/anim
	convert anim/${i}.gif -coalesce -resize $resolution ~/.dontblink/${i}/anim/%05d.gif
done

# build binary
echo "-> building dontblink.c"
make

echo "-> done"
