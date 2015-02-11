#!/bin/sh

sloccount Address Bridge ClientList CommonTests Exemplars FindServices JavaScript KinectV2 LeapMotion MovementDb ParserTest PortLister RecordAsJSON RegistryService RequestCounter RequestInfo ServiceLister ServiceMetrics Unreal Version ViconDataStream examples mpm odl
echo ---------------
echo subtract the following -
echo ---------------
sloccount KinectV2/KinectV2InputService/KinectLib LeapMotion/LeapMotionInputService/Leap*.h ViconDataStream/ViconDataStreamInputService/ViconLib mpm/swig_mpm_in mpm/swig_mpm_out_csharp mpm/swig_mpm_out_python odl/swig_odl_in odl/swig_odl_out_csharp odl/swig_odl_out_python
