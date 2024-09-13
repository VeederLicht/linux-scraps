# LINUX
How to record from webcam and microphone in Linux with FFMPEG?

## webcam
https://trac.ffmpeg.org/wiki/Capture/Webcam

## microphone
https://trac.ffmpeg.org/wiki/Capture/PulseAudio

## examples
`ffmpeg -f v4l2 -framerate 30 -video_size 1920x1080  -i /dev/video0 -f pulse -i default -c:v libx264 -preset medium -crf 26 -c:a mp3 -b:a 128K output.mp4 -y`
