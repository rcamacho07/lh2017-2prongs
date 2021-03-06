#!/bin/bash
#
# get the cut on a variable that would correspond to a requested signal efficiency
#
# Usage:
#  get_cut.sh "ntuple_file" "expression" requested_efficiency "optional_condition_list"

function usage {
    echo "Usage:"
    echo "  get_cut.sh \"ntuple_file\" \"requested_efficiency \"optional_condition_list\""
    exit
}

#----------------------------------------------------------------------
# parse the command line
if [[ $# -eq 0 ]]; then usage; fi; ntuple_file="$1"; shift
if [[ $# -eq 0 ]]; then usage; fi; expression="$1" ; shift
if [[ $# -eq 0 ]]; then usage; fi; eff="$1"        ; shift

#----------------------------------------------------------------------
# decide which too to use according to file compression
mygrep="grep"
mycat="cat"
if [[ "$ntuple_file" == *.gz ]]; then
    mygrep="zgrep"
    mycat="zcat"
fi

#----------------------------------------------------------------------
script_full_path=$(dirname "$0")

cdtstring=''
while [[ $# -gt 0 ]]; do
    cdtstring="${cdtstring} \"$1\""
    shift
done

Nentries=`${myzcat} ${ntuple_file} | tail -n1 | sed 's/.*=//'`
Nexpected=`echo "$Nentries*$eff" | bc -l`
Nexpected=${Nexpected%%.*}
${script_full_path}/extract.sh "${ntuple_file}" "${expression}" ${cdtstring} | sort -g | grep -v "^#" | sed "${Nexpected}q;d"


