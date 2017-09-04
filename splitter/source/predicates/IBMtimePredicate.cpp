#include "header/predicates/IBMtimePredicate.hpp"

using namespace util;
namespace splitter
{

IBMtimePredicate::IBMtimePredicate(unsigned long time)
{
    this->timeOpen = time;

    this->symbols.insert("AAAP");
    this->symbols.insert("FLWS");
    this->symbols.insert("XOM");
    this->symbols.insert("PTR");
    this->symbols.insert("BHP");
    this->symbols.insert("PBR");
    this->symbols.insert("HSBC");
    this->symbols.insert("WMT");
    this->symbols.insert("CHL");
    this->symbols.insert("RDS.A");
    this->symbols.insert("CVX");
    this->symbols.insert("AAPL");
    this->symbols.insert("JPM");
    this->symbols.insert("MSFT");
    this->symbols.insert("AIG");
    this->symbols.insert("WFC");
    this->symbols.insert("IBM");
    this->symbols.insert("WMT");

/*
    this->symbols={"AAAP", "FLWS", "FCCY", "VNET", "TWOU", "JOBS", "CAFD", "EGHT", "AVHI", "SHLM", "AAON", "ABAX",
      "ABEO",
                 "ABMD", "AXAS", "ACIU", "ACIA", "ACTG", "ACHC", "ACAD", "ACST", "AXDX", "XLRN", "ANCX", "ARAY"};
*/
}

bool IBMtimePredicate::ps(const events::AbstractEvent &event) const
{
    if (event.getContent().size() == 0)
        return false;
    auto content = event.getContent();

    /*
     * if first event is an IMB event
     */
    try
    {

        //        if (content.at("SS") == "IBM")// && std::stof(content.at("close")) > std::stof(content.at("open")))
        if (this->symbols.count(content.at("SS")) != 0)
        {
            return true;
        }
        else
            return false;
    }
    catch (...)
    {
        return false;
    }
}

bool IBMtimePredicate::pc(const events::AbstractEvent &event, selection::AbstractSelection &abstractSelection)
{
    if (event.getContent().size() == 0)
        return false;

    try
    {
        if (event.getTimestamp() > abstractSelection.getStartTimestamp() + this->timeOpen)
        {
            return true;
        }
        else
            return false;
    }
    catch (...)
    {
        return false;
    }
}
}
