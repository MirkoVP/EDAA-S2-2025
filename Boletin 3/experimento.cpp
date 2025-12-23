#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>
#include <cmath>
#include <sdsl/suffix_arrays.hpp>

using namespace std;
using namespace sdsl;

//Rabin-Karp Implementation
int64_t rabin_karp(const string& pat, const string& txt, int q) {
    const int d = 256;
    int64_t count = 0;
    int txt_length = txt.length();
    int pat_length = pat.length();
    
    if (pat_length > txt_length) return 0;
    
    int p = 0, t = 0, h = 1;
    
    for (int i = 0; i < pat_length - 1; i++) {
        h = (h * d) % q;
    }
    
    for (int i = 0; i < pat_length; i++) {
        p = (d * p + pat[i]) % q;
        t = (d * t + txt[i]) % q;
    }
    
    for (int i = 0; i <= txt_length - pat_length; i++) {
        if (p == t) {
            int j = 0;
            while (j < pat_length && txt[i + j] == pat[j]) {
                j++;
            }
            if (j == pat_length) {
                count++;
            }
        }
        
        if (i < txt_length - pat_length) {
            t = (d * (t - txt[i] * h) + txt[i + pat_length]) % q;
            if (t < 0) t += q;
        }
    }
    
    return count;
}

//FM-Index Wrapper
class FMIndexWrapper {
private:
    csa_wt<> fm_index;
    
public:
    void build(const string& text) {
        construct_im(fm_index, text, 1);
    }
    
    size_t count(const string& pattern) {
        return sdsl::count(fm_index, pattern.begin(), pattern.end());
    }
    
    size_t memory_usage() {
        return size_in_bytes(fm_index);
    }
};

//Utility Functions
string load_text(const string& filename, size_t max_size = 0) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Cannot open file " << filename << endl;
        exit(1);
    }
    
    string text;
    if (max_size > 0) {
        text.resize(max_size);
        file.read(&text[0], max_size);
        text.resize(file.gcount());
    } else {
        text.assign(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
    }
    
    return text;
}

string generate_pattern(const string& text, int length, mt19937& rng) {
    if (length > text.length()) return "";
    uniform_int_distribution<size_t> dist(0, text.length() - length);
    size_t pos = dist(rng);
    return text.substr(pos, length);
}

vector<double> compute_stats(vector<double>& times) {
    sort(times.begin(), times.end());
    size_t n = times.size();
    
    double mean = 0;
    for (double t : times) mean += t;
    mean /= n;
    
    double stdev = 0;
    for (double t : times) {
        double dev = t - mean;
        stdev += dev * dev;
    }
    stdev = sqrt(stdev / (n - 1));
    
    double q0 = times[0];
    double q1 = times[n / 4];
    double q2 = times[n / 2];
    double q3 = times[3 * n / 4];
    double q4 = times[n - 1];
    
    return {mean, stdev, q0, q1, q2, q3, q4};
}

//Experimento 1: Efecto del tamaño del texto
void experiment_text_size(const string& base_filename, const string& output_file) {
    cout << "\n=== Experiment 1: Text Size Effect ===" << endl;
    
    ofstream out(output_file);
    out << "text_size_mb,algorithm,construction_time_ms,mean_search_time_us,stdev_search_time_us,memory_mb,pattern_length,occurrences" << endl;
    
    random_device rd;
    mt19937 rng(rd());
    
    //Tamaños de texto a probar (en MB)
    vector<size_t> sizes_mb = {10, 20, 50, 100, 200, 500, 1000};
    int pattern_length = 10;
    int runs = 30;
    
    for (size_t size_mb : sizes_mb) {
        size_t size_bytes = size_mb * 1024 * 1024;
        
        cout << "\nTesting with text size: " << size_mb << " MB" << endl;
        
        //Cargar texto
        string text = load_text(base_filename, size_bytes);
        if (text.empty()) continue;
        
        //Generar patrón
        string pattern = generate_pattern(text, pattern_length, rng);
        
        cout << "  Building FM-Index..." << endl;
        FMIndexWrapper fm;
        
        auto start = chrono::high_resolution_clock::now();
        fm.build(text);
        auto end = chrono::high_resolution_clock::now();
        double fm_construction_time = chrono::duration<double, milli>(end - start).count();
        
        cout << "  Searching with FM-Index..." << endl;
        vector<double> fm_times(runs);
        size_t fm_count = 0;
        
        for (int i = 0; i < runs; i++) {
            start = chrono::high_resolution_clock::now();
            fm_count = fm.count(pattern);
            end = chrono::high_resolution_clock::now();
            fm_times[i] = chrono::duration<double, micro>(end - start).count();
        }
        
        auto fm_stats = compute_stats(fm_times);
        double fm_memory = fm.memory_usage() / (1024.0 * 1024.0);
        
        out << size_mb << ",FM-Index," << fm_construction_time << "," 
            << fm_stats[0] << "," << fm_stats[1] << "," << fm_memory << "," 
            << pattern_length << "," << fm_count << endl;
        


        cout << "  Searching with Rabin-Karp..." << endl;
        vector<double> rk_times(runs);
        size_t rk_count = 0;
        
        for (int i = 0; i < runs; i++) {
            start = chrono::high_resolution_clock::now();
            rk_count = rabin_karp(pattern, text, 101);
            end = chrono::high_resolution_clock::now();
            rk_times[i] = chrono::duration<double, micro>(end - start).count();
        }
        
        auto rk_stats = compute_stats(rk_times);
        double rk_memory = text.size() / (1024.0 * 1024.0);
        
        out << size_mb << ",Rabin-Karp,0," 
            << rk_stats[0] << "," << rk_stats[1] << "," << rk_memory << "," 
            << pattern_length << "," << rk_count << endl;
        
        cout << "  FM-Index: " << fm_count << " occurrences, " 
             << fm_stats[0] << " us (construction: " << fm_construction_time << " ms)" << endl;
        cout << "  Rabin-Karp: " << rk_count << " occurrences, " 
             << rk_stats[0] << " us" << endl;
    }
    
    out.close();
    cout << "\nResults saved to " << output_file << endl;
}

//Experimento 2: Efecto del tamaño del patrón
void experiment_pattern_size(const string& filename, const string& output_file) {
    cout << "\n=== Experiment 2: Pattern Size Effect ===" << endl;
    
    ofstream out(output_file);
    out << "pattern_length,algorithm,mean_search_time_us,stdev_search_time_us,occurrences" << endl;
    
    random_device rd;
    mt19937 rng(rd());
    
    //Cargar texto completo
    cout << "Loading text..." << endl;
    string text = load_text(filename);
    cout << "Text size: " << text.size() / (1024.0 * 1024.0) << " MB" << endl;
    
    //Construir FM-Index una sola vez
    cout << "Building FM-Index..." << endl;
    FMIndexWrapper fm;
    auto start = chrono::high_resolution_clock::now();
    fm.build(text);
    auto end = chrono::high_resolution_clock::now();
    cout << "FM-Index built in " << chrono::duration<double, milli>(end - start).count() << " ms" << endl;
    
    //Longitudes de patrón a probar
    vector<int> pattern_lengths = {5, 10, 15, 20, 25, 30, 40, 50, 75, 100};
    int runs = 30;
    
    for (int plen : pattern_lengths) {
        cout << "\nTesting pattern length: " << plen << endl;
        
        string pattern = generate_pattern(text, plen, rng);
        
        //FM-Index
        vector<double> fm_times(runs);
        size_t fm_count = 0;
        
        for (int i = 0; i < runs; i++) {
            start = chrono::high_resolution_clock::now();
            fm_count = fm.count(pattern);
            end = chrono::high_resolution_clock::now();
            fm_times[i] = chrono::duration<double, micro>(end - start).count();
        }
        
        auto fm_stats = compute_stats(fm_times);
        out << plen << ",FM-Index," << fm_stats[0] << "," << fm_stats[1] << "," << fm_count << endl;
        
        //Rabin-Karp
        vector<double> rk_times(runs);
        size_t rk_count = 0;
        
        for (int i = 0; i < runs; i++) {
            start = chrono::high_resolution_clock::now();
            rk_count = rabin_karp(pattern, text, 101);
            end = chrono::high_resolution_clock::now();
            rk_times[i] = chrono::duration<double, micro>(end - start).count();
        }
        
        auto rk_stats = compute_stats(rk_times);
        out << plen << ",Rabin-Karp," << rk_stats[0] << "," << rk_stats[1] << "," << rk_count << endl;
        
        cout << "  FM-Index: " << fm_stats[0] << " us" << endl;
        cout << "  Rabin-Karp: " << rk_stats[0] << " us" << endl;
    }
    
    out.close();
    cout << "\nResults saved to " << output_file << endl;
}



int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <experiment_type>" << endl;
        cerr << "Experiments:" << endl;
        cerr << "  1 - Text size effect using proteins.txt (1GB)" << endl;
        cerr << "       Tests with text sizes: 10, 20, 50, 100, 200, 500, 1000 MB" << endl;
        cerr << "  2 - Pattern size effect on proteins200MB.txt (200MB)" << endl;
        cerr << "       Tests with pattern lengths: 5, 10, 15, 20, 25, 30, 40, 50, 75, 100" << endl;
        cerr << "  3 - Pattern size effect on proteins.txt (1GB)" << endl;
        cerr << "       Tests with pattern lengths: 5, 10, 15, 20, 25, 30, 40, 50, 75, 100" << endl;
        cerr << "\nNote: Make sure proteins200MB.txt and proteins.txt are in the current directory" << endl;
        return 1;
    }
    
    int experiment = atoi(argv[1]);
    
    cout << "Starting experiments..." << endl;
    cout << "Files expected:" << endl;
    cout << "  - proteins200MB.txt (200 MB)" << endl;
    cout << "  - proteins.txt (1 GB)" << endl << endl;
    
    switch (experiment) {
        case 1:
            cout << "Running Experiment 1: Text Size Effect" << endl;
            cout << "Using: proteins.txt (loading different sized chunks)" << endl;
            experiment_text_size("proteins.txt", "results_text_size.csv");
            break;
        case 2:
            cout << "Running Experiment 2: Pattern Size Effect" << endl;
            cout << "Using: proteins200MB.txt (200 MB)" << endl;
            experiment_pattern_size("proteins200MB.txt", "results_pattern_size_200mb.csv");
            break;
        case 3:
            cout << "Running Experiment 3: Pattern Size Effect" << endl;
            cout << "Using: proteins.txt (1 GB)" << endl;
            experiment_pattern_size("proteins.txt", "results_pattern_size_1gb.csv");
            break;
        default:
            cerr << "Invalid experiment type. Use 1, 2, or 3." << endl;
            return 1;
    }
    
    cout << "\n=== Experiment completed successfully ===" << endl;
    
    return 0;
}