#!/bin/sh

for WVF in *.wvf; do
	../gpif_compiler < ${WVF} > $(basename ${WVF} .wvf).inc
done
