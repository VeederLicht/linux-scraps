# LINUX
How to record from webcam and microphone in Linux with FFMPEG?

## webcam
https://trac.ffmpeg.org/wiki/Capture/Webcam

## microphone
https://trac.ffmpeg.org/wiki/Capture/PulseAudio

## examples
`ffmpeg -f v4l2 -framerate 30 -video_size 1920x1080 -input_format yuyv422 -i /dev/video0 -f pulse -i default -c:v libx264 -preset medium -crf 26 -c:a mp3 -b:a 128K output.mp4 -y`

nb: to add noise filtering and audio enhancement add this filter:
`-af "highpass=f=200,compand=attacks=.01=decays=.01:points=-65/-90|-30/-30|-20/-18|0/0,crystalizer"`
