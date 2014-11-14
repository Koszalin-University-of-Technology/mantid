from mantid.kernel import *
from mantid.api import *
from mantid.simpleapi import *
import numpy as np
import os.path


class CutMD(DataProcessorAlgorithm):
    
    def category(self):
        return 'MDAlgorithms'

    def summary(self):
        return 'Slices multidimensional workspaces using input projection information'

    def PyInit(self):
        self.declareProperty(IMDEventWorkspaceProperty('InputWorkspace', '', direction=Direction.Input),
                             doc='MDWorkspace to slice')
        
        self.declareProperty(ITableWorkspaceProperty('Projection', '', direction=Direction.Input, optional = PropertyMode.Optional), doc='Projection')
        
        self.declareProperty(FloatArrayProperty(name='P1Bin', values=[]),
                             doc='Projection 1 binning')
        
        self.declareProperty(FloatArrayProperty(name='P2Bin', values=[]),
                             doc='Projection 2 binning')
        
        self.declareProperty(FloatArrayProperty(name='P3Bin', values=[]),
                             doc='Projection 3 binning')
        
        self.declareProperty(FloatArrayProperty(name='P4Bin', values=[]),
                             doc='Projection 4 binning')

        self.declareProperty(IMDWorkspaceProperty('OutputWorkspace', '',
                             direction=Direction.Output),
                             doc='Output cut workspace')
        
        self.declareProperty(name="NoPix", defaultValue=False, doc="If False creates a full MDEventWorkspaces as output. True to create an MDHistoWorkspace as output. This is DND only in Horace terminology.")
        
        
    def __to_mantid_slicing_binning(self, horace_binning, to_cut, dimension_index):
        dim = to_cut.getDimension(dimension_index)
        min = dim.getMinimum()
        max = dim.getMaximum()
        n_arguments = len(horace_binning)
        if n_arguments == 0:
            raise ValueError("binning parameter cannot be empty")
        elif n_arguments == 1:
            step_size = horace_binning[0]
            n_bins = (max - min) / step_size
        elif n_arguments == 2:
            min = horace_binning[0]
            max = horace_binning[1]
            n_bins = 1
        elif n_arguments == 3:
            step_size = horace_binning[1]
            n_bins = (max - min) / step_size
            min = horace_binning[0]
            max = horace_binning[2]
            pass
        else:
            raise ValueError("Too many arguments given to the binning parameter")
        if min >= max:
            raise ValueError("Dimension Min >= Max value. Min %.2f Max %.2f", min, max)
        if n_bins < 1:
            raise ValueError("Number of bins calculated to be < 1")
        return (min, max, n_bins)
    
    
    def __calculate_extents(self, v, u, w, ws):
        M=np.array([u,v,w])
        Minv=np.linalg.inv(M)
        
        # We are assuming that the workspace has dimensions H, K, L in that order. The workspace MUST have 3 dimensions at least for the following to work.
        Hrange=[ws.getDimension(0).getMinimum(), ws.getDimension(0).getMaximum()]
        Krange=[ws.getDimension(1).getMinimum(), ws.getDimension(1).getMaximum()]
        Lrange=[ws.getDimension(2).getMinimum(), ws.getDimension(2).getMaximum()]
        
        # Create a numpy 2D array. Makes finding minimums and maximums for each transformed coordinates over every corner easier.
        new_coords = np.empty([8, 3])
        counter = 0
        for h in Hrange:
           for k in Krange:
               for l in Lrange:
                   original_corner=np.array([h,k,l])
                   new_coords[counter]=original_corner.dot(Minv)
                   counter += 1
        
        # Get the min max extents
        extents = list()
        for i in range(0,3):
            # Vertical slice down each corner for each dimension, then determine the max, min and use as extents
            extents.append(np.amin(new_coords[:,i]))
            extents.append(np.amax(new_coords[:,i]))
        
            # Copy extents for non crystallographic dimensions    
        non_crystallographic_dimensions = ws.getNumDims() - 3
        if non_crystallographic_dimensions > 0:
            for i in range(0, non_crystallographic_dimensions):
                extents.append(ws.getDimension(i + 3).getMinimum())
                extents.append(ws.getDimension(i + 3).getMaximum())
        
        return extents
    
    def __uvw_from_projection_table(self, projection_table):
        if not isinstance(projection_table, ITableWorkspace):
            I = np.identity(3)
            return (I[0, :], I[1, :], I[2, :])
        column_names = projection_table.getColumnNames()
        u = np.array(projection_table.column('u'))
        v = np.array(projection_table.column('v'))
        if not 'w' in column_names:
            w = np.cross(v,u)
        else:
            w = np.array(projection_table.column('w'))
        return (u, v, w)
    
    def __make_labels(self, projection):

        class Mapping:
            
            def __init__(self, replace):
                self.__replace = replace
                
            def replace(self, entry):
                if np.absolute(entry) == 1:
                    if entry > 0:
                        return self.__replace 
                    else:
                        return "-" + self.__replace
                elif entry == 0:
                    return 0
                else:
                    return "%.2f%s" % ( entry, self.__replace )
                return entry
                
        crystallographic_names = ['zeta', 'eta', 'xi' ]  
        labels = list()
        for i in range(len(projection)):
            cmapping = Mapping(crystallographic_names[i])
            labels.append( [cmapping.replace(x) for x in projection[i]  ] )
        
        return labels
            
        
    def PyExec(self):
        to_cut = self.getProperty("InputWorkspace").value
        nopix = self.getProperty("NoPix").value
        coord_system = to_cut.getSpecialCoordinateSystem()
        if not coord_system == SpecialCoordinateSystem.HKL:
            raise ValueError("Input Workspace must be in reciprocal lattice dimensions (HKL)")
        
        
        projection_table = self.getProperty("Projection").value
        if isinstance(projection_table, ITableWorkspace):
            column_names = set(projection_table.getColumnNames())
            logger.warning(str(column_names)) 
            if not column_names == set(['u', 'v', 'type']):
                    if not column_names == set(['u', 'v', 'offsets', 'type']):
                        if not column_names == set(['u', 'v', 'w', 'offsets', 'type']):
                            raise ValueError("Projection table schema is wrong! Column names received: " + str(column_names) )
            if projection_table.rowCount() != 3:
                raise ValueError("Projection table expects 3 rows")
            
        p1_bins = self.getProperty("P1Bin").value
        p2_bins = self.getProperty("P2Bin").value
        p3_bins = self.getProperty("P3Bin").value
        p4_bins = self.getProperty("P4Bin").value # TODO handle 3D only slicing.
        
        # TODO. THESE ARE WRONG. Need to consider the acutal transformed extents as part of this.
        xbins = self.__to_mantid_slicing_binning(p1_bins, to_cut, 0);
        ybins = self.__to_mantid_slicing_binning(p1_bins, to_cut, 1);
        zbins = self.__to_mantid_slicing_binning(p1_bins, to_cut, 2);
        #ebins = self.__to_mantid_slicing_binning(p1_bins, to_cut, 3); # TODO. cannot guarantee this one is here
        
        # TODO. check that workspace is x=H, y=K, z=L before using the projections and slicing.
        projection = self.__uvw_from_projection_table(projection_table)
        u,v,w = projection
         
        extents = self.__calculate_extents(v, u, w, to_cut)
        
        
            
        projection_labels = self.__make_labels(projection)
        
        cut_alg_name = "BinMD" if nopix else "SliceMD"
        cut_alg = AlgorithmManager.Instance().create(cut_alg_name)
        cut_alg.setChild(True)
        cut_alg.initialize()
        cut_alg.setProperty("InputWorkspace", to_cut)
        cut_alg.setPropertyValue("OutputWorkspace", "sliced")
        cut_alg.setProperty("NormalizeBasisVectors", False)
        cut_alg.setProperty("AxisAligned", False)
        # Now for the basis vectors.
        for i in range(0, to_cut.getNumDims()):
            if i <= 2:
                label = projection_labels[i]
                unit = "TODO" # Haven't figured out how to do this yet.
                vec = projection[i]
                value = "%s, %s, %s" % ( label, unit, ",".join(map(str, vec))) 
                cut_alg.setPropertyValue("BasisVector{0}".format(i) , value)
            if i > 2:
                raise RuntimeError("Not implmented yet for non-crystallographic basis vector generation.")
        cut_alg.setProperty("OutputExtents", extents)
        bins = [ int(xbins[2]), int(ybins[2]), int(zbins[2]) ] # Again this is a hack for 3 dimensional data only.
        cut_alg.setProperty("OutputBins", bins)
         
        cut_alg.execute()
        slice = cut_alg.getProperty("OutputWorkspace").value
        self.setProperty("OutputWorkspace", slice)
        
AlgorithmFactory.subscribe(CutMD)
