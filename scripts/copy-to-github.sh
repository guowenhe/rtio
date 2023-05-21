
WS_ROOT=$(dirname $(dirname `realpath $0`))
echo WORK_ROOT: $WS_ROOT

# EXCLUDE_FILES={$WS_ROOT/docs/misc.md}

# echo EXCLUDE_FILES: $EXCLUDE_FILES

rsync -av \
 --exclude=docs/misc.md \
 --exclude=docs/rtio-story-cmp-mqtt.md \
 --exclude=docs/top-interactive.md \
 --exclude=docs/u2m.md \
 --exclude=docs/useraccessproto.md \
 --exclude=docs/device_sdkapis.md \
 --exclude=scripts/add_license.sh \
 --exclude=scripts/Apache2.0_FileHeader.txt\
 --exclude=scripts/add_license.sh \
 --exclude=scripts/add_license.sh \
 --exclude=scripts/add_license.sh \
 --exclude=.git\
 --exclude=out\
 --exclude=build*\
 $WS_ROOT/ $WS_ROOT/../rtio/
