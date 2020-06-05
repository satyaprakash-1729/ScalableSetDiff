# CS216: Set Difference using Inverse Bloom Filter

see main.cpp for some data sets for testing

To run the p2p application:
1. use cmake and make to build CLIENT and SERVER. I used ALPHA=20 BETA=10 and NUM_HAHES=3 for cmake.
2. run SERVER first, and then CLIENT
3. CLIENT reads file name "host1.data" and SERVER reads file named "host2.data", both in the test_files directory. Feel free to replace those with other files for testing, but the names need to be the same.
4. When reading the output, CALCULATED SET DIFFERENCE is difference calculated with the algorithm, and ACTUAL SET DIFFERENCE is calculated with the naive method of sending all the data in your set.
