#include <ctime>
#include <string>

int record_quality = 1;
bool record_enabled = false;
bool is_recording = false;
std::string current_record_file;
int record_time = 300;

void StartRecording() {
    if (is_recording) return;
    
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);
    char filename[80];
    strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.mp4", timeinfo);
    
    system("mkdir -p /sdcard/#wintyx_ext");
    
    char command[256];
    const char* quality_cmd = "";
    
    switch(record_quality) {
        case 0: quality_cmd = "--size 1280x720 --bit-rate 4000000"; break;
        case 1: quality_cmd = "--size 1920x1080 --bit-rate 8000000"; break;
        case 2: quality_cmd = "--size 2560x1440 --bit-rate 12000000"; break;
    }
    
    snprintf(command, sizeof(command), 
             "screenrecord %s --time-limit %d /sdcard/#wintyx_ext/%s &",
             quality_cmd, record_time, filename);
    
    system(command);
    is_recording = true;
    current_record_file = filename;
}

void StopRecording() {
    if (!is_recording) return;
    
    system("pkill -2 screenrecord");
    is_recording = false;
    current_record_file = "";
}