#include <opencv2/opencv.hpp>
#include <iostream>
#include <sstream>
#include <random>

int main()
{
    
    cv::VideoCapture cap;
    cap.open(0);

    if (!cap.isOpened()) {
        std::cerr << "Couldn't open camera" << std::endl;
        return -1;
    }

    cv::Mat frame;
    cap >> frame;

    int width = frame.size().width;
    int height = frame.size().height;
    int yy = 21;
    int d = 7;
    const double PI = std::atan(1.0) * 4;
    int rad;
    int morph_size;
    int elType;
    int new_xx;
    int new_yy;
    int new_angle;
    int framesWait = 10;
    double result = -1;

    // Random x value, angle for start posiiton
    std::random_device seed;
    std::mt19937 gen{ seed() }; // seed the generator
    std::uniform_int_distribution<int> dist{ 20, (width - 20) }; // set min and max
    int xx = dist(gen);
    std::uniform_int_distribution<int> dist2{ 35, 145 }; // set min and max
    int angle = dist2(gen);
    cv::RNG rng(12345);

    //create Background Subtractor objects
    cv::Ptr<cv::BackgroundSubtractor> pBackSub;
    pBackSub = cv::createBackgroundSubtractorMOG2(500, 256, false);

    for (;;) {
        cap >> frame;
        if (frame.empty()) break;

        // Mirroring image
        cv::Mat frame_flipped;
        cv::flip(frame, frame_flipped, 1);

        cv::Mat frame_flipped2;
        cv::flip(frame, frame_flipped2, 1);

        // Grayscale
        cv::Mat greyMat;
        cv::cvtColor(frame_flipped, greyMat, cv::COLOR_BGR2GRAY);
        // Blur
        cv::blur(greyMat, greyMat, cv::Size(3, 3));

        // Threshold (Did not end up using)
        //cv::threshold(greyMat, greyMat, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
        
        // Background separation
        cv::Mat fgMask;
        pBackSub->apply(greyMat, fgMask, 0.75);

        //imshow("Blurred Greyscale", greyMat);
        imshow("Background Removal", fgMask);

        // Morph
        morph_size = 2;
        elType = 0;
        cv::Mat element = cv::getStructuringElement(elType, cv::Size(2 * morph_size + 1, 2 * morph_size + 1), cv::Point(morph_size, morph_size));
        cv::morphologyEx(fgMask, fgMask, cv::MORPH_OPEN, element);
        cv::morphologyEx(fgMask, fgMask, cv::MORPH_CLOSE, element);



        // Contours and convexHull
        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;

        cv::findContours(fgMask, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

        std::vector<std::vector<cv::Point> >hull(contours.size());
        for (size_t i = 0; i < contours.size(); i++)
        {
            convexHull(cv::Mat(contours[i]), hull[i], true);
        }

        std::vector<std::vector<cv::Vec4i>> defects(contours.size());
        for (size_t i = 0; i < contours.size(); i++)
        {
            cv::Scalar color = cv::Scalar(128, 255, 8);
            drawContours(frame_flipped2, hull, (int)i, color, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
        }

        /// Check collision
        // For each contour
        if (framesWait <= 0) {
            for (int i = 0; i < hull.size(); ++i)
            {
                // For each 10 degree increment around the circle
                for (int j = 0; j <= 360; j = j + 5)
                {
                    new_angle = j;
                    new_xx = xx + (d * cos(new_angle * PI / 180));
                    new_yy = yy + (d * sin(new_angle * PI / 180));
                    result = cv::pointPolygonTest(hull[i], cv::Point2f((float)new_xx, (float)new_yy), false);
                    // if a collision is detected
                    if (result >= 0) {
                        angle = (180 + new_angle);
                        framesWait = 10;
                        result = -1;
                        goto contourBreak;
                    }
                }
            }
        }

        contourBreak:

        // Drawing Circle
        cv::circle(frame_flipped, cv::Point(xx, yy), 20, (0, 0, 255), 2);

        cv::circle(fgMask, cv::Point(xx, yy), 20, (0, 0, 255), 2);
        imshow("After Morphology", fgMask);

        // Calculating next circle location
        xx = xx + (d * cos(angle * PI / 180));
        yy = yy + (d * sin(angle * PI / 180));

        // If it hits a wall
        if (xx <= 20) angle = (180 - angle), xx = xx + d;
        if (xx >= width - 20) angle = (180 - angle), xx = xx - d;
        if (yy >= height - 20) angle = (360 - angle), yy = yy - d;
        if (yy <= 20 ) angle = (360 - angle), yy = yy + d;

        // If we touch it
        if (result >= 0) angle = (180 - angle);

        // Display
        cv::imshow("Contours", frame_flipped2);
        cv::imshow("Final Output", frame_flipped);

        // Display new frame every 33 ms (30 fps)
        if ((char)cv::waitKey(33) >= 0) break;

        //Waiting for time to pass before the next collision is allowed
        framesWait = framesWait - 1;

    }
    return 0;
 
}