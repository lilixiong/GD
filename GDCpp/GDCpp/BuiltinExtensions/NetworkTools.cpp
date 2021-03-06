/*
 * GDevelop C++ Platform
 * Copyright 2008-2015 Florian Rival (Florian.Rival@gmail.com). All rights reserved.
 * This project is released under the MIT License.
 */
#include <fstream>
#include <cstring>
#include <string>
#include <iomanip>
#include <SFML/Network.hpp>
#include "GDCpp/BuiltinExtensions/NetworkTools.h"
#include "GDCpp/RuntimeScene.h"
#include "GDCpp/Variable.h"
#include "GDCpp/Tools/md5.h"
#include "GDCpp/CommonTools.h"

using namespace std;

/**
 * Send data to a php web page.
 * To prevent sending hacked data ( like a modified hi score )
 * a md5 checksum is applied on the data+a password defined by the developer
 * Php web page has to known the same password, and will applied md5 ont on the data+the password to
 * be sure data were not modified.
*/
void GD_API SendDataToPhpWebPage(const std::string & webpageurl,
    const std::string & password,
    const std::string & data1,
    const std::string & data2,
    const std::string & data3,
    const std::string & data4,
    const std::string & data5,
    const std::string & data6)
{
    string data1md5 = md5(data1+password); //On leur ajoute le mot de passe
    string data2md5 = md5(data2+password); //Et on effectue la somme de contr�le
    string data3md5 = md5(data3+password);
    string data4md5 = md5(data4+password);
    string data5md5 = md5(data5+password);
    string data6md5 = md5(data6+password);

#ifdef WINDOWS
    //Cr�ation de l'adresse internet � lancer
    string call = "start \"\" \""+webpageurl+
                   "?data1="+data1+"&check1="+data1md5+
                   "&data2="+data2+"&check2="+data2md5+
                   "&data3="+data3+"&check3="+data3md5+
                   "&data4="+data4+"&check4="+data4md5+
                   "&data5="+data5+"&check5="+data5md5+
                   "&data6="+data6+"&check6="+data6md5+"\"";

    system(call.c_str());
#elif defined(LINUX)
    //N�cessite le paquet xdg-utils
    string call = "xdg-open \""+webpageurl+
                   "?data1="+data1+"&check1="+data1md5+
                   "&data2="+data2+"&check2="+data2md5+
                   "&data3="+data3+"&check3="+data3md5+
                   "&data4="+data4+"&check4="+data4md5+
                   "&data5="+data5+"&check5="+data5md5+
                   "&data6="+data6+"&check6="+data6md5+"\"";

    system(call.c_str());
#elif defined(MACOS)
    string call = "open \""+webpageurl+
                   "?data1="+data1+"&check1="+data1md5+
                   "&data2="+data2+"&check2="+data2md5+
                   "&data3="+data3+"&check3="+data3md5+
                   "&data4="+data4+"&check4="+data4md5+
                   "&data5="+data5+"&check5="+data5md5+
                   "&data6="+data6+"&check6="+data6md5+"\"";

    system(call.c_str());
#endif

    return;
}

void GD_API SendHttpRequest(const std::string & host, const std::string & uri, const std::string & body,
    const std::string & method, const std::string & contentType, gd::Variable & responseVar)
{
    // Create Http
    sf::Http Http;
    Http.setHost(host);

    // Create request
    sf::Http::Request request;
    request.setMethod(method == "POST" ? sf::Http::Request::Post : sf::Http::Request::Get);
    request.setField("Content-Type", contentType.empty() ? "application/x-www-form-urlencoded" : contentType);
    request.setUri(uri);
    request.setBody(body);

    // Send request & Get response
    sf::Http::Response response = Http.sendRequest(request);

    if (response.getStatus() == sf::Http::Response::Ok)
    {
        responseVar.SetString(response.getBody());
    }
    //else request failed.
}

void GD_API DownloadFile( const std::string & host, const std::string & uri, const std::string & outputfilename )
{
    // Create Http
    sf::Http Http;
    Http.setHost(host);

    // Create request
    sf::Http::Request Request;
    Request.setMethod(sf::Http::Request::Get);
    Request.setUri(uri);

    // Send request & Get response
    sf::Http::Response datas = Http.sendRequest(Request);

    ofstream ofile(outputfilename.c_str(), ios_base::binary);
    if ( ofile.is_open() )
    {
        ofile.write(datas.getBody().c_str(),datas.getBody().size());
        ofile.close();

        return;
    }

    cout << "Downloading file : Unable to open output file " << outputfilename;
    return;
}

//Private functions for JSON reading
namespace
{
    /**
     * Adapted from public domain library "jsoncpp" (http://sourceforge.net/projects/jsoncpp/).
     */
    static inline bool isControlCharacter(char ch)
    {
       return ch > 0 && ch <= 0x1F;
    }

    /**
     * Adapted from public domain library "jsoncpp" (http://sourceforge.net/projects/jsoncpp/).
     */
    static bool containsControlCharacter( const char* str )
    {
       while ( *str )
       {
          if ( isControlCharacter( *(str++) ) )
             return true;
       }
       return false;
    }

    /**
     * Tool function converting a string to a quoted string that can be inserted into
     * a JSON file.
     * Adapted from public domain library "jsoncpp" (http://sourceforge.net/projects/jsoncpp/).
     */
    std::string StringToQuotedJSONString( const char *value )
    {
       if (value == NULL)
          return "";
       // Not sure how to handle unicode...
       if (strpbrk(value, "\"\\\b\f\n\r\t") == NULL && !containsControlCharacter( value ))
          return std::string("\"") + value + "\"";
       // We have to walk value and escape any special characters.
       // Appending to std::string is not efficient, but this should be rare.
       // (Note: forward slashes are *not* rare, but I am not escaping them.)
       std::string::size_type maxsize = strlen(value)*2 + 3; // allescaped+quotes+NULL
       std::string result;
       result.reserve(maxsize); // to avoid lots of mallocs
       result += "\"";
       for (const char* c=value; *c != 0; ++c)
       {
          switch(*c)
          {
             case '\"':
                result += "\\\"";
                break;
             case '\\':
                result += "\\\\";
                break;
             case '\b':
                result += "\\b";
                break;
             case '\f':
                result += "\\f";
                break;
             case '\n':
                result += "\\n";
                break;
             case '\r':
                result += "\\r";
                break;
             case '\t':
                result += "\\t";
                break;
             //case '/':
                // Even though \/ is considered a legal escape in JSON, a bare
                // slash is also legal, so I see no reason to escape it.
                // (I hope I am not misunderstanding something.
                // blep notes: actually escaping \/ may be useful in javascript to avoid </
                // sequence.
                // Should add a flag to allow this compatibility mode and prevent this
                // sequence from occurring.
             default:
                if ( isControlCharacter( *c ) )
                {
                   std::ostringstream oss;
                   oss << "\\u" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << static_cast<int>(*c);
                   result += oss.str();
                }
                else
                {
                   result += *c;
                }
                break;
          }
       }
       result += "\"";
       return result;
    }
}

std::string GD_API VariableStructureToJSON(const gd::Variable & variable)
{
    if ( !variable.IsStructure() ) {
        if ( variable.IsNumber() )
            return ToString(variable.GetValue());
        else
            return "\""+StringToQuotedJSONString(variable.GetString().c_str())+"\"";
    }

    std::string str = "{";
    bool firstChild = true;
    for(std::map<std::string, gd::Variable>::const_iterator i = variable.GetAllChildren().begin();
        i != variable.GetAllChildren().end();++i)
    {
        if ( !firstChild ) str += ",";
        str += "\""+i->first+"\": "+VariableStructureToJSON(i->second);

        firstChild = false;
    }

    str += "}";
    return str;
}

//Private functions for JSON parsing
namespace
{
    size_t SkipBlankChar(const std::string & str, size_t pos)
    {
        const std::string blankChar = " \n";
        return str.find_first_not_of(blankChar, pos);
    }

    /**
     * Adapted from https://github.com/hjiang/jsonxx
     */
    std::string DecodeString(const std::string & original)
    {
        std::string value;
        value.reserve(original.size());
        std::istringstream input("\""+original+"\"");

        char ch = '\0', delimiter = '"';
        input.get(ch);
        if (ch != delimiter) return "";

        while(!input.eof() && input.good()) {
            input.get(ch);
            if (ch == delimiter) {
                break;
            }
            if (ch == '\\') {
                input.get(ch);
                switch(ch) {
                    case '\\':
                    case '/':
                        value.push_back(ch);
                        break;
                    case 'b':
                        value.push_back('\b');
                        break;
                    case 'f':
                        value.push_back('\f');
                        break;
                    case 'n':
                        value.push_back('\n');
                        break;
                    case 'r':
                        value.push_back('\r');
                        break;
                    case 't':
                        value.push_back('\t');
                        break;
                    case 'u': {
                            int i;
                            std::stringstream ss;
                            for( i = 0; (!input.eof() && input.good()) && i < 4; ++i ) {
                                input.get(ch);
                                ss << ch;
                            }
                            if( input.good() && (ss >> i) )
                                value.push_back(i);
                        }
                        break;
                    default:
                        if (ch != delimiter) {
                            value.push_back('\\');
                            value.push_back(ch);
                        } else value.push_back(ch);
                        break;
                }
            } else {
                value.push_back(ch);
            }
        }
        if (input && ch == delimiter) {
            return value;
        } else {
            return "";
        }
    }

    /**
     * Return the position of the end of the string. Blank are skipped if necessary
     * @param str The string to be used
     * @param startPos The start position
     * @param strContent A reference to a string that will be filled with the string content.
     */
    size_t SkipString(const std::string & str, size_t startPos, std::string & strContent)
    {
        startPos = SkipBlankChar(str, startPos);
        if ( startPos >= str.length() ) return std::string::npos;

        size_t endPos = startPos;

        if ( str[startPos] == '"' )
        {
            if ( startPos+1 >= str.length() ) return std::string::npos;

            while (endPos == startPos || (str[endPos-1] == '\\'))
            {
                endPos = str.find_first_of('\"', endPos+1);
                if ( endPos == std::string::npos ) return std::string::npos; //Invalid string
            }

            strContent = DecodeString(str.substr(startPos+1, endPos-1-startPos));
            return endPos;
        }

        endPos = str.find_first_of(" \n,:");
        if ( endPos >= str.length() ) return std::string::npos; //Invalid string

        strContent = DecodeString(str.substr(startPos, endPos-1-startPos));
        return endPos-1;
    }

    /**
     * Parse a JSON string, starting from pos, and storing the result into the specified variable.
     * Note that the parsing is stopped as soon as a valid object is parsed.
     * \return The position at the end of the valid object stored into the variable.
     */
    size_t ParseJSONObject(const std::string & jsonStr, size_t startPos, gd::Variable & variable)
    {
        size_t pos = SkipBlankChar(jsonStr, startPos);
        if ( pos >= jsonStr.length() ) return std::string::npos;

        if ( jsonStr[pos] == '{' ) //Object
        {
            bool firstChild = true;
            while ( firstChild || jsonStr[pos] == ',' )
            {
                pos++;
                if (pos < jsonStr.length() && jsonStr[pos] == '}' ) break;

                std::string childName;
                pos = SkipString(jsonStr, pos, childName);

                pos++;
                pos = SkipBlankChar(jsonStr, pos);
                if ( pos >= jsonStr.length() || jsonStr[pos] != ':' ) return std::string::npos;

                pos++;
                pos = ::ParseJSONObject(jsonStr, pos, variable.GetChild(childName));

                pos = SkipBlankChar(jsonStr, pos);
                if ( pos >= jsonStr.length()) return std::string::npos;
                firstChild = false;
            }

            if ( jsonStr[pos] != '}' ) {
                std::cout << "Parsing error: Object not properly formed.";
                return std::string::npos;
            }
            return pos+1;
        }
        else if ( jsonStr[pos] == '[' ) //Array are translated into child named 0,1,2...
        {
            unsigned int index = 0;
            while ( index == 0 || jsonStr[pos] == ',' )
            {
                pos++;
                if (pos < jsonStr.length() && jsonStr[pos] == ']' ) break;
                pos = ::ParseJSONObject(jsonStr, pos, variable.GetChild(ToString(index)));

                pos = SkipBlankChar(jsonStr, pos);
                if ( pos >= jsonStr.length()) {
                    std::cout << "Parsing error: element of array not properly formed.";
                    return std::string::npos;
                }
                index++;
            }

            if ( jsonStr[pos] != ']' ) {
                std::cout << "Parsing error: array not properly ended";
                return std::string::npos;
            }
            return pos+1;
        }
        else if ( jsonStr[pos] == '"' ) //String
        {
            std::string str;
            pos = SkipString(jsonStr, pos, str);
            if ( pos >= jsonStr.length() ) {
                std::cout << "Parsing error: Invalid string";
                return std::string::npos;
            }

            variable.SetString(str);
            return pos+1;
        }
        else
        {
            std::string str;
            size_t endPos = pos;
            const std::string separators = " \n,}";
            while (endPos < jsonStr.length() && separators.find_first_of(jsonStr[endPos]) == std::string::npos ) {
                endPos++;
            }

            str = jsonStr.substr(pos, endPos-pos);
            if ( str == "true" )
                variable.SetValue(1);
            else if ( str == "false" )
                variable.SetValue(0);
            else
                variable.SetValue(ToDouble(str));
            return endPos;
        }
    }
}

void GD_API JSONToVariableStructure(const std::string & jsonStr, gd::Variable & variable)
{
    if ( jsonStr.empty() ) return;
    ::ParseJSONObject(jsonStr, 0, variable);
}
