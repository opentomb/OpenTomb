#include "engine/engine.h"
#include "util/helpers.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <chrono>
#include <thread>

int main(int /*argc*/, char** /*argv*/)
{
    BOOST_LOG_TRIVIAL(info) << "*** This is OpenTomb.";
    BOOST_LOG_TRIVIAL(info) << "*** If you experience problems, report them and include this text:";
    BOOST_LOG_TRIVIAL(info) << "***    Git Checkout " << GIT_SHA;

    boost::property_tree::ptree config;
    try
    {
        boost::property_tree::read_xml("config.xml", config);
    }
    catch(boost::property_tree::xml_parser_error& ex)
    {
        BOOST_LOG_TRIVIAL(error) << "Cannot load configuration: " << ex.what();
        BOOST_LOG_TRIVIAL(info) << "Engine configuration will be initialized with defaults";
        config = boost::property_tree::ptree();
    }

    engine::Engine engine{config};

    boost::property_tree::write_xml("config.xml", config, std::locale(), boost::property_tree::xml_writer_settings<std::string>(' ', 4));

    // Entering main loop.

    util::TimePoint prev_time = util::now();

    while(!engine.m_done)
    {
        BOOST_ASSERT(engine.m_timeScale > 0);

        util::TimePoint now = util::now();
        util::Duration delta = now - prev_time;
        delta *= engine.m_timeScale;

        if(delta.count() <= 0)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            continue;
        }

        prev_time = now;

        engine.frame(delta);
        engine.display();
    }

    // Main loop interrupted; shutting down.

    return EXIT_SUCCESS;
}
