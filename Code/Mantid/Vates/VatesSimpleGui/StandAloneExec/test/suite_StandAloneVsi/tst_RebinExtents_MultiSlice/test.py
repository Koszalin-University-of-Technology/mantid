def main():
    source(findFile("scripts", "test_helpers.py"))
    source(findFile("scripts", "common_checks.py"))
    startApplication("VatesSimpleGui")
    open_file("MDEW_4D.nxs")

    clickButton(":splitter_2.Rebin_QPushButton")
    set_ptw_lineedit_property(100, "Bins")
    set_ptw_lineedit_property(100, "Bins", "2")
    set_ptw_lineedit_property(100, "Bins", "3")
    set_ptw_lineedit_property(3, "Bins", "4")
    apply_ptw_settings()

    switch_mode("multiSlice")
    
    # Num slices: X = 3, Y = 2, Z = 1
    make_slice("xAxisWidget", 0.0)
    make_slice("yAxisWidget", 0.0)
    make_slice("zAxisWidget", 0.0)
    make_slice("xAxisWidget", 1.0)
    make_slice("yAxisWidget", 1.0)
    make_slice("xAxisWidget", -1.0)
    apply_ptw_settings()
  
    mouseClick(waitForObject(":splitter.pipelineBrowser_pqPipelineBrowserWidget"), 87, 54, 0, Qt.LeftButton)
    
    # Change extents on dataset
    mouseClick(waitForObject(":qt_tabwidget_stackedwidget.objectInspector_pqObjectInspectorWidget"), 10, 20, 0, Qt.LeftButton)
    set_ptw_lineedit_property(-0.5, "Min") 
    set_ptw_lineedit_property(0.5, "Max") 
    set_ptw_lineedit_property(0.5, "Max", "2") 
    apply_ptw_settings()
    
    # New num slices: X = 1, Y = 1, Z = 1
    check_slices("xAxisWidget", 1)
    check_slices("yAxisWidget", 1)
    check_slices("zAxisWidget", 1)
        
    quit_program()
