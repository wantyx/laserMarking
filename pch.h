// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "framework.h"
#include <string>
#include "line2Dup.h"
#include <memory>
#include <assert.h>
#include <chrono>

using namespace std;
using namespace cv;

struct point{
	float x;
	float y;
};

struct recognizedObjectLocation
{
	point topleft;
	point topright;
	point bottomleft;
	point bottomright;
	point center;
	float angle;
	float scale;
};

extern "C" _declspec(dllexport) void train(string imgLocation, string saveTemplateDIR, int num_feature, Rect roi, int padding, string class_id);
_declspec(dllexport) vector<recognizedObjectLocation> test(string testlocation, string loadTemplateDir, int num_feature, Rect train_roi, int train_padding, string class_id, int score_thershold = 90, int nms_thershold = 0.1);
#endif //PCH_H