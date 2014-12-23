# RF Simulators

This library may be used to simulate an RF Digitizer.  At present, the RF Simulators library contains a single implementation; the FM RDS Simulator.  While the RF Simulators library was designed to be used within the REDHAWK SDR environment, the library itself has no dependency on the REDHAWK framework and may be used as a stand alone C++ library.

## Dependencies

* tinyxml
* libsndfile
* log4cxx
* fftw
* boost

Additional Build Time Dependencies

* autoconf
* automake
* libtool


## Building & Installation
    ./reconf
    ./configure
    make -j
    sudo make install

See details below regarding required configuration files.

# The FM RDS Simulator

The FM RDS Simulator will generate FM modulated mono or stereo audio with RDS encoded PI (Call Sign), PS (Short Text), and RT (Full Text) data. The code used to generate the RDS modulation leverages code developed for [PiFmRds](https://github.com/ChristopheJacquet/PiFmRds), the FM Modulation implementation is from [GnuRadio](www.http://gnuradio.org), and the filtering classes from REDHAWKs [dsplib](https://github.com/RedhawkSDR/dsp) and [fftlib](https://github.com/RedhawkSDR/fftlib).  Licensing / Copyright files can be found in the respective folders.

The simulator will provide complex samples at a maximum sample rate of 2.28 Msps.  Supported sample rates must be integer multiples of this maximum rate (ex. 2.28Msps, 1.14Msps, 760Ksps etc) down to a minimum of 2.28Ksps.    

## Configuration Files

The FM RDS Simulator is initialized with a directory containing XML configuration files.  These files must have the following format: 


    <TxProps>
      <FileName>File_Name.wav</FileName>
      <CenterFrequency>104900000</CenterFrequency>
      <RDS>
        <CallSign>WSDR</CallSign>
        <ShortText>REDHAWK!</ShortText>
        <FullText>REDHAWK Radio, Rock the Hawk! (www.redhawksdr.org)</FullText>
      </RDS>
    </TxProps>

Required fields are FileName and CenterFrequency.  RDS data is optional and will be filled in with default values if not provided.  Call sign must be four characters and the short text cannot exceed eight characters. 

Example files are provided within this git repository but are checked in as a single blob to reduce repository size.  To extract the tarball of examples:

    git show ExampleFiles > ExampleFiles.tar.gz

The example files contain nine example wav files.  Eight of which were pulled from (FreeSound)[https://www.freesound.org/] and one is a reading from the REDHAWK SDR Manual.  All are licensed under the Creative Commons License.  There are twenty XML files, each describing an individual FM station and lastly a simple bash script to quickly generate randomly distributed stations given a directory of wav files.  See the included README and LICENSE file for additional information.

## Getting started & API Notes

The RF Simulator library has a fairly simple API currently.  A factory pattern is used to instantiate the type of simulator and a user provided callback class is used for data delivery.  All RF Simulators inherit from the RfSimulator class and the provided callback class must inherit from the CallbackInterface class.  See the RfSimulator header for available methods and control.

Below is a trivial example which uses the RfSimulators namespace.

	SampleCallback callback;

	std::string p("/tmp/xmlFileLocation/");

	RfSimulator * digSim = RfSimulatorFactory::createFmRdsSimulator();

	digSim->init(p, &callback, TRACE);

	digSim->start();

	sleep(120);

	delete(digSim);

## Notes

The FmRdsSimulator creates a processing thread for each station within the currently visible 2.28 Mhz bandwidth (even if bandwidth is set smaller).  Since each of these threads is resampling a wav file, FM modulating, encoding RDS, and upsampling to 2.28 Msps a non-trivial amount of CPU is used.  Keep this in mind when distributing the FM Stations.