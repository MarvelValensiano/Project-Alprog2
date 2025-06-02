#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm> // For std::ws
#include <limits>   // For std::numeric_limits
#include <iomanip> // For std::setw and std::left
#include <numeric>  // For std::accumulate

using namespace std;

const int MAX_STUDENTS = 100;
const int BASE_TUITION = 15000000; // Define base tuition globally or pass as needed

// Struct for new student registration data
struct student {
    string name;
    string gender;
    string placeofbirth;
    string dateofbirth;
    int NISN;
    float grade; // Admission grade
    vector<pair<string, int>> subject_grades;
    vector<string> conduct_log;
};

// Struct for simple student listing (from data_student.txt)
struct StudentSimple {
    string name;
    string NISN;
};

// Struct for tuition records, used by helper and payTuition
struct TuitionRecord_t { // Renamed to avoid conflict with local struct in searchTuitionStatus if any
    int id;
    string name;
    int paid_this_transaction; // Amount paid in the specific transaction being recorded
    int unpaid_balance;      // The remaining balance after this transaction
};


student newstudent_arr[MAX_STUDENTS];
int count_new_students = 0;

int admission_capacity = 2;
string main_student_data_file = "data_student.txt";
string student_details_folder = "class/";
string tuition_file = "tuition.txt";


// --- Function Declarations ---
void registration();
void showRegistrationResult();
void inputGradesLoader(int mode);
void inputGradesRecursive(const vector<StudentSimple>& students, int num_students);
void displayAndCalculateAverage(const StudentSimple& selected_student, const string& student_details_folder);
bool getLatestTuitionRecordForPayment(int nisn_to_search, string& out_student_name, int& out_outstanding_balance); // New Helper
void payTuition();
void searchTuitionStatus();
void menuTuition();
void displayStudentDetailsWithPointer(const student* s);
void clearInputBuffer();
bool isValidNisn(const string& nisn_str, int& nisn_int);
bool isValidGrade(const string& grade_str, float& grade_float);
void menuConductLog();
void addConductNote();
void viewConductNotes();
void loadStudentDetailForConduct(student& s_detail, const string& nisn, const string& name);
void saveStudentDetailWithConduct(const student& s_detail);

// --- Function Implementations ---

void clearInputBuffer() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool isValidNisn(const string& nisn_str, int& nisn_int) {
    if (nisn_str.empty()) return false;
    for (char const &c : nisn_str) {
        if (std::isdigit(c) == 0) return false;
    }
    try {
        nisn_int = stoi(nisn_str);
        return true;
    } catch (const std::out_of_range&) { return false;
    } catch (const std::invalid_argument&) { return false; }
}

bool isValidGrade(const string& grade_str, float& grade_float) {
    if (grade_str.empty()) return false;
    size_t dot_count = 0;
    for (char const &c : grade_str) {
        if (c == '.') { dot_count++; if (dot_count > 1) return false; continue; }
        if (std::isdigit(c) == 0) return false;
    }
    try {
        grade_float = stof(grade_str);
        if (grade_float >= 0.0f && grade_float <= 100.0f) return true;
        return false;
    } catch (const std::out_of_range&) { return false;
    } catch (const std::invalid_argument&) { return false; }
}

void displayStudentDetailsWithPointer(const student* s) {
    if (s == nullptr) { cout << "Error: Null student pointer." << endl; return; }
    cout << "  Name (via ptr): " << s->name << endl;
    cout << "  NISN (via ptr): " << s->NISN << endl;
    cout << "  Grade (via ptr): " << s->grade << endl;
    cout << "---------------------------" << endl;
}

void registration() {
    int num_to_register;
    cout << "How many students will register? : ";
    while (!(cin >> num_to_register) || num_to_register <= 0 || num_to_register > MAX_STUDENTS) {
        cout << "Invalid input. Please enter a positive number up to " << MAX_STUDENTS << ": ";
        cin.clear(); clearInputBuffer();
    }
    clearInputBuffer();
    count_new_students = 0;
    for (int i = 0; i < num_to_register; ++i) {
        if (count_new_students >= MAX_STUDENTS) { cout << "Maximum student capacity reached." << endl; break; }
        cout << "\n--- Student " << i + 1 << " ---" << endl;
        cout << "Name of student: "; getline(cin, newstudent_arr[count_new_students].name);
        string nisn_str;
        cout << "NISN of student: ";
        while (getline(cin, nisn_str) && !isValidNisn(nisn_str, newstudent_arr[count_new_students].NISN)) {
            cout << "Invalid NISN. Please enter a numeric NISN: ";
        }
        cout << "Place of birth of student: "; getline(cin, newstudent_arr[count_new_students].placeofbirth);
        cout << "Date of birth of student (DD/MM/YYYY): "; getline(cin, newstudent_arr[count_new_students].dateofbirth);
        cout << "Gender of student (L/P): "; getline(cin, newstudent_arr[count_new_students].gender);
        while(newstudent_arr[count_new_students].gender != "L" && newstudent_arr[count_new_students].gender != "P" &&
              newstudent_arr[count_new_students].gender != "l" && newstudent_arr[count_new_students].gender != "p") {
            cout << "Invalid gender. Please enter L or P: "; getline(cin, newstudent_arr[count_new_students].gender);
        }
        string grade_str;
        cout << "Grade of student (0-100): ";
        while (getline(cin, grade_str) && !isValidGrade(grade_str, newstudent_arr[count_new_students].grade)) {
             cout << "Invalid Grade. Please enter a numeric grade between 0-100: ";
        }
        newstudent_arr[count_new_students].conduct_log.clear();
        newstudent_arr[count_new_students].subject_grades.clear();
        cout << endl;
        count_new_students++;
    }
    if (count_new_students > 0) cout << count_new_students << " student(s) have been provisionally registered!" << endl;
    else cout << "No students were registered." << endl;
}

void showRegistrationResult() {
    if (count_new_students == 0) { cout << "No students registered to show results for." << endl; return; }
    cout << "\n--- REGISTRATION RESULTS & ADMISSION ---" << endl;
    cout << "CONGRATULATIONS TO THE ADMITTED STUDENTS!" << endl;
    for (int i = 0; i < count_new_students - 1; i++) {
        for (int j = 0; j < count_new_students - i - 1; j++) {
            if (newstudent_arr[j].grade < newstudent_arr[j + 1].grade) {
                student temp = newstudent_arr[j]; newstudent_arr[j] = newstudent_arr[j + 1]; newstudent_arr[j + 1] = temp;
            }
        }
    }
    int admitted_count = 0;
    cout << "\nAdmitted Students (Top " << admission_capacity << "):" << endl;
    for (int i = 0; i < admission_capacity && i < count_new_students; i++) {
        cout << "\nStudent Rank " << i + 1 << ":" << endl;
        cout << "Name: " << newstudent_arr[i].name << endl;
        cout << "NISN: " << newstudent_arr[i].NISN << endl;
        cout << "Place of Birth: " << newstudent_arr[i].placeofbirth << endl;
        cout << "Date of Birth: " << newstudent_arr[i].dateofbirth << endl;
        cout << "Gender: " << newstudent_arr[i].gender << endl;
        cout << "Admission Grade: " << newstudent_arr[i].grade << endl;
        admitted_count++;
    }
    if (admitted_count > 0) {
        string decision;
        cout << "\nDo you want to save these " << admitted_count << " admitted students' data? (y/n): ";
        cin >> decision; clearInputBuffer();
        if (decision == "y" || decision == "Y") {
            ofstream ofs_local_main_data;
            ofs_local_main_data.open(main_student_data_file, ios::app);
            if (!ofs_local_main_data.is_open()) { cout << "Error: Failed to open " << main_student_data_file << "!" << endl; }
            else {
                for (int i = 0; i < admitted_count; i++) {
                    ofs_local_main_data << newstudent_arr[i].NISN << endl; ofs_local_main_data << newstudent_arr[i].name << endl;
                }
                ofs_local_main_data.close();
            }
            ofstream ofs_local_student_detail;
            for (int i = 0; i < admitted_count; i++) {
                string student_file_path = student_details_folder + to_string(newstudent_arr[i].NISN) + "_" + newstudent_arr[i].name + ".txt";
                ofs_local_student_detail.open(student_file_path, ios::out);
                if (!ofs_local_student_detail.is_open()) { cout << "Error: Failed to open file " << student_file_path << endl; continue; }
                // Save all details using the saveStudentDetailWithConduct logic for consistency
                // This requires newstudent_arr[i] to be a 'student' struct, which it is.
                saveStudentDetailWithConduct(newstudent_arr[i]); // Use the comprehensive save function
                ofs_local_student_detail.close(); // saveStudentDetailWithConduct opens and closes its own stream. So this might be redundant or handled inside.
                                                 // Let's ensure saveStudentDetailWithConduct handles its own stream.
                                                 // The call to saveStudentDetailWithConduct is better.
            }
            cout << "Admitted students' data processed." << endl;
        } else { cout << "Student data not saved." << endl; }
    }
}

void inputGradesLoader(int mode) {
    vector<StudentSimple> current_accepted_students;
    ifstream ifs_loader;
    ifs_loader.open(main_student_data_file);

    if (!ifs_loader.is_open()) {
        cout << "Error: Failed to open " << main_student_data_file << " to load student list." << endl;
        return;
    }
    string line1, line2;
    while (getline(ifs_loader, line1) && getline(ifs_loader, line2)) {
        current_accepted_students.push_back({line2, line1}); 
    }
    ifs_loader.close();

    if (current_accepted_students.empty()) {
        cout << "No students found in " << main_student_data_file << ". Please register and admit students first." << endl;
        return;
    }

    if (mode == 1) { 
        inputGradesRecursive(current_accepted_students, current_accepted_students.size());
    } else if (mode == 2) { 
        cout << "\n--- Select Student to View Average ---" << endl;
        for (size_t i = 0; i < current_accepted_students.size(); i++) {
            cout << i + 1 << ". " << current_accepted_students[i].name << " (NISN: " << current_accepted_students[i].NISN << ")" << endl;
        }
        int choice;
        cout << "Enter student number (or 0 to go back): ";
        while (!(cin >> choice) || choice < 0 || choice > static_cast<int>(current_accepted_students.size())) {
            cout << "Invalid choice. Enter a number between 0 and " << current_accepted_students.size() << ": ";
            cin.clear(); clearInputBuffer();
        }
        clearInputBuffer();

        if (choice == 0) {
            cout << "Returning from student selection." << endl;
            return;
        }
        StudentSimple selected_student = current_accepted_students[choice - 1];
        displayAndCalculateAverage(selected_student, student_details_folder);
    } else {
        cout << "Unknown mode in inputGradesLoader." << endl;
    }
}

void inputGradesRecursive(const vector<StudentSimple>& students_list, int num_students) {
    cout << "\n--- Input Subject Grades ---" << endl;
    if (num_students == 0 || students_list.empty()) {
        cout << "No students available to input grades for." << endl;
        return;
    }
    cout << "Select student to input grade for:" << endl;
    for (int i = 0; i < num_students; i++) {
        cout << i + 1 << ". " << students_list[i].name << " (NISN: " << students_list[i].NISN << ")" << endl;
    }
    int choice_recursive;
    cout << "Enter student number (or 0 to go back): ";
    while (!(cin >> choice_recursive) || choice_recursive < 0 || choice_recursive > num_students) {
        cout << "Invalid choice. Please enter a number between 0 and " << num_students << ": ";
        cin.clear(); clearInputBuffer();
    }
    clearInputBuffer();

    if (choice_recursive == 0) {
        cout << "Returning from grade input." << endl;
        return;
    }

    StudentSimple selected_student_recursive = students_list[choice_recursive - 1];
    string student_file_path = student_details_folder + selected_student_recursive.NISN + "_" + selected_student_recursive.name + ".txt";
    
    ofstream ofs_local_grades;
    ofs_local_grades.open(student_file_path, ios::app);
    if (!ofs_local_grades.is_open()) {
        cout << "Error: Failed to open file " << student_file_path << " for appending grades." << endl;
        return; 
    }

    cout << "\nInputting grades for: " << selected_student_recursive.name << endl;
    string subject_name;
    int subject_grade_val;
    string add_more_subjects;
    do {
        cout << "Subject name: "; getline(cin, subject_name);
        cout << "Grade for " << subject_name << ": ";
        while(!(cin >> subject_grade_val) || subject_grade_val < 0 || subject_grade_val > 100){
            cout << "Invalid grade. Enter a number between 0-100: ";
            cin.clear(); clearInputBuffer();
        }
        clearInputBuffer();
        ofs_local_grades << "Subject: " << subject_name << ", Grade: " << subject_grade_val << endl;
        cout << "Grade for " << subject_name << " added." << endl;
        cout << "Add more subjects for THIS student? (y/n): ";
        cin >> add_more_subjects; clearInputBuffer(); cout << endl;
    } while (add_more_subjects == "y" || add_more_subjects == "Y");
    ofs_local_grades.close();
    cout << "Grades updated for " << selected_student_recursive.name << "." << endl;

    string input_for_another_student;
    cout << "Input grades for ANOTHER student? (y/n): ";
    getline(cin, input_for_another_student);
    if (input_for_another_student == "y" || input_for_another_student == "Y") {
        inputGradesRecursive(students_list, num_students); 
    } else {
        cout << "Finished inputting grades for this session." << endl;
    }
}

void displayAndCalculateAverage(const StudentSimple& selected_student, const string& student_details_folder) {
    cout << "\n--- Show Grades and Average for " << selected_student.name << " ---" << endl;
    string student_file_path = student_details_folder + selected_student.NISN + "_" + selected_student.name + ".txt";

    ifstream ifs_avg(student_file_path);
    if (!ifs_avg.is_open()) {
        cout << "Error: Failed to open file: " << student_file_path << endl;
        return;
    }

    string line;
    vector<int> grades_vec; 
    bool grades_found_flag = false; 

    cout << "Grades for " << selected_student.name << ":" << endl;
    while (getline(ifs_avg, line)) {
        size_t subject_pos = line.find("Subject: ");
        size_t grade_pos = line.find(", Grade: ");

        if (subject_pos != string::npos && grade_pos != string::npos && grade_pos > subject_pos) { 
            grades_found_flag = true;
            string subject_name = line.substr(subject_pos + string("Subject: ").length(),
                                              grade_pos - (subject_pos + string("Subject: ").length()));
            string grade_str_val = line.substr(grade_pos + string(", Grade: ").length()); 
            try {
                int grade_val_int = stoi(grade_str_val); 
                grades_vec.push_back(grade_val_int);
                cout << "- " << subject_name << ": " << grade_val_int << endl;
            } catch (const invalid_argument& e) {
                cerr << "Warning: Could not parse grade from line: " << line << " (Error: " << e.what() << ")" << endl;
            } catch (const out_of_range& e) {
                cerr << "Warning: Grade out of range in line: " << line << " (Error: " << e.what() << ")" << endl;
            }
        }
    }
    ifs_avg.close();

    if (!grades_found_flag) {
        cout << "No grades found for " << selected_student.name << "." << endl;
    } else {
        if (!grades_vec.empty()) {
            double sum_of_grades = accumulate(grades_vec.begin(), grades_vec.end(), 0.0);
            double average_grade_val = sum_of_grades / grades_vec.size(); 
            cout << fixed << setprecision(2);
            cout << "\nAverage grade for " << selected_student.name << ": " << average_grade_val << endl;
        } else {
            cout << "Cannot calculate average. No valid grades were parsed." << endl;
        }
    }
}

void menuConductLog() {
    int choice;
    do {
        cout << "\n+==============================+" << endl;
        cout << "|   Student Conduct Log Menu   |" << endl;
        cout << "+==============================+" << endl;
        cout << "1. Add Conduct Note" << endl;
        cout << "2. View Conduct Notes for Student" << endl;
        cout << "3. Back to Main Menu" << endl;
        cout << "Choose: ";
        while (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number: "; cin.clear(); clearInputBuffer();
        }
        clearInputBuffer();
        switch (choice) {
            case 1: addConductNote(); break;
            case 2: viewConductNotes(); break;
            case 3: cout << "Returning to Main Menu..." << endl; break;
            default: cout << "Invalid choice. Please try again!" << endl;
        }
        if (choice !=3) { cout << "Press Enter to continue..."; cin.get(); }
    } while (choice != 3);
}

void loadStudentDetailForConduct(student& s_detail, const string& nisn_str_param, const string& name_param) {
    s_detail.NISN = stoi(nisn_str_param);
    s_detail.name = name_param;
    s_detail.conduct_log.clear();
    s_detail.subject_grades.clear(); 
    s_detail.placeofbirth = "";
    s_detail.dateofbirth = "";
    s_detail.gender = "";
    s_detail.grade = 0.0f;

    string student_file_path = student_details_folder + nisn_str_param + "_" + name_param + ".txt";
    ifstream ifs_load_detail(student_file_path);
    if (ifs_load_detail.is_open()) {
        string line;
        bool conduct_section = false;
        while (getline(ifs_load_detail, line)) {
            if (line.rfind("Name: ", 0) == 0) s_detail.name = line.substr(6);
            else if (line.rfind("NISN: ", 0) == 0) { /* NISN from param */ }
            else if (line.rfind("Place of Birth: ", 0) == 0) s_detail.placeofbirth = line.substr(16);
            else if (line.rfind("Date of Birth: ", 0) == 0) s_detail.dateofbirth = line.substr(15);
            else if (line.rfind("Gender: ", 0) == 0) s_detail.gender = line.substr(8);
            else if (line.rfind("Admission Grade: ", 0) == 0) {
                try { s_detail.grade = stof(line.substr(17)); } catch(...) { s_detail.grade = 0.0f; }
            }
            else if (line.rfind("Subject: ", 0) == 0) {
                size_t subject_pos = line.find("Subject: ");
                size_t grade_pos = line.find(", Grade: ");
                if (subject_pos != string::npos && grade_pos != string::npos && grade_pos > subject_pos) {
                    string sn = line.substr(subject_pos + string("Subject: ").length(), grade_pos - (subject_pos + string("Subject: ").length()));
                    string gs_str = line.substr(grade_pos + string(", Grade: ").length());
                    try {
                        int gv_int = stoi(gs_str);
                        s_detail.subject_grades.push_back({sn, gv_int});
                    } catch (...) { /* ignore parsing error */ }
                }
            }
            else if (line == "--- Conduct Log ---") {
                conduct_section = true;
                continue;
            }
            if (conduct_section) {
                if (!line.empty() && line.rfind("Log: ", 0) == 0) {
                    s_detail.conduct_log.push_back(line);
                }
            }
        }
        ifs_load_detail.close();
    }
}

void saveStudentDetailWithConduct(const student& s_detail) {
    string student_file_path = student_details_folder + to_string(s_detail.NISN) + "_" + s_detail.name + ".txt";
    ofstream ofs_save_detail(student_file_path, ios::out);
    if (!ofs_save_detail.is_open()) {
        cout << "Error: Failed to open file " << student_file_path << " for saving details." << endl;
        return;
    }
    ofs_save_detail << "Name: " << s_detail.name << endl;
    ofs_save_detail << "NISN: " << s_detail.NISN << endl;
    ofs_save_detail << "Place of Birth: " << s_detail.placeofbirth << endl;
    ofs_save_detail << "Date of Birth: " << s_detail.dateofbirth << endl;
    ofs_save_detail << "Gender: " << s_detail.gender << endl;
    ofs_save_detail << "Admission Grade: " << s_detail.grade << endl;

    for(const auto& sg : s_detail.subject_grades) {
        ofs_save_detail << "Subject: " << sg.first << ", Grade: " << sg.second << endl;
    }

    if (!s_detail.conduct_log.empty()) {
        ofs_save_detail << "--- Conduct Log ---" << endl;
        for (const string& log_entry : s_detail.conduct_log) {
            ofs_save_detail << log_entry << endl;
        }
    }
    ofs_save_detail.close();
}

void addConductNote() {
    vector<StudentSimple> current_accepted_students;
    ifstream ifs_add_note_loader(main_student_data_file);
    if (!ifs_add_note_loader.is_open()) { cout << "Error: Failed to open " << main_student_data_file << endl; return; }
    string n, ni;
    while (getline(ifs_add_note_loader, ni) && getline(ifs_add_note_loader, n)) { current_accepted_students.push_back({n, ni}); }
    ifs_add_note_loader.close();
    if (current_accepted_students.empty()) { cout << "No admitted students found." << endl; return; }

    cout << "\n--- Add Conduct Note ---" << endl;
    cout << "Select student:" << endl;
    for (size_t i = 0; i < current_accepted_students.size(); ++i) {
        cout << i + 1 << ". " << current_accepted_students[i].name << " (NISN: " << current_accepted_students[i].NISN << ")" << endl;
    }
    int choice;
    cout << "Enter student number: ";
    while (!(cin >> choice) || choice < 1 || choice > static_cast<int>(current_accepted_students.size())) {
        cout << "Invalid choice. Try again: "; cin.clear(); clearInputBuffer();
    }
    clearInputBuffer();
    StudentSimple selected_simple_student = current_accepted_students[choice - 1];

    student student_to_update;
    loadStudentDetailForConduct(student_to_update, selected_simple_student.NISN, selected_simple_student.name);

    string date, type, note_desc;
    cout << "Enter date (YYYY-MM-DD): "; getline(cin, date);
    cout << "Enter note type (e.g., Praise, Warning, Observation): "; getline(cin, type);
    cout << "Enter note description: "; getline(cin, note_desc);

    string full_note = "Log: Date: " + date + ", Type: " + type + ", Note: " + note_desc;
    student_to_update.conduct_log.push_back(full_note);
    
    saveStudentDetailWithConduct(student_to_update);

    cout << "Conduct note added for " << student_to_update.name << "." << endl;
}

void viewConductNotes() {
    vector<StudentSimple> current_accepted_students;
    ifstream ifs_view_notes_loader(main_student_data_file);
    if (!ifs_view_notes_loader.is_open()) { cout << "Error: Failed to open " << main_student_data_file << endl; return; }
    string n, ni;
    while (getline(ifs_view_notes_loader, ni) && getline(ifs_view_notes_loader, n)) { current_accepted_students.push_back({n, ni}); }
    ifs_view_notes_loader.close();
    if (current_accepted_students.empty()) { cout << "No admitted students found." << endl; return; }

    cout << "\n--- View Conduct Notes ---" << endl;
    cout << "Select student:" << endl;
    for (size_t i = 0; i < current_accepted_students.size(); ++i) {
        cout << i + 1 << ". " << current_accepted_students[i].name << " (NISN: " << current_accepted_students[i].NISN << ")" << endl;
    }
    int choice;
    cout << "Enter student number: ";
    while (!(cin >> choice) || choice < 1 || choice > static_cast<int>(current_accepted_students.size())) {
        cout << "Invalid choice. Try again: "; cin.clear(); clearInputBuffer();
    }
    clearInputBuffer();
    StudentSimple selected_student = current_accepted_students[choice - 1];
    string student_file_path = student_details_folder + selected_student.NISN + "_" + selected_student.name + ".txt";

    ifstream ifs_view_student_file(student_file_path);
    if (!ifs_view_student_file.is_open()) { cout << "Error: Could not open student detail file: " << student_file_path << endl; return; }

    cout << "\n--- Conduct Log for " << selected_student.name << " ---" << endl;
    string line;
    bool in_conduct_section = false;
    bool found_logs = false;
    while (getline(ifs_view_student_file, line)) {
        if (line == "--- Conduct Log ---") {
            in_conduct_section = true;
            continue;
        }
        if (in_conduct_section) {
            if (line.rfind("Log: ", 0) == 0) {
                cout << line.substr(5) << endl;
                found_logs = true;
            }
        }
    }
    ifs_view_student_file.close();
    if (!found_logs) {
        if (in_conduct_section) cout << "No specific conduct log entries found." << endl;
        else cout << "Conduct log section not found for this student." << endl;
    }
}

// Helper function to get the latest tuition record for a student
bool getLatestTuitionRecordForPayment(int nisn_to_search, string& out_student_name, int& out_outstanding_balance) {
    ifstream tuition_ifs(tuition_file);
    if (!tuition_ifs.is_open()) {
        return false; // No file, so no previous record
    }

    int file_id;
    string line_content;
    bool record_found_for_nisn = false;
    
    // Temporary variables to store the details of the last found record for the specific NISN
    int latest_unpaid_temp = 0;
    string latest_name_temp = "";

    while (tuition_ifs >> file_id >> ws && getline(tuition_ifs, line_content)) {
        if (file_id == nisn_to_search) {
            stringstream ss(line_content);
            vector<string> tokens;
            string token_item;
            while (ss >> token_item) {
                tokens.push_back(token_item);
            }

            if (tokens.size() >= 2) { // Need at least 2 tokens for paid amount and unpaid balance
                string name_for_this_line;
                int paid_for_this_line, unpaid_for_this_line;
                try {
                    string unpaid_str = tokens.back(); tokens.pop_back();
                    if (tokens.empty()) continue; 
                    string paid_str = tokens.back(); tokens.pop_back();
                    
                    unpaid_for_this_line = stoi(unpaid_str);
                    // paid_for_this_line = stoi(paid_str); // Amount paid in that transaction, not needed for next calculation

                    for (const string& name_part : tokens) {
                        if (!name_for_this_line.empty()) name_for_this_line += " ";
                        name_for_this_line += name_part;
                    }
                    
                    record_found_for_nisn = true;
                    latest_unpaid_temp = unpaid_for_this_line;
                    latest_name_temp = name_for_this_line;
                } catch (const std::exception& e) {
                    // Skip malformed line
                    continue;
                }
            }
        }
    }
    tuition_ifs.close();

    if (record_found_for_nisn) {
        out_outstanding_balance = latest_unpaid_temp;
        out_student_name = latest_name_temp;
        return true;
    }
    return false; 
}

void payTuition() {
    string student_nisn_str;
    int student_nisn_int;
    string student_name_input_by_user; // Name input by user for this transaction
    int amount_paid_this_transaction;
    int new_outstanding_balance;

    cout << "\n--- Pay Tuition Fee ---" << endl;
    cout << "Please enter the student's NISN: ";
    while (true) {
        getline(cin, student_nisn_str);
        if (isValidNisn(student_nisn_str, student_nisn_int)) break;
        cout << "Invalid NISN. Please enter a numeric NISN: ";
    }

    int previous_outstanding_balance = 0;
    string name_from_file = ""; // Name from the last record in tuition.txt
    bool previous_record_found = getLatestTuitionRecordForPayment(student_nisn_int, name_from_file, previous_outstanding_balance);

    int current_tuition_due;
    string name_to_record;

    if (previous_record_found) {
        cout << "Found existing record for NISN " << student_nisn_int << "." << endl;
        cout << "Student Name (from last record): " << name_from_file << endl;
        name_to_record = name_from_file; // Use existing name for consistency

        if (previous_outstanding_balance == 0) {
            cout << "Tuition for " << name_from_file << " (NISN: " << student_nisn_int << ") is already fully paid." << endl;
            // Optionally, ask if they want to make an advance payment or donation
            // For now, just return if fully paid.
            return;
        }
        current_tuition_due = previous_outstanding_balance;
        cout << "Current outstanding balance: " << current_tuition_due << endl;
    } else {
        current_tuition_due = BASE_TUITION;
        cout << "No previous payment record found for NISN: " << student_nisn_int << "." << endl;
        cout << "Full tuition due: " << current_tuition_due << endl;
        cout << "Please enter the student's full name: ";
        getline(cin, student_name_input_by_user); // Only ask for name if no record found
        name_to_record = student_name_input_by_user;
    }
    
    cout << "Please enter the amount you wish to pay: ";
    while (!(cin >> amount_paid_this_transaction) || amount_paid_this_transaction < 0) {
        cout << "Invalid amount. Please enter a positive value: "; cin.clear(); clearInputBuffer();
    } clearInputBuffer();

    new_outstanding_balance = current_tuition_due - amount_paid_this_transaction;
    
    if (new_outstanding_balance <= 0) {
        cout << "Tuition has been paid in full. Thank you!" << endl;
        if (new_outstanding_balance < 0) {
            cout << "Change: " << -new_outstanding_balance << endl;
        }
        new_outstanding_balance = 0; // Ensure outstanding balance is not negative
    } else {
        cout << "New outstanding balance: " << new_outstanding_balance << endl;
    }

    ofstream ofs_local_tuition;
    ofs_local_tuition.open(tuition_file, ios::app);
    if (!ofs_local_tuition.is_open()) { cout << "Error: Failed to open " << tuition_file << " for writing!" << endl; return; }
    ofs_local_tuition << student_nisn_int << " " << name_to_record << " " << amount_paid_this_transaction << " " << new_outstanding_balance << endl; 
    ofs_local_tuition.close();
    cout << "Tuition payment record saved." << endl;
}

void searchTuitionStatus() { // This function's logic remains largely the same
    ifstream ifs_search_tuition(tuition_file);
    if (!ifs_search_tuition.is_open()) {
        cout << "Error: Failed to open " << tuition_file << ". No tuition data available or file not found." << endl;
        return;
    }
    int search_id_int;
    string search_id_str;
    cout << "\n--- Search Tuition Status ---" << endl;
    cout << "Enter student NISN to search: ";
    while (true) {
        getline(cin, search_id_str);
        if (isValidNisn(search_id_str, search_id_int)) {
            break;
        }
        cout << "Invalid NISN. Please enter a numeric NISN: ";
    }
    int file_id;
    string file_name_full_line;
    bool found = false;
    // Local struct for this function's specific needs is fine
    struct TuitionRecordSearch { 
        int id;
        string name;
        int paid_transaction; // Amount paid in the specific transaction
        int unpaid_final;     // The final unpaid balance in that record
    };
    TuitionRecordSearch latest_record_display = {-1, "", 0, 0};

    while (ifs_search_tuition >> file_id >> ws && getline(ifs_search_tuition, file_name_full_line)) {
        stringstream ss(file_name_full_line);
        vector<string> tokens;
        string token_item;
        while (ss >> token_item) {
            tokens.push_back(token_item);
        }
        if (tokens.size() >= 2) {
            string current_line_name;
            int current_file_paid_transaction, current_file_unpaid_final;
            try {
                string unpaid_str = tokens.back(); tokens.pop_back();
                if (tokens.empty()) { continue; }
                string paid_str = tokens.back(); tokens.pop_back();
                current_file_unpaid_final = stoi(unpaid_str);
                current_file_paid_transaction = stoi(paid_str);
                for (const string& name_part : tokens) {
                    if (!current_line_name.empty()) { current_line_name += " "; }
                    current_line_name += name_part;
                }
                if (file_id == search_id_int) {
                    latest_record_display = {file_id, current_line_name, current_file_paid_transaction, current_file_unpaid_final};
                    found = true;
                }
            } catch (const std::exception& e) {
                 // Silently skip malformed lines during search, or add cerr for debugging
            }
        }
    }
    ifs_search_tuition.close();
    if (found) {
        cout << "\n--- Student Tuition Status (Latest Record) ---" << endl;
        cout << "NISN: " << latest_record_display.id << endl;
        cout << "Name: " << (latest_record_display.name.empty() ? "[No Name Recorded]" : latest_record_display.name) << endl;
        cout << "Last Amount Paid (in that transaction): " << latest_record_display.paid_transaction << endl;
        cout << "Current Outstanding Balance: " << latest_record_display.unpaid_final << endl;
        if (latest_record_display.unpaid_final == 0) {
            cout << "Status: Tuition fully paid." << endl;
        } else {
            cout << "Status: Payment still outstanding." << endl;
        }
    } else {
        cout << "Student with NISN " << search_id_int << " not found in tuition records." << endl;
    }
}

void menuTuition() {
    int choice;
    do {
        cout << "\n+=========================+" << endl;
        cout << "|    Tuition Service Menu   |" << endl;
        cout << "+=========================+" << endl;
        cout << "1. Pay Tuition" << endl;
        cout << "2. Search Student's Tuition Status" << endl;
        cout << "3. Back to Main Menu" << endl;
        cout << "Choose a service: ";
        while (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number: "; cin.clear(); clearInputBuffer();
        } clearInputBuffer();
        switch (choice) {
            case 1: payTuition(); break;
            case 2: searchTuitionStatus(); break;
            case 3: cout << "Returning to Main Menu..." << endl; break;
            default: cout << "Invalid choice. Please try again!" << endl;
        } if(choice != 3) { cout << "Press Enter to continue..."; cin.get(); }
    } while (choice != 3);
}

int main() {
    int choice;
    do {
        // system("cls"); // Non-portable
        cout << "\n+===========================+" << endl;
        cout << "|   School Management Menu  |" << endl;
        cout << "+===========================+" << endl;
        cout << "1. Register New Students" << endl;
        cout << "2. Show Registration Results & Admit" << endl;
        cout << "3. Input Subject Grades" << endl;
        cout << "4. Show Student's Grades and Average" << endl;
        cout << "5. Tuition Fee Services" << endl;
        cout << "6. Manage Student Conduct Log" << endl;
        cout << "7. Exit " << endl;
        cout << "Choose: ";

        while (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number: "; cin.clear(); clearInputBuffer();
        }
        clearInputBuffer();

        switch (choice) {
            case 1: registration(); break;
            case 2: showRegistrationResult(); break;
            case 3: inputGradesLoader(1); break;
            case 4: inputGradesLoader(2); break;
            case 5: menuTuition(); break;
            case 6: menuConductLog(); break;
            case 7: cout << "Exiting program. Goodbye!" << endl; break;
            default: cout << "Invalid choice. Please try again!" << endl;
        }
        if (choice != 7 && choice != 5 && choice != 6) { 
             cout << "Press Enter to continue..."; cin.get();
        }
    } while (choice != 7); 
    return 0;
}