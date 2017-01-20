from __future__ import (absolute_import, division, print_function)
from recon.helper import Helper
from recon.filters.crop_coords import _crop_coords_sanity_checks


def execute(data, air_region, region_of_interest, crop_before_normalise, h=None):
    """
    normalise by beam intensity. This is not directly about proton
    charg - not using the proton charge field as usually found in
    experiment/nexus files. This uses an area of normalization, if
    provided in the pre-processing configuration. TODO: much
    of this method should be moved into filters.

    :param data :: stack of images as a 3d numpy array
    :param region_of_interest: Region of interest to ensure that the Air Region is in bounds
    :param crop_before_normalise: A switch to signify that the image has been cropped.
            This means the Air Region coordinates will have to be translated onto the cropped image
    :param air_region: The air region from which sums will be calculated and all images will be normalised
    :param h: Helper class, if not provided will be initialised with empty constructor


    :returns :: filtered data (stack of images)

    """
    import numpy as np
    h = Helper.empty_init() if h is None else h

    h.check_data_stack(data)

    if air_region:
        _crop_coords_sanity_checks(air_region, data)
        _crop_coords_sanity_checks(region_of_interest, data)

        air_right, air_top, air_left, air_bottom = translate_coords_onto_cropped_picture(
            region_of_interest, air_region, crop_before_normalise)

        h.pstart("Starting normalization by air region...")
        air_sums = []
        for idx in range(0, data.shape[0]):
            air_data_sum = data[
                idx, air_top:air_bottom, air_left:air_right].sum()
            air_sums.append(air_data_sum)

        air_sums = np.true_divide(air_sums, np.amax(air_sums))

        for idx in range(0, data.shape[0]):
            data[idx, :, :] = np.true_divide(data[idx, :, :],
                                             air_sums[idx])

        avg = np.average(air_sums)
        max_avg = np.max(air_sums) / avg
        min_avg = np.min(air_sums) / avg

        h.pstop(
            "Finished normalization by air region. Average: {0}, max ratio: {1}, min ratio: {2}.".
            format(avg, max_avg, min_avg))

    else:
        h.tomo_print_note(
            "NOT normalizing by air region, because no --air-region coordinates were given.")

    h.check_data_stack(data)
    return data


def translate_coords_onto_cropped_picture(crop_coords, air_region, crop_before_normalise):

    air_right = air_region[2]
    air_top = air_region[1]
    air_left = air_region[0]
    air_bottom = air_region[3]

    crop_right = crop_coords[2]
    crop_top = crop_coords[1]
    crop_left = crop_coords[0]
    crop_bottom = crop_coords[3]

    _check_air_region_in_bounds(air_bottom, air_left, air_right, air_top,
                                crop_bottom, crop_left, crop_right, crop_top)

    if not crop_before_normalise:
        return air_right, air_top, air_left, air_bottom

    # Translate the air region coordinates to the crop.
    air_right -= crop_left
    air_top -= crop_top
    air_left -= crop_left
    air_bottom -= crop_top

    _check_air_region_in_bounds(air_bottom, air_left, air_right, air_top,
                                crop_bottom, crop_left, crop_right, crop_top)

    return air_right, air_top, air_left, air_bottom


def _check_air_region_in_bounds(air_bottom, air_left, air_right, air_top,
                                crop_bottom, crop_left, crop_right, crop_top):
    # sanity check just in case
    if air_top < crop_top or \
            air_bottom > crop_bottom or \
            air_left < crop_left or \
            air_right > crop_right:
        raise ValueError(
            "Selected air region is outside of the cropped data range.")