// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include "paddle_api.h"
#include "paddle_inference_api.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <vector>

#include <cstring>
#include <fstream>
#include <math.h>
#include <numeric>

#include "preprocess_op.h"

namespace Feature {
    void Permute::Run(const cv::Mat *im, float *data) {
        int rh = im->rows;
        int rw = im->cols;
        int rc = im->channels();
        for (int i = 0; i < rc; ++i) {
            cv::extractChannel(*im, cv::Mat(rh, rw, CV_32FC1, data + i * rh * rw), i);
        }
    }

    void Normalize::Run(cv::Mat *im, const std::vector<float> &mean,
                        const std::vector<float> &std, float scale) {
        (*im).convertTo(*im, CV_32FC3, scale);
        for (int h = 0; h < im->rows; h++) {
            for (int w = 0; w < im->cols; w++) {
                im->at<cv::Vec3f>(h, w)[0] =
                        (im->at<cv::Vec3f>(h, w)[0] - mean[0]) / std[0];
                im->at<cv::Vec3f>(h, w)[1] =
                        (im->at<cv::Vec3f>(h, w)[1] - mean[1]) / std[1];
                im->at<cv::Vec3f>(h, w)[2] =
                        (im->at<cv::Vec3f>(h, w)[2] - mean[2]) / std[2];
            }
        }
    }

    void CenterCropImg::Run(cv::Mat &img, const int crop_size) {
        int resize_w = img.cols;
        int resize_h = img.rows;
        int w_start = int((resize_w - crop_size) / 2);
        int h_start = int((resize_h - crop_size) / 2);
        cv::Rect rect(w_start, h_start, crop_size, crop_size);
        img = img(rect);
    }

    void ResizeImg::Run(const cv::Mat &img, cv::Mat &resize_img,
                        int resize_short_size, int size) {
        int resize_h = 0;
        int resize_w = 0;
        if (size > 0) {
            resize_h = size;
            resize_w = size;
        } else {
            int w = img.cols;
            int h = img.rows;

            float ratio = 1.f;
            if (h < w) {
                ratio = float(resize_short_size) / float(h);
            } else {
                ratio = float(resize_short_size) / float(w);
            }
            resize_h = round(float(h) * ratio);
            resize_w = round(float(w) * ratio);
        }
        cv::resize(img, resize_img, cv::Size(resize_w, resize_h));
    }

} // namespace Feature

namespace MobileNetV3 {

void Permute::Run(const cv::Mat *im, float *data) {
  int rh = im->rows;
  int rw = im->cols;
  int rc = im->channels();
  for (int i = 0; i < rc; ++i) {
    cv::extractChannel(*im, cv::Mat(rh, rw, CV_32FC1, data + i * rh * rw), i);
  }
}

void Normalize::Run(cv::Mat *im, const std::vector<float> &mean,
                    const std::vector<float> &scale, const bool is_scale) {
  double e = 1.0;
  if (is_scale) {
    e /= 255.0;
  }
  (*im).convertTo(*im, CV_32FC3, e);
  for (int h = 0; h < im->rows; h++) {
    for (int w = 0; w < im->cols; w++) {
      im->at<cv::Vec3f>(h, w)[0] =
          (im->at<cv::Vec3f>(h, w)[0] - mean[0]) * scale[0];
      im->at<cv::Vec3f>(h, w)[1] =
          (im->at<cv::Vec3f>(h, w)[1] - mean[1]) * scale[1];
      im->at<cv::Vec3f>(h, w)[2] =
          (im->at<cv::Vec3f>(h, w)[2] - mean[2]) * scale[2];
    }
  }
}

void CenterCropImg::Run(cv::Mat &img, const int crop_size) {
  int resize_w = img.cols;
  int resize_h = img.rows;
  int w_start = int((resize_w - crop_size) / 2);
  int h_start = int((resize_h - crop_size) / 2);
  cv::Rect rect(w_start, h_start, crop_size, crop_size);
  img = img(rect);
}

void ResizeImg::Run(const cv::Mat &img, cv::Mat &resize_img,
                    int resize_short_size) {
  int w = img.cols;
  int h = img.rows;

  float ratio = 1.f;
  if (h < w) {
    ratio = float(resize_short_size) / float(h);
  } else {
    ratio = float(resize_short_size) / float(w);
  }

  int resize_h = round(float(h) * ratio);
  int resize_w = round(float(w) * ratio);

  cv::resize(img, resize_img, cv::Size(resize_w, resize_h));
}

} // namespace MobileNetV3
