import os

num=50
sumb_t=0.0
sumq1_t=0.0
sumq2_t=0.0
sumq3_t=0.0
sumb_m=0.0
sumq1_m=0.0
sumq2_m=0.0
sumq3_m=0.0
for p in range(0,num):
    print("--- Running #",p,"---")
    os.system("./build/src/app/openeda ~/open-edi/demo/contest/8-bits_rv_nangate/query.tcl >tmp.log")
    with open("tmp.log") as f:
        log_data=f.read()

    a=log_data.find('build Elapse: ')
    s=log_data[a+14:]
    a=s.find(' ')
    b_t=s[:a]
    a=s.find('query Elapse: ')
    s=s[a+14:]
    a=s.find(' ')
    q1_t=s[:a]
    a=s.find('query Elapse: ')
    s=s[a+14:]
    a=s.find(' ')
    q2_t=s[:a]
    a=s.find('query Elapse: ')
    s=s[a+14:]
    a=s.find(' ')
    q3_t=s[:a]

    a=log_data.find('build Elapse: ')
    s=log_data[a:]
    a=s.find('Virt/peak: ')
    s=s[a+11:]
    a=s.find('\n')
    b_m=s[:a]
    a=s.find('Virt/peak: ')
    s=s[a+11:]
    a=s.find('\n')
    q1_m=s[:a]
    a=s.find('Virt/peak: ')
    s=s[a+11:]
    a=s.find('\n')
    q2_m=s[:a]
    a=s.find('Virt/peak: ')
    s=s[a+11:]
    a=s.find('\n')
    q3_m=s[:a]

    print(b_t,b_m)
    print(q1_t,q1_m)
    print(q2_t,q2_m)
    print(q3_t,q3_m)

    sumb_t+=float(b_t)
    sumq1_t+=float(q1_t)
    sumq2_t+=float(q2_t)
    sumq3_t+=float(q3_t)
    sumb_m+=float(b_m)
    sumq1_m+=float(q1_m)
    sumq2_m+=float(q2_m)
    sumq3_m+=float(q3_m)

# os.system("rm openedi.log*")
print("--- Average ---")
aveb_t=sumb_t/num
aveq1_t=sumq1_t/num
aveq2_t=sumq2_t/num
aveq3_t=sumq3_t/num
aveb_m=sumb_m/num
aveq1_m=sumq1_m/num
aveq2_m=sumq2_m/num
aveq3_m=sumq3_m/num
print(aveb_t,aveb_m)
print(aveq1_t,aveq1_m)
print(aveq2_t,aveq2_m)
print(aveq3_t,aveq3_m)
