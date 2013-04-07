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
package com.sun.star.wizards.web;

import javax.swing.DefaultListModel;

import com.sun.star.awt.Size;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.ConfigSet;
import com.sun.star.wizards.common.Configuration;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.common.SystemDialog;
import com.sun.star.wizards.ui.ImageList;
import com.sun.star.wizards.web.data.CGImage;
import com.sun.star.wizards.web.data.CGSettings;

/**
 * @author rpiterman
 *
 * To change the template for this generated type comment go to
 * Window>Preferences>Java>Code Generation>Code and Comments
 */
public class BackgroundsDialog extends ImageListDialog
{

    private FileAccess fileAccess;
    private SystemDialog sd;
    private CGSettings settings;

    /**
     * @param xmsf
     */
    public BackgroundsDialog(
            XMultiServiceFactory xmsf,
            ConfigSet set_, WebWizardDialogResources resources) throws Exception
    {

        super(xmsf, WWHID.HID_BG, new String[]
                {
                    resources.resBackgroundsDialog,
                    resources.resBackgroundsDialogCaption,
                    resources.resOK,
                    resources.resCancel,
                    resources.resHelp,
                    resources.resDeselect,
                    resources.resOther,
                    resources.resCounter
                });

        sd = SystemDialog.createOpenDialog(xmsf);
        sd.addFilter(resources.resImages, "*.jpg;*.jpeg;*.jpe;*.gif", true);
        sd.addFilter(resources.resAllFiles, "*.*", false);

        settings = (CGSettings) set_.root;

        fileAccess = new FileAccess(xmsf);
        il.setListModel(new Model(set_));
        il.setImageSize(new Size(40, 40));
        il.setRenderer(new BGRenderer(0));
        build();


    }

    /**
     * trigered when the user clicks the "other" button.
     * opens a "file open" dialog, adds the selected 
     * image to the list and to the web wizard configuration,
     * and then jumps to the new image, selecting it in the list.
     * @see add(String)
     */
    public void other()
    {
        String filename[] = sd.callOpenDialog(false, settings.cp_DefaultSession.cp_InDirectory);
        if (filename != null && filename.length > 0 && filename[0] != null)
        {
            settings.cp_DefaultSession.cp_InDirectory = FileAccess.getParentDir(filename[0]);
            int i = add(filename[0]);
            il.setSelected(i);
            il.display(i);
        }
    }

    /**
     * adds the given image to the image list (to the model) 
     * and to the web wizard configuration.
     * @param s
     * @return
     */
    private int add(String s)
    {

        //first i check the item does not already exists in the list...
        for (int i = 0; i < il.getListModel().getSize(); i++)
        {
            if (il.getListModel().getElementAt(i).equals(s))
            {
                return i;
            }
        }
        ((DefaultListModel) il.getListModel()).addElement(s);
        try
        {
            Object configView = Configuration.getConfigurationRoot(xMSF, FileAccess.connectURLs(WebWizardConst.CONFIG_PATH, "BackgroundImages"), true);
            int i = Configuration.getChildrenNames(configView).length + 1;
            Object o = Configuration.addConfigNode(configView, PropertyNames.EMPTY_STRING + i);
            Configuration.set(s, "Href", o);
            Configuration.commit(configView);
        }
        catch (Exception ex)
        {
            ex.printStackTrace();
        }

        return il.getListModel().getSize() - 1;

    }

    /**
     * an ImageList Imagerenderer implemtation. 
     * The image URL is the object given from the list model.
     * the image name, got from the "render" method is 
     * the filename portion of the url.
     * @author rpiterman
     *
     */
    private class BGRenderer implements ImageList.IImageRenderer
    {

        private int cut;

        public BGRenderer(int cut_)
        {
            cut = cut_;
        }

        public Object[] getImageUrls(Object listItem)
        {
            Object[] sRetUrls;
            if (listItem != null)
            {
                sRetUrls = new Object[1];
                sRetUrls[0] = listItem;
                return sRetUrls;
            }
            return null;
        }

        public String render(Object object)
        {
            return object == null ? PropertyNames.EMPTY_STRING : FileAccess.getPathFilename(fileAccess.getPath((String) object, null));
        }
    }

    /**
     * This is a list model for the image list of the 
     * backgrounds dialog.
     * It takes the Backgrounds config set as an argument,
     * and "parses" it to a list of files:
     * It goes through each image in the set, and checks it:
     * if it is a directory it lists all image files in this directory.
     * if it is a file, it adds the file to the list. 
     * @author rpiterman
     */
    private class Model extends DefaultListModel
    {

        /**
         * constructor. </br>
         * see class description for a description of
         * the handling of the given model
         * @param model the configuration set of the background images.
         */
        public Model(ConfigSet model)
        {
            try
            {
                for (int i = 0; i < model.getSize(); i++)
                {
                    CGImage image = (CGImage) model.getElementAt(i);
                    String path = sd.xStringSubstitution.substituteVariables(image.cp_Href, false);
                    if (fileAccess.exists(path, false))
                    {
                        addDir(path);
                    }
                    else
                    {
                        remove((String) model.getKey(image));
                    }
                }
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }

        /**
         * when instanciating the model, it checks if each image
         * exists. If it doesnot, it will be removed from
         * the configuration.
         * This is what this method does...
         * @param imageName
         */
        private void remove(String imageName)
        {
            try
            {
                Object conf = Configuration.getConfigurationRoot(xMSF, WebWizardConst.CONFIG_PATH + "/BackgroundImages", true);
                Configuration.removeNode(conf, imageName);
            }
            catch (Exception ex)
            {
                ex.printStackTrace();
            }
        }

        /**
         * if the given url is a directory
         * adds the images in the given directory,
         * otherwise (if it is a file) adds the file to the list.
         * @param url
         */
        private void addDir(String url)
        {
            if (fileAccess.isDirectory(url))
            {
                add(fileAccess.listFiles(url, false));
            }
            else
            {
                add(url);
            }
        }

        /**
         * adds the given filenames (urls) to
         * the list
         * @param filenames
         */
        private void add(String[] filenames)
        {
            for (int i = 0; i < filenames.length; i++)
            {
                add(filenames[i]);
            }
        }

        /**
         * adds the given image url to the list.
         * if and only if it ends with jpg, jpeg or gif 
         * (case insensitive) 
         * @param filename image url.
         */
        private void add(String filename)
        {
            String lcase = filename.toLowerCase();
            if (lcase.endsWith("jpg") ||
                    lcase.endsWith("jpeg") ||
                    lcase.endsWith("gif"))
            {
                Model.this.addElement(filename);
            }
        }
    }
}
