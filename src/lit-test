#!/bin/bash

set -euo pipefail

readonly TEST_DIR=$(mktemp --tmpdir --directory lit-test.XXXXXX)

echo "== Using $TEST_DIR"
pushd "$TEST_DIR"

echo "== Initializing repository"
lit init

echo "== Creating the first commit"
cat >file1 <<-EOF
	This is the first line of the first file. 🚀
	This is the second line of the first file.
EOF

# Let's check the status. Should look something like this:
# Changes:
#   Add file1
lit status

# We expect the first commit to be identified as r0.
lit commit "Add file1"

echo "== Creating more commits"
echo >>file1 "A third line is added to the first file."

# Let's check the status again, just to be sure.
lit status

# This one would be r1.
lit commit "Extend file1"

echo >>file1 "A forth line is added."

# This should be r2.
lit commit "Extend file1 even further"

echo "== Displaying graph"

# o ← r2 Extend file1 even further
# o   r1 Extend file1
# o   r0 Add file1
lit log

echo "== Inspecting r0"
lit show r0

echo "== Switching to r0"
lit checkout r0

# Checking the file content.
diff -s file1 - <<-EOF
	This is the first line of the first file. 🚀
	This is the second line of the first file.
EOF

echo "== Switching back to r2"
lit checkout r2

# Checking the file content again.
diff -s file1 - <<-EOF
	This is the first line of the first file. 🚀
	This is the second line of the first file.
	A third line is added to the first file.
	A forth line is added.
EOF

echo "== Adding and discarding changes"
echo >>file1 "This fifth line should be gone in an instant."
lit checkout

# Let's confirm.
diff -s file1 - <<-EOF
	This is the first line of the first file. 🚀
	This is the second line of the first file.
	A third line is added to the first file.
	A forth line is added.
EOF

echo "== Creating another branch"
lit checkout r0
mkdir subfolder
cat >subfolder/file2 <<-EOF
	This is the first line of the second file.
	And another line in the second file.
EOF

# This should be r3.
lit commit "Add file2"

# o   ← r3 Add file2
# │ o   r2 Extend file1 even further
# │ o   r1 Extend file1
# o─┘   r0 Add file1
lit log

echo "== Going back"
lit checkout r2

# file2 should be gone.
test ! -f subfolder/file2

echo "== Merging (no conflict)"

# This creates a merge commit r4.
lit merge r3

# file2 should now be present.
diff -s subfolder/file2 <<-EOF
	This is the first line of the second file.
	And another line in the second file.
EOF

# o─┐ ← r4 Merge r3 into r2
# │ o   r3 Add file2
# o │   r2 Extend file1 even further
# o │   r1 Extend file1
# o─┘   r0 Add file1
lit log

echo "== Setting up a conflict"
echo >>file1 "Fifth line on top of r4."
lit commit "Extend file1 one way" # r5

lit checkout r3
echo >>file1 "Third line on top of r3."
lit commit "Extend file1 another way" # r6

#   o ← r6 Extend file1 another way
# o │   r5 Extend file1 one way
# o─┤   r4 Merge r3 into r2
# │ o   r3 Add file2
# o │   r2 Extend file1 even further
# o │   r1 Extend file1
# o─┘   r0 Add file1
lit log

# Going back and merging.
lit checkout r5
lit merge r6 || true

# Let's check all file versions:
#   Current commit
diff -s file1 - <<-EOF
	This is the first line of the first file. 🚀
	This is the second line of the first file.
	A third line is added to the first file.
	A forth line is added.
	Fifth line on top of r4.
EOF

#   other breanch
diff -s file1.r6 - <<-EOF
	This is the first line of the first file. 🚀
	This is the second line of the first file.
	Third line on top of r3.
EOF

#  common base
diff -s file1.r3 - <<-EOF
	This is the first line of the first file. 🚀
	This is the second line of the first file.
EOF

# Let's simulate some conflict resolution.
echo >>file1 "Sixth line added during merge conflict."

# Before continuing we need to cleanup the leftover files.
rm file1.r6 file1.r3

lit commit "Merge r6 into r5" # r7

# Let's verify the final result.

diff -s file1 - <<-EOF
	This is the first line of the first file. 🚀
	This is the second line of the first file.
	A third line is added to the first file.
	A forth line is added.
	Fifth line on top of r4.
	Sixth line added during merge conflict.
EOF

# o─┐ ← r7 Merge r6 into r5
# │ o   r6 Extend file1 another way
# o │   r5 Extend file1 one way
# o─┤   r4 Merge r3 into r2
# │ o   r3 Add file2
# o │   r2 Extend file1 even further
# o │   r1 Extend file1
# o─┘   r0 Add file1
lit log

echo "== Cleanup"
popd
rm -r "$TEST_DIR"
