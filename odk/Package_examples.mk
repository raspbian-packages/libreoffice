# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odk_examples,$(SRCDIR)/odk/examples))

$(eval $(call gb_Package_set_outdir,odk_examples,$(INSTDIR)))

$(eval $(call gb_Package_add_files_with_dir,odk_examples,$(SDKDIRNAME)/examples,\
    CLI/CSharp/Spreadsheet/GeneralTableSample.cs \
    CLI/CSharp/Spreadsheet/Makefile \
    CLI/CSharp/Spreadsheet/SpreadsheetDocHelper.cs \
    CLI/CSharp/Spreadsheet/SpreadsheetSample.cs \
    CLI/CSharp/Spreadsheet/ViewSample.cs \
    CLI/VB.NET/WriterDemo/Makefile \
    CLI/VB.NET/WriterDemo/WriterDemo.vb \
    DevelopersGuide/BasicAndDialogs/CreatingDialogs/CreatingDialogs.odt \
    DevelopersGuide/BasicAndDialogs/CreatingDialogs/Makefile \
    DevelopersGuide/BasicAndDialogs/CreatingDialogs/SampleDialog.java \
    DevelopersGuide/BasicAndDialogs/FirstStepsBasic.odt \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/Makefile \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/ToolkitControls.odt \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/ToolkitControls/FileDialog.xba \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/ToolkitControls/FileDialogDlg.xdl \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/ToolkitControls/MultiPage.xba \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/ToolkitControls/MultiPageDlg.xdl \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/ToolkitControls/ProgressBar.xba \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/ToolkitControls/ProgressBarDlg.xdl \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/ToolkitControls/ScrollBar.xba \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/ToolkitControls/ScrollBarDlg.xdl \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/ToolkitControls/dialog.xlb \
    DevelopersGuide/BasicAndDialogs/ToolkitControls/ToolkitControls/script.xlb \
    DevelopersGuide/Charts/AddInChart.ods \
    DevelopersGuide/Charts/CalcHelper.java \
    DevelopersGuide/Charts/ChartHelper.java \
    DevelopersGuide/Charts/ChartInCalc.java \
    DevelopersGuide/Charts/ChartInDraw.java \
    DevelopersGuide/Charts/ChartInWriter.java \
    DevelopersGuide/Charts/Helper.java \
    DevelopersGuide/Charts/JavaSampleChartAddIn.components \
    DevelopersGuide/Charts/JavaSampleChartAddIn.java \
    DevelopersGuide/Charts/ListenAtCalcRangeInDraw.java \
    DevelopersGuide/Charts/Makefile \
    DevelopersGuide/Charts/SelectionChangeListener.java \
    DevelopersGuide/Charts/bullet.gif \
    DevelopersGuide/Components/Addons/JobsAddon/Addons.xcu \
    DevelopersGuide/Components/Addons/JobsAddon/AsyncJob.components \
    DevelopersGuide/Components/Addons/JobsAddon/AsyncJob.java \
    DevelopersGuide/Components/Addons/JobsAddon/Jobs.xcu \
    DevelopersGuide/Components/Addons/JobsAddon/Makefile \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_cpp/Addons.xcu \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_cpp/Makefile \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_cpp/ProtocolHandler.xcu \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_cpp/addon.cxx \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_cpp/addon.hxx \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_cpp/component.cxx \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_java/Addons.xcu \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_java/Makefile \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_java/ProtocolHandler.xcu \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_java/ProtocolHandlerAddon.java \
    DevelopersGuide/Components/Addons/ProtocolHandlerAddon_java/ProtocolHandlerAddon_java.components \
    DevelopersGuide/Components/CppComponent/Makefile \
    DevelopersGuide/Components/CppComponent/SimpleComponent.odt \
    DevelopersGuide/Components/CppComponent/TestCppComponent.cxx \
    DevelopersGuide/Components/CppComponent/service1_impl.cxx \
    DevelopersGuide/Components/CppComponent/service2_impl.cxx \
    DevelopersGuide/Components/CppComponent/some.idl \
    DevelopersGuide/Components/JavaComponent/JavaComponent.components \
    DevelopersGuide/Components/JavaComponent/Makefile \
    DevelopersGuide/Components/JavaComponent/SomethingA.idl \
    DevelopersGuide/Components/JavaComponent/SomethingB.idl \
    DevelopersGuide/Components/JavaComponent/TestComponentA.java \
    DevelopersGuide/Components/JavaComponent/TestComponentB.java \
    DevelopersGuide/Components/JavaComponent/TestJavaComponent.java \
    DevelopersGuide/Components/JavaComponent/TestServiceProvider.java \
    DevelopersGuide/Components/JavaComponent/XSomethingA.idl \
    DevelopersGuide/Components/JavaComponent/XSomethingB.idl \
    DevelopersGuide/Components/SimpleLicense/LicenseTest.components \
    DevelopersGuide/Components/SimpleLicense/LicenseTest.idl \
    DevelopersGuide/Components/SimpleLicense/LicenseTest.java \
    DevelopersGuide/Components/SimpleLicense/Makefile \
    DevelopersGuide/Components/SimpleLicense/description.xml \
    DevelopersGuide/Components/SimpleLicense/registration/license_de.txt \
    DevelopersGuide/Components/SimpleLicense/registration/license_en_US.txt \
    DevelopersGuide/Components/Thumbs/Makefile \
    DevelopersGuide/Components/Thumbs/org/openoffice/comp/test/ImageShrink.components \
    DevelopersGuide/Components/Thumbs/org/openoffice/comp/test/ImageShrink.java \
    DevelopersGuide/Components/Thumbs/org/openoffice/comp/test/Makefile \
    DevelopersGuide/Components/Thumbs/org/openoffice/comp/test/Thumbs.java \
    DevelopersGuide/Components/Thumbs/org/openoffice/test/ImageShrink.idl \
    DevelopersGuide/Components/Thumbs/org/openoffice/test/Makefile \
    DevelopersGuide/Components/Thumbs/org/openoffice/test/XImageShrink.idl \
    DevelopersGuide/Components/Thumbs/org/openoffice/test/XImageShrinkFilter.idl \
    DevelopersGuide/Components/Thumbs/thumbs.mk \
    DevelopersGuide/Components/dialogcomponent/DialogComponent.components \
    DevelopersGuide/Components/dialogcomponent/DialogComponent.java \
    DevelopersGuide/Components/dialogcomponent/DialogComponent.odt \
    DevelopersGuide/Components/dialogcomponent/Makefile \
    DevelopersGuide/Components/dialogcomponent/TestDialogHandler.idl \
    DevelopersGuide/Components/dialogcomponent/XTestDialogHandler.idl \
    DevelopersGuide/Config/ConfigExamples.java \
    DevelopersGuide/Config/Makefile \
    DevelopersGuide/Database/CodeSamples.java \
    DevelopersGuide/Database/DriverSkeleton/How_to_write_my_own_driver.txt \
    DevelopersGuide/Database/DriverSkeleton/Makefile \
    DevelopersGuide/Database/DriverSkeleton/OSubComponent.hxx \
    DevelopersGuide/Database/DriverSkeleton/OTypeInfo.hxx \
    DevelopersGuide/Database/DriverSkeleton/SConnection.cxx \
    DevelopersGuide/Database/DriverSkeleton/SConnection.hxx \
    DevelopersGuide/Database/DriverSkeleton/SDatabaseMetaData.cxx \
    DevelopersGuide/Database/DriverSkeleton/SDatabaseMetaData.hxx \
    DevelopersGuide/Database/DriverSkeleton/SDriver.cxx \
    DevelopersGuide/Database/DriverSkeleton/SDriver.hxx \
    DevelopersGuide/Database/DriverSkeleton/SPreparedStatement.cxx \
    DevelopersGuide/Database/DriverSkeleton/SPreparedStatement.hxx \
    DevelopersGuide/Database/DriverSkeleton/SResultSet.cxx \
    DevelopersGuide/Database/DriverSkeleton/SResultSet.hxx \
    DevelopersGuide/Database/DriverSkeleton/SResultSetMetaData.cxx \
    DevelopersGuide/Database/DriverSkeleton/SResultSetMetaData.hxx \
    DevelopersGuide/Database/DriverSkeleton/SServices.cxx \
    DevelopersGuide/Database/DriverSkeleton/SStatement.cxx \
    DevelopersGuide/Database/DriverSkeleton/SStatement.hxx \
    DevelopersGuide/Database/DriverSkeleton/propertyids.cxx \
    DevelopersGuide/Database/DriverSkeleton/propertyids.hxx \
    DevelopersGuide/Database/Makefile \
    DevelopersGuide/Database/OpenQuery.java \
    DevelopersGuide/Database/RowSet.java \
    DevelopersGuide/Database/RowSetEventListener.java \
    DevelopersGuide/Database/Sales.java \
    DevelopersGuide/Database/SalesMan.java \
    DevelopersGuide/Database/sdbcx.java \
    DevelopersGuide/Drawing/ChangeOrderDemo.java \
    DevelopersGuide/Drawing/ControlAndSelectDemo.java \
    DevelopersGuide/Drawing/CustomShowDemo.java \
    DevelopersGuide/Drawing/DrawViewDemo.java \
    DevelopersGuide/Drawing/DrawingDemo.java \
    DevelopersGuide/Drawing/FillAndLineStyleDemo.java \
    DevelopersGuide/Drawing/GluePointDemo.java \
    DevelopersGuide/Drawing/GraphicExportDemo.java \
    DevelopersGuide/Drawing/Helper.java \
    DevelopersGuide/Drawing/LayerDemo.java \
    DevelopersGuide/Drawing/Makefile \
    DevelopersGuide/Drawing/ObjectTransformationDemo.java \
    DevelopersGuide/Drawing/Organigram.java \
    DevelopersGuide/Drawing/PageHelper.java \
    DevelopersGuide/Drawing/PresentationDemo.java \
    DevelopersGuide/Drawing/ShapeHelper.java \
    DevelopersGuide/Drawing/SimplePresentation.odp \
    DevelopersGuide/Drawing/StyleDemo.java \
    DevelopersGuide/Drawing/TextDemo.java \
    DevelopersGuide/Extensions/DialogWithHelp/Addons.xcu \
    DevelopersGuide/Extensions/DialogWithHelp/DialogWithHelp/Dialog1.xdl \
    DevelopersGuide/Extensions/DialogWithHelp/DialogWithHelp/Module1.xba \
    DevelopersGuide/Extensions/DialogWithHelp/DialogWithHelp/dialog.xlb \
    DevelopersGuide/Extensions/DialogWithHelp/DialogWithHelp/script.xlb \
    DevelopersGuide/Extensions/DialogWithHelp/META-INF/manifest.xml \
    DevelopersGuide/Extensions/DialogWithHelp/Makefile \
    DevelopersGuide/Extensions/DialogWithHelp/description.xml \
    DevelopersGuide/Extensions/DialogWithHelp/help/de/com.foocorp.foo-ext/page1.xhp \
    DevelopersGuide/Extensions/DialogWithHelp/help/de/com.foocorp.foo-ext/page2.xhp \
    DevelopersGuide/Extensions/DialogWithHelp/help/de/com.foocorp.foo-ext/subfolder/anotherpage.xhp \
    DevelopersGuide/Extensions/DialogWithHelp/help/en/com.foocorp.foo-ext/page1.xhp \
    DevelopersGuide/Extensions/DialogWithHelp/help/en/com.foocorp.foo-ext/page2.xhp \
    DevelopersGuide/Extensions/DialogWithHelp/help/en/com.foocorp.foo-ext/subfolder/anotherpage.xhp \
    DevelopersGuide/FirstSteps/FirstConnection.java \
    DevelopersGuide/FirstSteps/FirstLoadComponent.java \
    DevelopersGuide/FirstSteps/FirstUnoContact.java \
    DevelopersGuide/FirstSteps/HelloTextTableShape.java \
    DevelopersGuide/FirstSteps/Makefile \
    DevelopersGuide/FirstSteps/build_FirstUnoContact.xml \
    DevelopersGuide/Forms/BooleanValidator.java \
    DevelopersGuide/Forms/ButtonOperator.java \
    DevelopersGuide/Forms/ComponentTreeTraversal.java \
    DevelopersGuide/Forms/ControlLock.java \
    DevelopersGuide/Forms/ControlValidation.java \
    DevelopersGuide/Forms/ControlValidator.java \
    DevelopersGuide/Forms/DataAwareness.java \
    DevelopersGuide/Forms/DateValidator.java \
    DevelopersGuide/Forms/DocumentBasedExample.java \
    DevelopersGuide/Forms/DocumentHelper.java \
    DevelopersGuide/Forms/DocumentType.java \
    DevelopersGuide/Forms/DocumentViewHelper.java \
    DevelopersGuide/Forms/FLTools.java \
    DevelopersGuide/Forms/FormLayer.java \
    DevelopersGuide/Forms/GridFieldValidator.java \
    DevelopersGuide/Forms/HsqlDatabase.java \
    DevelopersGuide/Forms/InteractionRequest.java \
    DevelopersGuide/Forms/KeyGenerator.java \
    DevelopersGuide/Forms/ListSelectionValidator.java \
    DevelopersGuide/Forms/Makefile \
    DevelopersGuide/Forms/NumericValidator.java \
    DevelopersGuide/Forms/ProgrammaticScriptAssignment.odt \
    DevelopersGuide/Forms/RowSet.java \
    DevelopersGuide/Forms/SalesFilter.java \
    DevelopersGuide/Forms/SingleControlValidation.java \
    DevelopersGuide/Forms/SpreadsheetDocument.java \
    DevelopersGuide/Forms/SpreadsheetValueBinding.java \
    DevelopersGuide/Forms/SpreadsheetView.java \
    DevelopersGuide/Forms/TableCellTextBinding.java \
    DevelopersGuide/Forms/TextValidator.java \
    DevelopersGuide/Forms/TimeValidator.java \
    DevelopersGuide/Forms/UNO.java \
    DevelopersGuide/Forms/URLHelper.java \
    DevelopersGuide/Forms/ValueBinding.java \
    DevelopersGuide/Forms/WaitForInput.java \
    DevelopersGuide/GUI/DialogDocument.java \
    DevelopersGuide/GUI/ImageControlSample.java \
    DevelopersGuide/GUI/Makefile \
    DevelopersGuide/GUI/MessageBox.java \
    DevelopersGuide/GUI/RoadmapItemStateChangeListener.java \
    DevelopersGuide/GUI/SystemDialog.java \
    DevelopersGuide/GUI/UnoDialogSample.java \
    DevelopersGuide/GUI/UnoDialogSample2.java \
    DevelopersGuide/GUI/UnoMenu.java \
    DevelopersGuide/GUI/UnoMenu2.java \
    DevelopersGuide/GUI/oologo.gif \
    DevelopersGuide/OfficeBean/Makefile \
    DevelopersGuide/OfficeBean/OOoBeanViewer.java \
    DevelopersGuide/OfficeBean/OfficeIconColor16.jpg \
    DevelopersGuide/OfficeBean/OfficeIconColor32.jpg \
    DevelopersGuide/OfficeBean/OfficeIconMono16.jpg \
    DevelopersGuide/OfficeBean/OfficeIconMono32.jpg \
    DevelopersGuide/OfficeDev/Clipboard/Clipboard.java \
    DevelopersGuide/OfficeDev/Clipboard/ClipboardListener.java \
    DevelopersGuide/OfficeDev/Clipboard/ClipboardOwner.java \
    DevelopersGuide/OfficeDev/Clipboard/Makefile \
    DevelopersGuide/OfficeDev/Clipboard/TextTransferable.java \
    DevelopersGuide/OfficeDev/ContextMenuInterceptor.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/CustomizeView.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/Desk.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/DocumentView.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/FunctionHelper.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/IOnewayLink.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/IShutdownListener.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/Install.txt \
    DevelopersGuide/OfficeDev/DesktopEnvironment/Interceptor.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/JavaWindowPeerFake.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/Makefile \
    DevelopersGuide/OfficeDev/DesktopEnvironment/NativeView.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/OfficeConnect.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/OnewayExecutor.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/StatusListener.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/StatusView.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/ViewContainer.java \
    DevelopersGuide/OfficeDev/DesktopEnvironment/nativelib/unix/Makefile \
    DevelopersGuide/OfficeDev/DesktopEnvironment/nativelib/unix/nativeview.c \
    DevelopersGuide/OfficeDev/DesktopEnvironment/nativelib/unix/nativeview.h \
    DevelopersGuide/OfficeDev/DesktopEnvironment/nativelib/windows/Makefile \
    DevelopersGuide/OfficeDev/DesktopEnvironment/nativelib/windows/nativeview.c \
    DevelopersGuide/OfficeDev/DesktopEnvironment/nativelib/windows/nativeview.def \
    DevelopersGuide/OfficeDev/DesktopEnvironment/nativelib/windows/nativeview.h \
    DevelopersGuide/OfficeDev/DisableCommands/DisableCommandsTest.java \
    DevelopersGuide/OfficeDev/DisableCommands/Makefile \
    DevelopersGuide/OfficeDev/FilterDevelopment/AsciiFilter/AsciiReplaceFilter.java \
    DevelopersGuide/OfficeDev/FilterDevelopment/AsciiFilter/FilterOptions.java \
    DevelopersGuide/OfficeDev/FilterDevelopment/AsciiFilter/Makefile \
    DevelopersGuide/OfficeDev/FilterDevelopment/AsciiFilter/SampleFilter.components \
    DevelopersGuide/OfficeDev/FilterDevelopment/AsciiFilter/TypeDetection.xcu \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilterDetection/FlatXmlTypeDetection.xcu \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilterDetection/Makefile \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilterDetection/fdcomp.cxx \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilterDetection/filterdetect.cxx \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilterDetection/filterdetect.hxx \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_cpp/FlatXml.cxx \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_cpp/FlatXmlFilter_cpp.xcu \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_cpp/Makefile \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_java/FlatXml.java \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_java/FlatXmlFilter_java.components \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_java/FlatXmlFilter_java.xcu \
    DevelopersGuide/OfficeDev/FilterDevelopment/FlatXmlFilter_java/Makefile \
    DevelopersGuide/OfficeDev/Linguistic/LinguisticExamples.java \
    DevelopersGuide/OfficeDev/Linguistic/Makefile \
    DevelopersGuide/OfficeDev/Linguistic/OneInstanceFactory.java \
    DevelopersGuide/OfficeDev/Linguistic/PropChgHelper.java \
    DevelopersGuide/OfficeDev/Linguistic/PropChgHelper_Hyph.java \
    DevelopersGuide/OfficeDev/Linguistic/PropChgHelper_Spell.java \
    DevelopersGuide/OfficeDev/Linguistic/SampleHyphenator.components \
    DevelopersGuide/OfficeDev/Linguistic/SampleHyphenator.java \
    DevelopersGuide/OfficeDev/Linguistic/SampleSpellChecker.components \
    DevelopersGuide/OfficeDev/Linguistic/SampleSpellChecker.java \
    DevelopersGuide/OfficeDev/Linguistic/SampleThesaurus.components \
    DevelopersGuide/OfficeDev/Linguistic/SampleThesaurus.java \
    DevelopersGuide/OfficeDev/Linguistic/XHyphenatedWord_impl.java \
    DevelopersGuide/OfficeDev/Linguistic/XMeaning_impl.java \
    DevelopersGuide/OfficeDev/Linguistic/XPossibleHyphens_impl.java \
    DevelopersGuide/OfficeDev/Linguistic/XSpellAlternatives_impl.java \
    DevelopersGuide/OfficeDev/Makefile \
    DevelopersGuide/OfficeDev/MenuElement.java \
    DevelopersGuide/OfficeDev/Number_Formats.java \
    DevelopersGuide/OfficeDev/OfficeConnect.java \
    DevelopersGuide/OfficeDev/PathSettings/Makefile \
    DevelopersGuide/OfficeDev/PathSettings/PathSettingsTest.java \
    DevelopersGuide/OfficeDev/PathSubstitution/Makefile \
    DevelopersGuide/OfficeDev/PathSubstitution/PathSubstitutionTest.java \
    DevelopersGuide/OfficeDev/TerminationTest/Makefile \
    DevelopersGuide/OfficeDev/TerminationTest/TerminateListener.java \
    DevelopersGuide/OfficeDev/TerminationTest/TerminationTest.java \
    DevelopersGuide/ProfUNO/CppBinding/Makefile \
    DevelopersGuide/ProfUNO/CppBinding/office_connect.cxx \
    DevelopersGuide/ProfUNO/CppBinding/string_samples.cxx \
    DevelopersGuide/ProfUNO/InterprocessConn/ConnectionAwareClient.java \
    DevelopersGuide/ProfUNO/InterprocessConn/Makefile \
    DevelopersGuide/ProfUNO/InterprocessConn/UrlResolver.java \
    DevelopersGuide/ProfUNO/Lifetime/Makefile \
    DevelopersGuide/ProfUNO/Lifetime/MyUnoObject.java \
    DevelopersGuide/ProfUNO/Lifetime/object_lifetime.cxx \
    DevelopersGuide/ProfUNO/SimpleBootstrap_cpp/Makefile \
    DevelopersGuide/ProfUNO/SimpleBootstrap_cpp/SimpleBootstrap_cpp.cxx \
    DevelopersGuide/ProfUNO/SimpleBootstrap_java/Makefile \
    DevelopersGuide/ProfUNO/SimpleBootstrap_java/SimpleBootstrap_java.java \
    DevelopersGuide/ProfUNO/SimpleBootstrap_java/manifest.mf \
    DevelopersGuide/ScriptingFramework/SayHello/META-INF/manifest.xml \
    DevelopersGuide/ScriptingFramework/SayHello/Makefile \
    DevelopersGuide/ScriptingFramework/SayHello/SayHello.odt \
    DevelopersGuide/ScriptingFramework/SayHello/SayHello/SayHello.java \
    DevelopersGuide/ScriptingFramework/SayHello/SayHello/parcel-descriptor.xml \
    DevelopersGuide/ScriptingFramework/SayHello/build.xml \
    DevelopersGuide/ScriptingFramework/ScriptSelector/META-INF/manifest.xml \
    DevelopersGuide/ScriptingFramework/ScriptSelector/Makefile \
    DevelopersGuide/ScriptingFramework/ScriptSelector/ScriptSelector.odt \
    DevelopersGuide/ScriptingFramework/ScriptSelector/ScriptSelector/ScriptSelector.java \
    DevelopersGuide/ScriptingFramework/ScriptSelector/ScriptSelector/container.gif \
    DevelopersGuide/ScriptingFramework/ScriptSelector/ScriptSelector/parcel-descriptor.xml \
    DevelopersGuide/ScriptingFramework/ScriptSelector/ScriptSelector/script.gif \
    DevelopersGuide/ScriptingFramework/ScriptSelector/ScriptSelector/soffice.gif \
    DevelopersGuide/ScriptingFramework/ScriptSelector/build.xml \
    DevelopersGuide/Spreadsheet/DataPilotReadme.txt \
    DevelopersGuide/Spreadsheet/ExampleAddIn.components \
    DevelopersGuide/Spreadsheet/ExampleAddIn.java \
    DevelopersGuide/Spreadsheet/ExampleAddIn.ods \
    DevelopersGuide/Spreadsheet/ExampleDataPilotSource.components \
    DevelopersGuide/Spreadsheet/ExampleDataPilotSource.java \
    DevelopersGuide/Spreadsheet/GeneralTableSample.java \
    DevelopersGuide/Spreadsheet/Makefile \
    DevelopersGuide/Spreadsheet/SpreadsheetDocHelper.java \
    DevelopersGuide/Spreadsheet/SpreadsheetSample.java \
    DevelopersGuide/Spreadsheet/ViewSample.java \
    DevelopersGuide/Spreadsheet/XExampleAddIn.idl \
    DevelopersGuide/Text/Makefile \
    DevelopersGuide/Text/PrintDemo.odt \
    DevelopersGuide/Text/TextDocuments.java \
    DevelopersGuide/Text/TextTemplateWithUserFields.odt \
    DevelopersGuide/UCB/ChildrenRetriever.java \
    DevelopersGuide/UCB/DataStreamComposer.java \
    DevelopersGuide/UCB/DataStreamRetriever.java \
    DevelopersGuide/UCB/Helper.java \
    DevelopersGuide/UCB/Makefile \
    DevelopersGuide/UCB/MyActiveDataSink.java \
    DevelopersGuide/UCB/MyInputStream.java \
    DevelopersGuide/UCB/PropertiesComposer.java \
    DevelopersGuide/UCB/PropertiesRetriever.java \
    DevelopersGuide/UCB/ResourceCreator.java \
    DevelopersGuide/UCB/ResourceManager.java \
    DevelopersGuide/UCB/ResourceRemover.java \
    DevelopersGuide/UCB/data/data.txt \
    OLE/activex/Makefile \
    OLE/activex/README.txt \
    OLE/activex/SOActiveX.cpp \
    OLE/activex/SOActiveX.h \
    OLE/activex/SOActiveX.rgs \
    OLE/activex/SOComWindowPeer.cpp \
    OLE/activex/SOComWindowPeer.h \
    OLE/activex/SOComWindowPeer.rgs \
    OLE/activex/StdAfx2.cpp \
    OLE/activex/StdAfx2.h \
    OLE/activex/example.html \
    OLE/activex/resource.h \
    OLE/activex/so_activex.cpp \
    OLE/activex/so_activex.def \
    OLE/activex/so_activex.idl \
    OLE/activex/so_activex.rc \
    OLE/vbscript/WriterDemo.vbs \
    OLE/vbscript/readme.txt \
    basic/drawing/dirtree.txt \
    basic/drawing/importexportofasciifiles.odg \
    basic/forms_and_controls/beef.wmf \
    basic/forms_and_controls/burger.wmf \
    basic/forms_and_controls/burger_factory.odt \
    basic/forms_and_controls/chicken.wmf \
    basic/forms_and_controls/fish.wmf \
    basic/forms_and_controls/vegetable.wmf \
    basic/sheet/adapting_to_euroland.ods \
    basic/stock_quotes_updater/stock.ods \
    basic/text/creating_an_index/index.odt \
    basic/text/creating_an_index/indexlist.txt \
    basic/text/modifying_text_automatically/changing_appearance.odt \
    basic/text/modifying_text_automatically/inserting_bookmarks.odt \
    basic/text/modifying_text_automatically/replacing_text.odt \
    basic/text/modifying_text_automatically/using_regular_expressions.odt \
    cpp/DocumentLoader/DocumentLoader.cxx \
    cpp/DocumentLoader/Makefile \
    cpp/DocumentLoader/test.odt \
    cpp/complextoolbarcontrols/Addons.xcu \
    cpp/complextoolbarcontrols/CalcListener.cxx \
    cpp/complextoolbarcontrols/CalcWindowState.xcu \
    cpp/complextoolbarcontrols/Jobs.xcu \
    cpp/complextoolbarcontrols/ListenerHelper.cxx \
    cpp/complextoolbarcontrols/ListenerHelper.h \
    cpp/complextoolbarcontrols/Makefile \
    cpp/complextoolbarcontrols/MyJob.cxx \
    cpp/complextoolbarcontrols/MyJob.h \
    cpp/complextoolbarcontrols/MyListener.cxx \
    cpp/complextoolbarcontrols/MyListener.h \
    cpp/complextoolbarcontrols/MyProtocolHandler.cxx \
    cpp/complextoolbarcontrols/MyProtocolHandler.h \
    cpp/complextoolbarcontrols/ProtocolHandler.xcu \
    cpp/complextoolbarcontrols/WriterListener.cxx \
    cpp/complextoolbarcontrols/WriterWindowState.xcu \
    cpp/complextoolbarcontrols/description.xml \
    cpp/complextoolbarcontrols/exports.cxx \
    cpp/complextoolbarcontrols/logo_big.png \
    cpp/complextoolbarcontrols/logo_small.png \
    cpp/counter/Makefile \
    cpp/counter/XCountable.idl \
    cpp/counter/counter.cxx \
    cpp/counter/countermain.cxx \
    cpp/remoteclient/Makefile \
    cpp/remoteclient/remoteclient.cxx \
    java/DocumentHandling/DocumentConverter.java \
    java/DocumentHandling/DocumentLoader.java \
    java/DocumentHandling/DocumentPrinter.java \
    java/DocumentHandling/DocumentSaver.java \
    java/DocumentHandling/Makefile \
    java/DocumentHandling/test/test1.odt \
    java/Drawing/Makefile \
    java/Drawing/SDraw.java \
    java/EmbedDocument/EmbeddedObject/EditorFrame.java \
    java/EmbedDocument/EmbeddedObject/EmbeddedObject.odt \
    java/EmbedDocument/EmbeddedObject/EmbeddedObject.xcu \
    java/EmbedDocument/EmbeddedObject/Makefile \
    java/EmbedDocument/EmbeddedObject/OwnEmbeddedObject.components \
    java/EmbedDocument/EmbeddedObject/OwnEmbeddedObject.java \
    java/EmbedDocument/EmbeddedObject/OwnEmbeddedObjectFactory.java \
    java/Inspector/Addons.xcu \
    java/Inspector/HideableMutableTreeNode.java \
    java/Inspector/HideableTreeModel.java \
    java/Inspector/Inspector.java \
    java/Inspector/InspectorAddon.java \
    java/Inspector/InspectorPane.java \
    java/Inspector/InstanceInspector.idl \
    java/Inspector/InstanceInspectorTest.odt \
    java/Inspector/Introspector.java \
    java/Inspector/Makefile \
    java/Inspector/MethodParametersDialog.java \
    java/Inspector/ObjectInspector.xcs \
    java/Inspector/ObjectInspector.xcu \
    java/Inspector/ProtocolHandler.xcu \
    java/Inspector/ProtocolHandlerAddon.java \
    java/Inspector/SourceCodeGenerator.java \
    java/Inspector/SwingDialogProvider.java \
    java/Inspector/SwingTreeControlProvider.java \
    java/Inspector/SwingTreePathProvider.java \
    java/Inspector/SwingUnoFacetteNode.java \
    java/Inspector/SwingUnoMethodNode.java \
    java/Inspector/SwingUnoNode.java \
    java/Inspector/SwingUnoPropertyNode.java \
    java/Inspector/TDocSupplier.java \
    java/Inspector/TestInspector.java \
    java/Inspector/UnoMethodNode.java \
    java/Inspector/UnoNode.java \
    java/Inspector/UnoPropertyNode.java \
    java/Inspector/UnoTreeRenderer.java \
    java/Inspector/XDialogProvider.java \
    java/Inspector/XInstanceInspector.idl \
    java/Inspector/XLanguageSourceCodeGenerator.java \
    java/Inspector/XMethodParametersDialog.java \
    java/Inspector/XTreeControlProvider.java \
    java/Inspector/XTreePathProvider.java \
    java/Inspector/XUnoFacetteNode.java \
    java/Inspector/XUnoMethodNode.java \
    java/Inspector/XUnoNode.java \
    java/Inspector/XUnoPropertyNode.java \
    java/Inspector/images/containers_16.png \
    java/Inspector/images/content_16.png \
    java/Inspector/images/interfaces_16.png \
    java/Inspector/images/methods_16.png \
    java/Inspector/images/properties_16.png \
    java/Inspector/images/services_16.png \
    java/Inspector/manifest.mf \
    java/MinimalComponent/BuildMinimalComponent.xml \
    java/MinimalComponent/Makefile \
    java/MinimalComponent/MinimalComponent.components \
    java/MinimalComponent/MinimalComponent.idl \
    java/MinimalComponent/MinimalComponent.java \
    java/MinimalComponent/TestMinimalComponent.java \
    java/PropertySet/Makefile \
    java/PropertySet/PropTest.components \
    java/PropertySet/PropTest.java \
    java/PropertySet/PropertySet.odt \
    java/Spreadsheet/CalcAddins.components \
    java/Spreadsheet/CalcAddins.java \
    java/Spreadsheet/CalcAddins.ods \
    java/Spreadsheet/ChartTypeChange.java \
    java/Spreadsheet/EuroAdaption.java \
    java/Spreadsheet/Makefile \
    java/Spreadsheet/SCalc.java \
    java/Spreadsheet/XCalcAddins.idl \
    java/Text/BookmarkInsertion.java \
    java/Text/GraphicsInserter.java \
    java/Text/HardFormatting.java \
    java/Text/Makefile \
    java/Text/SWriter.java \
    java/Text/StyleCreation.java \
    java/Text/StyleInitialization.java \
    java/Text/TextDocumentStructure.java \
    java/Text/TextReplace.java \
    java/Text/WriterSelector.java \
    java/Text/oo_smiley.gif \
    java/ToDo/Makefile \
    java/ToDo/ToDo.components \
    java/ToDo/ToDo.idl \
    java/ToDo/ToDo.java \
    java/ToDo/ToDo.ods \
    java/ToDo/XToDo.idl \
    python/toolpanel/CalcWindowState.xcu \
    python/toolpanel/Factory.xcu \
    python/toolpanel/META-INF/manifest.xml \
    python/toolpanel/Makefile \
    python/toolpanel/description.xml \
    python/toolpanel/readme \
    python/toolpanel/toolPanelPocBasic/Module1.xba \
    python/toolpanel/toolPanelPocBasic/dialog.xlb \
    python/toolpanel/toolPanelPocBasic/script.xlb \
    python/toolpanel/toolpanel.component \
    python/toolpanel/toolpanel.py \
    python/toolpanel/toolpanels/poc.xdl \
))

# vim: set noet sw=4 ts=4:
