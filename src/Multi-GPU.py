
from datetime import datetime
from optparse import Values
import random
from random import SystemRandom, randrange
from datetime import timedelta
from asyncio import Task
from math import ceil
import os
from pickle import TRUE
from re import A, T
from secrets import choice
from numpy.random import randint
from colorama import Fore, Back, Style
import csv
import matplotlib
import matplotlib.pyplot as plt
import numpy as nmpi
import numpy as np
import pandas as pd
from openpyxl import load_workbook
from xlutils.copy import copy
import time

#os.system('cls')

#-------------Global Variables----------------------------
Gama = dict()                     #A dictionary for storing each Taskset
NWCET = list()                    #Normal Execution Time of Task based on Utilization
GPUusing_Tasks = list()           
number_of_tasks = float
number_of_CPUs = float             #Number of CPU cores
number_of_GPUs = float             #Number of GPUs
U_Total_TaskSet = float            #The utilization of each TaskSet 
T_min = float                      #Min Period(ms)
T_max = float                      #Max Period(ms)     
Perc_GPUusing_Tasks = float        #Percentage of GPU-using tasks 
Eta_min = float                    #Number of parallel segments per task (Eta)
Eta_max = float 
Ratio_G_to_NWCET = float           #Ratio of GPU segment len. to normal WCET
Ratio_misc_Gseg_min = float        #Min Ratio of misc. operation in Gseg(Gm)
Ratio_misc_Gseg_max = float        #Max Ratio of misc. operation in Gseg(Gm)
Eps = float                        #GPU server overhead(ms)
Miu = float                        #Conversion ratio of parallel segments (Miu)
not_schedulable = int              #Number of non-schedulable tasksets


#------------Random Value Generator Function--------------
def RandomValueGenerator(minimum, maximum):
    rand1 = random.random()
    rand_Value = (rand1 * (maximum - minimum)) + minimum 
    rand_Value = float("{0:.3f}".format(rand_Value))
    return(rand_Value) 


#--------------ExperimentSetup----------------------------
def ExperimentSetup():            

    SID = datetime.now().timestamp()
    #SID = 1705488438.331433
    random.seed(SID)
    print(Fore.RED) 
    #print('<<<<< SID:', SID, '>>>>>',"\n")
    

    global number_of_CPUs               
    #number_of_CPUs = random.choice([4,8])
    number_of_CPUs = 4
    
    global number_of_tasks
    #number_of_tasks = ceil(RandomValueGenerator(2 * number_of_CPUs, 5 * number_of_CPUs))
    #number_of_tasks = 8

    global number_of_GPUs                
    number_of_GPUs = 4

    global U_task_min                
    U_task_min = 0.05
    global U_task_max                
    U_task_max = 0.2
    

    global T_min             
    T_min = 100
    global T_max                        
    T_max = 500 


    global Perc_GPUusing_Tasks_min                          
    Perc_GPUusing_Tasks_min = 0
    global Perc_GPUusing_Tasks_max                          
    Perc_GPUusing_Tasks_max = 0.3
    global Perc_GPUusing_Tasks
    Perc_GPUusing_Tasks = RandomValueGenerator(Perc_GPUusing_Tasks_min, Perc_GPUusing_Tasks_max)
        


    global Eta_min         
    Eta_min = 1
    global Eta_max                       
    Eta_max = 3
    global Eta
    

    global Ratio_G_to_C_min
    Ratio_G_to_C_min = 0.1
    global Ratio_G_to_C_max
    Ratio_G_to_C_max = 0.3

    global Ratio_misc_Gseg_min           
    Ratio_misc_Gseg_min = 0
    global Ratio_misc_Gseg_max        
    Ratio_misc_Gseg_max = 0

    global Eps         
    Eps = 0

    global Miu                        
    Miu = 0                            



#------------UUniFast Algorithm---------------------------
def UUniFast(U_Total_TaskSet):
    global number_of_tasks
    global U_tasks_in_TaskSet 
    global number_of_CPUs
    U_tasks_in_TaskSet = list()

    U_tasks_in_TaskSet = []

    Temp = 0
    sum_U = U_Total_TaskSet 
    
    for i in range(1, number_of_tasks):
        #print('sum_U', sum_U)
        next_U = sum_U * (RandomValueGenerator(0, 0.9)**(1/(number_of_tasks - i)))
        #print('next_U', next_U)
        U = round(sum_U - next_U, 2)
        U = min(U,1)
        U_tasks_in_TaskSet.append(U)
        sum_U = next_U
    U_tasks_in_TaskSet.append(round(next_U, 2)) 
    
    # print(Fore.YELLOW)        
    #print('U_tasks_in_TaskSet:', U_tasks_in_TaskSet,"\n")
    return(U_tasks_in_TaskSet)

#------------Taskset Generator----------------------------
def TasksetGenerator():

    #print(Fore.CYAN)
    #print('For each task:',"\n", Fore.CYAN+'Number of CPU core,', Fore.MAGENTA+'P(Priority),', Fore.CYAN+ 'T(Period),', Fore.MAGENTA+ 'D(Deadline),', Fore.CYAN+ ' NWCET(Normal worst case execution time of the task),', Fore.MAGENTA+ ' Execution time of a task on CPU(C),', Fore.CYAN+ 'Execution time of a task on GPU(G),', Fore.MAGENTA+ 'Number of CPU using segments,', Fore.CYAN+ 'Number of GPU using segments,', Fore.MAGENTA+ '[Cseg, CPU] or [Gseg, GPU, Execution time of the segment if transferred]',"\n")
    
    global Gama 
    Gama = {}
    global number_of_CPUs               
    global number_of_tasks
    global number_of_GPUs                
    global U_task_min                
    global U_task_max
    global T_min             
    global T_max 
    global Perc_GPUusing_Tasks                       
    global Eta_min         
    global Eta_max                       
    global Ratio_G_to_C_min
    global Ratio_G_to_C_max 
    global Ratio_misc_Gseg_min                  
    global Ratio_misc_Gseg_max
    global Eps         
    global Miu
    global GPUusing_Tasks
    global Taskset_U
    Taskset_U = list()
    for task in range(number_of_tasks):
        Taskset_U.append(0)

    even = list()
    for j in range (0,(2*Eta_max) + 1):
        if j % 2 == 0:
            even.append(j)                                  
    Ta = list()                                           #A list for each Task's data
    Seg = list()                                          #A list for each Segment's data in a Task


    Number_of_GPUusing_Tasks = int(number_of_tasks * Perc_GPUusing_Tasks)
    ra = range(0,number_of_tasks)
    GPUusing_Tasks = random.sample(ra, Number_of_GPUusing_Tasks)

    if U_Total_TaskSet != None:
        U_tasks_in_TaskSet = UUniFast (U_Total_TaskSet)

    for i in range(number_of_tasks):
        if i in (GPUusing_Tasks):                             #Getting Tasks' data for GPU-using Tasks
            Ta.append('-')
            P = int(RandomValueGenerator(0, number_of_tasks)) 
            Ta.append(P)
            T = float("{0:.2f}".format(RandomValueGenerator(T_min, T_max)))
            Ta.append(T)
            D = T
            Ta.append(D) 
            if U_Total_TaskSet == None:     
                NWCET = float("{0:.3f}".format(T * RandomValueGenerator(U_task_min, U_task_max)))
            else:
                NWCET = float("{0:.3f}".format(T * U_tasks_in_TaskSet [i]))
            Ta.append(NWCET) 
            Ratio_G_to_C = (RandomValueGenerator(Ratio_G_to_C_min, Ratio_G_to_C_max))
            G = float("{0:.3f}".format((NWCET * Ratio_G_to_C) /(1 + Ratio_G_to_C)))
            C = float("{0:.3f}".format(NWCET - G))
            Ta.append(C)
            Ta.append(G)
            Eta = ceil(RandomValueGenerator(Eta_min, Eta_max))
            l = Eta * 2 + 1                                   #Number of the segments                                     
            Ta.append(l-Eta)                                  #Number of sequential segments
            Ta.append(Eta)                                    #Number of parallel segments


            tempc = 0
            temph = 0 
            for j in range (l):                               #For each Segment
                if j in even:
                    h = 'CPU'
                else:
                    h = 'GPU'                  

                if h == 'CPU':
                    if j == (l-1):
                        Cseg = float("{0:.3f}".format(C - tempc))
                    else:
                        Cseg = float("{0:.3f}".format(random.uniform(0.1 , 0.9)*(C - tempc)))
                        tempc = tempc + Cseg

                    Seg.append(Cseg)
                    Seg.append(h)                       
                  
                elif h == 'GPU':
                    if j == (l - 2):
                        Gseg = float("{0:.3f}".format(G - temph))
                    else:
                        Gseg = float("{0:.3f}".format(random.uniform(0.1 , 0.9)* (G - temph)))
                        temph = temph + Gseg

                    Seg.append(Gseg)
                    if_CPU = float("{0:.3f}".format(Gseg * Miu)) #Execution Time if this segment executes on CPU
                    Seg.append(h)
                    Seg.append(if_CPU)                                           
            
        
                Ta.append(Seg)
                Seg = []
            Taskset_U[i] = float("{0:.3f}".format(Ta[4] / Ta[2]))

        else:                                                   #Getting Tasks' data for CPU-using Tasks
            Ta.append('-')
            P = ceil(RandomValueGenerator(0, number_of_tasks)) 
            Ta.append(P)
            T = float("{0:.2f}".format(RandomValueGenerator(T_min, T_max)))
            Ta.append(T)
            D = T
            Ta.append(D)        
            if U_Total_TaskSet == None:     
                NWCET = float("{0:.3f}".format(T * RandomValueGenerator(U_task_min, U_task_max)))
            else:
                NWCET = float("{0:.3f}".format(T * U_tasks_in_TaskSet [i]))
            Ta.append(NWCET)
            C = NWCET 
            Ta.append(C)
            Ta.append(0)
            l = 1                                               #Number of the segments                                     
            Ta.append(l)                                        #Number of sequential segments
            Ta.append(0)                                        #Number of parallel segments
            h = 'CPU'
            Cseg = C 
            Seg.append(Cseg)
            Seg.append(h)
            Ta.append(Seg)
            Seg = []
            Taskset_U[i] = float("{0:.3f}".format(Ta[4] / Ta[2]))
        Gama[i] = Ta 
        Ta = []
    
    WFD(Taskset_U)

    for task in range(number_of_tasks):
        Gama[task][0] = Task_on_core[task]


#-----------TasksetGenerator1----------------------------
        
def TasksetGenerator1(Gama_info, number_of_tasks):
    global Gama
    Gama = {}

    global Taskset_U
    Taskset_U = list()
    for task in range(number_of_tasks):
        Taskset_U.append(0)

    Seg = list()                                          #A list for each Segment's data in a Task


    for task_id in Gama_info.keys():       
        Eta = Gama_info[task_id][6]
        C = Gama_info[task_id][3]
        G = Gama_info[task_id][4]


        tempc = 0
        temph = 0 
        for j in range (2*Eta + 1):                               #For each Segment
            if j % 2 == 0:
                h = 'CPU'
            else:
                h = 'GPU'                  

            if h == 'CPU':
                if j == (2*Eta):
                    Cseg = float("{0:.3f}".format(C - tempc))
                else:
                    Cseg = float("{0:.3f}".format(random.uniform(0.1 , 0.9)*(C - tempc)))
                    tempc = tempc + Cseg

                Seg.append(Cseg)
                Seg.append(h)                       
                
            elif h == 'GPU':
                if j == (2*Eta-1):
                    Gseg = float("{0:.3f}".format(G - temph))
                else:
                    Gseg = float("{0:.3f}".format(random.uniform(0.1 , 0.9)* (G - temph)))
                    temph = temph + Gseg

                Seg.append(Gseg)
                Seg.append(h)                                          
        
    
            Gama_info[task_id].append(Seg)
            Seg = []
            Gama[task_id] = Gama_info[task_id]
        Taskset_U[task_id] = float("{0:.3f}".format(Gama_info[task_id][2] / Gama_info[task_id][0]))
        
    #print(Taskset_U)
    taskset_utilizaion = sum(Taskset_U)
    #print(taskset_utilizaion)
    WFD(Taskset_U)
    Gama = {key:[Task_on_core[key], 0] + value for key, value in Gama_info.items()}
    #print(Gama)

    return(Gama)

#--------Worst Fit Decreasing Algorith (WFD)--------------

def WFD(Taskset_U):

    global number_of_CPUs
    global Task_on_core
    Task_on_core = list()
    for task in range(len(Taskset_U)):
        Task_on_core.append(0)
    core_U = list()
    for task in range(number_of_CPUs):
        core_U.append(0)
    
    min_core = 0
    for i in range(len(Taskset_U)):
        min_core = core_U.index(min(core_U))
        Task_on_core[i] = min_core
        core_U[min_core] = float("{0:.3f}".format(core_U[min_core] + Taskset_U[i]))

    # print(Fore.BLUE)
    #print('Core_U:', core_U,"\n")
    #print(Task_on_core)
    return(Task_on_core)

#------------Schedulability Test----------------------------

def Serverbased_Schedulability_Test():
    # print(Fore.MAGENTA)
    # print('---Schedulability Test---')

    global number_of_CPUs
    global GPU_Server_Core
    #print(Fore.CYAN)
    #GPU_Server_Core = int(RandomValueGenerator(0, number_of_CPUs))
    GPU_Server_Core = 0


    hp = list()                                                  #Tasks with a higher priority than the target task
    lp = list()                                                  #Tasks with a lower priority than the target task
    B = list()
    A = list()
    W = list()
    hpp = list() 
    Bgpu = list()
    W_Taskset = list()
    GPUusing_Tasks_dic = dict()
    GPUusing_Tasks_dic ={}
    global number_of_tasks
    global not_schedulable_S
    global not_schedulable_M
    global Gm_list
    Gm_list = list()
    for task in range(number_of_tasks):
        Gm_list.append(0)
    global Gc_list
    Gc_list = list()
    for task in range(number_of_tasks):
        Gc_list.append(0)   
    for task in range(number_of_tasks):
        Bgpu.append(0)
   
    GPUusing_Tasks = []

    for task_id, values in Gama.items():
        for item in values:
            # Check if any of the sub-items in the value array is a list and contains 'GPU'
            if isinstance(item, list) and 'GPU' in item:
                GPUusing_Tasks.append(task_id)
                break  # Break the inner loop once 'GPU' is found

    for task_id, task_array in Gama.items():
        if task_id in GPUusing_Tasks:
            GPUusing_Tasks_dic[task_id] = task_array 
    GPUusing_Tasks = (dict(sorted(GPUusing_Tasks_dic.items(), key = lambda x:x[1][2]))) #Sorting GPUusing_Tasks based on their T
    GPUusing_Tasks = list(GPUusing_Tasks.keys())
    #print('GPUusing_Tasks:', GPUusing_Tasks)

    number_of_used_GPUs = min(number_of_GPUs, len(GPUusing_Tasks))

    RTnext = list()
    RTnext = [[0] * len(GPUusing_Tasks) for _ in range(number_of_used_GPUs)] 

    WL = list()
    WL = [[0] * 1 for _ in range(number_of_used_GPUs)]

    beta = list()
    beta = [[0] * 1 for _ in range(number_of_used_GPUs)]

    for i in GPUusing_Tasks:         #For each task in GPUusing_Tasks
        if (i == GPUusing_Tasks[0]):
            print(Fore.YELLOW)
            #print('Task ', i)
            
            hp = []
            lp = []
            B = []
            A = []
                        
        
        
            i = GPUusing_Tasks[0]      #For the first task in GPUusing_Tasks    
            lp = []
            A = []
                
            for task in GPUusing_Tasks:              
                if (Gama[task][2] > Gama[i][2]):
                    lp.append(task)    
            lp.sort(reverse=True)
            print(Fore.CYAN)
            #print('lp:', lp)

            for k in lp:                                           #Calculating Blocking time (beta) for each GPU
                iter1 = 0
                bmax = 0
                for m in range(Gama[k][8]):
                    b = Gama[k][10 + iter1][0] + Eps
                    bmax = max(b,bmax)
                    iter1 = iter1 + 2 
                A.append(float("{0:.3f}".format(bmax)))  
            A.sort(reverse=True)
            #print('A:', A)
            for g in range(number_of_used_GPUs):
                if (g) < len(A):
                    beta[g] = A[g] + Eps
                else:
                    beta[g] = 0
                #g = g + 1
            #print('beta:', beta)
            WL = beta.copy()
            #print('WL=', WL)
            

    for i in GPUusing_Tasks:             #For each task in GPUusing_Tasks
        hp = []              
        for task in GPUusing_Tasks:    
            if (Gama[task][2] < Gama[i][2]):
                hp.append(task)          
            # elif (Gama[task][2] > Gama[i][2]):
            #     lp.append(task)    
        hp.sort(reverse=True)
        #lp.sort(reverse=True)
        print(Fore.CYAN)
        #print('hp:', hp)
        #print('lp:', lp)



        # Gc = (len(hp) + 1) * (0.0002 * len(GPUusing_Tasks) + 0.004)
        # Gc_list[i] = float("{0:.3f}".format(Gc))
        #print(Gc_list) 

                                    

        seg = 0
        for j in range(Gama[i][8]):               #For each GPU-using segment in task i
            n = 0
            sumb = 0
             
            hp_star = list()
            A = []
            hp_star = []

            print(Fore.GREEN)
            #print('G (',i, ',' , j,') will run on GPU', WL.index(min(WL)))
            print(Fore.CYAN)
            g = WL.index(min(WL))

            if mode == 'SGPU':
                hp_star = hp
            else:
                idx = 0
                for row in range(number_of_used_GPUs):
                    idx = GPUusing_Tasks.index(i)                    
                    if (j == 0 and number_of_used_GPUs >= 1 and row != g):
                        RTnext[row][idx] += Gama[i][2] + Gama[i][2*j+9][0]
                    elif (j > 0 and number_of_used_GPUs >= 1 and row != g):
                        RTnext[row][idx] += Gama[i][j+9][0]
                #print('RTnext:', RTnext) 
                
                
                for idx1 in range (len(hp)):
                    if (WL[g] > RTnext[g][idx1]) and all((WL[g] < row[idx1]) for row in RTnext[:g] + RTnext[g+1:]):
                        if (hp[idx1] != i):
                            hp_star.append(hp[idx1])
                #print('hp_star=', hp_star)

            B = []
            if(j == 0): 
                B.append(beta[g])
                #print('B0:', B)
            else:
                B.append (0)
             
            n = 1
            if hp_star == []:
                term2 = 0
            while (1):
                for k in hp_star:
                    iter1 = 0
                    for m in range(Gama[k][8]): 
                        sumb = float("{0:.3f}".format(sumb + (ceil(B[n-1]/Gama[k][2]) + 1) * (Gama[k][10 + iter1][0] + Eps)))
                        iter1 = iter1 + 2
                    term2 = sumb
                #print('term2:', term2)
                sumb = 0

                B.append(float("{0:.3f}".format(B[0] + term2)))               
                #print('B:', B)
                if (B[n] == B[n -1]) or (B[n] > Gama[i][3]): 
                    WL[g] = B[n] + Gama[i][seg + 10][0]
                    #print('WL=', WL)
                    seg = seg + 2
                    break    
                else:
                    n = n + 1


        Bgpu0 = Gama[i][8] * B[n] + Gama[i][6] + 2 * Gama[i][8] * Eps
        Bgpu[i] = float("{0:.3f}".format(Bgpu0))
    print(Fore.BLUE)
    #print('Bgpu:', Bgpu)

    Gama_sorted_T = (dict(sorted(Gama.items(), key = lambda x:x[1][2]))) #Sorting the tasks according their periority (T)
    #print('Gama_sorted_T:', "\n")
    # for k, v in Gama_sorted_T.items():
    #     print (Fore.YELLOW+'Task',k, Fore.GREEN+':', v, "\n")

    for task in range(number_of_tasks):
        W_Taskset.append(0)

    for i in list(Gama_sorted_T):
        # print(Fore.YELLOW)
        # print('Task', i)
        hpp = []                                                 #Higher priority tasks on the same core as task i
        sigma1 = 0
        sigma2 = 0
        W = []
        

        for ii in list(Gama):
            if ((float(Gama[i][0]) == float(Gama[ii][0])) and (Gama[i][2] > Gama[ii][2])):
                hpp.append(ii)
            else:
                continue
        # print(Fore.GREEN)
        # print('hpp:', hpp)
        
        Gm = RandomValueGenerator(Ratio_misc_Gseg_min, Ratio_misc_Gseg_max) * Gama[i][6]
        Gm_list[i] = float("{0:.3f}".format(Gm))
        Gm_list[i] = 0



        n = 0
        #print('n=', n)
        sigma1 = Gama[i][5] + Bgpu[i] + Gc_list[i]
        W.append(float("{0:.3f}".format(sigma1)))
        #print('W =', W)
        sigma1 = 0

        n = 1
        while(1):
            #print('n=', n)
            sigma1 = 0
            sigma2 = 0


            for k in hpp:                                        # W pwith GPU selection time Gc
                #print('k=', k)
                sigma1 = sigma1 + ceil((W[n - 1] + W_Taskset[k] - Gama[k][5]) / Gama[k][2]) * Gama[k][5]
                #print('W_Taskset[k] =', W_Taskset[k])
            if Gama[i][0] != GPU_Server_Core:

                W.append(float("{0:.3f}".format(Gama[i][5] + Bgpu[i] + sigma1)))
                
            else:
                for l in GPUusing_Tasks:    
                    if (l != i): 
                        sigma2 = sigma2 + ceil((W[n - 1] + Gama[l][3]- (Gm + 2* Gama[l][8] * Eps)) / Gama[l][2]) * (Gm + 2* Gama[l][8] * Eps)
                    else:
                        continue
                    #print('sigma2:', sigma2)
                W.append(float("{0:.3f}".format(Gama[i][5] + Bgpu[i] + sigma1 + sigma2)))
                #print('W =', W)
            W_Taskset[i] = W[n]


            if W[n] > Gama[i][3]:
                
                # print('Task set is not schedulable.')
                if mode == 'SGPU':
                    not_schedulable_S = not_schedulable_S + 1
                    #print('SGPU Task', i, 'missed the deadline!')
                    mode == 'MGPU'                   
                elif mode == 'MGPU':
                    not_schedulable_M = not_schedulable_M + 1
                    #print('MGPU Task', i, 'missed the deadline!')
                    mode == 'SGPU'
                return

            
            elif (W[n] == W[n - 1]):
                break
            else:
                n = n + 1

        print(Fore.GREEN)
        # print('W',i,'=', W)
        #print('W_Taskset=', W_Taskset)
        #print('Gm_list:', Gm_list)


#------------Validation----------------------------  
     
def Validation():

    global number_of_tasks
    global Perc_GPUusing_Tasks
    global GPUusing_Tasks
    global Gm_list
    global Ratio_misc_Gseg_min
    global Ratio_misc_Gseg_max
    global GPU_Server_Core
    U_core = list()
    for task in range(number_of_CPUs):
        U_core.append(0)
    U_server = 0
    U = 0
    
    print(Fore.MAGENTA)
    print('---------Validation Test--------')

    for i in (GPUusing_Tasks):
        if (Gm_list[i] == 0):
            Gm = float("{0:.3f}".format(RandomValueGenerator(Ratio_misc_Gseg_min, Ratio_misc_Gseg_max) * Gama[i][6]))
            Gm_list[i] = Gm  
        else:
            continue
    print(Fore.BLUE)
    print('Gm_list:', Gm_list)
    for i in (list(Gama)):
        U_server = U_server + (Gm_list[i] + 2 * Gama[i][8] * Eps) / Gama[i][2]
        U = U + ((Gama[i][5] + Gama[i][6]) / Gama[i][2])
    print('U_server:', float("{0:.3f}".format(U_server)))
    print('U:', float("{0:.3f}".format(U)))     
    U_Total_TaskSet =float("{0:.3f}".format(U + U_server))    
    print('U_Total_Taskset:', U_Total_TaskSet)
    Average_core_utilization = float("{0:.3f}".format(U_Total_TaskSet / number_of_CPUs))
    print('Average_core_utilization:', Average_core_utilization)

    for i in range(number_of_CPUs):
        for j in range(number_of_tasks):
            if Gama[j][0] == i:
                U_core[i] = float("{0:.2f}".format(U_core[i])) + float("{0:.2f}".format(Gama[j][4] / Gama[j][2]))
                if i == GPU_Server_Core:
                    U_core[i] =float("{0:.2f}".format(U_core[i] + U_server))
                else:
                    continue
    print('U_core:', U_core)

    with open('Number of Tasks in a Taskset.csv', 'a', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow([number_of_tasks])

    with open('Percentage of GPU using Tasks in a Taskset.csv', 'a', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow([Perc_GPUusing_Tasks])

    with open('Average core utilization.csv', 'a', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow([Average_core_utilization])



#---------------------Graph---------------------------

def generate_graph_from_csv(filename):
    x_values = []
    y_values = []

    with open(filename, 'r') as csvfile:
        reader = csv.reader(csvfile, delimiter=',')  # Specify the delimiter 
        for row in reader:
            x_value = float(row[0].replace(',', '').strip())   # Replace comma with dot and strip whitespace
            y_value = float(row[1])
            x_values.append(x_value)
            y_values.append(y_value)

    plt.ion()
    plt.plot(x_values, y_values)
    plt.xlabel('Utilization')
    plt.ylabel('Schedulability Rate')
    plt.title('Schedulability')
    plt.legend(loc = "lower left")
    plt.grid(True)
    #plt.xticks(x_Ratio)
    plt.ylim(0,100)
    plt.show()
    # Wait for the figure to be closed
    while True:
        if plt.get_fignums():  # Check if any figure windows are open
            plt.pause(0.1)
        else:
            break

#--------------------TasksetReader--------------------
        
def TasksetReader(csvfile):
    global Gama_info
    Gama_info = {}
    global number_of_tasks
    number_of_tasks_r = 0  # Parameter to count the number of tasks

    #with open('taskset.csv', 'r') as csvfile:
    reader = csv.reader(csvfile)
    #next(reader)  # Skip the header row if there is one
    for row in reader:
        # Check if the row is empty (i.e., end of a taskset)
        if not row or row == ['']:
            break  # Stop reading this taskset

        # Using the first value in the row as the key and converting it to an integer
        task_id = int(row[0])
        #print("task_id:", task_id)
        # Storing the rest of the values in an array, also converting them to integers
        values = [int(value) for value in row[1:]]
        # Assigning the array to the corresponding key in Gama
        Gama_info[task_id] = values
        number_of_tasks_r += 1 

        number_of_tasks = number_of_tasks_r
        #print("number of tasks:", number_of_tasks)
    return Gama_info, number_of_tasks


#--------------------Get file name--------------------

#def get_next_filename(base_name, index, step):
    if index == 0:
        return f"{base_name}{index}.csv"
    return f"{base_name}{index:.1f}.csv"

def get_next_filename(base_name, int_index, step):
    # Convert int_index to a float representation by dividing by 10
    index = int_index / 10.0

    # Check if the float index is a whole number to decide filename format
    if index.is_integer():
        return f"{base_name}{int(index)}.csv"
    else:
        return f"{base_name}{index:.1f}.csv"
#---------------------Main---------------------------- 

def main():
    
    #Number_of_TaskSets = int(input ('Number of TaskSets : '))
    Number_of_TaskSets = 1500
    global Perc_GPUusing_Tasks
    global number_of_tasks
    global not_schedulable_S
    global not_schedulable_M
    global U_Total_TaskSet
    global number_of_CPUs
    global number_of_GPUs
    global Eta
    global mode
    U_Total_TaskSet = None
    global output_file
    output_file = '4CPU-4GPU-0.3.csv'


    base_name = "taskset"
    int_index = 0
    step = 1

    while True:
        filename = get_next_filename(base_name, int_index, step)
        #print(f"Index: {int_index / 10.0}")
        if not os.path.exists(filename):
            break  # Stop if the file does not exist
        #print(filename)

                
        with open(filename, 'r') as tasksetfile:
            not_schedulable_S = 0
            not_schedulable_M = 0
            Schedulable_Tasksets_S = 0
            Schedulable_Tasksets_M = 0
            while True:
                Gama_info, number_of_tasks = TasksetReader(tasksetfile)
                if not Gama_info:
                    break  # Check if taskset is empty
                
                ExperimentSetup()
                TasksetGenerator1(Gama_info, number_of_tasks)
                
                mode = 'MGPU'
                Serverbased_Schedulability_Test()



        #Schedulable_Tasksets_S = float("{0:.1f}".format(((Number_of_TaskSets - not_schedulable_S) / Number_of_TaskSets) * 100))
        Schedulable_Tasksets_M = float("{0:.1f}".format(((Number_of_TaskSets - not_schedulable_M) / Number_of_TaskSets) * 100))

        with open(output_file, 'a', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow([Schedulable_Tasksets_M/100])
            #writer.writerow([Schedulable_Tasksets_S, Schedulable_Tasksets_M])   

        int_index += step

    
    
    #generate_graph_from_csv(filename)
    

if __name__ == '__main__':
    main()


