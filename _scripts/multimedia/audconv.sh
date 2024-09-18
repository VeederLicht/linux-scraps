#!/bin/bash

# User information to inject in metadata
m_copyright=''
m_encoded_by='Converted with audconv.sh (veederlicht@github)'
m_comment=''

# Define constants
scriptv="v2.0b"
sYe="\e[93m"
sNo="\033[1;35m"
outdir=./$(date +%Y%m%d_%H.%M_)audconv
logfile="$outdir"/$(date +%Y%m%d_%H.%M_)"audconv.rep"
mkdir -p "$outdir"
touch $logfile

# Show banner
function showbanner {
clear
echo -e "${sNo}  ======================================================================================================="
echo -e "       Batch convert audio, with options. Copyleft 2024 VeederLicht@Github"
echo -e "  --------------------------------------------${sYe} $scriptv ${sNo}----------------------------------------------------\n\n"
}
showbanner

# Test nr. of arguments
if [ ! -d "$outdir" ]; then
	echo "      ERROR: Output directory could not be created, exiting..."
	exit 2
fi
if [ $# -eq 0 ]; then
	echo "      ERROR:  No source files specified."
	exit 2
fi

# !!!! ARGUMENTS IN DOUBLE QOUTES TO AVOID PROBLEMS WITH SPACES IN FILENAMES!!! https://stackoverflow.com/questions/12314451/accessing-bash-command-line-args-vs
echo -e "  FILES TO BE PROCESSED:"
for f in "$@"; do
	echo -e "    ➢➢  $f"
done



# ... SET CONVERT OPTIONS
echo -e "\n ${sYe}  NOTE:"
echo -e "       - metadata will be injected, to change it edit this scriptheader${sNo} \n"

# ... select output format.........................................................................................
echo -e ""
echo -e "  SELECT OUTPUT ENCODER*: "
echo -e "   (0)   Convert to OPUS"
echo -e "   (1)   Convert to AAC"
echo -e "   (2)   Convert to MP3"
echo -e ""
echo -e ""
echo -e "	* Based on quality produced from high to low:"
echo -e "		libopus > libvorbis >= libfdk_aac > libmp3lame >= eac3/ac3 > aac > libtwolame > vorbis > mp2 > wmav2/wmav1"
echo -e "		(https://trac.ffmpeg.org/wiki/Encode/HighQualityAudio)"
echo -e ""
read -p "      --> " answer_format
echo -e ""

# ... select quality.........................................................................................
echo -e "  SELECT OUTPUT QUALITY: "
echo -e "   (0)   low quality (mono)"
echo -e "   (1)   medium quality"
echo -e "   (2)   high quality"
echo -e ""
read -p "      --> " answer_quality
echo -e ""

case $answer_format in

0)
	case $answer_quality in
	0)
		arg0b="[048k1-"
		arg1="-c:a libopus -b:a 48k -ac 1"
		;;

	1)
		arg0b="[080k-"
		arg1="-c:a libopus -b:a 80k"
		;;

	2)
		arg0b="[160k-"
		arg1="-c:a libopus -b:a 160k"
		;;
	esac
	arg0a="opus"
	arg0b=$arg0b"opus]"
	;;

1)
	case $answer_quality in
	0)
		arg0b="[064k1-"
		arg1="-c:a aac -b:a 64k -ac 1"
		;;

	1)
		arg0b="[096k-"
		arg1="-c:a aac -b:a 96k"
		;;

	2)
		arg0b="[192k-"
		arg1="-c:a aac -b:a 192k"
		;;
	esac
	arg0a="m4a"
	arg0b=$arg0b"aac]"
	;;

2)
	case $answer_quality in
	0)
		arg0b="[064k-"
		arg1="-c:a libmp3lame -b:a 64k -ac 1"
		;;

	1)
		arg0b="[128k-"
		arg1="-b:a 128k"
		;;

	2)
		arg0b="[256k-"
		arg1="-b:a 256k"
		;;

	esac
	arg0a="mp3"
	arg0b=$arg0b"lame]"
	;;
*)
	echo "Unknown option, exiting..."
	exit 3
	;;
esac
echo -e "  ============ Batch convert audio, with options. $scriptv ============\n" > $logfile
echo -e "  -----------------Convert to [ $arg0a ] with [ $arg1 ] \n" >> $logfile

# 	... select audio type.........................................................................................
#
# 	echo -e "      SELECT AUDIO TYPE:"
# 	echo -e "\n"
# 	echo -e "     (m) music"
# 	echo -e "     (v) voice"
# 	echo -e ""
# 	read -p "      --> " answer_type
# 	echo -e ""
#
# 	case $answer_type in
# 	  "m")
#         arg2="-application audio"
# 		;;
# 	  "v")
#         arg2="-application voip"
# 		;;
# 	  *)
# 		echo "Unknown option, exiting..."
# 		exit 3
# 		;;
# 	esac
#     echo -e "  ----------------[ $arg2 ] \n" >> $logfile




# ... AUDIO FILTERS



# ... compose filter-string.........................................................................................
afilt=""
function addcomma {
	[[ ! -z "$afilt" ]] && afilt=$afilt,
}



# ... select frequencies.........................................................................................
echo -e "  SELECT FREQUENCY CUTOUT:"
echo -e "   (0) none"
echo -e "   (1) low frequencies (<200Hz)"
echo -e "   (2) high frequencies (>3500Hz)"
echo -e "   (3) both (voice isolation)"
echo -e ""
read -p "      --> " answer_noise
echo -e ""

case $answer_noise in
"" | 0) ;;
1)
	addcomma
	afilt=$afilt"highpass=f=200"
	;;
2)
	addcomma
	afilt=$afilt"lowpass=f=3500"
	;;
3)
	addcomma
	afilt=$afilt"highpass=f=150,lowpass=f=3500"
	;;
*)
	echo "Unknown option, exiting..."
	exit 3
	;;
esac

# ... select crystalizer .........................................................................................
echo -e "  ENHANCE AUDIO (chrystalizer, compression, normalization)?"
echo -e "     (0) no"
echo -e "     (1) yes"
echo -e ""
read -p "      --> " answer_crystalizer
echo -e ""

case $answer_crystalizer in
"" | 0) ;;
1)
	addcomma
	afilt=$afilt"compand=attacks=.01=decays=.01:points=-50/-90|-30/-30|-20/-18|0/0,crystalizer,speechnorm=e=1:c=1:r=0"
	;;
*)
	echo "Unknown option, exiting..."
	exit 3
	;;
esac


# ... select extrastereo .........................................................................................
echo -e "  ENHANCE STEREO EFFECT?"
echo -e "     (0) no"
echo -e "     (1) yes"
echo -e ""
read -p "      --> " answer_stereo
echo -e ""

case $answer_stereo in
"" | 0) ;;
1)
	addcomma
	afilt=$afilt"extrastereo"
	;;
*)
	echo "Unknown option, exiting..."
	exit 3
	;;
esac


# ... select length.........................................................................................
echo -e "  SELECT LENGTH: "
echo -e "   (0) Full file length"
echo -e "   (1) Short preview  [15s]"
echo -e "   (2) Long preview  [60s]"
echo -e ""
read -p "       Select processing length: " answer1

case $answer1 in
"" | 0)
	arg4=""
	arg5=""
	;;
1)
	arg4="-ss 00:45 -t 15"
	arg5="[15s]"
	;;
2)
	arg4="-ss 00:45 -t 60"
	arg5="[60s]"
	;;
*)
	echo "Unknown option, exiting..."
	exit 3
	;;
esac
echo -e "  ----------------Process length: $arg5 \n" >>$logfile

# ... LET'S GET TO WORK.....................................................................
afilt_original=$afilt

for f in "$@"; do
	afilt=$afilt_original
	outfile="$outdir"/"$f".$arg0b$arg5.$arg0a

	echo -e ""
	echo -e "\n\n........................Processing "$f" ...to... $outfile" | tee -a $logfile

	if [[ -n $afilt ]]; then
		afilt="-af "$afilt
	fi
	ffmpeg -y -hide_banner -i "$f" $arg4 $arg1 $afilt $arg2 -metadata comment="$m_comment" -metadata encoded_by="$m_encoded_by" -metadata copyright="$m_copyright" -map_metadata 0 -id3v2_version 3 "$outfile"
done

echo -e "\n\n  ---------------------------------------------END-------------------------------------------------------\n" >>$logfile

echo -e "\n\n\n   Output has been written to: " $logfile
echo -e "\n ${sYe} Finished. ${sNo} \n\n\n"
