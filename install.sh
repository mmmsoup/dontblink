#!/bin/sh

image_res="1920x1080"
scene_num=6

screen_res="$(xdpyinfo | awk '/dimensions:/ {print $2}')"
echo "-> detected screen resolution of $screen_res"

img_x=${image_res%x*}
img_y=${image_res#*x}
res_x=${screen_res%x*}
res_y=${screen_res#*x}

ratio_x=$(echo "$res_x $img_x" | awk '{print 100*$1/$2}')
ratio_y=$(echo "$res_y $img_y" | awk '{print 100*$1/$2}')

shave="0x0"
scale="100%"

# determine how images should be cropped and enlarged to fit other resolutions than 1920x1080
if [[ $ratio_y > $ratio_x ]]; then
	# shave x
	shave="$(( ($img_y - $res_y) / 2 ))x0"
	scale="${ratio_y}%"
elif [[ $ratio_y < $ratio_x ]]; then
	# shave y
	shave="0x$(( ($img_x - $res_x) / 2 ))"
	scale="${ratio_x}%"
else
	# don't crop; just resize
	scale="${ratio_x}%"
fi

mkdir ~/.dontblink

echo "-> copying images and splitting gifs into frames (this might take a while)"
for i in $(seq 0 $((scene_num-1)))
do
	echo "   -> $(($i+1)) of $scene_num"

	mkdir ~/.dontblink/${i}

	# static images
	convert static/${i}.png -shave $shave -resize $scale ~/.dontblink/${i}/00000.png

	# frames for animated wallpaper
	mkdir ~/.dontblink/${i}/anim
	convert anim/${i}.gif -coalesce -shave $shave -resize $scale ~/.dontblink/${i}/anim/%05d.gif
done

# build binary
echo "-> building dontblink.c"
make

echo "-> done"
