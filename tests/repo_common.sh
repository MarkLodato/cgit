# Common tests for repo backends.

repo_run_all()
{
run_test 'generate index page' 'cgit_url "" >trash/tmp'
run_test 'no (null) values' '! grep -e "(null)" trash/tmp'
run_test 'find foo repo' 'grep -e ">foo<" trash/tmp'
run_test 'find foo description' 'grep -e "\[no description\]" trash/tmp'
run_test 'find bar repo' 'grep -e ">bar repo<" trash/tmp'
run_test 'find bar description' 'grep -e "the bar repo" trash/tmp'
run_test 'find bar owner' 'grep -e "barowner" trash/tmp'
run_test 'find foo+bar repo' 'grep -e ">foo+bar<" trash/tmp'
run_test 'verify foo+bar link' 'grep -e "/foo+bar/" trash/tmp'
run_test 'find sub-project repo' 'grep -e ">sub-project<" trash/tmp'
run_test 'find another project repo' 'grep -e ">another project<" trash/tmp'

run_test 'generate foo summary' 'cgit_url "foo" >trash/tmp'
run_test 'no (null) values' '! grep -e "(null)" trash/tmp'
run_test 'find commit 5' 'grep -e "commit 5" trash/tmp'

run_test 'generate foo+bar summary' 'cgit_url "foo%2bbar" >trash/tmp'
run_test 'no (null) values' '! grep -e "(null)" trash/tmp'
run_test 'find commit 10' 'grep -e "commit 10" trash/tmp'

run_test 'generate bar/tree' 'cgit_url "bar/tree" >trash/tmp'
run_test 'no (null) values' '! grep -e "(null)" trash/tmp'
run_test 'find file-1' 'grep -e "file-1" trash/tmp'

run_test 'generate bar/tree/file-50' 'cgit_url "bar/tree/file-50" >trash/tmp'
run_test 'no (null) values' '! grep -e "(null)" trash/tmp'
run_test 'find line 1' '
	grep -e "<a class=.no. id=.n1. name=.n1. href=.#n1.>1</a>" trash/tmp
'

run_test 'generate repolist "f"' 'cgit_url "f" >trash/tmp'
run_test 'no (null) values' '! grep -e "(null)" trash/tmp'
run_test 'find foo repo' 'grep -e ">foo<" trash/tmp'
run_test 'find foo+bar repo' 'grep -e ">foo+bar<" trash/tmp'
run_test 'no bar repo' '! grep -e ">bar repo<" trash/tmp'
run_test 'no another repo' '! grep -e "another" trash/tmp'

run_test 'no commit 6 in sub/project/log' '
	! cgit_url sub/project/log | grep -e "commit 6"
'

run_test 'find commit 6 in sub/another/log' '
	cgit_url sub/another/log | grep -e "commit 6"
'
}
