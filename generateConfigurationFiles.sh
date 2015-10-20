#!/bin/bash

#Quick script to generate FM RDS Simulator configuration XML files for use with libRfSimulators.
#Requires WAV audio files (with .wav extension) to be located in input directory (specified with -i)
#View the libRfSimulators README.md for further instructions on usage.

SCRIPT=`basename ${BASH_SOURCE[0]}`

#Defaults. Frequency values are in tenths of MHz (or hundreds of kHz).
min_freq=880    # Lower limit of the frequency range
max_freq=1080   # Upper limit of the frequency range
num_stations=20 # Number of stations to generate in the defined frequency range
wav_path="/usr/share/libFmRdsSimulator/examples/" # Trailing forward slash is important!
xml_path="/usr/share/libFmRdsSimulator/examples/" # Trailing forward slash is important!

#Set fonts for Help.
NORM=`tput sgr0`
BOLD=`tput bold`
REV=`tput smso`

#Help function
function HELP {
    echo -e \\n"Help documentation for ${BOLD}${SCRIPT}.${NORM}"\\n
    echo -e "${REV}Basic usage:${NORM} ${BOLD}$SCRIPT <options>${NORM}"\\n
    echo "Command line switches are optional. The following switches are recognized."
    echo "${REV}-l${NORM}  --Sets the ${BOLD}lower frequency limit${NORM} in MHz. Default is ${BOLD}${min_freq%?}.${min_freq: -1}${NORM}."
    echo "${REV}-u${NORM}  --Sets the ${BOLD}upper frequency limit${NORM} in MHz. Default is ${BOLD}${max_freq%?}.${max_freq: -1}${NORM}."
    echo "${REV}-n${NORM}  --Sets the ${BOLD}number of stations${NORM}. Default is ${BOLD}${num_stations}${NORM}."
    echo "${REV}-i${NORM}  --Sets the *.wav audio file ${BOLD}input directory${NORM}. Default is ${BOLD}${wav_path}${NORM}."
    echo "${REV}-o${NORM}  --Sets the *.xml configuration file ${BOLD}output directory${NORM}. Default is ${BOLD}${xml_path}${NORM}."
    echo -e "${REV}-h${NORM}  --Displays this help message. No further functions are performed."\\n
    echo -e "Example: ${BOLD}$SCRIPT -l 88.0 -u 108.0 -n 20 -i /usr/share/libFmRdsSimulator/examples -o /usr/share/libFmRdsSimulator/examples${NORM}"\\n
    exit 1
}

#Parse options
while getopts ":l:u:n:i:o:h" opt; do
    case $opt in
        l)
            min_freq=$(echo "($OPTARG*10)/1" | bc)
            ;;
        u)
            max_freq=$(echo "($OPTARG*10)/1" | bc)
            ;;
        n)
            num_stations=$(echo "($OPTARG)/1" | bc)
            ;;
        i)
            wav_path="$(readlink -m $OPTARG)/"
            ;;
        o)
            xml_path="$(readlink -m $OPTARG)/"
            ;;
        h)
          HELP
          ;;
        \?)
            echo -e \\n"Option -${BOLD}$OPTARG${NORM} not allowed." >&2
            echo -e "Use ${BOLD}$SCRIPT -h${NORM} to see the help documentation."\\n
            exit 2
            ;;
        :)
            echo -e \\n"Option -${BOLD}$OPTARG${NORM} requires an argument." >&2
            echo -e "Use ${BOLD}$SCRIPT -h${NORM} to see the help documentation."\\n
            exit 3
            ;;
    esac
done
shift $((OPTIND-1))  #This tells getopts to move on to the next argument.

#Check for additional command line arguments
if [ $# -ne 0 ]; then
    echo -e \\n"Too many command line arguments."
    echo -e "Use ${BOLD}$SCRIPT -h${NORM} to see the help documentation."\\n
    exit 4
fi

#Check to make sure the input directory exists
if [ ! -d "${wav_path}" ]; then
    echo -e \\n"Invalid input path: ${BOLD}${wav_path}${NORM} does not exist."
    echo -e "Use ${BOLD}$SCRIPT -h${NORM} to see the help documentation."\\n
    exit 5
fi

#Build list of input audio files
file_pattern="${wav_path}*.wav"
file_list=(${file_pattern})
num_files=${#file_list[@]}
if [ "${file_list}" = "${file_pattern}" ]; then
    echo -e \\n"Invalid path: ${BOLD}${wav_path}${NORM} does not contain ${BOLD}*.wav${NORM} files."
    echo -e "Use ${BOLD}$SCRIPT -h${NORM} to see the help documentation."\\n
    exit 6
elif [ ${num_files} -eq 0 ]; then
    echo -e \\n"Invalid path: ${BOLD}${wav_path}${NORM} does not contain ${BOLD}*.wav${NORM} files."
    echo -e "Use ${BOLD}$SCRIPT -h${NORM} to see the help documentation."\\n
    exit 7
fi
index=0

#Create output directory if it does not already exist
if ! mkdir -p $xml_path; then
    echo -e \\n"Output path does not exist and could not be created: ${BOLD}${xml_path}${NORM}."
    echo -e "Use ${BOLD}$SCRIPT -h${NORM} to see the help documentation."\\n
    exit 8
fi

echo "Generating ${BOLD}$num_stations${NORM} stations between ${BOLD}${min_freq%?}.${min_freq: -1}${NORM} MHz and ${BOLD}${max_freq%?}.${max_freq: -1}${NORM} MHz"
echo "Using ${BOLD}${num_files}${NORM} *.wav input files located in ${BOLD}${wav_path}${NORM}"
echo "Output configuration files located in ${BOLD}${xml_path}${NORM}"


#####################################################################

# Need at least 200kz seperation so shrink the range by a factor of 2
# Although we also want these fairly spread out so we'll actually reduce by a factor of 4 instead
# this means that if the odds are against us we'll have at most 5 stations in view but that is unlikely
int_min_freq=$(expr $min_freq / 4)
int_max_freq=$(expr $max_freq / 4)

freq_list=$(shuf -i $int_min_freq-$int_max_freq -n $num_stations)
for freq in $freq_list; do

# Readjust for above, adding one to get on odd stations
freq=$(($freq * 4 + 1))
pty=$[ 1 + $[ RANDOM % 31 ]]

cat << EOF > ${xml_path}Example$freq.xml
<TxProps>
  <FileName>${file_list[$index]}</FileName>
  <CenterFrequency>${freq}00000</CenterFrequency>
  <RDS>
    <CallSign>WSDR</CallSign>
    <ShortText>${freq%?}.${freq: -1} FM</ShortText>
    <StationType>Simulator</StationType>
    <FullText>REDHAWK Radio: ${file_list[$index]##*/}</FullText>
    <PTY>${pty}</PTY>
  </RDS>
</TxProps>
EOF

  let index=index+1

  if [ $index -eq $num_files ]; then
    let index=0
  fi
done

