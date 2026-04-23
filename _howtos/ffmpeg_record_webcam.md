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

---

## Bevindingen

* Het lijkt erop dat mjpeg een moeilijker te comprimeren stream levert dan NV12 of YUYV422, wat AV1 betreft in ieder geval. De bestandsgrootte is merkbaar groter.
* Voor de input format levert NV12 een iets lagere hoeveelheid data dan YUYV422 (ca 80%). > Toch lijkt YUYV422 de best comprimeerbare stream op te leveren!
* Pixel format yuv422p10le lijkt alleen mogelijk met FFMPEG wanneer gecompileert met speciale vlaggen. Maar **yuv420p10le** is ook prima, stukken beter dan yuv422p sowieso. 
* 1080P levert een dubbel zo grote eindstroom op als 720P, de kwaliteit is echter niet zoveel merkbaar beter (subjectief).
* Om te donkere stukken op te lichten zonder de rest aan te tasten (dynamic range) kan je -vf "lutyuv=y=gammaval(0.8)" gebruiken, waarbij een lager getal een hogere lichtheid aanduid.

AANBEVOLEN:

`ffmpeg -f v4l2 -input_format yuyv422 -framerate 30 -video_size 1280x720  -i /dev/video0 -f pulse -i default -vf unsharp=3:3:1,setsar=1:1 -c:v libsvtav1 -pix_fmt yuv420p10le -crf 35 -preset 7 -c:a libopus -ac 1 -b:a 64k -af "highpass=f=200,compand=attacks=.01=decays=.01:points=-65/-90|-30/-30|-20/-18|0/0,crystalizer,speechnorm=e=3:r=0.00001:l=1" out720@30_av1-yuyv422.webm -y`

of

`ffmpeg -f v4l2 -input_format yuyv422 -framerate 30 -video_size 1920x1080  -i /dev/video0 -f pulse -i default -vf unsharp=5:5:1,setsar=1:1 -c:v libsvtav1 -pix_fmt yuv420p10le -crf 35 -preset 7 -c:a libopus -ac 1 -b:a 64k -af "highpass=f=200,compand=attacks=.01=decays=.01:points=-65/-90|-30/-30|-20/-18|0/0,crystalizer,speechnorm=e=3:r=0.00001:l=1" out1080@30_av1-yuyv422.webm -y`

of, in geval van NVIDIA encoder met HEVC (voor latere bewerking)

`ffmpeg -f v4l2 -input_format yuyv422 -framerate 30 -video_size 1920x1080  -i /dev/video0 -f pulse -i default -vf "lutyuv=y=gammaval(0.6),unsharp=5:5:1,setsar=1:1" -c:v hevc_nvenc -pix_fmt p010le -profile:v main10 -preset p7 -rc vbr -cq 22 -b:v 0 -c:a libopus -b:a 96k -ac 1 -af "highpass=f=200,compand=attacks=.01=decays=.01:points=-65/-90|-30/-30|-20/-18|0/0,crystalizer,speechnorm=e=3:r=0.00001:l=1" out1080.mkv -y`
