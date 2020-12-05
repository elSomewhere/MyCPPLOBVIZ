//
//  Replay.cpp
//  FastTextRendering
//
//  Created by Esteban Lanter on 05.12.20.
//

#include "Replay.hpp"

// parsing
#include <iostream>
#include <fstream>
#include <vector>
#include "Date/date.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <list>
#include <queue>


using namespace date;



// from: https://morestina.net/blog/1400/minimalistic-blocking-bounded-queue-for-c
template<typename T>
class queue {
    std::deque<T> content;
    size_t capacity;

    std::mutex mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;

    // i tihnk this to avoid that queue is called with these
    queue(const queue &) = delete;
    queue(queue &&) = delete;
    queue &operator = (const queue &) = delete;
    queue &operator = (queue &&) = delete;

public:
    queue(size_t capacity): capacity(capacity) {}

    void push(T &&item) {
        {
            std::unique_lock<std::mutex> lk(mutex);
            not_full.wait(lk, [this]() { return content.size() < capacity; });
            content.push_back(std::move(item));
        }
        not_empty.notify_one();
    }

    bool try_push(T &&item) {
        {
            std::unique_lock<std::mutex> lk(mutex);
            if (content.size() == capacity)
                return false;
            content.push_back(std::move(item));
        }
        not_empty.notify_one();
        return true;
    }

    void pop(T &item) {
        {
            std::unique_lock<std::mutex> lk(mutex);
            not_empty.wait(lk, [this]() { return !content.empty(); });
            item = std::move(content.front());
            content.pop_front();
        }
        not_full.notify_one();
    }

    bool try_pop(T &item) {
        {
            std::unique_lock<std::mutex> lk(mutex);
            if (content.empty())
                return false;
            item = std::move(content.front());
            content.pop_front();
        }
        not_full.notify_one();
        return true;
    }
};



// intermediate type for the loop
struct timed_chunk{
    std::vector<std::string> chunk;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> last_timestamp;
};


// the element that is held in the queue
struct Message{
    std::string chunk_bytes;
    std::chrono::milliseconds timer;
};


// make a template instead of holding messages with std::String
class Buffer
{
public:
    Buffer(size_t capacity, std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> init_timestamp): buf(capacity), tail_time(init_timestamp){}
    void push(std::string const & v, std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> last_timestamp){
        std::chrono::milliseconds delta_t = last_timestamp - tail_time;
        std::cout<<"pushed a delta_t message of "<<delta_t<<::std::endl;
        Message message{v, delta_t};
        buf.push(std::move(message));
        tail_time = last_timestamp;
    }
    void pop(Message &m){
        buf.pop(m);
    }

private:
    queue<Message> buf;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tail_time;
};



std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> get_current_timepoint(){
    return std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
}





void parse_line_to_individual_strings(std::string &line, std::vector<std::string> &splittedStringArray){
    splittedStringArray.clear();
    size_t last = 0, pos = 0;
    while ((pos = line.find(',', last)) != std::string::npos) {
        splittedStringArray.emplace_back(line, last, pos - last);
        last = pos + 1;
    }
    return;
}


std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> get_timestamp_from_line(std::string &line, std::vector<std::string> &splittedString, int date_col_id, std::string date_formatter){
    // parse string to tp
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> res;
    parse_line_to_individual_strings(line, splittedString);
    std::string current_packet_timestamp_raw;
    current_packet_timestamp_raw = splittedString[date_col_id]; // convert this to a timepoint!
    std::stringstream ss_raw(current_packet_timestamp_raw);
    ss_raw >> parse(date_formatter, res);
    return res;
}

// parse vector of lines to one big line and transform it to raw bytes (bytes type should rather be template type)
std::string chunk_to_rawbytes(std::vector<std::string> &chunk){
    std::string s;
    for (const auto &piece : chunk) s += piece;
    return s;
}

// raw line from csv file to string representation that will be sent
std::string parselinefun(std::string line){
    return line;
}










std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> peek_first_datepoint_from_file(std::string filename, int date_col_id, std::string date_formatter){
    // peak to get inital time
    std::fstream infile(filename);
    char buffer[65536];
    infile.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
    // current line
    std::string line;
    // splitted line (for date parsing)
    std::vector<std::string> splittedString;
    // resulting timepoint
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> last_timestamp;
    // dispose of header
    getline(infile, line);
    // get first line
    getline(infile, line);
    // parse to timesdatpm
    last_timestamp = get_timestamp_from_line(line, splittedString, date_col_id, date_formatter);
    return last_timestamp;
}


// should consume: filepath, regex date format string, optional number for timestamp column - default = 0, max_chunk_size=64, max_buff_size=1024
class Replayer {
    const std::string filepath;
    const std::string date_formatter;
    const int date_col_id;
    const int max_chunk_size;
    const int max_buff_size;
    Buffer buff;
public:
    Replayer(std::string filepath, std::string date_formatter, int date_col_id, int max_chunk_size, int max_buff_size):
            filepath(filepath),
            date_formatter(date_formatter),
            date_col_id(date_col_id),
            max_chunk_size(max_chunk_size),
            max_buff_size(max_buff_size),
            buff(max_buff_size, peek_first_datepoint_from_file(filepath, date_col_id, date_formatter)){};
    void sendlines(){
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> time_at_last_sent_message = get_current_timepoint();
        std::chrono::milliseconds next_message_delta;
        std::chrono::milliseconds time_passed_since_last_send_op;
        std::chrono::milliseconds wait_for;
        std::chrono::milliseconds zero_duration(0);
        while(true){
            std::cout<<"sendlines"<<std::endl;
            Message message;
            buff.pop(message);
            next_message_delta = message.timer;
            std::cout<<"next_message_delta is "<<next_message_delta<<std::endl;
            std::string rawmessage = message.chunk_bytes;
            time_passed_since_last_send_op = get_current_timepoint()-time_at_last_sent_message;
            wait_for = std::max(next_message_delta - time_passed_since_last_send_op, zero_duration);
            std::cout<<"wait for "<<wait_for<<std::endl;
            std::this_thread::sleep_for(wait_for);
            std::cout<<"sending"<<std::endl;
            time_at_last_sent_message = get_current_timepoint();
        };
    }

    timed_chunk parse_raw_line(std::string &line, std::vector<std::string> &splittedString, std::vector<std::string> &chunk, int max_chunk_size, int date_col_id, std::string date_formatter, std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> &last_timestamp){
        std::chrono::milliseconds timestamp_delta(0);
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> current_timestep = get_timestamp_from_line(line, splittedString, date_col_id, date_formatter);
        timestamp_delta = std::chrono::milliseconds{current_timestep - last_timestamp};
        std::cout<<"current_timestep is "<<current_timestep<<std::endl;
        std::cout<<"delta to last timestep is "<<timestamp_delta<<std::endl;
        if(timestamp_delta.count()==0 & chunk.size()<max_chunk_size){
            std::string parsedline = parselinefun(line);
            chunk.push_back(parsedline);
            timed_chunk res{
                    chunk,
                    last_timestamp
            };
            return res;
        }else if(timestamp_delta.count()==0 & chunk.size()>=max_chunk_size){
            std::string message = chunk_to_rawbytes(chunk);
            buff.push(message, last_timestamp);
            chunk.clear();
            std::string parsedline = parselinefun(line);
            chunk.push_back(parsedline);
            timed_chunk res{
                    chunk,
                    last_timestamp
            };
            return res;
        }else if(timestamp_delta.count()>0 & chunk.size()<max_chunk_size){
            std::string message = chunk_to_rawbytes(chunk);
            buff.push(message, last_timestamp);
            last_timestamp = get_timestamp_from_line(line, splittedString, date_col_id, date_formatter);
            chunk.clear();
            std::string parsedline = parselinefun(line);
            chunk.push_back(parsedline);
            timed_chunk res{
                    chunk,
                    last_timestamp
            };
            return res;
        }else if(timestamp_delta.count()>0 & chunk.size()>=max_chunk_size){
            std::string message = chunk_to_rawbytes(chunk);
            buff.push(message, last_timestamp);
            last_timestamp = get_timestamp_from_line(line, splittedString, date_col_id, date_formatter);
            chunk.clear();
            std::string parsedline = parselinefun(line);
            chunk.push_back(parsedline);
            timed_chunk res{
                    chunk,
                    last_timestamp
            };
            return res;
        }
    }

    void readlines(){
        std::fstream infile(filepath);
        char buffer[65536];
        infile.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
        // current line
        std::string line;
        // splitted line (for date parsing)
        std::vector<std::string> splittedString;
        // resulting timepoint
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> last_timestamp;
        // dispose of header
        getline(infile, line);
        // the container for individual chunks
        std::vector<std::string> chunk(max_chunk_size);
        // get first line
        getline(infile, line);
        // parse to timesdatpm
        last_timestamp = get_timestamp_from_line(line, splittedString, date_col_id, date_formatter);
        // probably now have to fil the first element into the chunk
        timed_chunk temp = parse_raw_line(line, splittedString, chunk, max_chunk_size, date_col_id, date_formatter, last_timestamp);
        bool reading = true;
        while(reading){
            while (getline(infile, line)){
                if(infile.eof()){
                    reading = false;
                    break;
                }
                temp = parse_raw_line(line, splittedString, chunk, max_chunk_size, date_col_id, date_formatter, last_timestamp);
                getline(infile, line);
            }
        }
    }

private:

};
