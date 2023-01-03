# Real time hand keypoint detection through motion

A simple program I made with traditional computer vision techiniques in OpenCV. Unlike every other hand detection method I found on github, you do not need to isolte your hand against a plain wall. It works by using background subtraction to find keypoints instead of simple image transforms to a binary image.

### 1) Image transform
Mirror, convert to grayscale, and blur webcam frame

### 2) Perform Background subtraction.
You are left with a binary image of the estimated foreground but the image is sparse

### 3) Morphological transformations in order to fill in sparsity and remove noise

### 4) Find contours and then convex hulls of morpholocial output
Provides final output of keypoints (good for fingertip detection), only detectable when moving hand

### 5) Game mechanics
Draw a circle and have it switch direction when hitting a wall or coming in contact with a hand keypoint.
In order to detect collision, use pointPolygonTest function to check if a point within the radius of the circle (10 degree incriments) is within any of the convex hull keypoints.


