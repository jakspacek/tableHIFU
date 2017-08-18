'''
Copyright (c) 2012, Thunder Bay Regional Research Institute
Samuel Pichardo, Benjamin Zaporzan
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THUNDER BAY REGIONAL RESEARCH INSTITUTE
 SAMUEL PICHARDO OR BENJAMIN ZAPORZAN BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


This Software is to be used only for research purposes
in a preclinical setting.


This license only covers the Matlab and Python files including this license. All binary files required to run this library belong to their respective owners.

We provide this software in the spirit of open source. We will truly appreciate if users of this library can provide us any correction or enhancement to the functionality of the library. The GNU LESSER GENERAL PUBLIC LICENSE allows end users to develop their applications with permissible rights. Please send any modification to library itself to Samuel Pichardo (spichard@lakeheadu.ca) or Benjamin Zaporzan (btzaporz@lakeheadu.ca).

If you find matHIFU useful for your research,
we will truly appreciate a proper reference and
a mention in the acknowledgement section would be
highly appreciated.
Referencing and mentioning in the acknowledgements
 will help us to assess the impact of this tool and
 keep the development to add new features.

Please use this reference:

T. Sinclair, C. Mougenot, J. Kivinen, S. Pichardo.
"matHIFU and MatHIFU: Matlab toolboxes for real-time monitoring and control of MR-HIFU.". 2012. 12th International Society for Therapeutic Ultrasound Symposium,

Includes the primary messaging protocol to be used by the server and
client communication. The messaging protocol i'm going to use is the
JSON messaging protocol, which should simplify interactions due to the
close data structure of a dictionary key-value pair.
'''
#import config_server

import clr
clr.AddReference("Jayrock.Json")
import Jayrock.Json
from Jayrock.Json.Conversion import JsonConvert

#######################################
########### Configuration #############
#######################################



#######################################
############## Begin ##################
#######################################
"""
class JSON_MessageProtocol:
    def __init__(self):
        pass

    def toString(self, data):
        '''
        converts a python data structure into the respective json
        string format
        '''
        jsonString = JsonConvert.ExportToString(data)
        return jsonString

    def fromString(self, string):
        '''
        converts the json string back into the respective data type
        #TODO, convert into generic int, float, str, dict, list object
        types.
        '''
        returnedData_json = JsonConvert.Import(string)
        returnedData = json_convertToPython(returnedData_json)
        return returnedData

def json_convertToPython(obj):
    '''
    Performs a full conversion of any particular data structure in
    JSON into a fully python data structure. (recursive)
    '''
    if isJson_dict(obj):
        return json_convertToDict(obj)
    elif isJson_bool(obj):
        return json_convertToBool(obj)
    elif isJson_array(obj):
        return json_convertToList(obj)
    elif isJson_string(obj):
        return json_convertToString(obj)
    elif isJson_number(obj):
        return json_convertToNumber(obj)
    elif type(obj) in [str, float, int, list, dict]:
        return obj
    else:
        eString = 'Unknown type! - %s' % type(obj)
        raise Exception(eString)

def json_convertToDict(obj):
    '''
    Used to convert a Jayrock.Json.JsonObject file into a
    respective python dictionary
    '''
    keyValueArray = obj.GetNamesArray()
    obj_conv = dict()
    for i in keyValueArray:
        obj_conv[i] = json_convertToPython(obj[i])
    return obj_conv

def isJson_dict(obj):
    '''
    Checks to see if an object is an instance of
    Jayrock.Json.JsonObject
    '''
    if type(obj) == Jayrock.Json.JsonObject:
        return True
    else:
        return False

def json_convertToNumber(obj):
    '''
    Used to convert a Jayrock.Json.JsonNumber into a number
    '''
    return float(obj)

def isJson_number(obj):
    '''
    Checks to see if an object is an instance of
    Jayrock.Json.JsonNumber
    '''
    if type(obj) == Jayrock.Json.JsonNumber:
        return True
    else:
        return False

def json_convertToString(obj):
    '''
    Used to convert a Jayrock.Json.JsonString into a python string
    '''
    return str(obj)

def isJson_string(obj):
    '''
    Checks to see if an object is an instance of
    Jayrock.Json.JsonString
    '''
    if type(obj) == Jayrock.Json.JsonString:
        return True
    else:
        return False

def json_convertToList(obj):
    '''
    Used to convert a Jayrock.Json.JsonArray into a python list
    '''
    return [ json_convertToPython(i) for i in list(obj) ]

def isJson_array(obj):
    '''
    Checks to see if it is an instance of a Jayrock.Json.JsonArray
    '''
    if type(obj) == Jayrock.Json.JsonArray:
        return True
    else:
        return False

def json_convertToBool(obj):
    '''
    Converts a Jayrock.Json.JsonBoolean into a python bool
    '''
    return bool(obj)

def isJson_bool(obj):
    '''
    Checks to see if it is an instance of Jayrock.Json.JsonBoolean
    '''
    if type(obj) == Jayrock.Json.JsonBoolean:
        return True
    else:
        return False
"""
