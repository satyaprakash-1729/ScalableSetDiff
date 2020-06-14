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

General instructions on how to run the p2p application (see next section below for example commands using provided test files):
1. Use cmake and make to build CLIENT and SERVER.
2. Run SERVER first, and then CLIENT after. See --help (either ./CLIENT --help or ./SERVER --help) for arguments.
3. Both SERVER and CLIENT need a filepath to be passed in as the input data.  Some samples can be found in the test_files directory. Feel free to replace those with other files for testing.
4. They also need the mode argument to be passed in, which determines what mode the programs should be run in. It can be IBF, naive, or both. IBF mode runs only the IBF set difference algorithm, naive runs only the naive algorithm, and both runs both. The CLIENT and SERVER modes need to be the same, e.g. "./SERVER --filepath testfile --mode both" and "./CLIENT --filepath testfile --mode both" works, but "./SERVER --filepath testfile --mode IBF" and "./CLIENT --filepath testfile --mode naive" will not work.
5. The other arguments (alpha, beta, and num_hashes) are parameters to the IBF set difference algorithm and will have default values if not specified. If specificed, the values of these arguments must be the same for CLIENT and SERVER, or the algorithm will not work.
6. When reading the output, CALCULATED SET DIFFERENCE is the difference calculated with the IBF algorithm, and NAIVE SET DIFFERENCE is calculated with the naive method of sending all the data in the set. The SERVER program also outputs whether both methods returned the same result if mode is "both".

Three pairs of test files are provided: p2p_1000_node1_diff3.dat and p2p_1000_node2_diff3.dat, p2p_1000_node1_diff6.dat and p2p_1000_node2_diff6.dat, and p2p_1000_node1_diff10.dat and p2p_1000_node2_diff10.dat. All are generated from a base file of 1000 rows of data. Each pair of files is denoted by node1 and node2, which means one file should be given as input to SERVER and the other as input to CLIENT. Each file also has some number of rows of data missing, as denoted by diff#. Thus, its corresponding file will have that many rows that this file does not have, resulting in a set difference. If each file in a pair is missing X rows, then the total set difference will be 2X (assuming all their missing rows are different).

Example commands to run p2p_application on provided test files:
1. building CLIENT/SERVER
    i. cd ScalableSetDiff
    ii. mkdir build
    iii. cd build
    iv. cmake ..
    v. make SERVER && make CLIENT (or just "make" to make everything)

2. running the CLIENT/SERVER with both IBF and naive algorithms (CLIENT and SERVER needs to be run from separate terminals)
    a. total set difference of 6 with alpha=3.35
        i. ./SERVER --alpha 3.35 --filepath ../test_files/p2p_1000_node2_diff3.dat --mode both
        ii. ./CLIENT --alpha 3.35 --filepath ../test_files/p2p_1000_node1_diff3.dat --mode both

    b. total set difference of 12 with alpha=5
        i. ./SERVER --alpha 5 --filepath ../test_files/p2p_1000_node2_diff6.dat --mode both
        ii. ./CLIENT --alpha 5 --filepath ../test_files/p2p_1000_node1_diff6.dat --mode both

    c. total set difference of 20 with alpha=6
        i. ./SERVER --alpha 6 --filepath ../test_files/p2p_1000_node2_diff10.dat --mode both
        ii. ./CLIENT --alpha 6 --filepath ../test_files/p2p_1000_node1_diff10.dat --mode both

3. see the last line of the output, i.e. "both methods are equal:". If true, then the IBF algorithm is the same as the naive algorithm, so it gave the correct difference. If false, then it gave the incorrect difference.
