from defusedxml.ElementTree import parse





benchmark_results = []

obj = parse('/box/result.xml')
for x in obj.getroot().iter('BenchmarkResults'):
    #print(x.tag)
    #print(x.attrib)
    NAME = x.attrib['name']

    for y in x.iter('mean'):
        HIGH_TIME = y.attrib['upperBound']
        LOW_TIME = y.attrib['lowerBound']
        AVG_TIME = y.attrib['value']
        #print(y.tag)
        #print(y.attrib)


    result = {'name': NAME, 'low_time': float(LOW_TIME)/1e+9, 'high_time': float(HIGH_TIME)/1e+9, 'avg_time': float(AVG_TIME)/1e+9}
    benchmark_results.append(result)
print(benchmark_results)



