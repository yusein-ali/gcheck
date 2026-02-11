/*BEGIN SOLUTION*/
#include "transcript.hpp"

#include <list>
#include <utility>
#include <algorithm>
#include <string>
#include <iterator>
#include <iostream>

void TranscriptRegistry::Add(const Transcript& t) {
    transcripts_.push_back(t);
}

void TranscriptRegistry::RemoveById(const std::string& student_id) {
    auto new_end = std::remove_if(transcripts_.begin(), transcripts_.end(),
        [&](const Transcript& t) {
            return t.student_id == student_id;
        });

    transcripts_.erase(new_end, transcripts_.end());
}

std::list<Transcript>::const_iterator TranscriptRegistry::FindTranscript(const std::string& student_id) const {
    return std::find_if(transcripts_.begin(), transcripts_.end(),
        [&](const Transcript& t) {
            return t.student_id == student_id;
        });
}

std::list<std::pair<std::string, size_t>> TranscriptRegistry::FindCourseResults(const std::string& course) const {
    std::list<std::pair<std::string, size_t>> ret;
    // doing it with lambdas
    std::for_each(transcripts_.begin(), transcripts_.end(),
        [&](const Transcript& t){
            
            std::for_each(t.grades.begin(), t.grades.end(),
                [&](const std::pair<std::string, size_t>& p){
                    if (p.first == course)
                        ret.emplace_back(t.student_id, p.second);
                });
        });
    
    // Do not use the above in real code, it is solely for demonstrating lambdas and the standard library
    // Here is an alternative, more sensible way:
    /*
    for(auto& student : transcripts_)
        for(auto& grade : student.grades)
            if (grade.first == course)
                ret.emplace_back(student.student_id, grade.second);
    */
    return ret;
}

const std::list<Transcript>& TranscriptRegistry::GetTranscripts() const {
    return transcripts_;
}
/*END SOLUTION*/