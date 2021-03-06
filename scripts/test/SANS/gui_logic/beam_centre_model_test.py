from __future__ import (absolute_import, division, print_function)

import unittest
import sys
from sans.gui_logic.models.beam_centre_model import BeamCentreModel
from sans.common.enums import FindDirectionEnum, SANSInstrument

if sys.version_info.major == 3:
    from unittest import mock
else:
    import mock


class BeamCentreModelTest(unittest.TestCase):
    def setUp(self):
        self.result = {'pos1':300, 'pos2':-300}
        self.centre_finder_instance = mock.MagicMock(return_value = self.result)
        self.SANSCentreFinder = mock.MagicMock(return_value = self.centre_finder_instance)
        self.beam_centre_model = BeamCentreModel(self.SANSCentreFinder)

    def test_that_model_initialises_with_correct_values(self):

        self.assertEqual(self.beam_centre_model.max_iterations, 10)
        self.assertEqual(self.beam_centre_model.r_min, 60)
        self.assertEqual(self.beam_centre_model.r_max, 280)
        self.assertEqual(self.beam_centre_model.left_right, True)
        self.assertEqual(self.beam_centre_model.up_down, True)
        self.assertEqual(self.beam_centre_model.tolerance, 0.000125)
        self.assertEqual(self.beam_centre_model.lab_pos_1, '')
        self.assertEqual(self.beam_centre_model.lab_pos_2, '')
        self.assertEqual(self.beam_centre_model.hab_pos_2, '')
        self.assertEqual(self.beam_centre_model.hab_pos_1, '')
        self.assertEqual(self.beam_centre_model.scale_1, 1000)
        self.assertEqual(self.beam_centre_model.scale_2, 1000)
        self.assertEqual(self.beam_centre_model.COM, False)
        self.assertEqual(self.beam_centre_model.verbose, False)
        self.assertEqual(self.beam_centre_model.q_min, 0.01)
        self.assertEqual(self.beam_centre_model.q_max, 0.1)

    def test_that_can_update_model_values(self):
        self.beam_centre_model.scale_2 = 1.0

        self.assertEqual(self.beam_centre_model.scale_2, 1.0)

    def test_that_correct_values_are_set_for_LARMOR(self):
        self.beam_centre_model.reset_to_defaults_for_instrument(SANSInstrument.LARMOR)

        self.assertEqual(self.beam_centre_model.scale_1, 1.0)

    def test_that_correct_values_are_set_for_LOQ(self):
        self.beam_centre_model.reset_to_defaults_for_instrument(SANSInstrument.LOQ)

        self.assertEqual(self.beam_centre_model.r_max, 200)

    def test_that_find_beam_centre_calls_centre_finder_once_when_COM_is_False(self):
        state = mock.MagicMock()

        self.beam_centre_model.find_beam_centre(state)

        self.SANSCentreFinder.return_value.assert_called_once_with(state, r_min=self.beam_centre_model.r_min,
                                                                   r_max=self.beam_centre_model.r_max,
                                                                   max_iter= self.beam_centre_model.max_iterations,
                                                                   x_start=self.beam_centre_model.lab_pos_1,
                                                                   y_start=self.beam_centre_model.lab_pos_2,
                                                                   tolerance=self.beam_centre_model.tolerance,
                                                                   find_direction=FindDirectionEnum.All,
                                                                   reduction_method=True,
                                                                   verbose=False)

        self.assertEqual(state.convert_to_q.q_min, self.beam_centre_model.q_min)
        self.assertEqual(state.convert_to_q.q_max, self.beam_centre_model.q_max)

    def test_that_find_beam_centre_calls_centre_finder_twice_when_COM_is_TRUE(self):
        state = mock.MagicMock()
        self.beam_centre_model.COM = True

        self.beam_centre_model.find_beam_centre(state)

        self.assertEqual(self.SANSCentreFinder.return_value.call_count, 2)

        self.SANSCentreFinder.return_value.assert_called_with(state, r_min=self.beam_centre_model.r_min,
                                                              r_max=self.beam_centre_model.r_max,
                                                              max_iter= self.beam_centre_model.max_iterations,
                                                              x_start=self.result['pos1'],
                                                              y_start=self.result['pos2'],
                                                              tolerance=self.beam_centre_model.tolerance,
                                                              find_direction=FindDirectionEnum.All,
                                                              reduction_method=True,
                                                              verbose=False)

        self.SANSCentreFinder.return_value.assert_any_call(state, r_min=self.beam_centre_model.r_min,
                                                           r_max=self.beam_centre_model.r_max,
                                                           max_iter=self.beam_centre_model.max_iterations,
                                                           x_start=self.beam_centre_model.lab_pos_1,
                                                           y_start=self.beam_centre_model.lab_pos_2,
                                                           tolerance=self.beam_centre_model.tolerance,
                                                           find_direction=FindDirectionEnum.All,
                                                           reduction_method=False)


if __name__ == '__main__':
    unittest.main()
