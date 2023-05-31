#!/bin/bash

tbfdw_root=$1

if [[ ! -d ${tbfdw_root} ]] || [[ "${tbfdw_root}" == "" ]] ; then
	echo "Provided path was invalid for the TBFDW_HOME"
	echo "(Current directory will be used for TBFDW_HOME)"
	tbfdw_root=$(pwd)
fi

export TBFDW_HOME="${tbfdw_root}"
export LD_LIBRARY_PATH="${TBFDW_HOME}"/lib:"${LD_LIBRARY_PATH}"
alias tbfdw_test="python ${TBFDW_HOME}/tests/tbfdw_test.py"