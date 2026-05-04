cd /home/mary/Projects/gRPS4AleoAlians/build

if command -v convert &> /dev/null; then
    convert -size 640x480 xc:black test.jpg
    echo "Created test.jpg using ImageMagick"
else
    dd if=/dev/zero of=test.jpg bs=1024 count=100 2>/dev/null
    echo "Created dummy test.jpg (100KB of zeros)"
fi

echo "Test image created: test.jpg ($(ls -lh test.jpg | awk '{print $5}'))"
