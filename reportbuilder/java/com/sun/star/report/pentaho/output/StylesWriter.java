/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.report.pentaho.output;

import com.sun.star.report.pentaho.OfficeNamespaces;
import com.sun.star.report.pentaho.model.DataStyle;
import com.sun.star.report.pentaho.model.FontFaceDeclsSection;
import com.sun.star.report.pentaho.model.FontFaceElement;
import com.sun.star.report.pentaho.model.OfficeMasterPage;
import com.sun.star.report.pentaho.model.OfficeMasterStyles;
import com.sun.star.report.pentaho.model.OfficeStyle;
import com.sun.star.report.pentaho.model.OfficeStyles;
import com.sun.star.report.pentaho.model.OfficeStylesCollection;
import com.sun.star.report.pentaho.model.PageLayout;
import com.sun.star.report.pentaho.model.RawText;

import java.io.IOException;
import java.io.Writer;

import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;

import org.jfree.layouting.namespace.Namespaces;
import org.jfree.layouting.util.AttributeMap;
import org.jfree.report.JFreeReportBoot;
import org.jfree.report.JFreeReportInfo;
import org.jfree.report.ReportProcessingException;
import org.jfree.report.structure.Element;
import org.jfree.report.structure.Node;
import org.jfree.report.structure.Section;
import org.jfree.report.structure.StaticText;

import org.pentaho.reporting.libraries.xmlns.common.AttributeList;
import org.pentaho.reporting.libraries.xmlns.writer.DefaultTagDescription;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriter;
import org.pentaho.reporting.libraries.xmlns.writer.XmlWriterSupport;


/**
 * This class writes the style collection into a 'styles.xml' document.
 *
 * @author Thomas Morgner
 * @since 09.03.2007
 */
public class StylesWriter
{

    private final XmlWriter xmlWriter;
    private final boolean writeOpeningTag;

    public StylesWriter(final Writer writer)
    {
        final DefaultTagDescription tagDescription = new DefaultTagDescription();
        tagDescription.configure(JFreeReportBoot.getInstance().getGlobalConfig(),
                OfficeDocumentReportTarget.TAG_DEF_PREFIX);

        this.xmlWriter = new XmlWriter(writer, tagDescription);
        this.xmlWriter.setAlwaysAddNamespace(true);
        this.writeOpeningTag = true;
    }

    public StylesWriter(final XmlWriter xmlWriter)
    {
        this.xmlWriter = xmlWriter;
        this.writeOpeningTag = false;
    }

    public void writeContentStyles(final OfficeStylesCollection predefined,
            final OfficeStylesCollection globals)
            throws IOException, ReportProcessingException
    {
        writeFontFaces(new OfficeStylesCollection[]
                {
                    globals
                });
        writeAutomaticStylesSection(new OfficeStylesCollection[]
                {
                    globals
                });
    }

    public void writeGlobalStyles(final OfficeStylesCollection predefined,
            final OfficeStylesCollection globals)
            throws IOException, ReportProcessingException
    {
        if (writeOpeningTag)
        {
            performWriteRootTag();
        }

        writeFontFaces(new OfficeStylesCollection[]
                {
                    globals
                });
        writeCommonStylesSection(new OfficeStylesCollection[]
                {
                    globals
                });
        writeAutomaticStylesSection(new OfficeStylesCollection[]
                {
                    globals
                });
        writeMasterStylesSection(new OfficeStylesCollection[]
                {
                    globals
                });

        if (writeOpeningTag)
        {
            xmlWriter.writeCloseTag();
        }
    }

    private void writeMasterStylesSection(final OfficeStylesCollection[] osc)
            throws IOException
    {
        xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "master-styles", XmlWriterSupport.OPEN);
        for (int sci = 0; sci < osc.length; sci++)
        {
            final OfficeStylesCollection collection = osc[sci];
            final OfficeMasterStyles officeStyles = collection.getMasterStyles();
            final OfficeMasterPage[] officeMasterPages = officeStyles.getAllMasterPages();
            for (int i = 0; i < officeMasterPages.length; i++)
            {
                final OfficeMasterPage masterPage = officeMasterPages[i];
                writeSection(masterPage);
            }

            writeSectionChilds(officeStyles.getOtherNodes().getNodeArray());
        }

        xmlWriter.writeCloseTag();
    }

    private void writeCommonStylesSection(final OfficeStylesCollection[] osc)
            throws IOException
    {
        xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "styles", XmlWriterSupport.OPEN);

        for (int sci = 0; sci < osc.length; sci++)
        {
            final OfficeStylesCollection collection = osc[sci];
            final OfficeStyles officeStyles = collection.getCommonStyles();
            writeStyles(officeStyles);
        }

        xmlWriter.writeCloseTag();
    }

    private void writeAutomaticStylesSection(final OfficeStylesCollection[] osc)
            throws IOException
    {
        xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "automatic-styles", XmlWriterSupport.OPEN);
        for (int sci = 0; sci < osc.length; sci++)
        {
            final OfficeStylesCollection collection = osc[sci];
            final OfficeStyles officeStyles = collection.getAutomaticStyles();
            writeStyles(officeStyles);
        }

        xmlWriter.writeCloseTag();
    }

    private void writeFontFaces(final OfficeStylesCollection[] osc)
            throws IOException
    {
        xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS, "font-face-decls", XmlWriterSupport.OPEN);

        final TreeMap fontFaces = new TreeMap();
        for (int sci = 0; sci < osc.length; sci++)
        {
            final OfficeStylesCollection collection = osc[sci];
            final FontFaceDeclsSection fontFaceDecls = collection.getFontFaceDecls();
            final FontFaceElement[] fontFaceElements = fontFaceDecls.getAllFontFaces();
            for (int i = 0; i < fontFaceElements.length; i++)
            {
                final FontFaceElement element = fontFaceElements[i];
                fontFaces.put(element.getStyleName(), element);
            }
        }

        final Iterator values = fontFaces.values().iterator();
        while (values.hasNext())
        {
            final FontFaceElement element = (FontFaceElement) values.next();
            writeElement(element);
        }

        xmlWriter.writeCloseTag();
    }

    private void writeStyles(final OfficeStyles styles)
            throws IOException
    {
        final OfficeStyle[] allStyles = styles.getAllStyles();
        for (int i = 0; i < allStyles.length; i++)
        {
            final OfficeStyle style = allStyles[i];
            writeSection(style);
        }

        final DataStyle[] allDataStyles = styles.getAllDataStyles();
        for (int i = 0; i < allDataStyles.length; i++)
        {
            final DataStyle style = allDataStyles[i];
            writeSection(style);
        }

        final PageLayout[] allPageStyles = styles.getAllPageStyles();
        for (int i = 0; i < allPageStyles.length; i++)
        {
            final PageLayout style = allPageStyles[i];
            writeSection(style);
        }

        writeSectionChilds(styles.getOtherStyles());
    }

    private void writeElement(final Element element)
            throws IOException
    {
        final String type = element.getType();
        final String namespace = element.getNamespace();
        final AttributeList attrList = buildAttributeList(element.getAttributeMap());
        xmlWriter.writeTag(namespace, type, attrList, XmlWriterSupport.CLOSE);
    }

    private void writeSection(final Section section)
            throws IOException
    {
        final String type = section.getType();
        final String namespace = section.getNamespace();
        final AttributeList attrList = buildAttributeList(section.getAttributeMap());
        if (section.getNodeCount() == 0)
        {
            xmlWriter.writeTag(namespace, type, attrList, XmlWriterSupport.CLOSE);
            return;
        }

        xmlWriter.writeTag(namespace, type, attrList, XmlWriterSupport.OPEN);
        writeSectionChilds(section.getNodeArray());

        xmlWriter.writeCloseTag();
    }

    private void writeSectionChilds(final Node[] nodes)
            throws IOException
    {
        for (int i = 0; i < nodes.length; i++)
        {
            final Node node = nodes[i];
            if (node instanceof Section)
            {
                writeSection((Section) node);
            }
            else if (node instanceof Element)
            {
                writeElement((Element) node);
            }
            else if (node instanceof RawText)
            {
                final RawText text = (RawText) node;
                xmlWriter.writeText(text.getText());
            }
            else if (node instanceof StaticText)
            {
                final StaticText text = (StaticText) node;
                xmlWriter.writeTextNormalized(text.getText(), false);
            }
        }
    }

    protected AttributeList buildAttributeList(final AttributeMap attrs)
    {
        final AttributeList attrList = new AttributeList();
        final String[] namespaces = attrs.getNameSpaces();
        for (int i = 0; i < namespaces.length; i++)
        {
            final String attrNamespace = namespaces[i];
            if (isFilteredNamespace(attrNamespace))
            {
                continue;
            }

            final Map localAttributes = attrs.getAttributes(attrNamespace);
            final Iterator entries = localAttributes.entrySet().iterator();
            while (entries.hasNext())
            {
                final Map.Entry entry = (Map.Entry) entries.next();
                final String key = String.valueOf(entry.getKey());
                attrList.setAttribute(attrNamespace, key, String.valueOf(entry.getValue()));
            }
        }
        return attrList;
    }

    protected boolean isFilteredNamespace(final String namespace)
    {
        if (Namespaces.LIBLAYOUT_NAMESPACE.equals(namespace))
        {
            return true;
        }
        if (JFreeReportInfo.REPORT_NAMESPACE.equals(namespace))
        {
            return true;
        }
        if (JFreeReportInfo.COMPATIBILITY_NAMESPACE.equals(namespace))
        {
            return true;
        }
        if (OfficeNamespaces.OOREPORT_NS.equals(namespace))
        {
            return true;
        }
        return false;
    }

    private void performWriteRootTag()
            throws IOException
    {
        final AttributeList rootAttributes = new AttributeList();
        rootAttributes.addNamespaceDeclaration("office",
                OfficeNamespaces.OFFICE_NS);
        rootAttributes.addNamespaceDeclaration("style", OfficeNamespaces.STYLE_NS);
        rootAttributes.addNamespaceDeclaration("text", OfficeNamespaces.TEXT_NS);
        rootAttributes.addNamespaceDeclaration("table", OfficeNamespaces.TABLE_NS);
        rootAttributes.addNamespaceDeclaration("draw", OfficeNamespaces.DRAWING_NS);
        rootAttributes.addNamespaceDeclaration("fo", OfficeNamespaces.FO_NS);
        rootAttributes.addNamespaceDeclaration("xlink", OfficeNamespaces.XLINK_NS);
        rootAttributes.addNamespaceDeclaration("dc", OfficeNamespaces.PURL_NS);
        rootAttributes.addNamespaceDeclaration("meta", OfficeNamespaces.META_NS);
        rootAttributes.addNamespaceDeclaration("number",
                OfficeNamespaces.DATASTYLE_NS);
        rootAttributes.addNamespaceDeclaration("svg", OfficeNamespaces.SVG_NS);
        rootAttributes.addNamespaceDeclaration("chart", OfficeNamespaces.CHART_NS);
        rootAttributes.addNamespaceDeclaration("chartooo", OfficeNamespaces.CHARTOOO_NS);
        rootAttributes.addNamespaceDeclaration("dr3d", OfficeNamespaces.DR3D_NS);
        rootAttributes.addNamespaceDeclaration("math", OfficeNamespaces.MATHML_NS);
        rootAttributes.addNamespaceDeclaration("form", OfficeNamespaces.FORM_NS);
        rootAttributes.addNamespaceDeclaration("script",
                OfficeNamespaces.SCRIPT_NS);
        rootAttributes.addNamespaceDeclaration("ooo", OfficeNamespaces.OO2004_NS);
        rootAttributes.addNamespaceDeclaration("ooow", OfficeNamespaces.OOW2004_NS);
        rootAttributes.addNamespaceDeclaration("oooc", OfficeNamespaces.OOC2004_NS);
        rootAttributes.addNamespaceDeclaration("dom",
                OfficeNamespaces.XML_EVENT_NS);
        rootAttributes.addNamespaceDeclaration("xforms",
                OfficeNamespaces.XFORMS_NS);
        rootAttributes.addNamespaceDeclaration("xsd", OfficeNamespaces.XSD_NS);
        rootAttributes.addNamespaceDeclaration("xsi", OfficeNamespaces.XSI_NS);
        rootAttributes.addNamespaceDeclaration("grddl", OfficeNamespaces.GRDDL_NS);
        rootAttributes.setAttribute(OfficeNamespaces.OFFICE_NS, "version",
                OfficeDocumentReportTarget.ODF_VERSION);

        this.xmlWriter.writeXmlDeclaration("UTF-8");
        this.xmlWriter.writeTag(OfficeNamespaces.OFFICE_NS,
                "document-styles", rootAttributes, XmlWriterSupport.OPEN);
    }

    public void close()
            throws IOException
    {
        xmlWriter.close();
    }
}
