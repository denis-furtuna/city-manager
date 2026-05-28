#!/bin/bash
gcc -g -Wall -o city_manager city_manager.c
echo "------------------"
gcc -g -Wall -o monitor monitor_reports.c
echo "------------------"
gcc -g -Wall -o city_hub city_hub.c
