#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = librfsimulators.la

#######################################
# Build information for each library

# Sources for libdigitizersim
librfsimulators_la_SOURCES = Transmitter.cpp RfSimulatorFactory.cpp FmRdsSimulatorImpl.cpp UserDataQueue.cpp ./PiFmRds/src/fm_mpx.c ./PiFmRds/src/rds.c ./PiFmRds/src/waveforms.c ./gnuradio/src/FrequencyModulator.cpp ./dsp/src/resampler.cpp ./dsp/src/Tuner.cpp ./dsp/src/FIRFilter.cpp 

# Linker options libTestProgram
librfsimulators_la_LDFLAGS = $(BOOST_LDFLAGS) $(BOOST_FILESYSTEM_LIB) $(BOOST_THREAD_LIB) -ltinyxml -lsndfile -llog4cxx

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
librfsimulators_la_CPPFLAGS = -I$(top_srcdir)/include -I./dsp/inc -I./fft/inc -I./gnuradio/inc -I./PiFmRds/inc $(BOOST_CPPFLAGS)

