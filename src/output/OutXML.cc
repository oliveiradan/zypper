#include <iostream>
#include <sstream>
#include <vector>

#include "zypp/base/String.h"

#include "OutXML.h"
#include "../zypper-utils.h"

using std::cout;
using std::endl;
using std::string;
using std::ostringstream;
using std::vector;

OutXML::OutXML(Verbosity verbosity) : Out(TYPE_XML, verbosity)
{
  cout << "<?xml version='1.0'?>" << endl;
  cout << "<stream>" << endl;
}

OutXML::~OutXML()
{
  cout << "</stream>" << endl;
}

bool OutXML::mine(Type type)
{
  // Type::TYPE_NORMAL is mine
  if (type & Out::TYPE_XML)
    return true;
  return false;
}

bool OutXML::infoWarningFilter(Verbosity verbosity, Type mask)
{
  if (!mine(mask))
    return true;
  if (this->verbosity() < verbosity)
    return true;
  return false;
}

/*
string xmlEncode(const string & s)
{
  string result;
  result.reserve(s.size()*2);
  char buff[7];

  for(string::const_iterator it = s.begin(); it != s.end(); ++it)
  {
    switch(*it)
    {
    case '&':
    case '<':
    case '>':
    case '"':
    {
      snprintf(buff, 7, "&#%d;", (unsigned)*it);
      result.append(buff);
      break;
    }
    default:
      result.append(1, *it);
    }
  }

  return result;
}
*/

void OutXML::info(const string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;
  
  cout << "<message type=\"info\">" << xml_encode(msg)
       << "</message>" << endl;
}

void OutXML::warning(const string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;

  cout << "<message type=\"warning\">" << xml_encode(msg)
       << "</message>" << endl;
}

void OutXML::error(const string & problem_desc, const string & hint)
{
  cout << "<message type=\"error\">" << xml_encode(problem_desc)
       << "</message>" << endl;
  //! \todo hint
}

void OutXML::error(const zypp::Exception & e,
                      const string & problem_desc,
                      const string & hint)
{
  ostringstream s;

  // problem
  s << problem_desc << endl;
  // cause
  s << zyppExceptionReport(e) << endl;
  // hint
  if (!hint.empty())
    s << hint << endl;

  cout << "<message type=\"error\">" << xml_encode(s.str())
       << "</message>" << endl;
}

void OutXML::writeProgressTag(const string & id, const string & label,
                              int value, bool done, bool error)
{
  cout << "<progress";
  cout << " id=\"" << xml_encode(id) << "\"";
  cout << " name=\"" << xml_encode(label) << "\"";
  if (done)
    cout << " done=\"" << error << "\"";
  // print value only if it is known (percentage progress)
  // missing value means 'is-alive' notification
  else if (value >= 0) 
    cout << " value=\"" << value << "\"";
  cout << "/>" << endl;
}

void OutXML::progressStart(const string & id,
                           const string & label,
                           bool has_range)
{
  if (progressFilter())
    return;

  //! \todo there is a bug in progress data which returns has_range false incorrectly
  writeProgressTag(id, label, has_range ? 0 : -1, false);
}

void OutXML::progress(const string & id,
                      const string& label,
                      int value)
{
  if (progressFilter())
    return;

  writeProgressTag(id, label, value, false);
}

void OutXML::progressEnd(const string & id, const string& label, bool error)
{
  if (progressFilter())
    return;

  writeProgressTag(id, label, 100, true, error);
}

void OutXML::dwnldProgressStart(const zypp::Url & uri)
{
  cout << "<download"
    << " url=\"" << xml_encode(uri.asString()) << "\""
    << " percent=\"-1\""
    << " rate=\"-1\""
    << "/>" << endl;
}

void OutXML::dwnldProgress(const zypp::Url & uri,
                           int value,
                           long rate)
{
  cout << "<download"
    << " url=\"" << xml_encode(uri.asString()) << "\""
    << " percent=\"" << value << "\""
    << " rate=\"" << rate << "\""
    << "/>" << endl;
}

void OutXML::dwnldProgressEnd(const zypp::Url & uri, long rate, bool error)
{
  cout << "<download"
    << " url=\"" << xml_encode(uri.asString()) << "\""
    << " rate=\"" << rate << "\""
    << " done=\"" << error << "\""
    << "/>" << endl;
}

void OutXML::prompt(PromptId id,
                    const string & prompt,
                    const string & answer_hint)
{
  cout << "<prompt id=\"" << id << "\">" << endl;
  cout << xml_encode(prompt);

  vector<string> answers;
  zypp::str::split(answer_hint, back_inserter(answers), "/");
  for (vector<string>::const_iterator ansit = answers.begin();
       ansit != answers.end(); ++ansit)
  {
    string answer = *ansit;
    cout << "<option";
    //if (is_default) // TODO
    //  cout << " default=\"1\"";
    cout << " value=\"" << xml_encode(answer) << "\"";
    cout << "/>" << endl;
  }
  cout << "</prompt>" << endl;
}
