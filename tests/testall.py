import os
import sys
import subprocess
import argparse
rootdir = "./"

#directly test under csi
def test_from_csi():
    tb_case = 1
    for testfiles in os.listdir(rootdir):
        if testfiles == os.path.basename(__file__):
            continue

        if ".ans" == os.path.splitext(testfiles)[1]:
            continue

        input_prog = subprocess.check_output(['cat', testfiles])
        output1 = subprocess.check_output(['csi', '-q'], input = input_prog).splitlines()
        output2 = subprocess.check_output(['../run', '-q'], input = input_prog).splitlines()

        output1 = [line for line in output1 if line.strip()]
        output2 = [line for line in output2 if line.strip()]
            
        print("Case:", tb_case, "TestFile:", testfiles)
        if(output1 == output2):
            print("PASSED")
        else:
            print("FAILED")

        tb_case += 1


#Generate .ans files
def gen_ans():
    subprocess.run("rm *.ans", shell=True)
    for testfiles in os.listdir(rootdir):
        if testfiles == os.path.basename(__file__):
            continue
        if ".ans" == os.path.splitext(testfiles)[1]:
            continue
    
        input_prog = subprocess.check_output(['cat', testfiles])
        output1 = subprocess.check_output(['csi', '-q'], input = input_prog).splitlines()

        output1 = [line for line in output1 if line.strip()]

        ans_file = open(testfiles+'.ans', "a")
        for ans_line in output1:
            ans_file.write(ans_line.decode() + '\n')

        ans_file.close()
        print("Ans file", os.path.basename(ans_file.name), "Generated")


#test under .ans files
def test_from_ans():
    tb_case = 1
    for testfiles in os.listdir(rootdir):
        if testfiles == os.path.basename(__file__):
            continue

        if ".ans" == os.path.splitext(testfiles)[1]:
            continue

        with open(testfiles + '.ans', 'r') as ans_file:
            ans_list = [line.strip() for line in ans_file]

        input_prog = subprocess.check_output(['cat', testfiles])
        output1 = subprocess.check_output(['../run', '-q'], input = input_prog).splitlines()

        output1 = [line.decode() for line in output1 if line.strip()]
        
        print("Case:", tb_case, "TestFile:", testfiles)
        if(output1 == ans_list):
            print("PASSED")
        else:
            print("FAILED")
            print(output1)
            print(ans_list)

        tb_case += 1


parser = argparse.ArgumentParser(description="Test script for mikal-eval")
parser.add_argument('--gen-ans', help='Generate answer files', action='store_const', const=1)
parser.add_argument('--csi-test', help='Test directly with CSI evaluator', action='store_const', const=1)
parser.add_argument('--ans-test', help='Test with pre-generated test files', action='store_const', const=1)

args = parser.parse_args()

if args.gen_ans:
    gen_ans()

if args.csi_test:
    test_from_csi()
elif args.ans_test:
    test_from_ans()

