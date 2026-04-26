#!/usr/bin/env bash
set -o nounset
set -o pipefail
set -o errexit
set -o errtrace
trap 'echo error:$? line:$LINENO cmd:$BASH_COMMAND' ERR


self_dir=$(dirname "$0")
source ${self_dir}/common.bash $@

json_tool="$1"

# test valid json files
for json_file in $(find ${root_dir}/test-data/json/valid -name '*.json' -type f); do
  if ! $json_tool check $json_file; then
    printf "'%s' failed to parse valid json file '%s'\n" $json_tool $json_file
    exit 1
  fi
done

# test invalid json files
for json_file in $(find ${root_dir}/test-data/json/invalid -name '*.json' -type f); do
  if $json_tool check $json_file; then
    printf "'%s' failed to reject invalid json file '%s'\n" $json_tool $json_file
    exit 1
  fi
done

# count-nodes returns > 0 for valid files
for json_file in $(find ${root_dir}/test-data/json/valid -name '*.json' -type f); do
  count=$($json_tool count-nodes $json_file)
  if [[ "$count" -le 0 ]]; then
    printf "'%s' count-nodes returned %s for valid file '%s'\n" $json_tool "$count" $json_file
    exit 1
  fi
done

# count-nodes returns 0 for invalid files
for json_file in $(find ${root_dir}/test-data/json/invalid -name '*.json' -type f); do
  count=$($json_tool count-nodes $json_file 2>/dev/null || true)
  if [[ "$count" != "0" && -n "$count" ]]; then
    printf "'%s' count-nodes returned %s for invalid file '%s'\n" $json_tool "$count" $json_file
    exit 1
  fi
done

if [ -f ${root_dir}/compile_commands.json ]; then
  json_file=${root_dir}/compile_commands.json
  if ! $json_tool check $json_file; then
    printf "'%s' failed to parse valid json file '%s'\n" $json_tool $json_file
    exit 1
  fi
fi

assert_get() {
  local json_path="$1"
  local expected="$2"
  local file="$3"
  local actual
  actual=$($json_tool get "$json_path" "$file")
  if [[ "$actual" != "$expected" ]]; then
    printf "'%s' get '%s': expected '%s', got '%s'\n" $json_tool "$json_path" "$expected" "$actual"
    exit 1
  fi
}

assert_get_fails() {
  local json_path="$1"
  local file="$2"
  if $json_tool get "$json_path" "$file" 2>/dev/null; then
    printf "'%s' get '%s' should have failed but succeeded\n" $json_tool "$json_path"
    exit 1
  fi
}

# get from simple object
simple_object=${root_dir}/test-data/json/get/simple_object.json

assert_get ".level"        '"info"'  "$simple_object"
assert_get ".line"         '42'      "$simple_object"
assert_get ".nested.x"     '1'       "$simple_object"
assert_get ".nested.y.z"   'true'    "$simple_object"
assert_get ".tags[0]"      '"a"'     "$simple_object"
assert_get ".tags[1]"      '"b"'     "$simple_object"
assert_get ".tags[2]"      '"c"'     "$simple_object"
assert_get ".items[0].id"  '0'       "$simple_object"
assert_get ".items[0].val" '"zero"'  "$simple_object"
assert_get ".items[1].val" '"one"'   "$simple_object"

assert_get_fails ".missing"      "$simple_object"
assert_get_fails ".tags[9]"      "$simple_object"
assert_get_fails ".level.nested" "$simple_object"

# get from simple array
simple_array=${root_dir}/test-data/json/get/simple_array.json

assert_get "[0]"       '1'                "$simple_array"
assert_get "[1]"       '"hello"'          "$simple_array"
assert_get "[2]"       'null'             "$simple_array"
assert_get "[3]"       'true'             "$simple_array"
assert_get "[4]"       'false'            "$simple_array"
assert_get "[5]"       '-3'               "$simple_array"
assert_get "[6]"       '1.5'              "$simple_array"
assert_get "[7]"       '{"key": "val"}'   "$simple_array"
assert_get "[7].key"   '"val"'            "$simple_array"
assert_get "[8]"       '[10, 20]'         "$simple_array"
assert_get "[8][0]"    '10'               "$simple_array"
assert_get "[8][1]"    '20'               "$simple_array"

assert_get_fails ".key"              "$simple_array"
assert_get_fails "[100]"             "$simple_array"
assert_get_fails "[10000000000000]"  "$simple_array"
assert_get_fails "[01]"              "$simple_array"
assert_get_fails "[-1]"              "$simple_array"
assert_get_fails "[key]"             "$simple_array"


# get various different types
types=${root_dir}/test-data/json/get/types.json

assert_get ".null_val"        'null'           "$types"
assert_get ".true_val"        'true'           "$types"
assert_get ".false_val"       'false'          "$types"
assert_get ".neg"             '-42'            "$types"
assert_get ".float"           '1.5'            "$types"
assert_get ".exp"             '2e3'            "$types"
assert_get ".escaped"         '"line1\nline2"' "$types"
assert_get ".unicode"         '"\u0041BC"'     "$types"
assert_get ".nested.x"        '0'              "$types"
assert_get ".nested.arr[0]"   '10'             "$types"
assert_get ".nested.arr[1]"   '20'             "$types"

assert_get ".empty_obj"  '{}'                       "$types"
assert_get ".empty_arr"  '[]'                       "$types"
assert_get ".nested"     '{"x": 0, "arr": [10, 20]}' "$types"

assert_get_fails ".null_val.x"   "$types"
assert_get_fails ".empty_obj.x"  "$types"
assert_get_fails ".empty_arr[0]" "$types"

# get from partially valid json with duplicate keys
duplicates=${root_dir}/test-data/json/get/duplicates_partial.json

# valid up to path
assert_get ".a.b.c"  '1'         "$duplicates"
assert_get ".a.c.b"  '2'         "$duplicates"
assert_get ".a.b.d"  '4'         "$duplicates"
assert_get ".a.b"    '{"c": 1}'  "$duplicates"
