#!/bin/sh

. ./setup.sh
. ./repo_common.sh

prepare_tests "scan-tree backend"

create_cgitrc()
{
	touch $path/cgitrc
	test "${name-xxx}" != xxx && echo "name=$name" >> $path/cgitrc
	test "${desc-xxx}" != xxx && echo "desc=$desc" >> $path/cgitrc
	test "${owner-xxx}" != xxx && echo "owner=$owner" >> $path/cgitrc
}

for_each_repo create_cgitrc

sed -e '/^repo\./d' trash/cgitrc > trash/cgitrc.new
mv trash/cgitrc.new trash/cgitrc
cat <<EOF >>trash/cgitrc
#project-list=$PWD/trash/projects
scan-path=$PWD/trash/repos
scan-path=$PWD/trash/repos2
EOF

cat <<EOF >>trash/projects
foo
b
sub
EOF

repo_run_all

# Turn on the project list.
rm -rf trash/cache
mkdir trash/cache
sed -e 's/^#\(project-list=\)/\1/' trash/cgitrc > trash/cgitrc.new
mv trash/cgitrc.new trash/cgitrc

run_test 'generate index page with project list' 'cgit_url "" >trash/tmp'
run_test 'find foo repo' 'grep -e ">foo<" trash/tmp'
run_test 'find sub-project repo' 'grep -e ">sub-project<" trash/tmp'
run_test 'find another project repo' 'grep -e ">another project<" trash/tmp'
run_test 'find no bar or foo+bar repos' '! grep -e "bar" trash/tmp'


tests_done
