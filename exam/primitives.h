//
// Created by Henrique on 9/17/2019.
//

#ifndef GRAPHICSPROGRAMMINGEXERCISES_PRIMITIVES_H
#define GRAPHICSPROGRAMMINGEXERCISES_PRIMITIVES_H

// 2D triangle
std::vector<float> triangleVertices { 0.0f, 0.0f, 0.0f,
                                      1.0f, 0.0f, 0.0f,
                                      0.5f, 1.0f, 0.0f };

std::vector<unsigned int> triangleIndices {0, 1, 2};

std::vector<float> triangleColors { 0.8f, 0.4f, 0.4f, 1.0f,
                                    0.7f, 0.7f, 0.4f, 1.0f,
                                    0.4f, 0.7f, 0.7f, 1.0f };

std::vector<float> triangleNormals { 0.0f, 0.0f, 1.0f,
                                     0.0f, 0.0f, 1.0f,
                                     0.0f, 0.0f, 1.0f };

std::vector<float> triangleNormals2 { -0.59f, -0.29f, 0.75f,
                                       0.59f, -0.29f, 0.75f,
                                       0.00f,  0.62f, 0.78f };

// 2D Quad
std::vector<float> QuadVertices { 0.0f, 0.0f, 0.0f,
                                  1.0f, 0.0f, 0.0f,
                                  1.0f, 1.0f, 0.0f,
                                  0.0f, 1.0f, 0.0f };

std::vector<unsigned int> QuadIndices { 0, 1, 2, 0, 2, 3 };

std::vector<float> QuadColors { 0.8f, 0.4f, 0.4f, 1.0f,
                                0.7f, 0.7f, 0.4f, 1.0f,
                                0.4f, 0.7f, 0.7f, 1.0f,
                                0.7f, 0.4f, 0.7f, 1.0f };

std::vector<float> QuadNormals { 0.0f, 0.0f, 1.0f,
                                 0.0f, 0.0f, 1.0f,
                                 0.0f, 0.0f, 1.0f,
                                 0.0f, 0.0f, 1.0f };

std::vector<float> QuadNormals2 { -0.61f, -0.61f, 0.49f,
                                   0.61f, -0.61f, 0.49f,
                                   0.61f,  0.61f, 0.49f,
                                  -0.61f,  0.61f, 0.49f };

#endif //GRAPHICSPROGRAMMINGEXERCISES_PRIMITIVES_H
