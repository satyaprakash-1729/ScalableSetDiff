# CS216: Set Difference using Inverse Bloom Filter

Prerequisites:
- cmake version >=3.15
- GNU Make 4.1
- C++-14

see main.cpp for some data sets for testing

to run the basic set difference run:
1. from build folder --> cmake .. --> make IBFSetDiff
2. ./IBFSetDiff --alpha 4 --beta 80 --num_hashes 3 --file1path path1 --file2path path2
3. Or for a random list ./IBFSetDiff --alpha 4 --beta 80 --num_hashes 3 --set1size 10 --set2size 10

Test Files:

1. Test files are 3 pairs of files with 1000 random numbers which are common in the pair and 6, 12 and 20 numbers which are distinct in the files dataset1_1000_6.dat -- dataset2_1000_6.dat, dataset1_1000_12.dat -- dataset2_1000_12.dat and dataset1_1000_20.dat -- dataset2_1000_20.dat respectively.
2. Commands to run:<br/>
	i. cd build<br/>
	ii. cmake ..<br/>
	iii. make IBFSetDiff<br/>
	iv. ./IBFSetDiff --file1path ../test_files/dataset1_1000_6.dat --file2path ../test_files/dataset2_1000_6.dat --alpha 3.35<br/>
	v. ./IBFSetDiff --file1path ../test_files/dataset1_1000_12.dat --file2path ../test_files/dataset2_1000_12.dat --alpha 5<br/>
	vi. ./IBFSetDiff --file1path ../test_files/dataset1_1000_20.dat --file2path ../test_files/dataset2_1000_20.dat --alpha 6<br/>

General instructions on how to run the p2p application (see section below for example commands using provided test files):
1. use cmake and make to build CLIENT and SERVER.
2. run "./SERVER path/to/file_name mode" first, and then "./CLIENT path/to/filename mode" after.
3. both SERVER and CLIENT are need a data file to be passed in, and some samples can be found in the test_files directory. Feel free to replace those with other files for testing.
4. mode determines which mode the programs should be run in. It can be IBF, naive, or both. IBF mode runs only the IBF set difference algorithm, naive runs only the naive algorithm, and both runs both. The CLIENT and SERVER modes need to be the same, e.g. "./SERVER testfile both" and "./CLIENT testfile both" works, but "./SERVER testfile IBF" and "./CLIENT testfile naive" will not work.
5. When reading the output, CALCULATED SET DIFFERENCE is the difference calculated with the algorithm, and NAIVE SET DIFFERENCE is calculated with the naive method of sending all the data in the set. The SERVER program also outputs whether boths methods returned the same result if mode is "both".

Three pairs of test files are provided: p2p_1000_node1_diff3.dat and p2p_1000_node2_diff3.dat, p2p_1000_node1_diff6.dat and p2p_1000_node2_diff6.dat, and p2p_1000_node1_diff10.dat and p2p_1000_node2_diff10.dat. All are generated from a base file of 1000 rows of data. Each pair of files is denoted by node1 and node2, which means one file should be given as input to SERVER and the other as input to CLIENT. Each file also has some number of rows of data missing, as denoted by diff#. Thus, its corresponding file will have that many rows that this file does not have, resulting in a set difference. If each file in a pair is missing X rows, then the total set difference will be 2X (assuming all their missing rows are different).

Example commands to run p2p_application on provided test files:
1. building CLIENT/SERVER
    i. cd ScalableSetDiff
    ii. mkdir build
    iii. cd build
    iv. cmake ..
    v. make SERVER && make CLIENT (or just "make" to make everything)

2. running the CLIENT/SERVER with both algorithms
    a. total set difference of 6
        i. ./SERVER ../test_files/p2p_1000_node2_diff3.dat both
        ii. ./CLIENT ../test_files/p2p_1000_node1_diff3.dat both
    b. total set difference of 12
        i. ./SERVER ../test_files/p2p_1000_node2_diff6.dat both
        ii. ./CLIENT ../test_files/p2p_1000_node1_diff6.dat both
    c. total set difference of 20
        i. ./SERVER ../test_files/p2p_1000_node2_diff10.dat both
        ii. ./CLIENT ../test_files/p2p_1000_node1_diff10.dat both

3. see the last line of the output of SERVER, i.e. "both methods are equal:". If true, then the IBF algorithm is the same as the naive algorithm, so gave the correct difference. If false, then it gave the incorrect difference.
