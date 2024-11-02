# Pathloss Simulation Using NS-3

This repository contains a series of NS-3 simulations designed for the Wireless Networks course assignment. The simulations create a network topology with multiple WiFi clients connected to an access point (AP), which in turn is connected to a server through point-to-point (p2p) links. The objective is to analyze the network behavior under various conditions, including file transfers and the use of different WiFi management features like RTS/CTS and rate adaptation.

## Project Structure

The code is organized into separate files for each part of the assignment:

- `a.cc`: Sets up the basic WiFi and p2p networks.
- `b.cc`: Adds file download capabilities to each client and plots completion times.
- `c.cc`: Integrates an upload application along with the download.
- `d.cc`: Implements a path loss and fading model with MistrelHTManager for rate adaptation.
- `e.cc`: Turns on RTS/CTS to analyze its effect on network performance.

Each part builds on the previous, adding complexity to the simulation.

## Prerequisites

To run these simulations, you will need to have NS-3 installed on your machine. Follow the installation guide on the [NS-3 website](https://www.nsnam.org/wiki/Installation).

## Running Simulations

To run any part of the assignment, navigate to your NS-3 project directory and use the following command:

./ns3 run "scratch/a --numClients=10" # Replace 'scratch/a' with the part you want to run.

## Visualization

To visualize the network topology and packet flows, you can use NetAnim or other NS-3 supported visual tools. Instructions for setting up NetAnim can be found [here](https://www.nsnam.org/wiki/NetAnim).

I've plotted the download completion times using a Python script.

## Plots

![b](https://github.com/nikhil21268/ns3-Simulations/blob/main/Plots/b.png)

![c](https://github.com/nikhil21268/ns3-Simulations/blob/main/Plots/c.png)

![d](https://github.com/nikhil21268/ns3-Simulations/blob/main/Plots/d.png)

![e](https://github.com/nikhil21268/ns3-Simulations/blob/main/Plots/e.png)

## Screenshots

![c](https://github.com/nikhil21268/ns3-Simulations/blob/main/Screenshots/Screenshot%20from%202024-09-25%2019-32-21.png)

![d](https://github.com/nikhil21268/ns3-Simulations/blob/main/Screenshots/Screenshot%20from%202024-09-25%2019-33-15.png)

![e](https://github.com/nikhil21268/ns3-Simulations/blob/main/Screenshots/Screenshot%20from%202024-09-25%2019-38-13.png)

## Contributions

Contributions are welcome. Please fork this repository, make your changes, and submit a pull request.

# Copyright and License

## Copyright (c) 2024, Nikhil Suri

## All rights reserved

This code and the accompanying materials are made available on an "as is" basis, without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose and noninfringement. In no event shall the authors or copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising from, out of or in connection with the software or the use or other dealings in the software.

## No Licensing
This project is protected by copyright and other intellectual property laws. It does not come with any license that would permit reproduction, distribution, or creation of derivative works. You may not use, copy, modify, or distribute this software and its documentation without express written permission from the copyright holder.

## Contact Information
For further inquiries, you can reach me at nikhil21268@iiitd.ac.in
