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

cat > "$TMP_DIR/matmul_0.json" <<EOF
{
	"pipeline": [
	{
		"uri": "anyloop:test_source",
			"params": {
				"type": "matrix",
				"size1": 4,
				"size2": 8,
				"kind": "constant",
				"offset": 0.25
			}
	},
	{
		"uri": "anyloop:file_sink",
		"params": {
			"filename": "${TMP_DIR}/matmul_0.aylp"
		}
	},
	{
		"uri": "anyloop:stop_after_count",
		"params": {
			"count": 1
		}
	}
	]
}
EOF

cat > "$TMP_DIR/matmul_1.json" <<EOF
{
	"pipeline": [
	{
		"uri": "anyloop:test_source",
			"params": {
				"type": "vector",
				"size1": 8,
				"kind": "constant",
				"offset": 4
			}
	},
	{
		"uri": "anyloop:matmul",
		"params": {
			"filename": "${TMP_DIR}/matmul_0.aylp",
			"type": "vector"
		}
	},
	{
		"uri": "anyloop:logger"
	},
	{
		"uri": "anyloop:stop_after_count",
		"params": {
			"count": 1
		}
	}
	]
}
EOF

cat > "$TMP_DIR/matmul_2.json" <<EOF
{
	"pipeline": [
	{
		"uri": "anyloop:test_source",
			"params": {
				"type": "matrix",
				"size1": 8,
				"size2": 8,
				"kind": "constant",
				"offset": 4
			}
	},
	{
		"uri": "anyloop:matmul",
		"params": {
			"filename": "${TMP_DIR}/matmul_0.aylp",
			"type": "matrix"
		}
	},
	{
		"uri": "anyloop:logger"
	},
	{
		"uri": "anyloop:stop_after_count",
		"params": {
			"count": 1
		}
	}
	]
}
EOF

"$BUILD_DIR"/anyloop -pl TRACE "$TMP_DIR/matmul_0.json" 2>/dev/null

res=$( \
	"$BUILD_DIR"/anyloop -pl TRACE "$TMP_DIR/matmul_1.json" 2>&1 \
	| grep -F "[2, 2, 2, 2]" | wc -l \
)

if [ $res != "1" ]; then
	echo "matmul FAIL"
	exit 1
fi

res=$( \
	"$BUILD_DIR"/anyloop -pl TRACE "$TMP_DIR/matmul_2.json" 2>&1 \
	| grep -F "2, 2, 2, 2, 2, 2, 2, 2" | wc -l \
)

if [ $res != "4" ]; then
	echo "matmul FAIL"
	exit 1
fi

echo "matmul PASS"

