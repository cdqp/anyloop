#!/bin/sh
set -e

if [ -z "$TMP_DIR" ]; then
	echo "TMP_DIR not set"
	exit 1
fi
if [ -z "$BUILD_DIR" ]; then
	echo "BUILD_DIR not set"
	exit 1
fi

cat > "$TMP_DIR/com.json" <<EOF
{
	"pipeline": [
	{
		"uri": "anyloop:test_source",
			"params": {
				"type": "matrix_uchar",
				"kind": "sine",
				"size1": 256,
				"size2": 256
			}
	},
	{
		"uri": "anyloop:center_of_mass",
		"params": {
			"region_height": 128,
			"region_width": 128,
			"thread_count": 1
		}
	},
	{
		"uri": "anyloop:logger",
	},
	{
		"uri": "anyloop:stop_after_count",
		"params": {
			"count": 8
		}
	}
	]
}
EOF

res=$( \
	"$BUILD_DIR"/anyloop -pl TRACE "$TMP_DIR/com.json" 2>&1 \
	| grep -F "[0, 0, 0, 0, 0, 0, 0, 0]" | wc -l \
)

if [ $res != "8" ]; then
	echo "com FAIL"
	exit 1
fi

echo "com PASS"
