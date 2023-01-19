#!/bin/bash
MPI_EXEC=$1
HERMES_ROOT=$2
HERMES_BUILD=$3
HERMES_CONF=$4
N_STAGE=$5
STAGE_OFFSET_BLOCKS=$6

BLOCK_SIZE=1048576
BLOCK_SIZE_KB=1024
STAGE_OFFSET=$((${STAGE_OFFSET_BLOCKS}*${BLOCK_SIZE}))
ITER=10

echo "START"
echo "${MPI_EXEC}"
echo "${HERMES_ROOT}"
echo "${HERMES_BUILD}"
echo "${HERMES_CONF}"

mkdir -p /tmp/staging
mkdir hermes_dir

if ! command -v ior &> /dev/null
then
    echo "ior could not be found"
    exit 1
fi

# Create files using IOR.
#
# -F option is for file per process.
# -w for write-only
# -k to keep file
# -m multiple file
# -i iteration
#
# https://ior.readthedocs.io/en/latest/userDoc/options.html
echo "CREATE FILE"
${MPI_EXEC} -n ${N_STAGE} ior -F2 -w -k -m -i2 -o /tmp/staging/test
if [ $? != 0 ]; then
  exit ${RET}
fi

# Start daemon.
${MPI_EXEC} -n 1 \
-genv HERMES_CONF "${HERMES_CONF}" \
-genv LD_LIBRARY_PATH "${HERMES_BUILD}:$LD_LIBRARY_PATH" \
"${HERMES_BUILD}/hermes_daemon" &
if [ $? != 0 ]; then
  exit ${RET}
fi

sleep 1

# Stage file.
echo "STAGE IN"
${MPI_EXEC} -n "${N_STAGE}" \
-genv HERMES_CONF "${HERMES_CONF}" \
-genv LD_LIBRARY_PATH "${HERMES_BUILD}:$LD_LIBRARY_PATH" \
-genv LSAN_OPTIONS=suppressions="${HERMES_ROOT}/test/data/asan.supp" \
-genv SET_PATH=0 \
-genv HERMES_CLIENT 1 \
"${HERMES_BUILD}/stage_in" /tmp/staging/ "${STAGE_OFFSET}" 0 kMinimizeIoTime
if [ $? != 0 ]; then
  exit ${RET}
fi

# Read with hermes.
echo "READ STAGE IN"
${MPI_EXEC} -n ${N_STAGE} \
-genv HERMES_CONF "${HERMES_CONF}" \
-genv LD_PRELOAD "${HERMES_BUILD}/libhermes_posix.so" \
-genv HERMES_STOP_DAEMON 0 \
-genv ADAPTER_MODE WORKFLOW \
-genv HERMES_CLIENT 1 \
-genv HERMES_PAGE_SIZE 1048576 \
ior  -F2 -r -m -o /tmp/staging/test 
if [ $? != 0 ]; then
  exit ${RET}
fi


# Finalize hermes daemon.
echo "FINALIZE"
${MPI_EXEC} -n 1 "${HERMES_BUILD}/finalize_hermes"
if [ $? != 0 ]; then
  exit ${RET}
fi

rm -rf /tmp/staging
rm -rf hermes_dir

