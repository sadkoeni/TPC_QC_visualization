from bokeh.io import export_svgs, output_file
import math, statistics, uproot
import numpy as np, pandas as pd, holoviews as hv, datashader as ds

from bokeh.io import show, output_notebook, save
from bokeh.models import LogColorMapper, LinearColorMapper, ColorBar, HoverTool
from bokeh.palettes import Turbo256 as palette
from bokeh.plotting import figure
from bokeh.layouts import row, column

import datashader.utils as du, datashader.transfer_functions as tf
from holoviews import streams
import holoviews.operation.datashader as hd
#from holoviews.operation.datashader import datashade
from holoviews.streams import RangeXY

hd.shade.cmap=["lightblue", "darkblue"]
hv.extension("bokeh") 
output_notebook()

from test_data_config import rotatePointTo, addRotateListTo, configureData

def plotter_2D_test_histogram(name):
	# IMPORTING AND FOMRATTING DATA

	# ===| open root demo file with pedestal and noise values |===
	t = uproot.open("Data/CalibTree.root")["calibTree"]
	#padData = t.pandas.df("Pedestals", flatten = False)
	padData = t.array(name)

	# ===| pad plane plane meta data |===
	d = pd.read_csv("Data/pad_plane_data.txt", sep='\t')

	# ===| fuction to prepare input root demo file for plotting |===
	[vcor,vtri] = configureData(padData,d);

	# PLOTTING

	hd.shade.cmap=['#FBFCBF','#FD9F6C','#DD4968','#8C2980','#3B0F6F','#000003']
	cvs = ds.Canvas(plot_height=400,plot_width=400)

	trim = hv.TriMesh((vtri,hv.Points(vcor, vdims='za'))).opts(show_grid=True)
	trim2 = hv.TriMesh((vtri,hv.Points(vcor, vdims='zc'))).opts(show_grid=True)
	trim.opts(colorbar=True)
	trim.opts(cmap='Blues') 

	trimesh = hd.datashade(trim, aggregator=ds.mean('za'), precompute=True, link_inputs=False);
	trimesh2 = hd.datashade(trim2, aggregator=ds.mean('zc'), precompute=True, link_inputs=False);
	trimesh.opts(height=450,width=450,show_grid=False, xaxis=None,yaxis=None);
	trimesh2.opts(height=450,width=450,show_grid=False, xaxis=None,yaxis=None);

	# ADDING INTERACTIVITY

	# Small hover tool
	tooltips_small = [
		("X:", "$x"),
		("Y:", "$y"),
		("Value:", "NaN")
	]
	hover_small = HoverTool(tooltips=tooltips_small)
	dynamic = hv.util.Dynamic(hd.aggregate(trim, width=30, height=30, streams=[RangeXY]), 
							  operation=hv.QuadMesh) \
			  .opts(tools=[hover_small], alpha=0, hover_alpha=0, hover_line_color='black',hover_line_alpha=0)

	# Sector select hover tool

	sector_edge_phi = np.linspace(0,np.pi*2,19)
	sector_edge_r = np.array([850,2530])
	Phi,R = np.meshgrid(sector_edge_phi,sector_edge_r)
	Qx = np.cos(Phi)*np.abs(R)
	Qy = np.sin(Phi)*np.abs(R)
	Z = np.linspace(0,17,18).reshape(1, 18)
	#Z = Z*0

	hover_data = dict(x=Qx,
					 y=Qy,
					 z=Z
					 )

	tooltips_a = [
		("Side","A"),
		("Sector", "@z")
	]
	tooltips_c = [
		("Side","C"),
		("Sector", "@z")
	]
	hover_a = HoverTool(tooltips=tooltips_a)
	hover_c = HoverTool(tooltips=tooltips_c)

	qmesh_a = hv.QuadMesh(hover_data)\
				.opts(tools=[hover_a,'tap'], alpha=0, hover_fill_alpha=0.1, hover_color='white',
					  hover_line_color='black',hover_line_alpha=1)
	qmesh_c = hv.QuadMesh(hover_data)\
				.opts(tools=[hover_c], alpha=0, hover_fill_alpha=0.1, hover_color='white',
					  hover_line_color='black',hover_line_alpha=1)

	# CREATING OUTPUT
	
	tpc_plot_a = trimesh * qmesh_a * hv.Text(0, 0, 'A', fontsize=40)
	tpc_plot_c = trimesh2 * qmesh_c * hv.Text(0, 0, 'C', fontsize=40)
	final_layout = (tpc_plot_a + tpc_plot_c).opts(merge_tools=False)
	
	return final_layout