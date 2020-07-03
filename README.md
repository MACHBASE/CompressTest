[이진형태의 데이터 압축률 테스트]
----------------------------------
```
1. BinaryCompress_Test1 은 Binary 타입에 대한 압축률 테스트로써, 샘플데이터를 읽어 동일한 데이터를 입력하는 테스트이다.
    1) 테스트케이스 이동
        cd BinaryCompress_Test1  
    2) 테이블 생성(테스트마다 재생성)
        machsql -u sys -p manager -s 127.0.0.1 -P 5656 -f ../1.create_table_binary.sql
    3) 테스트 데이터 입력
        text_binary.txt 파일 확인
        ./bincompress1 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay
    4) DB 파일 사이즈 측정
        du -bs [DB 파일 위치] 

2. BinaryCompress_Test2 은 Binary 타입에 대한 압축률 테스트로써, 랜덤데이터를 생성하여 데이터를 입력하는 테스트이다.
    1) 테스트케이스 이동
        cd BinaryCompress_Test2
    2) 테이블 생성(테스트마다 재생성)
        machsql -u sys -p manager -s 127.0.0.1 -P 5656 -f ../1.create_table_binary.sql
    3) 테스트 데이터 입력
        ./bincompress2 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay --raw-size=8204
        또는
        ./bincompress2 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay --raw-size=65632
        또는
        ./bincompress2 -s 20200701 -a 1 -n 50 -P 5656 --disable-delay --raw-size=400000
    4) DB 파일 사이즈 측정
        du -bs [DB 파일 위치] 

3. TextCompress_Test1 은 Text 타입에 대한 압축률 테스트로써, 샘플데이터를 읽어 동일한 데이터를 입력하는 테스트이다.
    1) 테스트케이스 이동
        cd TextCompress_Test1
    2) 테이블 생성(테스트마다 재생성)
        machsql -u sys -p manager -s 127.0.0.1 -P 5656 -f ../2.create_table_text.sql
    3) 테스트 데이터 입력
        text_binary.txt 파일 확인
        ./txtcompress1 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay
    4) DB 파일 사이즈 측정
        du -bs [DB 파일 위치] 

4. TextCompress_Test2 은 Text 타입에 대한 압축률 테스트로써, 랜덤데이터를 생성하여 데이터를 입력하는 테스트이다.
    1) 테스트케이스 이동
        cd TextCompress_Test2
    2) 테이블 생성(테스트마다 재생성)
        machsql -u sys -p manager -s 127.0.0.1 -P 5656 -f ../2.create_table_text.sql
    3) 테스트 데이터 입력
        ./txtcompress2 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay --raw-size=8204
        또는
        ./txtcompress2 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay --raw-size=65632
    4) DB 파일 사이즈 측정
        du -bs [DB 파일 위치] 

* 모든 테스트는 1회 입력시 기본 8만건씩 입력된다. (-e 옵션으로 조절 가능)
* 압축률 계산은 100 - ( DB 파일 사이즈 * 100 / 계산상 사이즈) 이다.
* BinaryCompress_Test1을 예로 설명하면 다음과 같다.
  - 계산상 입력 크기  : 8204 byte * (8만 * 10회) = 6,563,200,000 byte (약 6.5GB)
  - 실제 입력 DB 크기 :                          = 1,904,083,753 byte (약 1.8GB)
  - 압축률            : 100 - (1,904,083,753 * 100 / 6,563,200,000) = 70.9 %
```
