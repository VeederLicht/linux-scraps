# LINUX
How to record from webcam and microphone in Linux with FFMPEG?

## webcam
https://trac.ffmpeg.org/wiki/Capture/Webcam

## microphone
https://trac.ffmpeg.org/wiki/Capture/PulseAudio

## examples
1. with image scaling, sharpening, audio noise reduction and speech enhancement:
`ffmpeg -f v4l2 -input_format yuyv422 -framerate 30 -video_size 1280x720  -i /dev/video0 -f pulse -i default -vf scale=-2:360,unsharp=3:3:1,setsar=1:1 -c:v libsvtav1  -crf 31 -preset 8  -c:a libopus -ac 1 -b:a 48k -af "highpass=f=200,compand=attacks=.01=decays=.01:points=-65/-90|-30/-30|-20/-18|0/0,crystalizer,speechnorm=e=3:r=0.00001:l=1" out360_av1.webm -y`
