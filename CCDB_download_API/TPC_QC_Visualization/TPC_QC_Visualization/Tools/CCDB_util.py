from datetime import date, datetime
import math
import ROOT
import numpy as np, pandas as pd
import bokeh
import yaml
from bokeh.io import output_notebook, show
from bokeh.models import CustomJS, MultiSelect, Button, ColumnDataSource, DateRangeSlider, CheckboxButtonGroup, MultiChoice
from bokeh.events import ButtonClick, MenuItemClick
from bokeh.layouts import row, column


class dataGetter():
  
  CCDB = pd.read_csv("../../Data/UserFiles/CCDB.csv",header=0, sep = r',', skipinitialspace = True)
  files = list(set(CCDB.Name))
  paths = list(set(CCDB.Path))
  time_stamps = list(set(CCDB.TimeStamp))
  types = list(set(CCDB.Type))
  tasks = list(set(CCDB.Task))

  files_data = CCDB.copy(deep=True)

  selected_files = files_data.copy(deep=True)
  shown_files = files_data.copy(deep=True)

  selected_data = []
  selected_types = []
  selected_tasks = []
  time_range = [files_data['TimeStamp'].min(),files_data['TimeStamp'].max()]


def file_browser(doc):
    def update_button_label():
        label = "Download {} files".format(str(len(selected_files.index)))
        button.label = label
        
    def update_file_select_list(shown_files):
        new_list = list(set(shown_files['Name']))
        file_select.options = new_list
        file_select.value = []
        update_time_range(shown_files)
        
    def update_time_range(shown_files):
        if not shown_files.empty:
            date_range.value=[shown_files['TimeStamp'].min(),shown_files['TimeStamp'].max()]
            date_range.start=shown_files['TimeStamp'].min()
            date_range.end=shown_files['TimeStamp'].max()
    
    def download_selected(selection):
        paths2download = selection['Path'].to_list()
        #ROOT.runCCDBDownloadTarget(str(paths2download))
        
    def select_callback(attr,old,new):
        global selected_data, selected_files, files_data, time_range, selected_types
        selected_data = file_select.value
        time_condition = ((files_data['TimeStamp'] >= time_range[0]) & (files_data['TimeStamp'] <= time_range[1]))
        path_condition = files_data['Path'].map(lambda x: x in selected_data)
        type_condition = files_data['Type'].map(lambda x: x in selected_types)
        selected_files = files_data[path_condition & time_condition & type_condition]
        update_button_label()
        
    def date_callback(attr,old,new):
        global selected_data, selected_files, files_data, time_range, selected_types
        time_range = date_range.value
        time_condition = ((files_data['TimeStamp'] >= time_range[0]) & (files_data['TimeStamp'] <= time_range[1]))
        path_condition = files_data['Path'].map(lambda x: x in selected_data)
        type_condition = files_data['Type'].map(lambda x: x in selected_types)
        selected_files = files_data[path_condition & time_condition & type_condition]
        update_button_label()
        
    def type_callback(attr,old,new):
        global selected_data, selected_files, files_data, time_range, selected_types, shown_files
        selected_types = type_select.value
        time_condition = ((files_data['TimeStamp'] >= time_range[0]) & (files_data['TimeStamp'] <= time_range[1]))
        path_condition = files_data['Path'].map(lambda x: x in selected_data)
        type_condition = files_data['Type'].map(lambda x: x in selected_types)
        selected_files = files_data[path_condition & time_condition & type_condition]
        shown_files = files_data[type_condition]
        update_file_select_list(shown_files)
        update_button_label()
        
    def task_callback(attr,old,new):
        global selected_data, selected_files, files_data, time_range, selected_types, shown_files
        selected_tasks = task_select.value
        time_condition = ((files_data['TimeStamp'] >= time_range[0]) & (files_data['TimeStamp'] <= time_range[1]))
        path_condition = files_data['Path'].map(lambda x: x in selected_data)
        type_condition = files_data['Type'].map(lambda x: x in selected_types)
        task_condition = files_data['Task'].map(lambda x: x in selected_tasks)
        selected_files = files_data[path_condition & time_condition & type_condition]
        
        shown_files = files_data[type_condition]
        update_file_select_list(shown_files)
        update_button_label()
        
    def button_callback(event):
        global selected_data, selected_files, files_data, time_range, selected_types
        print("Will download\n", selected_files)
        #download_selected(selected_files)   

    type_select = MultiChoice(title='Object type',options=types,value=[])
    type_select.on_change("value",type_callback)
    
    task_select = MultiChoice(title='QC task',options=tasks,value=[])
    task_select.on_change("value",task_callback)
        
    date_range = DateRangeSlider(title='Time range',value=[files_data['TimeStamp'].min(),files_data['TimeStamp'].max()],
                                    start=files_data['TimeStamp'].min(), end=files_data['TimeStamp'].max())
    date_range.on_change("value_throttled",date_callback)    
    
    file_select = MultiSelect(title="Files", value=[], options=files, height=300)
    file_select.on_change("value", select_callback)

    button = Button(label="Download all", button_type="success") 
    button.on_event(ButtonClick, button_callback)
    
    doc.add_root(row(column(task_select,type_select,date_range,button),file_select))