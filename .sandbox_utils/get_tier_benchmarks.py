import os

if "SERVER_EXECUTION" in os.environ and os.environ["SERVER_EXECUTION"] == "1":
    info = eval(open(f'/sandbox/SPEC.json').read())
else:
    info = eval(open(f'SPEC.json').read())



ret = []
for i in range(info['max_tiers']):
    ret.append(f'[tier{i+1}]')
print(",".join(ret))
