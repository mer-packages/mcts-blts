/* blts_result_xml.c -- Results XML writer

   Copyright (C) 2010, Nokia Corporation.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 2.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <libxml/xmlwriter.h>

#include "blts_results_xml.h"

static const char *encoding = "UTF-8";
static int append_cases = 0;
static xmlDocPtr doc = NULL;
static xmlTextWriterPtr writer;
static char *xml_file;

static xmlNode *search_node(xmlNode *parent, const char *name);

static xmlChar *make_string_xml_safe(const char *str)
{
	if (!doc || !str)
		return NULL;

	return xmlEncodeEntitiesReentrant(doc, str);
}

int xml_result_open(const char *filename, const char *suite, const char *set,
	int append)
{
	xmlChar *tmp;

	if (!filename || !suite || !set || doc)
		return -EFAULT;

	append_cases = append;
	xml_file = strdup(filename);
	if (!xml_file)
		return -ENOMEM;

	if (append && !access(filename, W_OK)) {
		doc = xmlParseFile(filename);
		if (doc)
			return 0;
	}

	append_cases = 0;
	doc = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);
	if (!doc)
		return -1;

	writer = xmlNewTextWriterTree(doc, NULL, 0);
	if (!writer)
		return -1;

	xmlTextWriterSetIndent(writer, 1);
	if (xmlTextWriterStartDocument(writer, NULL, encoding, NULL) < 0)
		return -1;

	if (xmlTextWriterStartElement(writer, BAD_CAST "testresults") < 0)
		return -1;

	if (xmlTextWriterWriteAttribute(writer, BAD_CAST "version",
		BAD_CAST "1.0") < 0)
		return -1;

	if (xmlTextWriterWriteAttribute(writer, BAD_CAST "environment",
		BAD_CAST "unknown") < 0)
		return -1;

	if (xmlTextWriterWriteAttribute(writer, BAD_CAST "hwproduct",
		BAD_CAST "unknown") < 0)
		return -1;

	if (xmlTextWriterWriteAttribute(writer, BAD_CAST "hwbuild",
		BAD_CAST "unknown") < 0)
		return -1;

	if (xmlTextWriterStartElement(writer, BAD_CAST "suite") < 0)
		return -1;

	tmp = make_string_xml_safe(suite);
	if (!tmp)
		return -1;
	if (xmlTextWriterWriteAttribute(writer, BAD_CAST "name", tmp) < 0) {
		xmlFree(tmp);
		return -1;
	}
	xmlFree(tmp);

	if (xmlTextWriterStartElement(writer, BAD_CAST "set") < 0)
		return -1;

	tmp = make_string_xml_safe(set);
	if (!tmp)
		return -1;
	if (xmlTextWriterWriteAttribute(writer, BAD_CAST "name", tmp) < 0) {
		xmlFree(tmp);
		return -1;
	}
	xmlFree(tmp);

	return 0;
}

void xml_result_close()
{
	if (doc) {
		if (writer) {
			xmlTextWriterEndDocument(writer);
			xmlFreeTextWriter(writer);
			writer = NULL;
		}

		if (xml_file) {
			xmlSaveFileEnc(xml_file, doc, encoding);
			free(xml_file);
			xml_file = NULL;
		}

		xmlFreeDoc(doc);
		doc = NULL;
	}
}

int xml_result_write_step(int expected_result, int return_code, time_t *start,
	time_t *end, const char *cmdline, const char *failure_info)
{
	struct tm *tm;
	xmlChar *tmp;

	if (!writer || !start || !end)
		return -EFAULT;

	if (xmlTextWriterStartElement(writer, BAD_CAST "step") < 0)
		return -1;

	if (cmdline) {
		tmp = make_string_xml_safe(cmdline);
		if (!tmp)
			return -1;
		if (xmlTextWriterWriteAttribute(writer, BAD_CAST "command",
			tmp) < 0) {
			xmlFree(tmp);
			return -1;
		}
		xmlFree(tmp);
	}

	if (xmlTextWriterWriteAttribute(writer, BAD_CAST "result",
		expected_result == return_code ?
		BAD_CAST "PASS" : BAD_CAST "FAIL") < 0)
		return -1;

	if (failure_info) {
		tmp = make_string_xml_safe(failure_info);
		if (!tmp)
			return -1;
		if (xmlTextWriterWriteAttribute(writer, BAD_CAST "failure_info",
			failure_info) < 0) {
			xmlFree(tmp);
			return -1;
		}
		xmlFree(tmp);
	}

	if (xmlTextWriterWriteFormatElement(writer, BAD_CAST "expected_result",
		"%d", expected_result) < 0)
		return -1;

	if (xmlTextWriterWriteFormatElement(writer, BAD_CAST "return_code",
		"%d", return_code) < 0)
		return -1;

	tm = localtime(start);
	if (xmlTextWriterWriteFormatElement(writer, BAD_CAST "start",
		"%02d-%02d-%02d %02d:%02d:%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec) < 0)
		return -1;

	tm = localtime(end);
	if (xmlTextWriterWriteFormatElement(writer, BAD_CAST "end",
		"%02d-%02d-%02d %02d:%02d:%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec) < 0)
		return -1;

	return xmlTextWriterEndElement(writer);
}

int xml_result_start_case(const char *name, int manual, int result)
{
	xmlChar *tmp;

	if (!name)
		return -EFAULT;

	if (append_cases) {
		writer = xmlNewTextWriterTree(doc,
			search_node(xmlDocGetRootElement(doc), "set"), 0);
		if (!writer)
			return -1;

		xmlTextWriterSetIndent(writer, 1);
		if (xmlTextWriterStartDocument(writer, NULL, encoding, NULL) < 0)
			return -1;
	} else if(!writer)
		return -EFAULT;

	if (xmlTextWriterStartElement(writer, BAD_CAST "case") < 0)
		return -1;

	tmp = make_string_xml_safe(name);
	if (!tmp)
		return -1;
	if (xmlTextWriterWriteAttribute(writer, BAD_CAST "name", tmp) < 0) {
		xmlFree(tmp);
		return -1;
	}
	xmlFree(tmp);

	if (xmlTextWriterWriteAttribute(writer, BAD_CAST "manual",
		BAD_CAST (manual ? "true" : "false")) < 0)
		return -1;

	if (xmlTextWriterWriteAttribute(writer, BAD_CAST "insignificant",
		BAD_CAST "false") < 0)
		return -1;

	if (xmlTextWriterWriteAttribute(writer, BAD_CAST "result",
		BAD_CAST (result ? "FAIL" : "PASS")) < 0)
		return -1;

	return 0;
}

static xmlNode *search_node(xmlNode *parent, const char *name)
{
	xmlNode *cur_node, *node;

	if (!parent || !name)
		return NULL;

	for (cur_node = parent; cur_node; cur_node = cur_node->next) {
		if (cur_node->type == XML_ELEMENT_NODE) {
			if (!strcmp((char*)cur_node->name, name))
				return cur_node;
			if (cur_node->children)
				if ((node = search_node(cur_node->children, name)))
					return node;
		}
	}

	return NULL;
}

int xml_result_end_case()
{
	if (!writer)
		return -EFAULT;

	xmlTextWriterEndElement(writer);
	if (append_cases) {
		xmlTextWriterEndDocument(writer);
		xmlFreeTextWriter(writer);
		writer = NULL;
	}

	return 0;
}

