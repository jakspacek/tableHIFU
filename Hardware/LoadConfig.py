from __future__ import print_function, absolute_import

from future.utils import iteritems
import os
import yaml
import collections

import logging
from Proteus.ThermometryLibrary.ThermometryLib import LOGGER_NAME
logger = logging.getLogger(LOGGER_NAME)

def LoadConfiguration(bVerbose=True):
    configuration=[]
    try:
        with open(os.path.join(os.path.dirname(os.path.realpath(__file__)),'../configuration_files/default.yaml'), 'r') as file:
            try:
                configuration = yaml.safe_load(file)
            except yaml.YAMLError:
                if bVerbose:
                    logger.warning( 'Could not read the default yaml configuration.')
                raise
    except IOError:
        if bVerbose:
            logger.warning('Could not find the default yaml configuration (configuration_files/default.yaml).')
        raise
    #
    try:
        with open(os.path.join(os.path.dirname(os.path.realpath(__file__)),'../configuration_files/local.yaml'), 'r') as file:
            try:
                update_dict_recursive(configuration, yaml.safe_load(file))
            except yaml.YAMLError:
                if bVerbose:
                    logger.warning('Could not read the local yaml configuration.')
                raise
        if bVerbose:
            logger.info(' configuration_files/local.yaml found and loaded with ' + str(configuration) )
    except IOError:
        if bVerbose:
            logger.warning('Could not find a local yaml configuration (configuration_files/local.yaml).')
    return configuration

def update_dict_recursive( d, u):
    # taken from here: http://stackoverflow.com/a/3233356
    for (k, v) in iteritems(u):
        if isinstance(v, collections.Mapping):
            r = update_dict_recursive(d.get(k, {}), v)
            d[k] = r
        else:
            d[k] = u[k]
    return d
