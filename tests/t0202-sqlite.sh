#!/bin/sh

. ./setup.sh
. ./repo_common.sh

prepare_tests "SQLite backend"

if ! cgit_has_feature sqlite3; then
	skip_tests "cgit compiled without SQLite support"
	exit 0
fi

DB=trash/repos.sqlite3
SQLITE=sqlite3
echo 'CREATE TABLE cgit (url TEXT, path TEXT, name TEXT, owner TEXT, desc TEXT);' >trash/tmp

add_row()
{
	url=$(test "${url-xxx}" = xxx && echo NULL || echo "'$url'")
	path=$(test "${path-xxx}" = xxx && echo NULL || echo "'$path'")
	name=$(test "${name-xxx}" = xxx && echo NULL || echo "'$name'")
	desc=$(test "${desc-xxx}" = xxx && echo NULL || echo "'$desc'")
	owner=$(test "${owner-xxx}" = xxx && echo NULL || echo "'$owner'")
	echo "INSERT INTO cgit VALUES ($url, $path, $name, $desc, $owner);" >>trash/tmp
}

for_each_repo add_row

sed -e '/^repo\./d' trash/cgitrc > trash/cgitrc.new
mv trash/cgitrc.new trash/cgitrc
cat <<EOF >>trash/cgitrc
db.driver=sqlite3
db.database=$DB
db.base-path=$PWD/trash/repos
db.query.all=SELECT * FROM cgit
EOF

rm -f "$DB"
run_test 'generate sqlite3 database' "'$SQLITE' '$DB' < trash/tmp"

repo_run_all

tests_done
