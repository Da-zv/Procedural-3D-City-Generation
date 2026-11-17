# Procedural 3D "City" Generation
A small C++ project exploring procedural generation and real-time rendering of 3D urban environments using OpenGL.

The application generates a simple city-like environment consisting of buildings with randomized positions and dimensions. Each building features textured surfaces, creating a basic 3D urban landscape.

The user can interact with the application to regenerate the environment and adjust the number of buildings and the overall size of the city.

## Features

Procedural Generation – Randomized building placement, dimensions, and city layout.

3D Rendering – Real-time rendering of the generated environment using OpenGL.

Custom GLSL Shaders – Custom vertex and fragment shaders used in the rendering pipeline.

Texture Mapping – Textured building surfaces.

Interactive Controls – Regenerate the city and adjust building density and city size.

## Technologies & Libraries

C++ – Main programming language.

OpenGL – 3D graphics rendering.

GLSL – Custom vertex and fragment shaders.

GLEW – OpenGL extension loading.

GLUT – Window management and keyboard input handling.

GLM – Mathematics library for graphics programming.

STB Image – Image loading for texture handling.

## Screenshots

Screenshots of the generated city:

<img width="1282" height="752" alt="image" src="https://github.com/user-attachments/assets/f7eca2f2-38e0-4c2d-bed3-4f25e6728eb1" />

## Controls

The application supports keyboard input for:

Regenerating the city.

Increasing or decreasing the number of buildings.

Adjusting the overall city size.

• (arrow key up) increase the amount of buildings.
• (arrow key down) decrease the amount of buildings.
• (arrow key right) increase the size of the city.
• (arrow key left) decrease the size of the city.
• (space key) regenerate the city with a new seed.

## Implementation

The city is generated procedurally by creating buildings with randomized dimensions and positions.

OpenGL is responsible for rendering the 3D scene, while custom GLSL shaders handle the rendering of objects. Building textures are loaded from image files using STB Image.

The project combines procedural generation, 3D mathematics, texture mapping, and shader programming in a standalone C++ application.

## Project Scope

This project focuses on the procedural generation and visualization of buildings rather than creating a fully detailed urban environment.

The current implementation includes buildings with textured surfaces. Elements such as roads, streetlights, vehicles, and other environmental decorations are not included.

## Dependencies

The project uses the following libraries:

OpenGL, GLEW, freeglut, GLM, STB Image
