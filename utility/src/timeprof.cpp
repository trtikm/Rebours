#include <rebours/utility/timeprof.hpp>
#include <rebours/utility/timestamp.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/noncopyable.hpp>
#include <list>
#include <ostream>
#include <map>
#include <tuple>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <mutex>
#include <thread>


namespace tmprof_internal_private_implementation_details {


struct Record
{
    Record(char const* const file, int const line, char const* const func);

    uint64_t  number_of_executions() const;
    double  summary_duration() const;
    double  duration_of_longest_execution() const;
    uint32_t  num_running_executions() const;
    double  genuine_duration() const;

    std::string  file_name() const { return std::string(m_file_name); }
    uint32_t  line() const { return m_line; }
    std::string  function_name() const { return std::string(m_function_name); }

    void  on_begin_of_execution(boost::chrono::high_resolution_clock::time_point&  start_time_point);
    void  on_end_of_execution(boost::chrono::high_resolution_clock::time_point const&  begin_time_point,
                              boost::chrono::high_resolution_clock::time_point const&  end_time_point);

private:
    uint64_t  m_number_of_executions;
    boost::chrono::high_resolution_clock::duration  m_summary_duration;
    boost::chrono::high_resolution_clock::duration  m_duration_of_longest_execution;
    uint32_t  m_num_running_executions;
    boost::chrono::high_resolution_clock::time_point  m_run_begin_time_point;
    boost::chrono::high_resolution_clock::time_point  m_run_end_time_point;
    boost::chrono::high_resolution_clock::duration  m_genuine_duration;
    char const*  m_file_name;
    int  m_line;
    char const*  m_function_name;
    mutable std::mutex  m_mutex;
};

Record::Record(char const* const file, int const line, char const* const func)
    : m_number_of_executions(0ULL)
    , m_summary_duration(0U)
    , m_duration_of_longest_execution(0U)
    , m_num_running_executions(0U)
    , m_run_begin_time_point()
    , m_run_end_time_point()
    , m_genuine_duration(0U)
    , m_file_name(file)
    , m_line(line)
    , m_function_name(func)
    , m_mutex()
{}

uint64_t  Record::number_of_executions() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    uint64_t const  result = m_number_of_executions;
    return result;
}

double  Record::summary_duration() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    double const  result = boost::chrono::duration<double>(m_summary_duration).count();
    return result;
}

double  Record::duration_of_longest_execution() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    double const  result = boost::chrono::duration<double>(m_duration_of_longest_execution).count();
    return result;
}

uint32_t  Record::num_running_executions() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    uint32_t const  result = m_num_running_executions;
    return result;
}

double  Record::genuine_duration() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    double  result = boost::chrono::duration<double>(m_genuine_duration).count();
    if (m_num_running_executions != 0U)
        result += boost::chrono::duration<double>(
                            boost::chrono::high_resolution_clock::now() - m_run_begin_time_point
                            ).count();
    return result;
}

void  Record::on_begin_of_execution(boost::chrono::high_resolution_clock::time_point&  start_time_point)
{
    std::lock_guard<std::mutex> const  lock(m_mutex);
    start_time_point = boost::chrono::high_resolution_clock::now();
    if (m_num_running_executions == 0U)
    {
        m_run_begin_time_point = start_time_point;
        m_run_end_time_point = m_run_begin_time_point;
    }
    ++m_num_running_executions;
}

void  Record::on_end_of_execution(boost::chrono::high_resolution_clock::time_point const&  begin_time_point,
                                  boost::chrono::high_resolution_clock::time_point const&  end_time_point)
{
    boost::chrono::high_resolution_clock::duration const  duration_of_execution = end_time_point - begin_time_point;
    std::lock_guard<std::mutex> lock(m_mutex);
    ++m_number_of_executions;
    m_summary_duration += duration_of_execution;
    if (m_duration_of_longest_execution < duration_of_execution)
        m_duration_of_longest_execution = duration_of_execution;
    --m_num_running_executions;
    if (m_run_end_time_point < end_time_point)
        m_run_end_time_point = end_time_point;
    if (m_num_running_executions == 0U)
        m_genuine_duration += m_run_end_time_point - m_run_begin_time_point;
}



block_stop_watches::block_stop_watches(Record* const  storage_for_results)
    : m_storage_for_results(storage_for_results)
    , m_start_time()
{
    m_storage_for_results->on_begin_of_execution(m_start_time);
}

block_stop_watches::~block_stop_watches()
{
    m_storage_for_results->on_end_of_execution(m_start_time,boost::chrono::high_resolution_clock::now());
}



struct time_profile_statistics : private boost::noncopyable
{
    time_profile_statistics()
        : m_records()
        , m_mutex_to_list_of_records()
        , m_start_time()
    {}

    Record*  add_record(char const* const file, int const line, char const* const func);
    void copy_time_profile_data(std::vector<time_profile_data_of_block>& storage);
    boost::chrono::high_resolution_clock::time_point  start_time() const { return m_start_time; }

private:
    std::list<Record>  m_records;
    std::mutex  m_mutex_to_list_of_records;
    boost::chrono::high_resolution_clock::time_point  m_start_time;
};


Record*  time_profile_statistics::add_record(char const* const file, int const line, char const* const func)
{
    std::lock_guard<std::mutex> lock(m_mutex_to_list_of_records);
    if (m_records.empty())
        m_start_time = boost::chrono::high_resolution_clock::now();
    m_records.emplace_back(file,line,func);
    Record* const  result = &m_records.back();
    return result;
}

void time_profile_statistics::copy_time_profile_data(std::vector<time_profile_data_of_block>& storage)
{
    std::size_t  num_records;
    std::list<Record>::const_iterator  it;
    {
        std::lock_guard<std::mutex> lock(m_mutex_to_list_of_records);
        num_records = m_records.size();
        it = m_records.begin();
    }
    for (std::size_t  index = 0U; index < num_records; ++index, ++it)
        storage.push_back(
                    time_profile_data_of_block(
                            it->number_of_executions(),
                            it->genuine_duration(),
                            it->summary_duration(),
                            it->duration_of_longest_execution(),
                            it->num_running_executions(),
                            it->file_name(),
                            it->line(),
                            it->function_name()
                            )
                    );
}


static time_profile_statistics  statistics;

Record* create_new_record_for_block(char const* const file, int const line, char const* const func)
{
    return statistics.add_record(file,line,func);
}


}



time_profile_data_of_block::time_profile_data_of_block(
        uint64_t  num_executions,
        double  genuine_duration,
        double  summary_duration,
        double  longest_duration,
        uint32_t  num_running_executions,
        std::string  file_name,
        uint32_t  line,
        std::string  function_name
        )
    : m_num_executions(num_executions)
    , m_genuine_duration(genuine_duration)
    , m_summary_duration(summary_duration)
    , m_longest_duration(longest_duration)
    , m_num_running_executions(num_running_executions)
    , m_file_name(file_name)
    , m_line(line)
    , m_function_name(function_name)
{}

uint64_t  time_profile_data_of_block::number_of_executions() const
{
    return m_num_executions;
}

double  time_profile_data_of_block::genuine_duration_of_all_executions_in_seconds() const
{
    return m_genuine_duration;
}

double  time_profile_data_of_block::summary_duration_of_all_executions_in_seconds() const
{
    return m_summary_duration;
}

double  time_profile_data_of_block::duration_of_longest_execution_in_seconds() const
{
    return m_longest_duration;
}

uint32_t  time_profile_data_of_block::num_running_executions() const
{
    return m_num_running_executions;
}

std::string const&  time_profile_data_of_block::file_name() const
{
    return m_file_name;
}

uint32_t  time_profile_data_of_block::line() const
{
    return m_line;
}

std::string const&  time_profile_data_of_block::function_name() const
{
    return m_function_name;
}



using ::tmprof_internal_private_implementation_details::statistics;



void copy_time_profile_data_of_all_measured_blocks_into_vector(
        std::vector<time_profile_data_of_block>& storage_for_the_copy_of_data,
        bool const  sort_data
        )
{
    struct local {
        static bool first_is_slower(time_profile_data_of_block const& first,
                                    time_profile_data_of_block const& second)
        {
            return first.genuine_duration_of_all_executions_in_seconds() >
                   second.genuine_duration_of_all_executions_in_seconds() ;
        }
    };
    statistics.copy_time_profile_data(storage_for_the_copy_of_data);
    if (sort_data)
        std::sort(storage_for_the_copy_of_data.begin(),storage_for_the_copy_of_data.end(),
                  &local::first_is_slower);
}

double  compute_genuine_duration_of_all_executions_of_all_blocks_in_seconds(
        std::vector<time_profile_data_of_block> const& collected_profile_data)
{
    double  result = 0.0;
    for (auto it = collected_profile_data.begin(); it != collected_profile_data.end(); ++it)
        result += it->genuine_duration_of_all_executions_in_seconds();
    return result;
}

double  compute_summary_duration_of_all_executions_of_all_blocks_in_seconds(
        std::vector<time_profile_data_of_block> const& collected_profile_data)
{
    double  result = 0.0;
    for (auto it = collected_profile_data.begin(); it != collected_profile_data.end(); ++it)
        result += it->summary_duration_of_all_executions_in_seconds();
    return result;
}

boost::chrono::high_resolution_clock::time_point  get_time_profiling_start_time_point()
{
    return statistics.start_time();
}


namespace tmprof_internal_private_implementation_details {


static std::string normalise_duration(double const d, uint32_t const prec = 3)
{
//    auto const dur =
//        std::floor((float_32_bit)d * 1000.0f + 0.5f) / 1000.0f
//        //d
//        ;
//{
//    std::stringstream sstr;
//    sstr << std::setprecision(prec) << std::fixed << dur;
//std::string sss = sstr.str();
//sss=sss;
//}
//{
//    std::stringstream sstr;
//    sstr << std::setprecision(prec) << std::fixed << d;
//std::string sss = sstr.str();
//sss=sss;
//}
//{
//    std::stringstream sstr;
//    sstr << std::fixed << d;
//std::string sss = sstr.str();
//sss=sss;
//}
//{
//    std::stringstream sstr;
//    sstr << std::setprecision(prec) << d;
//std::string sss = sstr.str();
//sss=sss;
//}
//    std::stringstream sstr;
//    sstr << std::setprecision(prec) << std::fixed << dur;
//    return sstr.str();
    std::stringstream sstr;
    sstr << std::setprecision(prec) << std::fixed << d;
    return sstr.str();
}

static boost::filesystem::path get_common_prefix(boost::filesystem::path const& p,
                                                 boost::filesystem::path const& q)
{
    boost::filesystem::path res;
    auto pit = p.begin();
    auto qit = q.begin();
    for ( ; pit != p.end() && qit != q.end() && *pit == *qit; ++pit, ++qit)
        res = res / *pit;
    return res;
}

static boost::filesystem::path get_relative_path(boost::filesystem::path const& dir,
                                                 boost::filesystem::path const& file)
{
    auto dit = dir.begin();
    auto fit = file.begin();
    for ( ; dit != dir.end() && fit != file.end() && *dit == *fit; ++dit, ++fit)
        ;
    boost::filesystem::path res;
    for ( ; fit != file.end(); ++fit)
        res = res / *fit;
    return res;
}


}


std::ostream& print_time_profile_data_to_stream(
        std::ostream& os,
        std::vector<time_profile_data_of_block> const& data
        )
{
    using namespace tmprof_internal_private_implementation_details;

    double  genuine_duration =
            compute_genuine_duration_of_all_executions_of_all_blocks_in_seconds(data);
    if (genuine_duration < 0.00001)
        genuine_duration = 0.00001;

    double  summary_duration =
            compute_summary_duration_of_all_executions_of_all_blocks_in_seconds(data);
    if (summary_duration < 0.00001)
        summary_duration = 0.00001;

    boost::filesystem::path common_path_prefix(
        data.empty() ?
            boost::filesystem::path("") :
            boost::filesystem::path(data.front().file_name()).branch_path()
        );
    for (auto it = data.begin(); it != data.end(); ++it)
        if (it->number_of_executions() > 0ULL)
            common_path_prefix = get_common_prefix(common_path_prefix,it->file_name());

    os <<   "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n"
            "<html>\n"
            "<head>\n"
            "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
            "    <title>Time Profile Log</title>\n"
            "    <style type=\"text/css\">\n"
            "        body {\n"
            "            background-color: white;\n"
            "            color: black;\n"
//            "            width: 480pt;\n"
//            "            height: 720pt;\n"
            "            margin-left: auto;\n"
            "            margin-right: auto;\n"
            "        }\n"
            "        h1, h2, h3, h4, h5, h6, table { font-family:\"Liberation serif\"; }\n"
            "        p, table {\n"
            "            font-size:12pt;\n"
            "            margin-left: auto;\n"
            "            margin-right: auto;\n"
            "            text-align: justify\n"
            "        }\n"
            "        th, td {\n"
            "            font-family:\"Liberation mono\", monospace;\n"
            "            font-size:10pt;\n"
            "            text-align:right;\n"
            "            padding: 3pt;\n"
            "        }\n"
            "        tr:nth-child(even){background-color: #f2f2f2}\n"
            "   </style>\n"
            "</head>\n"
            "<body>\n"
            "    <h1>Time Profile Log</h1>\n"
            ;

    os <<   "    <p>\n"
            "        <b>Time-Profile Data Per Block.</b>\n"
            "        All times (durations) in the table below are in seconds.\n"
            "        Genuine duration is a real time spent in a measured block.\n"
            "        Genuine average duration is a genuine duration divided by a number of executions.\n"
            "        Summary duration is such a time spent in a measured block as if\n"
            "               all its executions never overlap (i.e. no concurrency).\n"
            "        Summary average duration is a summary duration divided by a number of executions.\n"
            "        Numbers in the speed-up column represent ratios between numbers in respective columns\n"
            "               summary duration and genuine duration.\n"
            "        If a function name is preffixed by '*', then it means that the measured block\n"
            "               inside the function was being executed while the presented data was read.\n"
            "        The integer in the leftmost data-cell in the summary line identifies a number of blocks\n"
            "               that were executed at least once during the time profiling.\n"
            "        Common path preffix for all files in the table is:<br/><b>'"
                            << common_path_prefix.string() << "/'</b>\n"
            "    </p>\n"
            ;

    os <<   "    <table>\n"
            "    <caption>\n"
            "    </caption>\n"
            "    <tr>\n"
//            "        <th rowspan=\"2\">NameX</th>\n"
//            "        <th colspan=\"3\">NameY</th>\n"
            "        <th>Percentage</th>\n"
            "        <th>Function</th>\n"
            "        <th>Number of<br/>Executions</th>\n"
            "        <th>Genuine<br/>Duration</th>\n"
            "        <th>Gen.Ave.<br/>Duration</th>\n"
            "        <th>Longest<br/>Duration</th>\n"
            "        <th>Summary<br/>Duration</th>\n"
            "        <th>Sum.Ave.<br/>Duration</th>\n"
            "        <th>Speed-up</th>\n"
            "        <th>Line</th>\n"
            "        <th style=\"text-align:left\">File</th>\n"
            "    </tr>\n"
            ;

    for (auto const& record : data)
    {
        if (record.number_of_executions() == 0ULL)
            continue;

        os <<   "    <tr>\n";

        os <<   "        <td>";
        os <<   normalise_duration(100.0 * record.genuine_duration_of_all_executions_in_seconds() / genuine_duration);
        os <<   "</td>\n";

        os <<   "        <td>";
        os <<   (record.num_running_executions() > 0U ? " * " : "");
        os <<   record.function_name();
        os <<   "</td>\n";

        os <<   "        <td>";
        os <<   record.number_of_executions();
        os <<   "</td>\n";

        os <<   "        <td>";
        os <<   normalise_duration(record.genuine_duration_of_all_executions_in_seconds());
        os <<   "</td>\n";

        os <<   "        <td>";
        os <<   normalise_duration(record.genuine_duration_of_all_executions_in_seconds() / record.number_of_executions());
        os <<   "</td>\n";

        os <<   "        <td>";
        os <<   normalise_duration(record.duration_of_longest_execution_in_seconds());
        os <<   "</td>\n";

        os <<   "        <td>";
        os <<   normalise_duration(record.summary_duration_of_all_executions_in_seconds());
        os <<   "</td>\n";

        os <<   "        <td>";
        os <<   normalise_duration(record.summary_duration_of_all_executions_in_seconds() / record.number_of_executions());
        os <<   "</td>\n";

        os <<   "        <td>";
        os <<   normalise_duration(record.summary_duration_of_all_executions_in_seconds() /
                                        (record.genuine_duration_of_all_executions_in_seconds() < 0.00001 ?
                                            0.00001 : record.genuine_duration_of_all_executions_in_seconds())
                                   );
        os <<   "</td>\n";

        os <<   "        <td>";
        os <<   record.line();
        os <<   "</td>\n";

        os <<   "        <td style=\"text-align:left\">";
        os <<   get_relative_path(common_path_prefix,record.file_name()).string();
        os <<   "</td>\n";

        os <<   "    </tr>\n";
    }

    os <<   "    <tr></tr>\n"
            ;

    os <<   "    <tr style=\"background-color: white\">\n"
            "        <th>Summary</th>\n"
            "        <th>" << data.size() << "</th>\n"
            "        <th></th>\n"
            "        <th>" << normalise_duration(genuine_duration) << "</th>\n"
            "        <th></th>\n"
            "        <th></th>\n"
            "        <th>" << normalise_duration(summary_duration) << "</th>\n"
            "        <th>" << normalise_duration(summary_duration / genuine_duration) << "</th>\n"
            "        <th></th>\n"
            "        <th></th>\n"
            "    </tr>\n"
            ;

    os <<   "    </table>\n"
            "</body>\n"
            "</html>\n"
            ;
    return os;
}

std::ostream& print_time_profile_to_stream(std::ostream& os)
{
    std::vector<time_profile_data_of_block> data;
    copy_time_profile_data_of_all_measured_blocks_into_vector(data,true);
    print_time_profile_data_to_stream(os,data);
    return os;
}

void print_time_profile_to_file(std::string const& file_path_name,
                                bool const extend_file_name_by_timestamp)
{
    boost::filesystem::ofstream file(
                extend_file_name_by_timestamp ?
                        extend_file_path_name_by_timestamp(file_path_name) :
                        file_path_name
                );
    print_time_profile_to_stream(file);
}
