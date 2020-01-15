#!/usr/bin/env python

#example of usage:  ./vehicleFlow.py cars.rou.xml -m 5 -r 0.7 -s 13.89 -t 20
import os
import sys
import random
import datetime
import shutil
from optparse import OptionParser

def get_options():
    optParser = OptionParser()
    optParser.add_option(
        "-r", "--rate", type="float", default=0.1, help="creation rate")
    optParser.add_option(
        "-m", "--minGap", type="float", default=1.0, help="vehicle min gap, negative numbers denote the center of a uniform distribution [x-0.2, x+0.2]")
    optParser.add_option(
        "-s", "--maxSpeed", type="float", default=13.89, help="vehicle max speed, negative numbers denote the center of a uniform distribution [x-3, x+3]")
    optParser.add_option(
        "-d", "--speedDev", type="float", default=0.5, help="speed dev (see SUMO spec)")
    optParser.add_option(
        "-f", "--speedFactor", type="float", default=1.5, help="speed factor (see SUMO spec)")
    optParser.add_option(
        "-b", "--begin", type="int", default=0, help="begin time")
    optParser.add_option(
        "-e", "--end", type="int", default=600, help="end time")
    optParser.add_option("-i", "--index", type="int",
                         default=1, help="starting index for naming vehicles")
    optParser.add_option(
        "-n", "--name", default="veh", help="base name for vehicles")
    optParser.add_option(
        "-t", "--total", default=0, help="Total number of vehicles to be created. If 0, begin and end are used to create the vehicles")
    
    (options, args) = optParser.parse_args()
    
    options.output = args[0]
    return options

def line_prepender(filename, line):
    with open(filename, 'r+') as f:
        content = f.read()
        f.seek(0, 0)
        f.write(line.rstrip('\r\n') + '\n' + content)

def write_init(f,options):
    f.write('<routes>\n')
    f.write('  <route id="0" edges="w_to_c1 c1_to_c2 c2_to_e"/>\n')
    f.write('  <route id="1" edges="n1_to_c1 c1_to_s1"/>\n')
    f.write('  <route id="2" edges="s1_to_c1 c1_to_n1"/>\n')
    f.write('  <route id="3" edges="n2_to_c2 c2_to_s2"/>\n')
    f.write('  <route id="4" edges="e_to_c2 c2_to_c1 c1_to_w"/>\n')
    f.write('  <route id="5" edges="s2_to_c2 c2_to_n2"/>\n')
    f.write('  <route id="6" edges="w_to_c1 c1_to_s1"/>\n')
    f.write('  <route id="7" edges="w_to_c1 c1_to_c2 c2_to_s2"/>\n')
    f.write('  <route id="8" edges="w_to_c1 c1_to_c2 c2_to_n2"/>\n')
    f.write('  <route id="9" edges="w_to_c1 c1_to_n1"/>\n')
    f.write('  <route id="10" edges="s1_to_c1 c1_to_c2 c2_to_s2"/>\n')
    f.write('  <route id="11" edges="s1_to_c1 c1_to_c2 c2_to_e"/>\n')
    f.write('  <route id="12" edges="s1_to_c1 c1_to_c2 c2_to_n2"/>\n')
    f.write('  <route id="13" edges="s1_to_c1 c1_to_w"/>\n')
    f.write('  <route id="14" edges="s2_to_c2 c2_to_e"/>\n')
    f.write('  <route id="15" edges="s2_to_c2 c2_to_c1 c1_to_n1"/>\n')
    f.write('  <route id="16" edges="s2_to_c2 c2_to_c1 c1_to_w"/>\n')
    f.write('  <route id="17" edges="s2_to_c2 c2_to_c1 c1_to_s1"/>\n')
    f.write('  <route id="18" edges="e_to_c2 c2_to_n2"/>\n')
    f.write('  <route id="19" edges="e_to_c2 c2_to_c1 c1_to_n1"/>\n')
    f.write('  <route id="20" edges="e_to_c2 c2_to_c1 c1_to_s1"/>\n')
    f.write('  <route id="21" edges="e_to_c2 c2_to_s2"/>\n')
    f.write('  <route id="22" edges="n2_to_c2 c2_to_c1 c1_to_n1"/>\n')
    f.write('  <route id="23" edges="n2_to_c2 c2_to_c1 c1_to_w"/>\n')
    f.write('  <route id="24" edges="n2_to_c2 c2_to_c1 c1_to_s1"/>\n')
    f.write('  <route id="25" edges="n2_to_c2 c2_to_e"/>\n')
    f.write('  <route id="26" edges="n1_to_c1 c1_to_w"/>\n')
    f.write('  <route id="27" edges="n1_to_c1 c1_to_c2 c2_to_s2"/>\n')
    f.write('  <route id="28" edges="n1_to_c1 c1_to_c2 c2_to_e"/>\n')
    f.write('  <route id="29" edges="n1_to_c1 c1_to_c2 c2_to_n2"/>\n')


    f.write('\n')
    f.write('  <vType accel="4" decel="7.5" emergencyDecel="10" minGap="%s" speedDev="%s" speedFactor="%s" id="Car0" maxSpeed="%s" />\n' % (options.minGap, options.speedDev, options.speedFactor, options.maxSpeed))
    f.write('\n')

def write_veh(f, name, index, depart, route_var):
    # Random route
    #in this way we can have circular rotation among the street but random choice of the path
    f.write('      <vehicle id="%s%s" type="Car0" depart="%s" route="%s">\n' %
        (name, index, depart, route_var))
    f.write('    </vehicle>\n')

def main():
    options = get_options()
    i = 0
    with open(options.output, 'w') as f:
        f.write('<!-- generated on %s by "%s" -->\n' %
                (datetime.datetime.now(), os.path.basename(" ".join(sys.argv))))
        write_init(f,options)
        index = options.index
        # Time instant in which the last car is scheduled to depart
        last_depart = options.begin*10.0/10.0
        if options.total == 0:
            while last_depart < options.end:
                intertime = random.expovariate(options.rate)
                route_var = int(random.uniform(0,29))
                write_veh(f, options.name, index, last_depart+intertime, route_var)
                last_depart += intertime
                index += 1
        else:
            while i < int(options.total):
                intertime = random.expovariate(options.rate)
                route_var = int(random.uniform(0,29))
                write_veh(f, options.name, index, last_depart+intertime, route_var)
                last_depart += intertime
                index += 1
                i += 1
        f.write('</routes>')
    index = int(index)-1
    line = "<!-- number of vehicles:%d -->\n" % index
    line_prepender(options.output, line)

if __name__ == "__main__":
    main()
