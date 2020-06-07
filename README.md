# CS216: Set Difference using Inverse Bloom Filter

see main.cpp for some data sets for testing

To run the p2p application:
1. use cmake and make to build CLIENT and SERVER. I used ALPHA=20 BETA=10 and NUM_HAHES=3 for cmake.
2. run ./SERVER path/to/file_name first, and then ./CLIENT path/to/filename
3. both SERVER and CLIENT are need a data file to be passed in, and some samples can be found in the test_files directory. Feel free to replace those with other files for testing.
4. When reading the output, CALCULATED SET DIFFERENCE is the difference calculated with the algorithm, and NAIVE SET DIFFERENCE is calculated with the naive method of sending all the data in the set. The SERVER program also outputs whether boths methods returned the same result.
