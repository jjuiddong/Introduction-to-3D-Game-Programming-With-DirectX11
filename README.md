Introduction-to-3D-Game-Programming-With-DirectX11
==================================================

Book Example Code in [Introduction to 3D Game Programming With DirectX11 by Frank Luna ]
http://www.d3dcoder.net/



New Project setup:
==================

Do not modify the relative directory structure of these samples.  In particular, for each 
project sample, the Common folder must be obtained from it via "../../Common".

Debug libraries:
----------------

d3d11.lib;d3dx11d.lib;D3DCompiler.lib;Effects11d.lib;dxerr.lib;dxgi.lib;dxguid.lib;%(AdditionalDependencies)

Release libraries:
------------------

d3d11.lib;d3dx11.lib;D3DCompiler.lib;Effects11.lib;dxerr.lib;dxgi.lib;dxguid.lib;%(AdditionalDependencies)

C/C++ Additional Include Directories:
-------------------------------------

1) Path to DirectX Header files.

2) ../../Common (or Absolute Path to Common)

Linker Additional Library Directories:
--------------------------------------

1) Path to DirectX Library files.

2) ../../Common (or Absolute Path to Common)

FXC Call
--------

a) Debug mode:   fxc /Fc /Od /Zi /T fx_5_0 /Fo "%(RelativeDir)\%(Filename).fxo" "%(FullPath)"
b) Release mode: fxc /T fx_5_0 /Fo "%(RelativeDir)\%(Filename).fxo" "%(FullPath)"

a) Debug Description: fxc compile for debug: %(FullPath)
b) Release Description: fxc compile for release: %(FullPath)

Outputs: %(RelativeDir)\%(Filename).fxo


Compile Error
---------------
- Effects11d.lib Link Error
C:\Program Files\Microsoft DirectX SDK (June 2010)\Samples\C++\Effects11 
https://github.com/Microsoft/FX11/archive/nov2015.zip
Build And Link Effects11.lib 
http://gpgstudy.com/gpgiki/Visual_Studio_2015%EC%97%90%EC%84%9C_%EC%98%88%EC%A0%9C_%EC%8B%A4%ED%96%89%ED%95%98%EA%B8%B0#3._Effects11_.EB.9D.BC.EC.9D.B4.EB.B8.8C.EB.9F.AC.EB.A6.AC_.EB.B9.8C.EB.93.9C_.EB.B0.8F_.EC.A0.81.EC.9A.A9


- DirectX 11 초기화 오류
아래 사이트를 참조하자.
http://stackoverflow.com/questions/10586956/what-can-cause-d3d11createdevice-to-fail-with-e-fail

