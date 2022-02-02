#!/usr/bin/env python
# coding: utf-8

# In[7]:
from bokeh.io import export_svgs, output_file
import math, statistics, uproot
import numpy as np, pandas as pd, holoviews as hv, datashader as ds

from bokeh.io import show, output_notebook, save
from bokeh.models import LogColorMapper, LinearColorMapper, ColorBar
from bokeh.palettes import Turbo256 as palette
from bokeh.plotting import figure
from bokeh.layouts import row, column

import datashader.utils as du, datashader.transfer_functions as tf
from holoviews import streams
import holoviews.operation.datashader as hd

def rotatePointTo(x, y, angleDeg):
    angle = angleDeg / 180. * math.pi
    cs = math.cos(angle)
    sn = math.sin(angle)
    return x * cs - y * sn, x * sn + y * cs

def addRotateListTo(xList, yList, xListRot, yListRot, angleDeg):
    for xVals, yVals in zip(xList, yList):
        xValsRot = []
        yValsRot = []
        for x, y in zip (xVals, yVals):
            xRot, yRot = rotatePointTo(x, y, angleDeg)
            xValsRot.append(xRot)
            yValsRot.append(yRot)

        xListRot.append(xValsRot)
        yListRot.append(yValsRot)

    return xListRot, yListRot

def configureData(padData,d):

    # ===| pad corners of a single sector |===
    x_corners = []
    y_corners = []


    sectorXMin = 836.5
    sectorXMax = 2477
    sectorLength = sectorXMax - sectorXMin
    lengthWidthRatio = 0.6

    plot_width = 1000
    plot_height = int(plot_width * lengthWidthRatio)


    count = 0
    for index, row_data in d.iterrows():
        npr = row_data['npr']
        ro = row_data['ro']
        xm = row_data['xm']
        ph = row_data['ph']
        pw = row_data['pw']
        yro = row_data['yro']

        ks = ph / pw * math.tan(1.74532925199432948e-01)

        for irow in range(0, npr):
            npads = math.floor(ks * (irow + ro) + xm);
            for ipad in range(-1*npads, npads):
                xPadBottomRight = yro + ph * irow
                xPadTopRight    = yro + ph * (irow + 1)
                ri = xPadBottomRight
                yPadBottomRight = pw * ipad * xPadBottomRight/ (ri + ph/2)
                yPadTopRight = pw * ipad * xPadTopRight/ (ri + ph/2)
                x_corners.append([xPadBottomRight, xPadTopRight, xPadTopRight, xPadBottomRight])
                y_corners.append([yPadBottomRight, yPadTopRight, yPadTopRight + pw, yPadBottomRight +pw])

                #values.append(count)
                count += 1


    # ===| keep values for all pads |===
    values = []

    def setupData(dataIROC, dataOROC):
        for val in dataIROC:
            values.append(val)
        for val in dataOROC:
            values.append(val)

    # ===| prepare pad corner coordinates for all ROCs on one side |===
    xListRot = []
    yListRot = []
    secListRot = []

    for sec in range(0, 36):
        #print("Adding sector {}".format(sec))
        addRotateListTo(x_corners, y_corners, xListRot, yListRot, 10 + 20 * sec)
        setupData(padData[sec], padData[sec + 36])
        secListRot.append(sec)
    print("Added {} sectors".format(sec+1))
    #values *= 18
    #data=dict(
        #x=x_corners,
        #y=y_corners,
        #val=values
    #)

    # ===| actual drawing |===
    data=dict(
        x=xListRot,
        y=yListRot,
        val=values
    )


    vx = []
    vy = []
    vza = []
    vzc= []
    a=0
    pad_cor=[]

    #length = len(xListRot)
    length = 262080;
    for i in range(0,length):
        vx.append(xListRot[i][0])
        vx.append(xListRot[i][1])
        vx.append(xListRot[i][2])
        vx.append(xListRot[i][3])
        vy.append(yListRot[i][0])
        vy.append(yListRot[i][1])
        vy.append(yListRot[i][2])
        vy.append(yListRot[i][3])
        vza.append(values[i])
        vza.append(values[i])
        vza.append(values[i])
        vza.append(values[i])
        vzc.append(values[i+262080])
        vzc.append(values[i+262080])
        vzc.append(values[i+262080])
        vzc.append(values[i+262080])
        pad_cor.append([(xListRot[i][0],yListRot[i][0]),
                       (xListRot[i][1],yListRot[i][1]),
                       (xListRot[i][2],yListRot[i][2]),
                       (xListRot[i][3],yListRot[i][3])])
    #cornerspts = np.stack((vx,vy)).T
    vcor = pd.DataFrame(np.stack((vx,vy,vza,vzc)).T, columns=['x', 'y' , 'za', 'zc'])
    tri=[]
    for i in range(0,length):
            tri.append([4*i,4*i+1,4*i+2])
            tri.append([4*i+2,4*i+3,4*i])

    vtri = pd.DataFrame(np.stack(tri), columns=['v0', 'v1' , 'v2'])
    
    return vcor, vtri

