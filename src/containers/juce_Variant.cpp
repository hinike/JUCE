/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Variant.h"
#include "juce_DynamicObject.h"
#include "../io/streams/juce_MemoryOutputStream.h"

enum VariantStreamMarkers
{
    varMarker_Int       = 1,
    varMarker_BoolTrue  = 2,
    varMarker_BoolFalse = 3,
    varMarker_Double    = 4,
    varMarker_String    = 5,
    varMarker_Int64     = 6
};

//==============================================================================
class var::VariantType
{
public:
    VariantType() noexcept {}
    virtual ~VariantType() noexcept {}

    virtual int toInt (const ValueUnion&) const noexcept                        { return 0; }
    virtual int64 toInt64 (const ValueUnion&) const noexcept                    { return 0; }
    virtual double toDouble (const ValueUnion&) const noexcept                  { return 0; }
    virtual String toString (const ValueUnion&) const                           { return String::empty; }
    virtual bool toBool (const ValueUnion&) const noexcept                      { return false; }
    virtual ReferenceCountedObject* toObject (const ValueUnion&) const noexcept { return nullptr; }

    virtual bool isVoid() const noexcept      { return false; }
    virtual bool isInt() const noexcept       { return false; }
    virtual bool isInt64() const noexcept     { return false; }
    virtual bool isBool() const noexcept      { return false; }
    virtual bool isDouble() const noexcept    { return false; }
    virtual bool isString() const noexcept    { return false; }
    virtual bool isObject() const noexcept    { return false; }
    virtual bool isMethod() const noexcept    { return false; }

    virtual void cleanUp (ValueUnion&) const noexcept {}
    virtual void createCopy (ValueUnion& dest, const ValueUnion& source) const      { dest = source; }
    virtual bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept = 0;
    virtual void writeToStream (const ValueUnion& data, OutputStream& output) const = 0;
};

//==============================================================================
class var::VariantType_Void  : public var::VariantType
{
public:
    VariantType_Void() noexcept {}
    static const VariantType_Void instance;

    bool isVoid() const noexcept    { return true; }
    bool equals (const ValueUnion&, const ValueUnion&, const VariantType& otherType) const noexcept { return otherType.isVoid(); }
    void writeToStream (const ValueUnion&, OutputStream& output) const   { output.writeCompressedInt (0); }
};

//==============================================================================
class var::VariantType_Int  : public var::VariantType
{
public:
    VariantType_Int() noexcept {}
    static const VariantType_Int instance;

    int toInt (const ValueUnion& data) const noexcept       { return data.intValue; };
    int64 toInt64 (const ValueUnion& data) const noexcept   { return (int64) data.intValue; };
    double toDouble (const ValueUnion& data) const noexcept { return (double) data.intValue; }
    String toString (const ValueUnion& data) const          { return String (data.intValue); }
    bool toBool (const ValueUnion& data) const noexcept     { return data.intValue != 0; }
    bool isInt() const noexcept                             { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.toInt (otherData) == data.intValue;
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (5);
        output.writeByte (varMarker_Int);
        output.writeInt (data.intValue);
    }
};

//==============================================================================
class var::VariantType_Int64  : public var::VariantType
{
public:
    VariantType_Int64() noexcept {}
    static const VariantType_Int64 instance;

    int toInt (const ValueUnion& data) const noexcept       { return (int) data.int64Value; };
    int64 toInt64 (const ValueUnion& data) const noexcept   { return data.int64Value; };
    double toDouble (const ValueUnion& data) const noexcept { return (double) data.int64Value; }
    String toString (const ValueUnion& data) const          { return String (data.int64Value); }
    bool toBool (const ValueUnion& data) const noexcept     { return data.int64Value != 0; }
    bool isInt64() const noexcept                           { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.toInt64 (otherData) == data.int64Value;
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (9);
        output.writeByte (varMarker_Int64);
        output.writeInt64 (data.int64Value);
    }
};

//==============================================================================
class var::VariantType_Double   : public var::VariantType
{
public:
    VariantType_Double() noexcept {}
    static const VariantType_Double instance;

    int toInt (const ValueUnion& data) const noexcept       { return (int) data.doubleValue; };
    int64 toInt64 (const ValueUnion& data) const noexcept   { return (int64) data.doubleValue; };
    double toDouble (const ValueUnion& data) const noexcept { return data.doubleValue; }
    String toString (const ValueUnion& data) const          { return String (data.doubleValue); }
    bool toBool (const ValueUnion& data) const noexcept     { return data.doubleValue != 0; }
    bool isDouble() const noexcept                          { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.toDouble (otherData) == data.doubleValue;
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (9);
        output.writeByte (varMarker_Double);
        output.writeDouble (data.doubleValue);
    }
};

//==============================================================================
class var::VariantType_Bool   : public var::VariantType
{
public:
    VariantType_Bool() noexcept {}
    static const VariantType_Bool instance;

    int toInt (const ValueUnion& data) const noexcept       { return data.boolValue ? 1 : 0; };
    int64 toInt64 (const ValueUnion& data) const noexcept   { return data.boolValue ? 1 : 0; };
    double toDouble (const ValueUnion& data) const noexcept { return data.boolValue ? 1.0 : 0.0; }
    String toString (const ValueUnion& data) const          { return String::charToString (data.boolValue ? '1' : '0'); }
    bool toBool (const ValueUnion& data) const noexcept     { return data.boolValue; }
    bool isBool() const noexcept                            { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.toBool (otherData) == data.boolValue;
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (1);
        output.writeByte (data.boolValue ? (char) varMarker_BoolTrue : (char) varMarker_BoolFalse);
    }
};

//==============================================================================
class var::VariantType_String   : public var::VariantType
{
public:
    VariantType_String() noexcept {}
    static const VariantType_String instance;

    void cleanUp (ValueUnion& data) const noexcept                       { getString (data)-> ~String(); }
    void createCopy (ValueUnion& dest, const ValueUnion& source) const   { new (dest.stringValue) String (*getString (source)); }

    bool isString() const noexcept                          { return true; }
    int toInt (const ValueUnion& data) const noexcept       { return getString (data)->getIntValue(); };
    int64 toInt64 (const ValueUnion& data) const noexcept   { return getString (data)->getLargeIntValue(); };
    double toDouble (const ValueUnion& data) const noexcept { return getString (data)->getDoubleValue(); }
    String toString (const ValueUnion& data) const          { return *getString (data); }
    bool toBool (const ValueUnion& data) const noexcept     { return getString (data)->getIntValue() != 0
                                                                      || getString (data)->trim().equalsIgnoreCase ("true")
                                                                      || getString (data)->trim().equalsIgnoreCase ("yes"); }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.toString (otherData) == *getString (data);
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        const String* const s = getString (data);
        const int len = s->getNumBytesAsUTF8() + 1;
        HeapBlock<char> temp (len);
        s->copyToUTF8 (temp, len);
        output.writeCompressedInt (len + 1);
        output.writeByte (varMarker_String);
        output.write (temp, len);
    }

private:
    static inline const String* getString (const ValueUnion& data) noexcept { return reinterpret_cast <const String*> (data.stringValue); }
    static inline String* getString (ValueUnion& data) noexcept             { return reinterpret_cast <String*> (data.stringValue); }
};

//==============================================================================
class var::VariantType_Object   : public var::VariantType
{
public:
    VariantType_Object() noexcept {}
    static const VariantType_Object instance;

    void cleanUp (ValueUnion& data) const noexcept                      { if (data.objectValue != nullptr) data.objectValue->decReferenceCount(); }
    void createCopy (ValueUnion& dest, const ValueUnion& source) const  { dest.objectValue = source.objectValue; if (dest.objectValue != nullptr) dest.objectValue->incReferenceCount(); }

    String toString (const ValueUnion& data) const                            { return "Object 0x" + String::toHexString ((int) (pointer_sized_int) data.objectValue); }
    bool toBool (const ValueUnion& data) const noexcept                       { return data.objectValue != 0; }
    ReferenceCountedObject* toObject (const ValueUnion& data) const noexcept  { return data.objectValue; }
    bool isObject() const noexcept                                            { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.toObject (otherData) == data.objectValue;
    }

    void writeToStream (const ValueUnion&, OutputStream& output) const
    {
        jassertfalse; // Can't write an object to a stream!
        output.writeCompressedInt (0);
    }
};

//==============================================================================
class var::VariantType_Method   : public var::VariantType
{
public:
    VariantType_Method() noexcept {}
    static const VariantType_Method instance;

    String toString (const ValueUnion&) const               { return "Method"; }
    bool toBool (const ValueUnion& data) const noexcept     { return data.methodValue != 0; }
    bool isMethod() const noexcept                          { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const noexcept
    {
        return otherType.isMethod() && otherData.methodValue == data.methodValue;
    }

    void writeToStream (const ValueUnion&, OutputStream& output) const
    {
        jassertfalse; // Can't write a method to a stream!
        output.writeCompressedInt (0);
    }
};

//==============================================================================
const var::VariantType_Void    var::VariantType_Void::instance;
const var::VariantType_Int     var::VariantType_Int::instance;
const var::VariantType_Int64   var::VariantType_Int64::instance;
const var::VariantType_Bool    var::VariantType_Bool::instance;
const var::VariantType_Double  var::VariantType_Double::instance;
const var::VariantType_String  var::VariantType_String::instance;
const var::VariantType_Object  var::VariantType_Object::instance;
const var::VariantType_Method  var::VariantType_Method::instance;


//==============================================================================
var::var() noexcept : type (&VariantType_Void::instance)
{
}

var::~var() noexcept
{
    type->cleanUp (value);
}

const var var::null;

//==============================================================================
var::var (const var& valueToCopy)  : type (valueToCopy.type)
{
    type->createCopy (value, valueToCopy.value);
}

var::var (const int value_) noexcept : type (&VariantType_Int::instance)
{
    value.intValue = value_;
}

var::var (const int64 value_) noexcept : type (&VariantType_Int64::instance)
{
    value.int64Value = value_;
}

var::var (const bool value_) noexcept : type (&VariantType_Bool::instance)
{
    value.boolValue = value_;
}

var::var (const double value_) noexcept : type (&VariantType_Double::instance)
{
    value.doubleValue = value_;
}

var::var (const String& value_)  : type (&VariantType_String::instance)
{
    new (value.stringValue) String (value_);
}

var::var (const char* const value_)  : type (&VariantType_String::instance)
{
    new (value.stringValue) String (value_);
}

var::var (const wchar_t* const value_)  : type (&VariantType_String::instance)
{
    new (value.stringValue) String (value_);
}

var::var (ReferenceCountedObject* const object)  : type (&VariantType_Object::instance)
{
    value.objectValue = object;

    if (object != nullptr)
        object->incReferenceCount();
}

var::var (MethodFunction method_) noexcept : type (&VariantType_Method::instance)
{
    value.methodValue = method_;
}

//==============================================================================
bool var::isVoid() const noexcept     { return type->isVoid(); }
bool var::isInt() const noexcept      { return type->isInt(); }
bool var::isInt64() const noexcept    { return type->isInt64(); }
bool var::isBool() const noexcept     { return type->isBool(); }
bool var::isDouble() const noexcept   { return type->isDouble(); }
bool var::isString() const noexcept   { return type->isString(); }
bool var::isObject() const noexcept   { return type->isObject(); }
bool var::isMethod() const noexcept   { return type->isMethod(); }

var::operator int() const noexcept                      { return type->toInt (value); }
var::operator int64() const noexcept                    { return type->toInt64 (value); }
var::operator bool() const noexcept                     { return type->toBool (value); }
var::operator float() const noexcept                    { return (float) type->toDouble (value); }
var::operator double() const noexcept                   { return type->toDouble (value); }
String var::toString() const                            { return type->toString (value); }
var::operator String() const                            { return type->toString (value); }
ReferenceCountedObject* var::getObject() const noexcept { return type->toObject (value); }
DynamicObject* var::getDynamicObject() const noexcept   { return dynamic_cast <DynamicObject*> (getObject()); }

//==============================================================================
void var::swapWith (var& other) noexcept
{
    std::swap (type, other.type);
    std::swap (value, other.value);
}

const var& var::operator= (const var& newValue)               { type->cleanUp (value); type = newValue.type; type->createCopy (value, newValue.value); return *this; }
const var& var::operator= (const int newValue)                { type->cleanUp (value); type = &VariantType_Int::instance; value.intValue = newValue; return *this; }
const var& var::operator= (const int64 newValue)              { type->cleanUp (value); type = &VariantType_Int64::instance; value.int64Value = newValue; return *this; }
const var& var::operator= (const bool newValue)               { type->cleanUp (value); type = &VariantType_Bool::instance; value.boolValue = newValue; return *this; }
const var& var::operator= (const double newValue)             { type->cleanUp (value); type = &VariantType_Double::instance; value.doubleValue = newValue; return *this; }
const var& var::operator= (const char* const newValue)        { var v (newValue); swapWith (v); return *this; }
const var& var::operator= (const wchar_t* const newValue)     { var v (newValue); swapWith (v); return *this; }
const var& var::operator= (const String& newValue)            { var v (newValue); swapWith (v); return *this; }
const var& var::operator= (ReferenceCountedObject* newValue)  { var v (newValue); swapWith (v); return *this; }
const var& var::operator= (MethodFunction newValue)           { var v (newValue); swapWith (v); return *this; }

//==============================================================================
bool var::equals (const var& other) const noexcept
{
    return type->equals (value, other.value, *other.type);
}

bool var::equalsWithSameType (const var& other) const noexcept
{
    return type == other.type && equals (other);
}

bool operator== (const var& v1, const var& v2) noexcept     { return v1.equals (v2); }
bool operator!= (const var& v1, const var& v2) noexcept     { return ! v1.equals (v2); }
bool operator== (const var& v1, const String& v2)           { return v1.toString() == v2; }
bool operator!= (const var& v1, const String& v2)           { return v1.toString() != v2; }
bool operator== (const var& v1, const char* const v2)       { return v1.toString() == v2; }
bool operator!= (const var& v1, const char* const v2)       { return v1.toString() != v2; }


//==============================================================================
void var::writeToStream (OutputStream& output) const
{
    type->writeToStream (value, output);
}

var var::readFromStream (InputStream& input)
{
    const int numBytes = input.readCompressedInt();

    if (numBytes > 0)
    {
        switch (input.readByte())
        {
            case varMarker_Int:         return var (input.readInt());
            case varMarker_Int64:       return var (input.readInt64());
            case varMarker_BoolTrue:    return var (true);
            case varMarker_BoolFalse:   return var (false);
            case varMarker_Double:      return var (input.readDouble());
            case varMarker_String:
            {
                MemoryOutputStream mo;
                mo.writeFromInputStream (input, numBytes - 1);
                return var (mo.toUTF8());
            }

            default:    input.skipNextBytes (numBytes - 1); break;
        }
    }

    return var::null;
}

var var::operator[] (const Identifier& propertyName) const
{
    DynamicObject* const o = getDynamicObject();
    return o != nullptr ? o->getProperty (propertyName) : var::null;
}

var var::invoke (const Identifier& method, const var* arguments, int numArguments) const
{
    DynamicObject* const o = getDynamicObject();
    return o != nullptr ? o->invokeMethod (method, arguments, numArguments) : var::null;
}

var var::invokeMethod (DynamicObject* const target, const var* const arguments, const int numArguments) const
{
    jassert (target != nullptr);

    if (isMethod())
        return (target->*(value.methodValue)) (arguments, numArguments);

    return var::null;
}

var var::call (const Identifier& method) const
{
    return invoke (method, nullptr, 0);
}

var var::call (const Identifier& method, const var& arg1) const
{
    return invoke (method, &arg1, 1);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2) const
{
    var args[] = { arg1, arg2 };
    return invoke (method, args, 2);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3)
{
    var args[] = { arg1, arg2, arg3 };
    return invoke (method, args, 3);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const
{
    var args[] = { arg1, arg2, arg3, arg4 };
    return invoke (method, args, 4);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const
{
    var args[] = { arg1, arg2, arg3, arg4, arg5 };
    return invoke (method, args, 5);
}


END_JUCE_NAMESPACE
