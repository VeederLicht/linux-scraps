# LINUX
How to record from webcam and microphone in Linux with FFMPEG?

## webcam
https://trac.ffmpeg.org/wiki/Capture/Webcam

## microphone
https://trac.ffmpeg.org/wiki/Capture/PulseAudio

## examples
1. 360P with image scaling, sharpening, audio noise reduction and speech enhancement:
`ffmpeg -f v4l2 -input_format yuyv422 -framerate 30 -video_size 1280x720  -i /dev/video0 -f pulse -i default -vf scale=-2:360,unsharp=3:3:1,setsar=1:1 -c:v libsvtav1  -crf 35 -preset 8  -c:a libopus -ac 1 -b:a 48k -af "highpass=f=200,compand=attacks=.01=decays=.01:points=-65/-90|-30/-30|-20/-18|0/0,crystalizer,speechnorm=e=3:r=0.00001:l=1" out360_av1.webm -y`

2. 720P with image scaling, sharpening, audio noise reduction and speech enhancement:
`ffmpeg -f v4l2 -input_format yuyv422 -framerate 30 -video_size 1280x720  -i /dev/video0 -f pulse -i default -vf unsharp=3:3:1,setsar=1:1 -c:v libsvtav1  -crf 35 -preset 8  -c:a libopus -b:a 96k -af "highpass=f=200,compand=attacks=.01=decays=.01:points=-65/-90|-30/-30|-20/-18|0/0,crystalizer,speechnorm=e=3:r=0.00001:l=1" out720_av1.webm -y`


3. 1080P@60fps (only with mjpeg with this webcam, noticably worse dynamic range)
`ffmpeg -f v4l2 -input_format mjpeg -framerate 60 -video_size 1920x1080  -i /dev/video0 -f pulse -i default -vf unsharp=3:3:1,setsar=1:1 -c:v libsvtav1  -crf 35 -preset 7  -c:a libopus -b:a 128k -af "highpass=f=200,compand=attacks=.01=decays=.01:points=-65/-90|-30/-30|-20/-18|0/0,crystalizer,speechnorm=e=3:r=0.00001:l=1" out1080_av1.webm -y`
