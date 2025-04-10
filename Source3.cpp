#include <iostream>
#include <unordered_map>
#include <climits>  
#include <vector>
#include <algorithm>
#include <map>
#include <mutex>
#include <condition_variable>

using namespace std;

struct Data {
	int timeUs = 0;
	int densityInfo = -1;
	int positionInfo = -1;
};

class CircularVector {
private:
	vector<Data> data;
	int maxTime = 0;
public:
	void push(Data info) {

		// Determines upper bounds for time
		if (info.timeUs > maxTime) {
			maxTime = info.timeUs;
		}

		// Determines the minTime for the bounds of the last 5 seconds
		int minTime = maxTime - 5000000;
		if (minTime < 0) {
			minTime = 0;
		}

		// Deals with scenario where densityInfo is added but positionInfo isn't and vice-versa
		// Basically my way of making sure the two functions map onto the same timeUs assigned to them
		for (auto& it : data) {
			if (it.timeUs == info.timeUs) {
				if (info.densityInfo != -1) it.densityInfo = info.densityInfo;
				if (info.positionInfo != -1) it.positionInfo = info.positionInfo;
				return;
			}
		}

		// Instead of deallocation and allocation, we reuse already existing space
		/* Decided to just erase instead, better in terms of space complexity but kept this code anyway to show how I tried to 
		* replace old values
		for (auto& it : data) {
			if (it.timeUs < minTime) {
				it.timeUs = info.timeUs;
				it.densityInfo = info.densityInfo;
				it.positionInfo = info.positionInfo;
				return;
			}
		}
		*/
		data.push_back(info); // If we can't reuse, we'll just push

		// Remove spots in array that don't fit 5 second range
		data.erase(remove_if(data.begin(), data.end(), [minTime](const Data& d) {
				return d.timeUs < minTime;
		}), data.end());

	}

	// Gives us data
	vector<Data> getData() {
		return data;
	}

	// Gives us Max Time
	int getmaxTime() {
		return maxTime;
	}

};

// Circular Vector dataStructure that helps us use the methods push and getData()
CircularVector dataStructure;
// Variables to ensure thread safety
mutex mtx;
condition_variable cv;
bool dataDensityReady = false;
bool dataPositionReady = false;

// Functions ---------------------------------------------------------------------------------------------------------------------
void MeasureDensityReady(int density, int time_uS) {
	lock_guard<mutex> lock(mtx); // locks to ensure other threads wait

	Data info;
	info.densityInfo = density;
	info.timeUs = time_uS;
	dataStructure.push(info);

	dataDensityReady = true; // Establishes Density data is ready
	cv.notify_one(); // Only notifying CalculateDensityValues
}
//------------------------------------------------------------------------------------------------------------------------------
void MeasurePositionReady(int position_mm, int time_uS) {
	lock_guard<mutex> lock(mtx);

	Data info;
	info.positionInfo = position_mm;
	info.timeUs = time_uS;
	dataStructure.push(info);

	dataPositionReady = true; // Establishes Position data is ready
	cv.notify_one(); // Only notifying CalculateDensityValues
}
//------------------------------------------------------------------------------------------------------------------------
void CalculateDensityValues(int min_pos_mm, int max_pos_mm, int* mean_density,
	int* min_density, int* median_density) {
	unique_lock<mutex> lock(mtx);

	vector<Data> totalData = dataStructure.getData(); // Gives me the data vector array we populated
	vector<int> values; // Vector that will be filled with all density values between min_pos_mm and max_pos_mm

	int min = INT_MAX;
	int sum = 0;

	// There might be a few pieces of data that haven't been replaced yet
	// This condition is to deal with those scenarios
	int getMax = dataStructure.getmaxTime();
	int getMin = getMax - 5000000;

	// Establish values for min, sum (for mean), and values vector (for median)../
	// Makes sure its in between the established position
	// Makes sure we're not including old data that's not relevant
	for (auto d : totalData) {
		if (d.positionInfo >= min_pos_mm && d.positionInfo <= max_pos_mm &&
			d.timeUs >= getMin && d.timeUs <= getMax) {
			// Min
			if (d.densityInfo < min) {
				min = d.densityInfo;
			}
			// Sum (for mean)
			sum += d.densityInfo;
			// Filtered Data for median calculation
			values.push_back(d.densityInfo);
		}
	}

	*min_density = min;
	*mean_density = sum / values.size();

	// Determine the median
	sort(values.begin(), values.end());
	int middle = values.size() / 2;

	// If even, we need to add middle two numbers and divide by 2
	if (values.size() % 2 == 0) {
		*median_density = (values[middle - 1] + values[middle]) / 2;
	} // If odd, middle index will be where it should be
	else {
		*median_density = values[middle];
	}

	// Reset locks for next calculation
	dataDensityReady = false;
	dataPositionReady = false;
}

void testFunctions() {
	MeasureDensityReady(100, 1042350);
	MeasureDensityReady(110, 2132470);
	MeasureDensityReady(120, 3326890);
	MeasureDensityReady(130, 4024450);
	MeasureDensityReady(140, 5012030);
	MeasureDensityReady(150, 6102330);
	MeasureDensityReady(160, 7440560);
	MeasureDensityReady(170, 8032010);
	MeasureDensityReady(180, 9320350);
	MeasureDensityReady(190, 12030765);

	MeasurePositionReady(10, 1042350);
	MeasurePositionReady(15, 2132470);
	MeasurePositionReady(20, 3326890);
	MeasurePositionReady(25, 4024450);
	MeasurePositionReady(30, 5012030);
	MeasurePositionReady(35, 6102330);
	MeasurePositionReady(40, 7440560);
	MeasurePositionReady(45, 8032010);
	MeasurePositionReady(50, 9320350);
	MeasurePositionReady(55, 12030765);

	// Print data after wrap-around
	cout << "\nData after exceeding 5000000 threshold (wrap-around):\n";
	for (const auto& d : dataStructure.getData()) {
		cout << "TimeUs: " << d.timeUs << ", Density: " << d.densityInfo << ", Position: " << d.positionInfo << endl;
	}
	//----------------------------------------------------------
	int mean = 0;
	int min = 0;
	int median = 0;

	CalculateDensityValues(20, 55, &mean, &min, &median);

	cout << "Mean Density: " << mean << endl;
	cout << "Min Density: " << min << endl;
	cout << "Median Density: " << median << endl;
}

int main() {
	testFunctions();
	return 0;
}
