# Smart-Helmet

# Add stable HR signal processing with LMS filtering and anomaly detection (helmet.ipynb)

Algorithm Overview

1. Data Collection: Simultaneously collect raw PPG data (from MAX30100) and accelerometer data (X, Y, Z axes).
2. Preprocessing: Normalize and synchronize the data streams.
3. Adaptive Filtering: Use accelerometer data as a reference to remove motion-induced noise from the PPG signal.
4. Heart Rate Extraction: Detect peaks in the filtered PPG signal to calculate heart rate.
5. Anomaly Detection: Identify anomalies in the heart rate for distress or consciousness monitoring.
6. Validation: Test with sample data to ensure noise removal and accurate HR detection.




1. Data Collection
PPG Data: Raw signal from MAX30100 (e.g., infrared LED readings).

Accelerometer Data: 3-axis acceleration (X, Y, Z) to capture motion.


2. Preprocessing
Normalization: Scale PPG and accelerometer data to a common range (e.g., 0 to 1) to simplify processing.

Magnitude Calculation: For accelerometer, compute the total motion magnitude
￼



3. Adaptive Filtering (Software-Based)

Goal: Remove motion artifacts from the PPG signal using accelerometer data as a noise reference

Method: Use a Least Mean Squares (LMS) adaptive filter, which adjusts its coefficients dynamically to minimize the error between the noisy PPG signal and the clean signal.

￼
￼


4. Heart Rate Extraction

Peak Detection: Use a peak-finding algorithm (e.g., find local maxima) on the filtered PPG signal (e(n)).

Heart Rate Calculation: Measure the time between consecutive peaks (T) in seconds and compute:

￼



5. Anomaly Detection
Thresholds:
* Normal HR: 60–100 BPM.
* Anomaly: HR < 40 BPM (distress/unconsciousness) or HR > 150 BPM (panic).


* Cross-Check: If accelerometer data indicates a crash (e.g., A_{mag} > 10G), prioritize an SOS alert if HR anomaly is detected.



6. SOS Trigger

* If an anomaly persists for a set duration (e.g., 5 seconds), send an SOS alert with GPS coordinates via the ESP32.




### Explaining the Code 


#### 1. Loading the Data
"We start by grabbing data from a file called `hehe.csv`. Imagine this file is like a log from the helmet’s sensors:
- **Time**: A list of timestamps, like a clock ticking every 0.01 seconds for 2 seconds total.
- **PPG (Heart Signal)**: This is from a sensor called MAX30100 that shines infrared light on the rider’s skin to track blood flow—basically, their heartbeat.
- **Accelerometer (Motion)**: This tells us how much the helmet’s moving, like if the bike’s cruising or crashing.

The code reads this file and pulls out these three columns so we can work with them."

#### 2. Cleaning Up the Data (Preprocessing)
"Next, we tidy up the numbers. The PPG and motion data come in big, messy units—like PPG might be around 32,000, and motion’s in ‘g’ units like 1.5 or 15. We squash them down to a simpler scale, between 0 and 1, so it’s easier to compare them. It’s like zooming everything to the same size on a map. The time stays as is because it’s just our clock."

#### 3. Filtering Out Noise
"Now, here’s the cool part. The PPG signal gets bumpy when the rider moves—like if they hit a pothole or crash, it messes up the heartbeat reading. We use something called an LMS filter—it’s like a smart cleaner. It looks at the motion data and says, ‘Oh, this bump is from the bike shaking, not the heart,’ and smooths it out. So, we end up with a cleaner heartbeat signal that’s not confused by all the jolts."

#### 4. Finding the Heart Rate
"With the cleaned-up signal, we hunt for the heartbeat peaks—those little spikes that show each beat. Imagine looking at a wavy line and spotting the tops. The code checks where the signal jumps up above a certain level (like 0.5 in our 0-to-1 scale) and marks those spots. Then it measures how long between peaks—like, if it’s 0.75 seconds apart, that’s 80 beats per minute (because 60 divided by 0.75 is 80). It does this for both the messy raw signal and the cleaned-up one."

#### 5. Spotting Trouble (Anomaly Detection)
- **Crash**: If the motion spikes over 10g—like a hard smack—that’s bad news.
- **Heart Rate Weirdness**: If the heart rate drops below 40 (super slow, maybe they’re knocked out) or shoots above 150 (panic mode), that’s also trouble.

The code keeps an eye on this over time. If either happens and sticks around for 5 seconds—like 500 ticks at our speed—it’s serious. But since our test file is only 2 seconds long, it also checks for quick red flags, like an instant crash."

#### 6. Sending an SOS
"If we spot that big trouble—like a crash or weird heart rate lasting too long—the code pretends to send an SOS. In real life, it’d use the helmet’s ESP32 chip to ping GPS coordinates to emergency services. For now, it just yells ‘SOS!’ on the screen. If it’s just a quick scare, like a crash with no long-term issue, it still warns us right away."

#### 7. Showing the Results
"Finally, it draws three pictures:
- **Raw PPG**: The messy heartbeat signal with all the bumps, showing where it thinks the beats are (red dots).
- **Motion**: How much the helmet’s shaking, with a line at 10g to spot crashes.
- **Cleaned PPG**: The smoother heartbeat signal after filtering, with its beat guesses.

It also tells us the heart rate—usually the raw one’s too noisy to figure out, but the cleaned one should say something like 80 beats per minute. Plus, it flags any trouble it found."



### Tying It to the Helmet
"So imagine you’re riding with this helmet. It’s quietly watching your heartbeat and how you’re moving. If you crash—bam, motion spikes—it cleans up the heartbeat signal to see if you’re okay. If your heart rate goes crazy or stops, and it lasts a bit, it’s like, ‘Yup, this rider needs help!’ and sends an alert. This code’s testing that idea with fake data from `hehe.csv`, but it’s the same logic we’d put in the real helmet."



### What They’ll See
"Run this, and you’ll see graphs: the raw heartbeat all over the place, the motion spiking at 1 second like a crash, and the cleaned heartbeat looking steadier. It’ll probably say the raw heart rate’s a mess, the cleaned one’s around 80, and it caught a crash at 1 second. That’s it keeping an eye out for you!"



###
- "The numbers—like 31000 or 30000—were for the raw data, but we scaled it down, so we use 0.5 and 0.4 instead."
- "The 5-second thing doesn’t fully kick in here because our test is short, but it’s ready for the real deal."

---










