# MicroTecExercise

Created Github to make edits if required, apologies for the multiple emails. I made my own test cases to test my code, you can ignore them.

Changes:
1. Added mutex and condition variable to achieve thread safety
2. Changed implementation of data structure so I wouldn't just run out of memory due to amount of insertions
--> Attempted a ciruclar buffer approach that replaced unneeded values
--> Created a new class in order to do so
3. Switched back and forth in regards to choosing to clear data (like a typical circular buffer) or to keep old values, ran into two problems...
--> Old values were relevant and important at times to establish the 5 second mark so they couldn't be cleared
--> Some old values were also not relevant and important to clear to make sure unneeded information wasn't put into the 5 second mark
4. Decided to establish the maximum amount of time attained through MeasureDensity and MeasurePosition, create a variable that subtracts that value by 5s in order to get the last 5 seconds which from my test cases seems to work 
5. I at first basically created a system where I just replaced old unneeded values within the spots of the array with new values. I wanted to try the less intuitive route which was to just erase/delete those spots in memory, but I thought about it and in terms of space complexity, I don't think my way of doing it would be very smart. I'm better off just deallocating the memory, so I ended up just using data.erase but kept my previous implementation in comments to show what I was going for 
