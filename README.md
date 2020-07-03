How to compile.
---------------

make  
or  
make clean  
make  


Binary format data compression test
----------------------------------



```
1. BinaryCompress_Test1 is a compression rate test for binary format. 
   A test that reads sample data and enters the same data.
    1) Go to ther test directory
        cd BinaryCompress_Test1  
    2) Create table (Regeneration is required for each test run)
        machsql -u sys -p manager -s 127.0.0.1 -P 5656 -f ../1.create_table_binary.sql
    3) Run test
        Check text_binary.txt files.
        ./bincompress1 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay
    4) Database file size measurement
        du -bs [DB file location] 

2. BinaryCompress_Test2 is a compression rate test for the binary type. 
   It is a test that generates random data and inputs the data.
    1) Go to ther test directory
        cd BinaryCompress_Test2
    2) Create table (Regeneration is required for each test run)
        machsql -u sys -p manager -s 127.0.0.1 -P 5656 -f ../1.create_table_binary.sql
    3) Run test
        ./bincompress2 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay --raw-size=8204
        or
        ./bincompress2 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay --raw-size=65632
        or
        ./bincompress2 -s 20200701 -a 1 -n 50 -P 5656 --disable-delay --raw-size=400000
    4) Database file size measurement
        du -bs [DB file location] 

3. TextCompress_Test1 is a compression rate test for text format. 
   A test that reads sample data and enters the same data.
    1) Go to ther test directory
        cd TextCompress_Test1
    2) Create table (Regeneration is required for each test run)
        machsql -u sys -p manager -s 127.0.0.1 -P 5656 -f ../2.create_table_text.sql
    3) Run test
        Check text_binary.txt files.
        ./txtcompress1 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay
    4) Database file size measurement
        du -bs [DB file location] 

4. TextCompress_Test2 is a compression rate test for the text type. 
   It is a test that generates random data and inputs the data.
    1) Go to ther test directory
        cd TextCompress_Test2
    2) Create table (Regeneration is required for each test run)
        machsql -u sys -p manager -s 127.0.0.1 -P 5656 -f ../2.create_table_text.sql
    3) Run test
        ./txtcompress2 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay --raw-size=8204
        or
        ./txtcompress2 -s 20200701 -a 1 -n 10 -P 5656 --disable-delay --raw-size=65632
    4) Database file size measurement
        du -bs [DB file location] 

* All tests are set with 80,000 inputs at a time. (-e Adjustable as option)
* The compression rate calculation is as follows.
    100 - ( DB file size * 100 / Data size to input)
* BinaryCompress_Test1 is described as an example below.
  - Data size to input  : 8,204 Byte * (80,000 * 10 times) = 6,563,200,000 Byte (About 6.5GB)
  - DB file size        :                                  = 1,904,083,753 Byte (About 1.8GB)
  - Compression ratio   : 100 - (1,904,083,753 * 100 / 6,563,200,000) = 70.9 %
```
