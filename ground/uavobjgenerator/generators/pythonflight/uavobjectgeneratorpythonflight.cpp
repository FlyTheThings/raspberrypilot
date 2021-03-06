/**
 ******************************************************************************
 *
 * @file       uavobjectgeneratorpython.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      produce python code for uavobjects
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "uavobjectgeneratorpythonflight.h"
using namespace std;

bool UAVObjectGeneratorPythonFlight::generate(UAVObjectParser* parser,QString templatepath,QString outputpath) {
    // Load template and setup output directory
    pythonCodePath = QDir( templatepath + QString("flight/python/raspberrypilot/templates"));
    pythonOutputPath = QDir( outputpath + QString("pythonflight") );
    pythonOutputPath.mkpath(pythonOutputPath.absolutePath());
    pythonCodeTemplate = readFile( pythonCodePath.absoluteFilePath("uavobjecttemplate.pyt") );
    if (pythonCodeTemplate.isEmpty()) {
        std::cerr << "Problem reading python flight templates" << endl;
        return false;
    }

    // Process each object
    for (int objidx = 0; objidx < parser->getNumObjects(); ++objidx) {
        ObjectInfo* info=parser->getObjectByIndex(objidx);
        process_object(info);
    }

    // Generate setup.py
    QString setup;
    setup += "from distutils.core import setup \n\r";
    setup += "import glob \n\r";
    setup += "setup(name='OpenPilot/RaspberryPilot UAV Objects', \n\r";
    setup += "		      version='1.0', \n\r";
    setup += "		      description='OpenPilot UAV Objects', \n\r";
    setup += "		      url='http://code.google.com/p/raspberrypilot', \n\r";
    setup += "		      package_dir={'raspberrypilot.uavobjects': '.'}, \n\r";
    setup += "		      packages=['raspberrypilot.uavobjects'], \n\r";
    setup += "		     ) \n\r";

    bool res = writeFileIfDiffrent( pythonOutputPath.absolutePath() + "/" + "setup.py", setup );
	if (!res) {
		cout << "Error: Could not write setup.py" << endl;
		return false;
	}

    // Generate setup.py
    QString init;
    init = "";
    for (int objidx = 0; objidx < parser->getNumObjects(); ++objidx) {
        ObjectInfo* info=parser->getObjectByIndex(objidx);
        init += "from " + info->namelc + " import *\n\r";

    }
    res = writeFileIfDiffrent( pythonOutputPath.absolutePath() + "/" + "__init__.py", init );
	if (!res) {
		cout << "Error: Could not write __init__.py" << endl;
		return false;
	}

    return true; // if we come here everything should be fine
}

/**
 * Generate the python object files
 */
bool UAVObjectGeneratorPythonFlight::process_object(ObjectInfo* info)
{
    if (info == NULL)
        return false;

    // Prepare output strings
    QString outCode = pythonCodeTemplate;

    // Replace common tags
    replaceCommonTags(outCode, info);

    // Replace the ($DATAFIELDS) tag
    QString datafields;
    for (int n = 0; n < info->fields.length(); ++n)
    {
        // Class header
        datafields.append(QString("# Field %1 definition\n").arg(info->fields[n]->name));
        datafields.append(QString("class %1Field(uavlink.uavObjectField):\n").arg(info->fields[n]->name));
        datafields.append(QString("\tname = '%1'\n").arg(info->fields[n]->name));
        // Only for enum types
        if (info->fields[n]->type == FIELDTYPE_ENUM)
        {
            datafields.append(QString("\t# Enumeration options\n"));
            // Go through each option
            QStringList options = info->fields[n]->options;
            datafields.append(QString("\tenums = ["));
            for (int m = 0; m < options.length(); ++m) {
                QString name = options[m].toUpper().replace(QRegExp(ENUM_SPECIAL_CHARS), "");
                if (name[0].isDigit())
                    name = QString("N%1").arg(name);
                datafields.append(QString("\"%1\", ").arg(name));
            }
            datafields.append(QString("]\n"));
        }
        // Generate element names (only if field has more than one element)
        if (info->fields[n]->numElements > 1 && !info->fields[n]->defaultElementNames)
        {
            datafields.append(QString("\t# Array element names\n"));
            // Go through the element names
            QStringList elemNames = info->fields[n]->elementNames;
            for (int m = 0; m < elemNames.length(); ++m)
            {
                QString name = elemNames[m].toUpper().replace(QRegExp(ENUM_SPECIAL_CHARS), "");
                if (name[0].isDigit())
                    name = QString("N%1").arg(name);
                datafields.append(QString("\t%1 = %2\n").arg(name).arg(m));
            }
        }
        // Constructor
        datafields.append(QString("\tdef __init__(self,obj):\n"));
        datafields.append(QString("\t\tuavlink.uavObjectField.__init__(self,obj, %1, %2)\n\n").arg(info->fields[n]->type).arg(info->fields[n]->numElements));
    }
    outCode.replace(QString("$(DATAFIELDS)"), datafields);

    // Replace the $(PROPERTYFUNCTIONS) tag
    QString properties;
    for (int n = 0; n < info->fields.length(); ++n)
    {
        // The property
        properties.append(QString("\t@property\n"));
        properties.append(QString("\tdef %1(self):\n").arg(info->fields[n]->name));
        properties.append(QString("\t\treturn self._%1.getValue()\n").arg(info->fields[n]->name));
        //the property setter
        properties.append(QString("\t@%1.setter\n").arg(info->fields[n]->name));
        properties.append(QString("\tdef %1(self,value):\n").arg(info->fields[n]->name));
        properties.append(QString("\t\tself._%1.setValue(value)\n\n").arg(info->fields[n]->name));
    }
    outCode.replace(QString("$(PROPERTYFUNCTIONS)"), properties);

    // Replace the $(DATAFIELDINIT) tag
    QString fields;
    for (int n = 0; n < info->fields.length(); ++n)
    {
        fields.append(QString("\t\tself._%1 = %1Field(self)\n").arg(info->fields[n]->name));
        fields.append(QString("\t\tself.addField(self._%1)\n").arg(info->fields[n]->name));
    }
    outCode.replace(QString("$(DATAFIELDINIT)"), fields);

    // Replace the $(INITFIELDS) tag
    QString initfields;
    for (int n = 0; n < info->fields.length(); ++n)
    {
        if (!info->fields[n]->defaultValues.isEmpty() )
        {
            // For non-array fields
            if ( info->fields[n]->numElements == 1)
            {
                if ( info->fields[n]->type == FIELDTYPE_ENUM )
                {
                    initfields.append( QString("\t\tself.%1 = %2\r\n")
                                .arg( info->fields[n]->name )
                                .arg( info->fields[n]->options.indexOf( info->fields[n]->defaultValues[0] ) ) );
                }
                else if ( info->fields[n]->type == FIELDTYPE_FLOAT32 )
                {
                    initfields.append( QString("\t\tself.%1 = %2\r\n")
                                .arg( info->fields[n]->name )
                                .arg( info->fields[n]->defaultValues[0].toFloat() ) );
                }
                else
                {
                    initfields.append( QString("\t\tself.%1 = %2\r\n")
                                .arg( info->fields[n]->name )
                                .arg( info->fields[n]->defaultValues[0].toInt() ) );
                }
            }
            else
            {
                initfields.append( QString("\t\tself.%1 = []\r\n")
                            .arg( info->fields[n]->name ) );
                // Initialize all fields in the array
                for (int idx = 0; idx < info->fields[n]->numElements; ++idx)
                {
                    if ( info->fields[n]->type == FIELDTYPE_ENUM )
                    {
                        initfields.append( QString("\t\tself.%1.append('%2')\r\n")
                                    .arg( info->fields[n]->name )
                                    .arg( info->fields[n]->options.indexOf( info->fields[n]->defaultValues[idx] ) ) );
                    }
                    else if ( info->fields[n]->type == FIELDTYPE_FLOAT32 )
                    {
                        initfields.append( QString("\t\tself.%1.append(%2)\r\n")
                                    .arg( info->fields[n]->name )
                                    .arg( info->fields[n]->defaultValues[idx].toFloat() ) );
                    }
                    else
                    {
                        initfields.append( QString("\t\tself.%1.append(%2)\r\n")
                                    .arg( info->fields[n]->name )
                                    .arg( info->fields[n]->defaultValues[idx].toInt() ) );
                    }
                }
            }
        } else {
            // For non-array fields
            if ( info->fields[n]->numElements == 1)
            {
                if ( info->fields[n]->type == FIELDTYPE_ENUM )
                {
                    initfields.append( QString("\t\tself.%1 = %2\r\n")
                                .arg( info->fields[n]->name )
                                .arg( QString("0") ) );
                }
                else if ( info->fields[n]->type == FIELDTYPE_FLOAT32 )
                {
                    initfields.append( QString("\t\tself.%1 = %2\r\n")
                                .arg( info->fields[n]->name )
                                .arg( QString("0") ) );
                }
                else
                {
                    initfields.append( QString("\t\tself.%1 = %2\r\n")
                                .arg( info->fields[n]->name )
                                .arg(  QString("0") ) );
                }
            }
            else
            {
                initfields.append( QString("\t\tself.%1 = []\r\n")
                            .arg( info->fields[n]->name ) );
                // Initialize all fields in the array
                for (int idx = 0; idx < info->fields[n]->numElements; ++idx)
                {
                    if ( info->fields[n]->type == FIELDTYPE_ENUM )
                    {
                        initfields.append( QString("\t\tself.%1.append(%2)\r\n")
                                    .arg( info->fields[n]->name )
                                    .arg(  QString("0") ) );
                    }
                    else if ( info->fields[n]->type == FIELDTYPE_FLOAT32 )
                    {
                        initfields.append( QString("\t\tself.%1.append(%2)\r\n")
                                    .arg( info->fields[n]->name )
                                    .arg(  QString("0") ) );
                    }
                    else
                    {
                        initfields.append( QString("\t\tself.%1.append(%2)\r\n")
                                    .arg( info->fields[n]->name )
                                    .arg(  QString("0") ) );
                    }
                }
            }
        }
    }
    outCode.replace(QString("$(INITFIELDS)"), initfields);

    // Write the Python code
    bool res = writeFileIfDiffrent( pythonOutputPath.absolutePath() + "/" + info->namelc + ".py", outCode );
    if (!res) {
        cout << "Error: Could not write Python output files" << endl;
        return false;
    }

    return true;
}

