import subprocess
import timeit
import shlex

NREPEATS = 10


cmd = "g++ Run.cpp -o Run.o -std=c++11"
args = shlex.split(cmd)
print args
subprocess.call(args)


def Run(nthreads):
         cmd="./Run.o -n "+str(nthreads)+"-s 10"

         args = shlex.split(cmd)

         t = 0.0
    
         for i in range(0, NREPEATS):
        
            st = subprocess.check_output(args)
            A =  st.split(' ')
            
            t  += float(A[0]) / NREPEATS
         
         return t




for i in range(1,10):
        t = Run(i)
        
        print "nthreads="+str(i)+" throughput="+str(t)
        
        

    











