<?xml version="1.0" encoding="utf-8"?>
<SwConfiguration xmlns="http://br-automation.co.at/AS/SwConfiguration" CpuAddress="SL1">
  <TaskClass Name="Cyclic#1" />
  <TaskClass Name="Cyclic#2" />
  <TaskClass Name="Cyclic#3">
    <Task Name="TestProg" Source="TestProg.TestProg.prg" Memory="UserROM" Language="ANSIC" Debugging="true" />
  </TaskClass>
  <TaskClass Name="Cyclic#4" />
  <TaskClass Name="Cyclic#5" />
  <TaskClass Name="Cyclic#6" />
  <TaskClass Name="Cyclic#7" />
  <TaskClass Name="Cyclic#8">
    <Task Name="LuxProg" Source="Lux.LuxProg.prg" Memory="UserROM" Language="IEC" Debugging="true" />
  </TaskClass>
  <Binaries>
    <BinaryObject Name="udbdef" Source="" Memory="UserROM" Language="Binary" />
    <BinaryObject Name="TCData" Source="" Memory="SystemROM" Language="Binary" />
    <BinaryObject Name="FWRules" Source="" Memory="UserROM" Language="Binary" />
    <BinaryObject Name="ashwd" Source="" Memory="SystemROM" Language="Binary" />
    <BinaryObject Name="ashwac" Source="" Memory="UserROM" Language="Binary" />
    <BinaryObject Name="User" Source="" Memory="UserROM" Language="Binary" />
    <BinaryObject Name="sysconf" Source="" Memory="SystemROM" Language="Binary" />
    <BinaryObject Name="asfw" Source="" Memory="SystemROM" Language="Binary" />
    <BinaryObject Name="arconfig" Source="" Memory="SystemROM" Language="Binary" />
    <BinaryObject Name="iomap" Source="" Memory="UserROM" Language="Binary" />
    <BinaryObject Name="Role" Source="" Memory="UserROM" Language="Binary" />
  </Binaries>
  <Libraries>
    <LibraryObject Name="AsBrStr" Source="Libraries._AS.AsBrStr.lby" Memory="UserROM" Language="binary" Debugging="true" />
    <LibraryObject Name="AsBrWStr" Source="Libraries._AS.AsBrWStr.lby" Memory="UserROM" Language="binary" Debugging="true" />
    <LibraryObject Name="AsHttp" Source="Libraries._AS.AsHttp.lby" Memory="UserROM" Language="binary" Debugging="true" />
    <LibraryObject Name="AsTCP" Source="Libraries._AS.AsTCP.lby" Memory="UserROM" Language="binary" Debugging="true" />
    <LibraryObject Name="astime" Source="Libraries._AS.astime.lby" Memory="UserROM" Language="binary" Debugging="true" />
    <LibraryObject Name="operator" Source="Libraries._AS.operator.lby" Memory="UserROM" Language="binary" Debugging="true" />
    <LibraryObject Name="runtime" Source="Libraries._AS.runtime.lby" Memory="UserROM" Language="binary" Debugging="true" />
    <LibraryObject Name="standard" Source="Libraries._AS.standard.lby" Memory="UserROM" Language="binary" Debugging="true" />
    <LibraryObject Name="sys_lib" Source="Libraries._AS.sys_lib.lby" Memory="UserROM" Language="binary" Debugging="true" />
    <LibraryObject Name="DataBuffer" Source="Libraries.Loupe.DataBuffer.lby" Memory="UserROM" Language="ANSIC" Debugging="true" />
    <LibraryObject Name="StringExt" Source="Libraries.Loupe.StringExt.lby" Memory="UserROM" Language="ANSIC" Debugging="true" />
    <LibraryObject Name="TCPComm" Source="Libraries.Loupe.TCPComm.lby" Memory="UserROM" Language="ANSIC" Debugging="true" />
    <LibraryObject Name="VarTools" Source="Libraries.Loupe.VarTools.lby" Memory="UserROM" Language="ANSIC" Debugging="true" />
    <LibraryObject Name="WebSocket" Source="Libraries.Loupe.WebSocket.lby" Memory="UserROM" Language="ANSIC" Debugging="true" />
    <LibraryObject Name="OMJSON" Source="Libraries.Loupe.OMJSON.lby" Memory="UserROM" Language="ANSIC" Debugging="true" />
  </Libraries>
</SwConfiguration>
