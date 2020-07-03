all:
	cd BinaryCompress_Test1;        make clean;
	cd BinaryCompress_Test1;        make;
	cd BinaryCompress_Test2;        make clean;
	cd BinaryCompress_Test2;        make;
	cd TextCompress_Test1;          make clean;
	cd TextCompress_Test1;          make;
	cd TextCompress_Test2;          make clean;
	cd TextCompress_Test2;          make;

install:
	cd BinaryCompress_Test1;        make;
	cd BinaryCompress_Test2;        make;
	cd TextCompress_Test1;          make;
	cd TextCompress_Test2;          make;

clean:
	cd BinaryCompress_Test1;        make clean;
	cd BinaryCompress_Test2;        make clean;
	cd TextCompress_Test1;          make clean;
	cd TextCompress_Test2;          make clean;
