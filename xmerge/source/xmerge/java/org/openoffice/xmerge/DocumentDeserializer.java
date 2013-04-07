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

package org.openoffice.xmerge;

import java.io.IOException;

/**
 *  <p>A <code>DocumentDeserializer</code> represents a converter that
 *  converts &quot;Device&quot; <code>Document</code> objects into the
 *  &quot;Office&quot; <code>Document</code> format.</p>
 *
 *  <p>The <code>DocumentDeserializer</code> object is created by
 *  the </code>PluginFactory</code> {@link
 *  org.openoffice.xmerge.DocumentDeserializerFactory#createDocumentDeserializer
 *  createDocumentDeserializer} method.  When it is constructed, a
 *  <code>ConvertData</code> object is passed in to be used as input.</p>
 *
 *  @author  Herbie Ong
 *  @see     org.openoffice.xmerge.PluginFactory
 *  @see     org.openoffice.xmerge.DocumentDeserializerFactory
 */
public interface DocumentDeserializer {

    /**
     *  <p>Convert the data passed into the <code>DocumentDeserializer</code>
     *  constructor into the &quot;Office&quot; <code>Document</code>
     *  format.</p>
     *
     *  <p>This method may or may not be thread-safe.  It is expected
     *  that the user code does not call this method in more than one
     *  thread.  And for most cases, this method is only done once.</p>
     *
     *  @return  The resulting <code>Document</code> object from conversion.
     *
     *  @throws  ConvertException  If any Convert error occurs.
     *  @throws  IOException       If any I/O error occurs.
     */
    public Document deserialize() throws ConvertException, IOException;
}

