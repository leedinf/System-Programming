make로 컴파일 후 ./myshell로 쉘 실행

cd {directory} : 해당 디렉토리로 이동

ls : 현재 디렉토리의 파일 목록 출력

mkdir , rmdir : 각각 명령어로 디렉토리 생성 및 삭제

touch, cat, echo {filename}: 파일의 생성, 읽기, 출력 가능

exit, quit : 쉘 종료

pipeline으로 redirection 가능
ex) cat myshell.c | grep "int"
""없어도 실행 가능. '|' 기호 앞뒤 띄어쓰기 무시

문장의 끝에 &있으면 백그라운드 실행 가능
& 뒤에 공백 또는 다른 문자가 있으면 오류 발생

백그라운드 실행과 동시에 프로세스가 종료되면 "CSE4100-SP-P2>" 출력과 섞일 수 있음.


