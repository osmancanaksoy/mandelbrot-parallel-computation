
# Mandelbrot Parallel Computation

This project demonstrates the parallel computation of the Mandelbrot set using the Qt framework and OpenMP. The application efficiently calculates and visualizes the Mandelbrot set, leveraging the power of multi-core processors to speed up the computation process.

## Table of Contents
1. [Features](#features)
2. [Prerequisites](#prerequisites)
3. [Installation](#installation)
4. [Usage](#usage)
5. [Performance Measurement](#performancemeasurement)
6. [Screenshots](#screenshots)
7. [Contributing](#contributing)
8. [License](#license)

## Features

- **Parallel Computation:** Uses OpenMP to parallelize the Mandelbrot set calculations, significantly reducing computation time.
- **Qt Framework:** Provides a graphical user interface (GUI) for visualizing the Mandelbrot set.
- **Zoom and Pan:** Allows users to zoom into and pan around the Mandelbrot set for detailed exploration.
- **Performance Measurement:** Calculates and displays speedup and efficiency graphs to showcase the performance gains achieved through parallelization.

## Prerequisites

- Qt (version 5 or later)
- OpenMP
- C++ Compiler with OpenMP support (e.g., GCC, Clang)

## Installation

1. **Clone the Repository:**

   ```sh
   git clone https://github.com/yourusername/mandelbrot-qt-openmp.git
   cd mandelbrot-qt-openmp
   ```

2. **Open the Project in Qt Creator:**

   Open Qt Creator and select `File` -> `Open File or Project` and navigate to the cloned repository to open the project file.

3. **Run the Application:**

   Click the **Run** button in Qt Creator to compile and run the application.

## Usage

1. **Launch the Application:**

   Run the `MandelbrotApp` executable from Qt Creator.

2. **Explore the Mandelbrot Set:**

   - Use the mouse to zoom in and out.
   - Click and drag to pan around the image.

3. **View Performance Metrics:**

   - The application calculates and displays speedup and efficiency graphs based on the number of threads used.

### Performance Measurement

- **Speedup:** The ratio of the time taken to solve the problem sequentially to the time taken to solve it in parallel.
- **Efficiency:** The speedup divided by the number of processors, indicating how effectively the processors are being used.

Performance metrics are calculated and displayed as graphs within the application.

## Screenshots

### Mandelbrot Set Visualization

![Mandelbrot Set](screenshots/mandelbrot.png)

### Performance Metrics

![Speedup Graph](screenshots/speedup.png)
![Efficiency Graph](screenshots/efficiency.png)

## Contributing

Contributions are welcome! Feel free to open issues or submit pull requests.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
