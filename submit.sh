#!/usr/bin/env bash

FILENAME="submission_hw3_$(date -Iminutes).zip"
zip -u ../$FILENAME submitters.txt Makefile server.c request.c request.h log.c log.h segel.c segel.h queue.c queue.h
