from defusedxml.ElementTree import parse
import json




benchmark_results = []

#obj = parse('/box/result.xml')
obj = parse('result.xml')
for x in obj.getroot().iter('TestCase'):
    #print(x.tag)
    #print(x.attrib)
    NAME = x.attrib['name']


    for y in x.iter('OverallResult'):
        SUCCESS = y.attrib['success']
        #HIGH_TIME = y.attrib['upperBound']
        #LOW_TIME = y.attrib['lowerBound']
        #AVG_TIME = y.attrib['value']
        #print(y.tag)
        #print(y.attrib)

    EXPRESSIONS=[]
    for y in x.iter('Expression'):
        orig = None
        expanded = None
        for z in y.iter("Original"):
            orig = z.text
        for z in y.iter("Expanded"):
            expanded = z.text
        INFO_LIST=[]
        for z in x.iter('Info'):
            INFO_LIST.append(z.text.strip())
            #print("newinfo" + INFO)
        EXPRESSIONS.append({'orig' : orig.strip(), 'expanded' : expanded.strip(), 'line' : y.attrib['line'].strip(), 'info' : INFO_LIST})
        #HIGH_TIME = y.attrib['upperBound']
        #LOW_TIME = y.attrib['lowerBound']
        #AVG_TIME = y.attrib['value']
        #print(y.tag)
        #print(y.attrib)
    benchmark_results.append({'name':NAME, 'success':SUCCESS, 'expressions':EXPRESSIONS})
    #result = {'name': NAME, 'low_time': float(LOW_TIME)/1e+9, 'high_time': float(HIGH_TIME)/1e+9, 'avg_time': float(AVG_TIME)/1e+9}
    #benchmark_results.append(result)
print(json.dumps(benchmark_results))



