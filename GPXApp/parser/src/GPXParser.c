#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "LinkedListAPI.h"
#include "GPXParser.h"

#define PI 3.14159265359

/*
Name: Joshua Stone
Student number: 1013900
Assignment: 2
Date: October 24, 2019
*/

static void print_element_names(xmlNode * a_node, GPXdoc **GPXdocPtr, Waypoint *wpt, Route *rte, Track *trk);
void getWptAttributes(xmlNode *a_node, Waypoint **wpt);
void getRteAttributes(xmlNode *a_node, Route **rte);
void getRteWaypointAttributes(xmlNode *a_node, Waypoint **rteWaypoint);
void getTrkAttributes(xmlNode *a_node, Track **trk);
void getTrkSegments(xmlNode *a_node, TrackSegment **trkseg);
xmlDocPtr createXMLTree();
float calcHaversine(float lat1, float lon1, float lat2, float lon2);

//this is the main function i have to fill all the linked lists
static void print_element_names(xmlNode * a_node, GPXdoc **GPXdocPtr, Waypoint *wpt, Route *rte, Track *trk){
  xmlNode *cur_node = NULL;
  //Attribute wptAtt = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

  for(cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if (cur_node->type == XML_ELEMENT_NODE) {
      //printf("node type: Element, name: %s\n", cur_node->name);

      //currently using this for grabbing version & creator
      xmlAttr* attribute = cur_node->properties;
      while(attribute != NULL){
        xmlChar* value = xmlNodeListGetString(cur_node->doc, attribute->children, 1);
        //printf("%s = %s\n", attribute->name, value);
        unsigned char *version =(unsigned char *) "version";
        if(xmlStrncmp(attribute->name, version, 7) == 0){
          //unsigned char *GPXversion = value;
          //        if(xmlStrncmp(attribute->name, xmlCharStrdup("version"), 7) == 0){
          (*GPXdocPtr)->version = atof((const char *)value);
        }
        unsigned char *creator =(unsigned char *) "creator";
        if(xmlStrncmp(attribute->name, creator, 7) == 0){
          //needed to malloc strlen+1 becuase of an invalid read size of 1 from xmlStrlen
          //memory leak here though
          (*GPXdocPtr)->creator = malloc(xmlStrlen(value)+1);
          strcpy((*GPXdocPtr)->creator, (char *)value);
        }
        xmlFree(value);
        attribute = attribute->next;
      }
      //end of Version and Creator

      //To fill wpt struct with what it needs
      unsigned char *wptName =(unsigned char *) "wpt";
      if(xmlStrcmp(cur_node->name, wptName) == 0){
        wpt = malloc(sizeof(Waypoint));
        wpt->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
        //name must might be empty but must not be null - check
        wpt->name = malloc(sizeof(char) + 1);
        strcpy(wpt->name, "");

        xmlAttr *attr;
        for (attr = cur_node->properties; attr != NULL; attr = attr->next){
          xmlNode *value = attr->children;
          char *attrName = (char *)attr->name;
          char *cont = (char *)(value->content);
          //printf("attribute name: %s, attribute value = %s\n", attrName, cont);
          if(strcmp(attrName, "lat") == 0){
            wpt->latitude = atof(cont);
          }
          else if(strcmp(attrName, "lon") == 0){
            wpt->longitude = atof(cont);
          }
        }
        getWptAttributes(cur_node->children, &wpt);
        //now that its filled, put it in the list
        insertBack((*GPXdocPtr)->waypoints, wpt);

      }

      //Fills the route struct with necessary stuff
      unsigned char *rteName =(unsigned char *) "rte";
      if(xmlStrcmp(cur_node->name, rteName) == 0){
        rte = malloc(sizeof(Route));
        rte->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
        rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
        //name must might be empty but must not be null - check
        rte->name = malloc(sizeof(char) + 1);
        strcpy(rte->name, "");

        getRteAttributes(cur_node->children, &rte);
        //now this one is filled too, add to gpx list
        insertBack((*GPXdocPtr)->routes, rte);
      }

      //fills the Track with necessary stuff
      unsigned char *trkName =(unsigned char *) "trk";
      if(xmlStrcmp(cur_node->name, trkName) == 0){
        trk = malloc(sizeof(Track));
        trk->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
        trk->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
        //name must might be empty but must not be null - check
        trk->name = malloc(sizeof(char) + 1);
        strcpy(trk->name, "");

        getTrkAttributes(cur_node->children, &trk);
        //Fill GPXDocPtr with the Tracks
        insertBack((*GPXdocPtr)->tracks, trk);
      }

    }

    print_element_names(cur_node->children, GPXdocPtr, wpt, rte, trk);
  }
}
//gets the atrributes for the track list
void getTrkAttributes(xmlNode *a_node, Track **trk){
  xmlNode *cur_node = NULL;

  for(cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if(cur_node->type == XML_ELEMENT_NODE){

      char *nodeName = (char *)cur_node->name;
      //get the name of the route
      if(strcmp(nodeName, "name") == 0){
        if(cur_node->children != NULL){
          unsigned char *trkName = cur_node->children->content;
          (*trk)->name = realloc((*trk)->name, strlen((char *)cur_node->children->content) + 5);
          strcpy((*trk)->name, (char *)trkName);
        }
      }
      //get route points and add them to the route->waypoint list
      else if(strcmp(nodeName, "trkseg") == 0){
        TrackSegment *trkseg;
        trkseg = malloc(sizeof(TrackSegment));
        trkseg->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

        getTrkSegments(cur_node->children, &trkseg);

        insertBack((*trk)->segments, trkseg);
      }
      //gets any node that is not a name node or rtept node AKA attribute node
      else{
        Attribute *trkAttr;
        trkAttr = malloc(sizeof(Attribute));

        trkAttr->name = malloc(strlen((char *)cur_node->name) + 5);
        strcpy(trkAttr->name, (char *)cur_node->name);
        if(cur_node->children != NULL){
          trkAttr->value = malloc(strlen((char *)cur_node->children->content) + 5);
          strcpy(trkAttr->value, (char *)cur_node->children->content);
        }
        else{
          trkAttr->value = malloc(1);
          strcpy(trkAttr->value, "");
        }
        insertBack((*trk)->attributes, trkAttr);
      }
    }//end of if
  }//end of for
}
//gets the trkseg's
void getTrkSegments(xmlNode *a_node, TrackSegment **trkseg){
  xmlNode *cur_node = NULL;

  for(cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if(cur_node->type == XML_ELEMENT_NODE){
      Waypoint *trkSegWpt;
      trkSegWpt = malloc(sizeof(Waypoint));
      trkSegWpt->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
      trkSegWpt->name = malloc(sizeof(char) + 1);
      strcpy(trkSegWpt->name, "");

      xmlAttr *attr;
      for (attr = cur_node->properties; attr != NULL; attr = attr->next){
        xmlNode *value = attr->children;
        char *attrName = (char *)attr->name;
        char *cont = (char *)(value->content);
        //printf("attribute name: %s, attribute value = %s\n", attrName, cont);
        if(strcmp(attrName, "lat") == 0){
          trkSegWpt->latitude = atof(cont);
        }
        else if(strcmp(attrName, "lon") == 0){
          trkSegWpt->longitude = atof(cont);
        }
      }

      getWptAttributes(cur_node->children, &trkSegWpt);
      //now that its filled, put it in the list
      insertBack((*trkseg)->waypoints, trkSegWpt);
    }
  }
}
//gets name, rtept's and attributes of the RTE nodes
void getRteAttributes(xmlNode *a_node, Route **rte){
  xmlNode *cur_node = NULL;

  for(cur_node = a_node; cur_node; cur_node = cur_node->next) {
    if(cur_node->type == XML_ELEMENT_NODE){

      char *nodeName = (char *)cur_node->name;
      //get the name of the route
      if(strcmp(nodeName, "name") == 0){
        if(cur_node->children != NULL){
          unsigned char *rteName = cur_node->children->content;
          (*rte)->name = realloc((*rte)->name, strlen((char *)cur_node->children->content) + 5);
          strcpy((*rte)->name, (char *)rteName);
        }
      }
      //get route points and add them to the route->waypoint list
      else if(strcmp(nodeName, "rtept") == 0){
        Waypoint *rteWaypoint;
        rteWaypoint = malloc(sizeof(Waypoint));
        rteWaypoint->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
        rteWaypoint->name = malloc(sizeof(char) + 1);
        strcpy(rteWaypoint->name, "");

        xmlAttr *attr;
        for(attr = cur_node->properties; attr != NULL; attr = attr->next){
          xmlNode *value = attr->children;
          char *attrName = (char *)attr->name;
          char *cont = (char *)(value->content);
          //printf("attribute name: %s, attribute value = %s\n", attrName, cont);
          if(strcmp(attrName, "lat") == 0){
            rteWaypoint->latitude = atof(cont);
          }
          else if(strcmp(attrName, "lon") == 0){
            rteWaypoint->longitude = atof(cont);
          }
        }
        getRteWaypointAttributes(cur_node->children, &rteWaypoint);
        //Then insert the waypoint node that is full of RTE's lat,lon & name
        //Into route's list of waypoints
        insertBack((*rte)->waypoints, rteWaypoint);

      }
      //gets any node that is not a name node or rtept node AKA attribute node
      else{
        Attribute *rteAttr;
        rteAttr = malloc(sizeof(Attribute));

        rteAttr->name = malloc(strlen((char *)cur_node->name) + 5);
        strcpy(rteAttr->name, (char *)cur_node->name);
        if(cur_node->children != NULL){
          rteAttr->value = malloc(strlen((char *)cur_node->children->content) + 5);
          strcpy(rteAttr->value, (char *)cur_node->children->content);
        }
        else{
          rteAttr->value = malloc(1);
          strcpy(rteAttr->value, "");
        }
        insertBack((*rte)->attributes, rteAttr);
      }
    }//end of if
  }//end of for
}
//need to make sure name is empty but not null fits or there'll be an error
void getRteWaypointAttributes(xmlNode *a_node, Waypoint **rteWaypoint){
  xmlNode *cur_node = NULL;

  for(cur_node = a_node; cur_node; cur_node = cur_node->next) {
    //printf("%s\n", cur_node->name);
    if(cur_node->type == XML_ELEMENT_NODE){
      //get name of the wpt Node
      if(strcmp((char *)cur_node->name, "name") == 0){
        if(cur_node->children != NULL){
          unsigned char *wptName = cur_node->children->content;
          (*rteWaypoint)->name = realloc((*rteWaypoint)->name, strlen((char*)cur_node->children->content)+5);
          strcpy((*rteWaypoint)->name, (char *)wptName);
        }
      }
      //if its not the name, its an attribute
      else{
        Attribute *rteWptAttr;
        rteWptAttr = malloc(sizeof(Attribute));
        //printf("%s = %s\n", cur_node->name, cur_node->children->content);
        rteWptAttr->name = malloc(strlen( (char *)cur_node->name ) + 5);
        strcpy(rteWptAttr->name, (char *)cur_node->name);
        if(cur_node->children != NULL){
          rteWptAttr->value = malloc(strlen( (char *)cur_node->children->content ) + 5);
          strcpy(rteWptAttr->value, (char *)cur_node->children->content);
          //printf("Added: %s = %s\n", wptAttr->name, wptAttr->value);
        }
        else{
          rteWptAttr->value = malloc(1);
          strcpy(rteWptAttr->value, "");
        }
        insertBack((*rteWaypoint)->attributes, rteWptAttr);
        //char *myString = toString(wpt->attributes);
        //printf("%s\n", myString);
      }
    }
    //if(cur_node->children != NULL){
      //getWptAttributes(cur_node->children, wpt);
    //}
  }
}
//gets the name of waypoints, then mallocs and adds it into the struct with strcpy
void getWptAttributes(xmlNode *a_node, Waypoint **wpt){
  xmlNode *cur_node = NULL;

  for(cur_node = a_node; cur_node; cur_node = cur_node->next) {
    //printf("%s\n", cur_node->name);
    if(cur_node->type == XML_ELEMENT_NODE){
      //get name of the wpt Node
      if(strcmp((char *)cur_node->name, "name") == 0){
        if(cur_node->children != NULL){
          unsigned char *wptName = cur_node->children->content;
          //this might be causing a leak
          (*wpt)->name = realloc((*wpt)->name, strlen((char*)cur_node->children->content)+5);
          strcpy((*wpt)->name, (char *)wptName);
        }
      }
      //if its not the name, its an attribute
      else{
        Attribute *wptAttr;
        wptAttr = malloc(sizeof(Attribute));
        //printf("%s = %s\n", cur_node->name, cur_node->children->content);
        wptAttr->name = malloc(strlen( (char *)cur_node->name ) + 5);
        strcpy(wptAttr->name, (char *)cur_node->name);
        if(cur_node->children != NULL){
          wptAttr->value = malloc(strlen( (char *)cur_node->children->content ) + 5);
          strcpy(wptAttr->value, (char *)cur_node->children->content);
          //printf("Added: %s = %s\n", wptAttr->name, wptAttr->value);
          //char *myString = toString(wpt->attributes);
          //printf("%s\n", myString);
        }
        else{
          wptAttr->value = malloc(1);
          strcpy(wptAttr->value, "");
        }
        insertBack((*wpt)->attributes, wptAttr);
      }
    }
    //if(cur_node->children != NULL){
      //getWptAttributes(cur_node->children, wpt);
    //}
  }
}
//Creates the gpx doc and returns a pointer to it
GPXdoc* createGPXdoc(char* fileName){
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  // char *toStringGPX = NULL;

  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION

  /*parse the file and get the DOM */
  doc = xmlReadFile(fileName, NULL, 0);
  //test with invalid file
  if (doc == NULL) {
      //xmlFreeDoc(doc);
      xmlCleanupParser();

      printf("error: could not parse file %s\n", fileName);
      return NULL;
  }
  /*Get the root element node */
  root_element = xmlDocGetRootElement(doc);

  GPXdoc *GPXdocPtr = malloc(sizeof(GPXdoc));
  Waypoint *wpt = malloc(sizeof(Waypoint));
  Route *rte = malloc(sizeof(Route));
  Track *trk = malloc(sizeof(Track));

  strcpy(GPXdocPtr->namespace, "http://www.topografix.com/GPX/1/1");

  GPXdocPtr->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
  GPXdocPtr->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
  GPXdocPtr->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

  print_element_names(root_element, &GPXdocPtr, wpt, rte, trk);
  //printf("version = %.2f\n", GPXdocPtr->version);
  //printf("creator = %s\n", GPXdocPtr->creator);

  //int length = getLength(wpt->attributes);
  //printf("length = %d\n", length);


  // toStringGPX = GPXdocToString(GPXdocPtr);
  // printf("%s", toStringGPX);
  //
  // printf("Waypoints Length = %d\n", getNumWaypoints(GPXdocPtr));
  // printf("Routes Length = %d\n", getNumRoutes(GPXdocPtr));
  // printf("Tracks Length = %d\n", getNumTracks(GPXdocPtr));
  // printf("TrackSegment Length = %d\n", getNumSegments(GPXdocPtr));
  //
  /*
  Waypoint *tempWpt = getWaypoint(GPXdocPtr, "Steele Cabin");
  if(tempWpt == NULL){
    printf("Could Not Find Waypoint\n");
  }
  else{
    printf("Waypoint found: %s\n", tempWpt->name);
  }

  Route *tempRte = getRoute(GPXdocPtr, "Some route");
  if(tempRte == NULL){
    printf("Could Not Find Route\n");
  }
  else{
    printf("Route found: %s\n", tempRte->name);
  }

  Track *tempTrk = getTrack(GPXdocPtr, "Mount Steele Trail");
  if(tempTrk == NULL){
    printf("Could Not Find Route\n");
  }
  else{
    printf("Track found: %s\n", tempTrk->name);
  }
  */

  // free(toStringGPX);

  /*free the document */
  xmlFreeDoc(doc);
  free(wpt);
  free(rte);
  free(trk);
  /*
   *Free the global variables that may
   *have been allocated by the parser.
   */
  xmlCleanupParser();

  return GPXdocPtr;
}
//validates the doc against the GPX schema specification. TESTED GOOD
bool validateGPXDoc(GPXdoc* doc, char* gpxSchemaFile){
  if(doc == NULL || gpxSchemaFile == NULL) return 0;
  xmlDocPtr docPtr = NULL;
  docPtr = createXMLTree(doc);       /* document pointer */

  xmlSchemaPtr schema = NULL;
  xmlSchemaParserCtxtPtr ctxt;

  //xmlLineNumbersDefault(1);
  ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);

  schema = xmlSchemaParse(ctxt);
  xmlSchemaFreeParserCtxt(ctxt);
  int ret = 0;

  if (docPtr == NULL){
    fprintf(stderr, "Could not parse %s\n", gpxSchemaFile);
  }
  else{
    xmlSchemaValidCtxtPtr ctxt;

    ctxt = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(ctxt, (xmlSchemaValidityErrorFunc) fprintf, (xmlSchemaValidityWarningFunc) fprintf, stderr);
    ret = xmlSchemaValidateDoc(ctxt, docPtr);

    // if (ret == 0){
    //   printf("%s validates\n", gpxSchemaFile);
    // }
    // else if (ret > 0){
    // }
    // else{
    //   printf("%s validation generated an internal error\n", gpxSchemaFile);
    // }

    xmlSchemaFreeValidCtxt(ctxt);
    xmlFreeDoc(docPtr);
  }

  // free the resource
  if(schema != NULL) xmlSchemaFree(schema);

  if(ret == 0) ret = 1;
  else ret = 0;


  return ret;
}
//Function to write GPXdoc struct to a file. TESTED GOOD.
bool writeGPXdoc(GPXdoc* doc, char* fileName){
  if(doc == NULL || fileName == NULL) return 0;
  int returnVal = 0;

  xmlDocPtr docPtr = NULL;
  docPtr = createXMLTree(doc);

  char *fileDirectory = malloc(1000);
  //FIX BELOW
  strcpy(fileDirectory,"./uploads/");
  // strcpy(fileDirectory,"../uploads/");
  strcat(fileDirectory, fileName);
  // printf("Directory = %s\n", fileDirectory);
  returnVal = xmlSaveFormatFileEnc(fileDirectory, docPtr, "UTF-8", 1);
  xmlFreeDoc(docPtr);
  free(fileDirectory);

  if(returnVal < 0) return false;
  else return true;
}
//make xmlDoc out of our GPXdoc. TESTED GOOD.
xmlDocPtr createXMLTree(GPXdoc *GPXdocPtr){
  if(GPXdocPtr == NULL) return NULL;
  xmlDocPtr doc = NULL;
  xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL;/* node pointers */
  //char buff[256];
  xmlNsPtr ns = NULL;
  LIBXML_TEST_VERSION;

  //create new doc and give it XML version
  doc = xmlNewDoc((unsigned char *) "1.0");
  root_node = xmlNewNode(NULL, BAD_CAST "gpx");
  //block to make GPX the parent node + give it properties
  ns = xmlNewNs(root_node, BAD_CAST "http://www.topografix.com/GPX/1/1", NULL);
  xmlSetNs(root_node, ns);
  //xmlNewNsProp(root_node, ns, BAD_CAST "xmlns", BAD_CAST BAD_CAST "http://www.topografix.com/GPX/1/1");

  xmlDocSetRootElement(doc, root_node);

  char *floatString = malloc(10);
  sprintf(floatString, "%.1f", (float)GPXdocPtr->version);
  xmlNewProp(root_node, BAD_CAST "version", BAD_CAST floatString);
  xmlNewProp(root_node, BAD_CAST "creator", BAD_CAST GPXdocPtr->creator);
  free(floatString);
  //xmlNewNode(ns, BAD_CAST "xmlns");
  //setting up traversal of the wpts
  List * waypointList = (List *)GPXdocPtr->waypoints;
  Node * waypointNode = (Node *)waypointList->head;
  //block for putting waypoints into the tree
  while(waypointList->head != NULL){
    Waypoint *wpt = (Waypoint *)waypointNode->data;
    //make wpt node
    node = xmlNewChild(root_node, NULL, BAD_CAST "wpt", BAD_CAST "");
    floatString = malloc(12);
    sprintf(floatString, "%.6f", (float)wpt->latitude);
    //give it latitude
    xmlNewProp(node, BAD_CAST "lat", BAD_CAST floatString);
    free(floatString);
    floatString = malloc(12);
    sprintf(floatString, "%.6f", (float)wpt->longitude);
    //give it longitude
    xmlNewProp(node, BAD_CAST "lon", BAD_CAST floatString);
    free(floatString);

    if(strlen(wpt->name) > 0){
      xmlNewChild(node, NULL, BAD_CAST "name", strlen(wpt->name) > 0 ? BAD_CAST wpt->name : BAD_CAST "");
    }

    //setting up traversal of wpts attributes
    List * attributeList = (List *)wpt->attributes;
    Node * attributeNode = (Node *)attributeList->head;

    while(attributeList->head != NULL){
      Attribute *wptAttr = (Attribute *)attributeNode->data;
      //make the node with attribute name & value
      xmlNewChild(node, NULL, strlen(wptAttr->name) > 0 ? BAD_CAST wptAttr->name : BAD_CAST "", strlen(wptAttr->value) > 0 ? BAD_CAST wptAttr->value : BAD_CAST "");
      //loop through attribute nodes
      if(attributeNode->next != NULL){
        attributeNode = attributeNode->next;
      }
      else{
        break;
      }
    }
    //loop through the WPT nodes
    if(waypointNode->next != NULL){
      waypointNode = waypointNode->next;
    }
    else{
      break;
    }
  }
  //At this point, waypoints are added

  //setting up traversal of routes
  List * routeList = (List *)GPXdocPtr->routes;
  Node * routeNode = (Node *)routeList->head;
  //loop to get routes
  while(routeList->head != NULL){
    Route *rte = (Route *)routeNode->data;
    node = xmlNewChild(root_node, NULL, BAD_CAST "rte", BAD_CAST "");

    if(strlen(rte->name) > 0){
      xmlNewChild(node, NULL, BAD_CAST "name", strlen(rte->name) > 0 ? BAD_CAST rte->name : BAD_CAST "");
    }
    List *attributeList = (List *)rte->attributes;
    Node *attributeNode = (Node *)attributeList->head;
    //attribute loop for route
    while(attributeList->head != NULL){
      Attribute *rteAttr = (Attribute *)attributeNode->data;
      //make the node with attribute name & value
      xmlNewChild(node, NULL, strlen(rteAttr->name) > 0 ? BAD_CAST rteAttr->name : BAD_CAST "", strlen(rteAttr->value) > 0 ? BAD_CAST rteAttr->value : BAD_CAST "");

      //loop through route attribute nodes
      if(attributeNode->next != NULL){
        attributeNode = attributeNode->next;
      }
      else{
        break;
      }
    }
    //setting up traversal of route waypoints AKA route points
    List *rteWaypointList = (List *)rte->waypoints;
    Node *rteWaypointNode = (Node *)rteWaypointList->head;
    //block to traverse route points
    while(rteWaypointList->head != NULL){
      Waypoint *rteWaypoint = (Waypoint *)rteWaypointNode->data;
      node1 = xmlNewChild(node, NULL, BAD_CAST "rtept", BAD_CAST "");
      floatString = malloc(12);
      sprintf(floatString, "%.6f", (float)rteWaypoint->latitude);
      //give it latitude
      xmlNewProp(node1, BAD_CAST "lat", BAD_CAST floatString);
      free(floatString);
      floatString = malloc(12);
      sprintf(floatString, "%.6f", (float)rteWaypoint->longitude);
      //give it longitude
      xmlNewProp(node1, BAD_CAST "lon", BAD_CAST floatString);
      free(floatString);

      if(strlen(rteWaypoint->name) > 0){
        xmlNewChild(node1, NULL, BAD_CAST "name", strlen(rteWaypoint->name) > 0 ? BAD_CAST rteWaypoint->name : BAD_CAST "");
      }
      List * rteWptAttributeList = (List *)rteWaypoint->attributes;
      Node * rteWptAttributeNode = (Node *)rteWptAttributeList->head;
      //loop for route's waypoints' attributes
      while(rteWptAttributeList->head != NULL){
        Attribute *rteWptAttr = (Attribute *)rteWptAttributeNode->data;
        //make the node with attribute name & value
        xmlNewChild(node1, NULL, strlen(rteWptAttr->name) > 0 ? BAD_CAST rteWptAttr->name : BAD_CAST "", strlen(rteWptAttr->value) > 0 ? BAD_CAST rteWptAttr->value : BAD_CAST "");

        //loop through route waypoints attribute nodes
        if(rteWptAttributeNode->next != NULL){
          rteWptAttributeNode = rteWptAttributeNode->next;
        }
        else{
          break;
        }
      }
      //loop through route waypoints
      if(rteWaypointNode->next != NULL){
        rteWaypointNode = rteWaypointNode->next;
      }
      else{
        break;
      }
    }

    //loop through the route nodes
    if(routeNode->next != NULL){
      routeNode = routeNode->next;
    }
    else{
      break;
    }
  }
  //end of route

  //setting up tracks itterator
  List * trackList = (List *)GPXdocPtr->tracks;
  Node * trackNode = (Node *)trackList->head;
  //get waypoint, then waypoint attributes toString
  while(trackList->head != NULL){
    Track *trk = (Track *)trackNode->data;
    node = xmlNewChild(root_node, NULL, BAD_CAST "trk", BAD_CAST "");
    if(strlen(trk->name) > 0){
      xmlNewChild(node, NULL, BAD_CAST "name", strlen(trk->name) > 0 ? BAD_CAST trk->name : BAD_CAST "");
    }

    List * attributeList = (List *)trk->attributes;
    Node * attributeNode = (Node *)attributeList->head;
    while(attributeList->head != NULL){
      Attribute *trkAttr = (Attribute *)attributeNode->data;
      //make the node with attribute name & value
      xmlNewChild(node, NULL, strlen(trkAttr->name) > 0 ? BAD_CAST trkAttr->name : BAD_CAST "", strlen(trkAttr->value) > 0 ? BAD_CAST trkAttr->value : BAD_CAST "");

      //loop through the nodes
      if(attributeNode->next != NULL){
        attributeNode = attributeNode->next;
      }
      else{
        break;
      }
    }

    List * trackSegmentsList = (List *)trk->segments;
    Node * trackSegmentsNode = (Node *)trackSegmentsList->head;
    while(trackSegmentsList->head != NULL){
      TrackSegment *trkSeg = (TrackSegment *)trackSegmentsNode->data;
      node1 = xmlNewChild(node, NULL, BAD_CAST "trkseg", BAD_CAST "");
      List * waypointList = (List *)trkSeg->waypoints;
      Node * waypointNode = (Node *)waypointList->head;
      //block for putting waypoints into the tree
      while(waypointList->head != NULL){
        Waypoint *trkWpt = (Waypoint *)waypointNode->data;
        //make wpt node
        node2 = xmlNewChild(node1, NULL, BAD_CAST "trkpt", BAD_CAST "");
        floatString = malloc(12);
        sprintf(floatString, "%.6f", (float)trkWpt->latitude);
        //give it latitude
        xmlNewProp(node2, BAD_CAST "lat", BAD_CAST floatString);
        free(floatString);
        floatString = malloc(12);
        sprintf(floatString, "%.6f", (float)trkWpt->longitude);
        //give it longitude
        xmlNewProp(node2, BAD_CAST "lon", BAD_CAST floatString);
        free(floatString);

        if(strlen(trkWpt->name) > 0){
          xmlNewChild(node2, NULL, BAD_CAST "name", strlen(trkWpt->name) > 0 ? BAD_CAST trkWpt->name : BAD_CAST "");
        }
        //setting up traversal of wpts attributes
        List * attributeList = (List *)trkWpt->attributes;
        Node * attributeNode = (Node *)attributeList->head;

        while(attributeList->head != NULL){
          Attribute *trkWptAttr = (Attribute *)attributeNode->data;
          //make the node with attribute name & value
          xmlNewChild(node2, NULL, strlen(trkWptAttr->name) > 0 ? BAD_CAST trkWptAttr->name : BAD_CAST "", strlen(trkWptAttr->value) > 0 ? BAD_CAST trkWptAttr->value : BAD_CAST "");
          //loop through attribute nodes
          if(attributeNode->next != NULL){
            attributeNode = attributeNode->next;
          }
          else{
            break;
          }
        }
        //loop through the WPT nodes
        if(waypointNode->next != NULL){
          waypointNode = waypointNode->next;
        }
        else{
          break;
        }
      }

      //loop through the trackSEg nodes
      if(trackSegmentsNode->next != NULL){
        trackSegmentsNode = trackSegmentsNode->next;
      }
      else{
        break;
      }
    }
    //loop through the nodes
    if(trackNode->next != NULL){
      trackNode = trackNode->next;
    }
    else{
      break;
    }
  }
  //Free the global variables that may have been allocated by the parser.
  xmlCleanupParser();
  //this is to debug memory for regression tests
  xmlMemoryDump();

  return doc;
}
//function to print out the whole gpx doc. It calls waypointToString, routeToString, etc
char* GPXdocToString(GPXdoc* doc){
  char *toStringGPX = malloc(100000);
  char floatString[500];
  strcpy(toStringGPX, "");

  sprintf(floatString, "%.1f", doc->version);
  //get namespace, version and creator
  strcat(toStringGPX, "Namespace: ");
  strcat(toStringGPX, doc->namespace);
  strcat(toStringGPX, "\n");
  strcat(toStringGPX, "Version: ");
  strcat(toStringGPX, floatString);
  strcat(toStringGPX, "\n");
  strcat(toStringGPX, "Creator: ");
  strcat(toStringGPX, doc->creator);
  //strcat(toStringGPX, "\n");

  List * waypointList = (List *)doc->waypoints;
  Node * waypointNode = (Node *)waypointList->head;

  //get waypoint, then waypoint attributes toString
  char *waypointData = NULL;
  char *attributeData = NULL;
  while(waypointList->head != NULL){
    waypointData = waypointToString(waypointNode->data);
    Waypoint *wpt = (Waypoint *)waypointNode->data;
    List * attributeList = (List *)wpt->attributes;
    Node * attributeNode = (Node *)attributeList->head;
    //int length = getLength(wpt->attributes);
    //printf("length = %d\n", length);
    while(attributeList->head != NULL){
      attributeData = attributeToString(attributeNode->data);
      strcat(waypointData, attributeData);
      strcat(waypointData, "\n");
      free(attributeData);
      if(attributeNode->next != NULL){
        attributeNode = attributeNode->next;
      }
      else{
        break;
      }
    }

    strcat(toStringGPX, waypointData);
    //strcat(toStringGPX, "\n");
    free(waypointData);

    //loop through the nodes
    if(waypointNode->next != NULL){
      waypointNode = waypointNode->next;
    }
    else{
      break;
    }
  }

  strcat(toStringGPX, "\n");

  List * routeList = (List *)doc->routes;
  Node * routeNode = (Node *)routeList->head;
  //get route then route attribute toString
  char *routeData = NULL;
  while(routeList->head != NULL){
    routeData = routeToString(routeNode->data);

    Route *rte = (Route *)routeNode->data;
    List *attributeList = (List *)rte->attributes;
    Node *attributeNode = (Node *)attributeList->head;

    //attribute loop goes here
    while(attributeList->head != NULL){
      attributeData = attributeToString(attributeNode->data);
      strcat(routeData, attributeData);
      strcat(routeData, "\n");
      free(attributeData);
      if(attributeNode->next != NULL){
        attributeNode = attributeNode->next;
      }
      else{
        break;
      }
    }

    //adding route waypoints
    //rte = (Route *)routeNode->data; //- dont think we need this line
    List *rteWaypointList = (List *)rte->waypoints;
    Node *rteWaypointNode = (Node *)rteWaypointList->head;
    char *rteWaypointData = NULL;

    while(rteWaypointList->head != NULL){
      rteWaypointData = waypointToString(rteWaypointNode->data);
      //if there are any, print rteWaypoint attributes
      Waypoint *rteWaypoint = (Waypoint *)rteWaypointNode->data;
      List * rteWptAttributeList = (List *)rteWaypoint->attributes;
      Node * rteWptAttributeNode = (Node *)rteWptAttributeList->head;
      char *rteWaypointAttributeData = NULL;

      while(rteWptAttributeList->head != NULL){
        rteWaypointAttributeData = attributeToString(rteWptAttributeNode->data);
        strcat(rteWaypointData, rteWaypointAttributeData);
        //strcat(rteWaypointData, "\n");
        free(rteWaypointAttributeData);
        if(rteWptAttributeNode->next != NULL){
          rteWptAttributeNode = rteWptAttributeNode->next;
        }
        else{
          break;
        }
      }

      strcat(routeData, rteWaypointData);
      strcat(routeData, "\n");
      free(rteWaypointData);

      if(rteWaypointNode->next != NULL){
        rteWaypointNode = rteWaypointNode->next;
      }
      else{
        break;
      }
    }
    //end of route Waypoints


    strcat(toStringGPX, routeData);
    //strcat(toStringGPX, "\n");
    free(routeData);
    //loop through the nodes
    if(routeNode->next != NULL){
      routeNode = routeNode->next;
    }
    else{
      break;
    }
  }

  //ToString part for Tracks
  List * trackList = (List *)doc->tracks;
  Node * trackNode = (Node *)trackList->head;
  //get waypoint, then waypoint attributes toString
  char *trackData = NULL;
  char *trackAttributeData = NULL;
  while(trackList->head != NULL){
    trackData = trackToString(trackNode->data);
    strcat(toStringGPX, trackData);
    //strcat(toStringGPX, "\n");
    free(trackData);

    Track *trk = (Track *)trackNode->data;
    List * attributeList = (List *)trk->attributes;
    Node * attributeNode = (Node *)attributeList->head;
    while(attributeList->head != NULL){
      trackAttributeData = attributeToString(attributeNode->data);

      strcat(toStringGPX, trackAttributeData);
      //strcat(toStringGPX, "\n");
      free(trackAttributeData);

      //loop through the nodes
      if(attributeNode->next != NULL){
        attributeNode = attributeNode->next;
      }
      else{
        break;
      }
    }

    List * trackSegmentsList = (List *)trk->segments;
    Node * trackSegmentsNode = (Node *)trackSegmentsList->head;
    char * trkSegData = NULL;
    while(trackSegmentsList->head != NULL){
      trkSegData = trackSegmentToString(trackSegmentsNode->data);

      strcat(toStringGPX, trkSegData);
      //strcat(toStringGPX, "\n");
      free(trkSegData);

      //loop through the nodes
      if(trackSegmentsNode->next != NULL){
        trackSegmentsNode = trackSegmentsNode->next;
      }
      else{
        break;
      }
    }

    //loop through the nodes
    if(trackNode->next != NULL){
      trackNode = trackNode->next;
    }
    else{
      break;
    }
  }

  strcat(toStringGPX, "\n");


  return toStringGPX;
}
//Frees the GPX doc, freeList will then invoke deleteWaypoint, etc.
void deleteGPXdoc(GPXdoc* doc){
  if(doc == NULL){
    return;
  }
  freeList(doc->waypoints);
  freeList(doc->routes);
  freeList(doc->tracks);
  free(doc->creator);
  free(doc);
}
//get number of waypoints in the doc
int getNumWaypoints(const GPXdoc* doc){
  if(doc == NULL){
    return 0;
  }

  int length = 0;
  length = getLength(doc->waypoints);

  return length;
}
//get number of routes in the doc
int getNumRoutes(const GPXdoc* doc){
  if(doc == NULL){
    return 0;
  }

  int length = 0;
  length = getLength(doc->routes);

  return length;
}
//get number of tracks in the doc
int getNumTracks(const GPXdoc* doc){
  if(doc == NULL){
    return 0;
  }

  int length = 0;
  length = getLength(doc->tracks);

  return length;
}
//get number of track segments in the doc
int getNumSegments(const GPXdoc* doc){
  if(doc == NULL){
    return 0;
  }

  int length = 0;

  List * trackList = (List *)doc->tracks;
  Node * trackNode = (Node *)trackList->head;

  while(trackList->head != NULL){
    Track *trk = (Track *)trackNode->data;

    length += getLength(trk->segments);

    //loop through the nodes
    if(trackNode->next != NULL){
      trackNode = trackNode->next;
    }
    else{
      break;
    }
  }

  return length;
}
//get number of attributes in the doc
int getNumAttributes(const GPXdoc* doc){
  if(doc == NULL){
    return 0;
  }

  int length = 0;
  //Getting number of attributes for all of wpt nodes
  List * waypointList = (List *)doc->waypoints;
  Node * waypointNode = (Node *)waypointList->head;
  while(waypointList->head != NULL){
    Waypoint *wpt = (Waypoint *)waypointNode->data;
    //if there is something in name, increment the attribute count
    if(strlen(wpt->name) > 0){
      length += 1;
    }
    length += getLength(wpt->attributes);
    //loop through the nodes
    if(waypointNode->next != NULL){
      waypointNode = waypointNode->next;
    }
    else{
      break;
    }
  }
  //Getting number of attributes for all of rte nodes
  List * routeList = (List *)doc->routes;
  Node * routeNode = (Node *)routeList->head;
  while(routeList->head != NULL){
    Route *rte = (Route *)routeNode->data;
    //if there is something in name, increment the attribute count
    if(strlen(rte->name) > 0){
      length += 1;
    }
    length += getLength(rte->attributes);

    List *rteWaypointList = (List *)rte->waypoints;
    Node *rteWaypointNode = (Node *)rteWaypointList->head;
    while(rteWaypointList->head != NULL){
      Waypoint *rteWaypoint = (Waypoint *)rteWaypointNode->data;
      //if there is something in name, increment the attribute count
      if(strlen(rteWaypoint->name) > 0){
        length += 1;
      }
      length += getLength(rteWaypoint->attributes);


      if(rteWaypointNode->next != NULL){
        rteWaypointNode = rteWaypointNode->next;
      }
      else{
        break;
      }
    }
    if(routeNode->next != NULL){
      routeNode = routeNode->next;
    }
    else{
      break;
    }
  }
  //Getting number of attributes for all of trk nodes
  List * trackList = (List *)doc->tracks;
  Node * trackNode = (Node *)trackList->head;
  while(trackList->head != NULL){
    Track *trk = (Track *)trackNode->data;
    if(strlen(trk->name) > 0){
      length += 1;
    }
    length += getLength(trk->attributes);

    List * trackSegmentsList = (List *)trk->segments;
    Node * trackSegmentsNode = (Node *)trackSegmentsList->head;
    while(trackSegmentsList->head != NULL){
      TrackSegment *trkSegment = (TrackSegment *)trackSegmentsNode->data;
      List * trkSegmentWaypointsList = (List *)trkSegment->waypoints;
      Node * trkSegmentWaypointsNode = (Node *)trkSegmentWaypointsList->head;
      while(trkSegmentWaypointsList->head != NULL){
        Waypoint *wpt = (Waypoint *)trkSegmentWaypointsNode->data;
        if(strlen(wpt->name) > 0){
          length += 1;
        }
        length += getLength(wpt->attributes);
        //loop through the nodes
        if(trkSegmentWaypointsNode->next != NULL){
          trkSegmentWaypointsNode = trkSegmentWaypointsNode->next;
        }
        else{
          break;
        }
      }

      if(trackSegmentsNode->next != NULL){
        trackSegmentsNode = trackSegmentsNode->next;
      }
      else{
        break;
      }
    }

    //loop through the nodes
    if(trackNode->next != NULL){
      trackNode = trackNode->next;
    }
    else{
      break;
    }
  }

  return length;
}
//return a specific waypoint with name
Waypoint* getWaypoint(const GPXdoc* doc, char* name){
  if(doc == NULL || name == NULL) return NULL;

  List *ittr = (List *)doc->waypoints;
  Node *ittrNode = (Node *)ittr->head;

  while(ittr->head != NULL){
    Waypoint *wpt = (Waypoint *)ittrNode->data;
    if(strcmp(wpt->name, name) == 0){
      return wpt;
    }

    if(ittrNode->next != NULL){
      ittrNode = ittrNode->next;
    }
    else{
      break;
    }
  }

  //If we got here, there is no Waypoint.
  return NULL;
}
//return a specific track with name
Track* getTrack(const GPXdoc* doc, char* name){
  if(doc == NULL || name == NULL) return NULL;

  List *ittr = (List *)doc->tracks;
  Node *ittrNode = (Node *)ittr->head;

  while(ittr->head != NULL){
    Track *trk = (Track *)ittrNode->data;
    if(strcmp(trk->name, name) == 0){
      return trk;
    }

    if(ittrNode->next != NULL){
      ittrNode = ittrNode->next;
    }
    else{
      break;
    }
  }

  //If we got here, there is no Waypoint.
  return NULL;
}
//return a specific route with name
Route* getRoute(const GPXdoc* doc, char* name){
  if(doc == NULL || name == NULL) return NULL;

  List *ittr = (List *)doc->routes;
  Node *ittrNode = (Node *)ittr->head;

  while(ittr->head != NULL){
    Route *rte = (Route *)ittrNode->data;
    if(strcmp(rte->name, name) == 0){
      return rte;
    }


    if(ittrNode->next != NULL){
      ittrNode = ittrNode->next;
    }
    else{
      break;
    }
  }

  //If we got here, there is no Waypoint.
  return NULL;
}
//frees attribute, called from deleteGPXdoc
void deleteAttribute( void* data){
  if(data == NULL){
    return;
  }
  Attribute *attr = (Attribute *)data;
  //printf("DelAttribute %s, %s\n", attr->name, attr->value);
  free(attr->name);
  free(attr->value);
  free(attr);
}
//prints the attribute node
char* attributeToString( void* data){
  char *attributeData = malloc(10000);
  Attribute *attr = (Attribute *)data;
  strcpy(attributeData, "");
  strcat(attributeData, "\n\t\t");
  strcat(attributeData, "Attribute: ");
  //strcat(attributeData, "\t\t\t");
  strcat(attributeData, attr->name);
  strcat(attributeData, "\n");
  strcat(attributeData, "\t\t\t");
  strcat(attributeData, attr->value);
  return attributeData;
}
//didnt use compare functions
int compareAttributes(const void *first, const void *second){

  return 0;
}
//Helper functions
void deleteWaypoint(void* data){
  if(data == NULL){
    return;
  }
  Waypoint *wpt;
  wpt = (Waypoint*)data;

  if (wpt->attributes!=NULL){
    freeList(wpt->attributes);
  }

  free(wpt->name);
  free(wpt);
}
//this will return one node at a time
char* waypointToString( void* data){
  char *waypointData = malloc(10000);
  Waypoint *wpt = (Waypoint *)data;
  char floatString[50];
  strcpy(waypointData, "");
  strcat(waypointData, "\nWaypoints:");


  //get waypoint node name
  strcat(waypointData, "\t");
  strcat(waypointData, wpt->name);
  strcat(waypointData, "\n");
  //get  waypoint node lat and lon
  strcat(waypointData, "\t\t");
  strcat(waypointData, "Longitude ");
  sprintf(floatString, "%.6f", wpt->longitude);
  strcat(waypointData, floatString);
  strcat(waypointData, "\n");
  strcat(waypointData, "\t\t");
  strcat(waypointData, "Latitude ");
  sprintf(floatString, "%.6f", wpt->latitude);
  strcat(waypointData, floatString);

  return waypointData;
}
//didnt use compare functions
int compareWaypoints(const void *first, const void *second){

  return 0;
}
//frees route
void deleteRoute(void* data){
  if(data == NULL){
    return;
  }
  Route *rte;
  rte = (Route *)data;

  if(rte->attributes != NULL){
    freeList(rte->attributes);
  }

  if(rte->waypoints != NULL){
    freeList(rte->waypoints);
  }

  free(rte->name);
  free(rte);
}
//prints route
char* routeToString(void* data){
  char *routeData = malloc(10000);
  Route *rte = (Route *)data;

  strcpy(routeData, "");
  strcat(routeData, "\nRoute:");

  strcat(routeData, "\t");
  strcat(routeData, rte->name);
  //strcat(routeData, "\n");

  return routeData;
}
//didnt use compare functions
int compareRoutes(const void *first, const void *second){

  return 0;
}
//free track segments struct
void deleteTrackSegment(void* data){
  if(data == NULL){
    return;
  }
  TrackSegment *trkSeg;
  trkSeg = (TrackSegment *)data;

  if(trkSeg->waypoints!=NULL){
    freeList(trkSeg->waypoints);
  }

  free(trkSeg);
}
//prints the track seg
char* trackSegmentToString(void* data){
  char *trkSegData = malloc(10000);
  strcpy(trkSegData, "");

  TrackSegment *trkSegment = (TrackSegment *)data;
  List * trkSegmentWaypointsList = (List *)trkSegment->waypoints;
  Node * trkSegmentWaypointsNode = (Node *)trkSegmentWaypointsList->head;
  char * trkSegWptData = NULL;
  while(trkSegmentWaypointsList->head != NULL){
    trkSegWptData = waypointToString(trkSegmentWaypointsNode->data);

    strcat(trkSegData, trkSegWptData);
    //strcat(toStringGPX, "\n");
    free(trkSegWptData);

    Waypoint *wpt = (Waypoint *)trkSegmentWaypointsNode->data;
    List * attributeList = (List *)wpt->attributes;
    Node * attributeNode = (Node *)attributeList->head;
    char *attributeData = NULL;
    while(attributeList->head != NULL){
      attributeData = attributeToString(attributeNode->data);
      strcat(trkSegData, attributeData);
      //strcat(trkSegData, "\n");
      free(attributeData);
      if(attributeNode->next != NULL){
        attributeNode = attributeNode->next;
      }
      else{
        break;
      }
    }

    //loop through the nodes
    if(trkSegmentWaypointsNode->next != NULL){
      trkSegmentWaypointsNode = trkSegmentWaypointsNode->next;
    }
    else{
      break;
    }
  }

  return trkSegData; //trkSegData
}
//didnt use compare functions
int compareTrackSegments(const void *first, const void *second){

  return 0;
}
//frees the track struct
void deleteTrack(void* data){
  if(data == NULL){
    return;
  }
  Track *trk;
  trk = (Track *)data;

  if(trk->attributes != NULL){
    freeList(trk->attributes);
  }

  if(trk->segments != NULL){
    freeList(trk->segments);
  }

  free(trk->name);
  free(trk);
}
//prints the track
char* trackToString(void* data){
  char *trackData = malloc(1000);
  Track *trk = (Track *)data;

  strcpy(trackData, "");
  strcat(trackData, "\nTrack:");

  strcat(trackData, "\t");
  strcat(trackData, trk->name);
  return trackData;
}
//didnt use compare functions
int compareTracks(const void *first, const void *second){

  return 0;
}
//Start of A2 Module 2/3 functions

//Function to calculate haversine. Based this off of geeksforgeeks.com example
float calcHaversine(float lat1, float lon1, float lat2, float lon2){
  float dLat = (lat2 - lat1) * PI / 180.0;
  float dLon = (lon2 - lon1) * PI / 180.0;
  // convert to radians
  lat1 = (lat1) * PI / 180.0;
  lat2 = (lat2) * PI / 180.0;
  // apply formulae
  float a = pow(sin(dLat / 2), 2) + pow(sin(dLon / 2), 2) * cos(lat1) * cos(lat2);
  float rad = 6371;
  float c = 2 * asin(sqrt(a));
  return rad * c * 1000;
}
//Function to get the total route length, given the route struct pointer. TESTED GOOD.
float getRouteLen(const Route *rt){
  if(rt == NULL) return 0.0;

  float totalLength = 0.0;
  float lat1 = 0.0, lat2 = 0.0;
  float lon1 = 0.0, lon2 = 0.0;

  List *rteWaypointList = (List *)rt->waypoints;
  Node *rteWaypointNode = (Node *)rteWaypointList->head;
  while(rteWaypointList->head != NULL){
    if(rteWaypointNode->next != NULL){
      Waypoint *rteWaypoint = (Waypoint *)rteWaypointNode->data;
      Waypoint *rteWaypoint2 = (Waypoint *)rteWaypointNode->next->data;

      lat1 = rteWaypoint->latitude;
      lon1 = rteWaypoint->longitude;
      lat2 = rteWaypoint2->latitude;
      lon2 = rteWaypoint2->longitude;

      totalLength += calcHaversine(lat1, lon1, lat2, lon2);

      rteWaypointNode = rteWaypointNode->next;
    }
    else{
      break;
    }
  }

  return totalLength;
}
//Function to get the total track length, given the track struct pointer TESTED GOOD.
float getTrackLen(const Track *tr){
  if(tr == NULL) return 0.0;
  int length = 0;
  float totalLength = 0.0;

  List *trackSegmentsList = (List *)tr->segments;
  Node *trackSegmentsNode = (Node *)trackSegmentsList->head;

  while(trackSegmentsList->head != NULL){
    //if there is more than one track Seg
    TrackSegment *trkSeg = (TrackSegment *)trackSegmentsNode->data;
    length += getLength(trkSeg->waypoints);
    if (trackSegmentsNode->next==NULL){
      break;
    }
    trackSegmentsNode = trackSegmentsNode->next;
  }

  float lat[length], lon[length];
  int i = 0;
  int j = 0;
  List *trackSegmentsList_2 = (List *)tr->segments;
  Node *trackSegmentsNode_2 = (Node *)trackSegmentsList_2->head;
  while(trackSegmentsList_2->head != NULL){
    //if there is more than one track Seg
    TrackSegment *trkSeg = (TrackSegment *)trackSegmentsNode_2->data;
    List *trkSegWptList = (List*)trkSeg->waypoints;
    Node *trkSegWptNode = (Node*)trkSegWptList->head;
    while(trkSegWptList->head != NULL){
      Waypoint *wpt = (Waypoint*) trkSegWptNode->data;
      lat[i] = wpt->latitude;
      lon[i] = wpt->longitude;
      i++;
      if(trkSegWptNode->next == NULL){
        break;
      }
      trkSegWptNode = trkSegWptNode->next;
    }
    if(trackSegmentsNode_2->next == NULL){
      break;
    }
    trackSegmentsNode_2 = trackSegmentsNode_2->next;
  }
  for(j = 0; j < length - 1; j++){
    totalLength += calcHaversine(lat[j], lon[j], lat[j+1], lon[j+1]);
  }

  return totalLength;
}
//function to round a float to nearest 10. TESTED GOOD.
float round10(float len){
  int a = (len / 10);

  char *string = malloc(10);
  sprintf(string, "%.1f", len);
  if(string[strlen(string) - 1 == 48] && string[strlen(string) - 3] == 53){
    a += 1;
  }
  free(string);

  a = a * 10;
  int b = a + 10;

  if(len - a > b - len){
    return (float)b;
  }
  else{
    return (float)a;
  }
}
//returns the # of routes that have a specified length, within the delta. TESTED GOOD
int numRoutesWithLength(const GPXdoc* doc, float len, float delta){
  if(doc == NULL || len < 0 || delta < 0) return 0;

  float routeLength = 0;
  int numRoutesWL = 0;

  List * routeList = (List *)doc->routes;
  Node * routeNode = (Node *)routeList->head;

  while(routeList->head != NULL){
    Route *rte = (Route *)routeNode->data;

    routeLength = getRouteLen(rte);
    if( (routeLength >= len - delta) && (routeLength <= len + delta) ){
      numRoutesWL++;
    }

    if(routeNode->next != NULL){
      routeNode = routeNode->next;
    }
    else{
      break;
    }
  }

  return numRoutesWL;
}
//returns the # of tracks that have a specified length, within the delta. TESTED GOOD
int numTracksWithLength(const GPXdoc* doc, float len, float delta){
  if(doc == NULL || len < 0 || delta < 0) return 0;

  float trackLength = 0;
  int numTracksWL = 0;

  List * trackList = (List *)doc->tracks;
  Node * trackNode = (Node *)trackList->head;

  while(trackList->head != NULL){
    Track *trk = (Track *)trackNode->data;

    trackLength = getTrackLen(trk);
    if( (trackLength >= len - delta) && (trackLength <= len + delta) ){
      numTracksWL++;
    }

    if(trackNode->next != NULL){
      trackNode = trackNode->next;
    }
    else{
      break;
    }
  }

  return numTracksWL;
}
//Checks if the given route has a loop or not. TESTED GOOD
bool isLoopRoute(const Route* route, float delta){
  if(route == NULL || delta < 0) return 0;

  float lat1 = 0.0, lon1 = 0.0;
  float lat2 = 0.0, lon2 = 0.0;
  int i = 0;
  float distance = 0.0;

  List *rteWaypointList = (List *)route->waypoints;
  Node *rteWaypointNode = (Node *)rteWaypointList->head;

  while(rteWaypointList->head != NULL){
    Waypoint *rteWaypoint = (Waypoint *)rteWaypointNode->data;

    if(i == 0){
      lat1 = rteWaypoint->latitude;
      lon1 = rteWaypoint->longitude;
    }

    if(rteWaypointNode->next != NULL){
      i++;
      rteWaypointNode = rteWaypointNode->next;
    }
    else{
      lat2 = rteWaypoint->latitude;
      lon2 = rteWaypoint->longitude;
      break;
    }
  }
  if(lat1 != 0.0 && lon1 != 0.0 && lat2 != 0.0 && lon2 != 0.0 && i >= 3){
    distance = calcHaversine(lat1, lon1, lat2, lon2);
  }
  else{
    return 0;
  }

  if(distance < delta){
    return 1;
  }
  else{
    return 0;
  }
}
//Checks if the given track has a loop or not. TESTED GOOD
bool isLoopTrack(const Track *tr, float delta){
  if(tr == NULL || delta < 0) return 0;

  float lat1 = 0.0, lon1 = 0.0;
  float lat2 = 0.0, lon2 = 0.0;
  int i = 0;
  float distance = 0.0;

  List *trackSegmentsList = (List *)tr->segments;
  Node *trackSegmentsNode = (Node *)trackSegmentsList->head;

  while(trackSegmentsList->head != NULL){
    TrackSegment *trkSeg = trackSegmentsNode->data;
    List *trkSegWptList = (List *)trkSeg->waypoints;
    Node *trkSegWptNode = (Node *)trkSegWptList->head;

    while(trkSegWptList->head != NULL){
      Waypoint *wpt = (Waypoint *)trkSegWptNode->data;
      if(i == 0){
        lat1 = wpt->latitude;
        lon1 = wpt->longitude;
      }

      if(trkSegWptNode->next != NULL){
        i++;
        trkSegWptNode = trkSegWptNode->next;
      }
      //this means we are on the last waypoint of current track seg
      else{
        //we need to check if there are anymore track segs
        //if none, its the last point. get lon and lat
        if(trackSegmentsNode->next == NULL){
          lat2 = wpt->latitude;
          lon2 = wpt->longitude;
        }
        break;
      }
    }

    if(trackSegmentsNode->next != NULL){
      trackSegmentsNode = trackSegmentsNode->next;
    }
    else{
      break;
    }
  }

  if(lat1 != 0.0 && lon1 != 0.0 && lat2 != 0.0 && lon2 != 0.0 && i >= 3){
    distance = calcHaversine(lat1, lon1, lat2, lon2);
  }
  else{
    return 0;
  }

  if(distance < delta){
    return 1;
  }
  else{
    return 0;
  }
}
//returns all routes that are between the given starting and ending points. TESTED GOOD
List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
  if(doc == NULL || delta < 0) return NULL;

  float distance1 = 0.0, distance2 = 0.0;
  int i = 0;

  // GPXdoc *GPXdocPtr = malloc(sizeof(GPXdoc));
  // GPXdocPtr->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
  List *getList = initializeList(&routeToString, &deleteRoute, &compareRoutes);

  List * routeList = (List *)doc->routes;
  Node * routeNode = (Node *)routeList->head;
  //get route then route attribute toString
  while(routeList->head != NULL){
    Route *rte = (Route *)routeNode->data;
    i = 0;
    List *rteWaypointList = (List *)rte->waypoints;
    Node *rteWaypointNode = (Node *)rteWaypointList->head;
    while(rteWaypointList->head != NULL){
      Waypoint *rteWaypoint = (Waypoint *)rteWaypointNode->data;
      if(i == 0){
        distance1 = calcHaversine(sourceLat, sourceLong, rteWaypoint->latitude, rteWaypoint->longitude);
      }
      if(rteWaypointNode->next != NULL){
        i++;
        rteWaypointNode = rteWaypointNode->next;
      }
      else{
        distance2 = calcHaversine(destLat, destLong, rteWaypoint->latitude, rteWaypoint->longitude);
        break;
      }
    }

    if(distance1 < delta && distance2 < delta){
      //Do deep copy and add it to the list of routes
      Route *newRte = malloc(sizeof(Route));
      newRte->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
      newRte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
      //gets name of existing route and puts it in the new route
      newRte->name = malloc(strlen(rte->name) + 1);
      strcpy(newRte->name, rte->name);

      List * attrList = (List *)rte->attributes;
      Node * attrNode = (Node *)attrList->head;
      while(attrList->head != NULL){
        Attribute *attr = malloc(sizeof(Attribute));
        Attribute *oldAttr = (Attribute *)attrNode->data;
        attr->name = malloc(strlen(oldAttr->name) + 1);
        attr->value = malloc(strlen(oldAttr->value) + 1);
        strcpy(attr->name, oldAttr->name);
        strcpy(attr->value, oldAttr->value);

        insertBack(newRte->attributes, attr);
        if(attrNode->next != NULL){
          attrNode = attrNode->next;
        }
        else{
          break;
        }
      }

      List * wptList = (List *)rte->waypoints;
      Node * wptNode = (Node *)wptList->head;
      while(wptList->head != NULL){
        Waypoint *wpt = malloc(sizeof(Waypoint));
        wpt->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

        Waypoint *oldWpt = (Waypoint *)wptNode->data;
        wpt->name = malloc(strlen(oldWpt->name) + 1);
        strcpy(wpt->name, oldWpt->name);
        wpt->longitude = oldWpt->longitude;
        wpt->latitude = oldWpt->latitude;

        List * wptAttrList = (List *)oldWpt->attributes;
        Node * wptAttrNode = (Node *)wptAttrList->head;
        while(wptAttrList->head != NULL){
          Attribute *wptAttr = malloc(sizeof(Attribute));
          Attribute *oldWptAttr = (Attribute *)wptAttrNode->data;
          wptAttr->name = malloc(strlen(oldWptAttr->name) + 1);
          wptAttr->value = malloc(strlen(oldWptAttr->value) + 1);
          strcpy(wptAttr->name, oldWptAttr->name);
          strcpy(wptAttr->value, oldWptAttr->value);

          insertBack(wpt->attributes, wptAttr);
          if(wptAttrNode->next != NULL){
            wptAttrNode = wptAttrNode->next;
          }
          else{
            break;
          }
        }

        insertBack(newRte->waypoints, wpt);
        if(wptNode->next != NULL){
          wptNode = wptNode->next;
        }
        else{
          break;
        }
      }

      insertBack(getList, newRte);
    }

    if(routeNode->next != NULL){
      routeNode = routeNode->next;
    }
    else{
      break;
    }
  }

  if(getLength(getList) < 1){
    freeList(getList);
    return NULL;
  }
  else{
    return getList;
  }
}
//returns all tracks that are between the given starting and ending points. TESTED GOOD
List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
  if(doc == NULL || delta < 0) return NULL;

  float distance1 = 0.0, distance2 = 0.0;
  int i = 0, j = 0;
  int length = 0;
  List *getList = initializeList(&trackToString, &deleteTrack, &compareTracks);

  List * trackList = (List *)doc->tracks;
  Node * trackNode = (Node *)trackList->head;
  //get route then route attribute toString
  while(trackList->head != NULL){
    Track *trk = (Track *)trackNode->data;
    length = getLength(trk->segments);
    i = 0;
    j = 0;
    List *trackSegList = (List *)trk->segments;
    Node *trackSegNode = (Node *)trackSegList->head;
    while(trackSegList->head != NULL){
      TrackSegment *trkSeg = (TrackSegment *)trackSegNode->data;
      List *trackSegWptList = (List *)trkSeg->waypoints;
      Node *trackSegWptNode = (Node *)trackSegWptList->head;
      j = 1;
      while(trackSegWptList->head != NULL){
        Waypoint *trkSegWaypoint = (Waypoint *)trackSegWptNode->data;
        if(i == 0){
          distance1 = calcHaversine(sourceLat, sourceLong, trkSegWaypoint->latitude, trkSegWaypoint->longitude);
        }
        if(trackSegWptNode->next != NULL){
          i++;
          trackSegWptNode = trackSegWptNode->next;
        }
        else{
          if(length == 1 || j == length){
            distance2 = calcHaversine(destLat, destLong, trkSegWaypoint->latitude, trkSegWaypoint->longitude);
          }
          break;
        }
      }
      if(trackSegNode->next != NULL){
        trackSegNode = trackSegNode->next;
      }
      else{
        break;
      }
    }

    if(distance1 < delta && distance2 < delta){
      //Do deep copy and add it to the list of tracks
      Track *newTrk = malloc(sizeof(Track));
      newTrk->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
      newTrk->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
      newTrk->name = malloc(strlen(trk->name) + 1);
      strcpy(newTrk->name, trk->name);

      List * attrList = (List *)trk->attributes;
      Node * attrNode = (Node *)attrList->head;
      while(attrList->head != NULL){
        Attribute *attr = malloc(sizeof(Attribute));
        Attribute *oldAttr = (Attribute *)attrNode->data;
        attr->name = malloc(strlen(oldAttr->name) + 1);
        attr->value = malloc(strlen(oldAttr->value) + 1);
        strcpy(attr->name, oldAttr->name);
        strcpy(attr->value, oldAttr->value);

        insertBack(newTrk->attributes, attr);
        if(attrNode->next != NULL){
          attrNode = attrNode->next;
        }
        else{
          break;
        }
      }

      List * trkSegList = (List *)trk->segments;
      Node * trkSegNode = (Node *)trkSegList->head;
      while(trkSegList->head != NULL){
        TrackSegment *trkSeg = malloc(sizeof(TrackSegment));
        trkSeg->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

        TrackSegment *oldTrkSeg = (TrackSegment *)trkSegNode->data;
        List * wptList = (List *)oldTrkSeg->waypoints;
        Node * wptNode = (Node *)wptList->head;
        while(wptList->head != NULL){
          Waypoint *wpt = malloc(sizeof(Waypoint));
          wpt->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

          Waypoint *oldWpt = (Waypoint *)wptNode->data;
          wpt->name = malloc(strlen(oldWpt->name) + 1);
          strcpy(wpt->name, oldWpt->name);
          wpt->longitude = oldWpt->longitude;
          wpt->latitude = oldWpt->latitude;

          List * wptAttrList = (List *)oldWpt->attributes;
          Node * wptAttrNode = (Node *)wptAttrList->head;
          while(wptAttrList->head != NULL){
            Attribute *wptAttr = malloc(sizeof(Attribute));
            Attribute *oldWptAttr = (Attribute *)wptAttrNode->data;
            wptAttr->name = malloc(strlen(oldWptAttr->name) + 1);
            wptAttr->value = malloc(strlen(oldWptAttr->value) + 1);
            strcpy(wptAttr->name, oldWptAttr->name);
            strcpy(wptAttr->value, oldWptAttr->value);

            insertBack(wpt->attributes, wptAttr);
            if(wptAttrNode->next != NULL){
              wptAttrNode = wptAttrNode->next;
            }
            else{
              break;
            }
          }

          insertBack(trkSeg->waypoints, wpt);
          if(wptNode->next != NULL){
            wptNode = wptNode->next;
          }
          else{
            break;
          }
        }

        insertBack(newTrk->segments, trkSeg);
        if(trkSegNode->next != NULL){
          trkSegNode = trkSegNode->next;
        }
        else{
          break;
        }
      }

      insertBack(getList, newTrk);
    }

    if(trackNode->next != NULL){
      trackNode = trackNode->next;
    }
    else{
      break;
    }
  }

  if(getLength(getList) < 1){
    freeList(getList);
    return NULL;
  }
  else{
    return getList;
  }
}

char* trackToJSON(const Track *tr){
  char *string = malloc(1000);

  if(tr == NULL){
    strcpy(string, "{}");
    return string;
  }
  else{
    strcpy(string, "{");
  }

  strcat(string, "\"name\":\"");
  if(strlen(tr->name) > 0){
    strcat(string, tr->name);
    strcat(string, "\",");
  }
  else{
    strcat(string, "None\",");
  }

  strcat(string, "\"len\":");
  float trLength = getTrackLen(tr);
  trLength = round10(trLength);
  char *lengthStr = malloc(15);
  sprintf(lengthStr, "%.1f", trLength);
  strcat(string, lengthStr);
  strcat(string, ",");
  free(lengthStr);

  int tf = 0;
  strcat(string, "\"loop\":");
  tf = isLoopTrack(tr, 10);
  if(tf == 1){
    strcat(string, "true}\0");
  }
  else{
    strcat(string, "false}\0");
  }

  return string;
}
//Should we null terminate these strings?
char* routeToJSON(const Route *rt){
  char *string = malloc(1000);
  int points = 0;

  if(rt == NULL){
    strcpy(string, "{}");
    return string;
  }
  else{
    strcpy(string, "{");
  }

  strcat(string, "\"name\":\"");
  if(strlen(rt->name) > 0){
    strcat(string, rt->name);
    strcat(string, "\",");
  }
  else{
    strcat(string, "None\",");
  }

  char *pointStr = malloc(5);
  strcat(string, "\"numPoints\":");
  points = getLength(rt->waypoints);
  sprintf(pointStr, "%d", points);
  strcat(string, pointStr);
  strcat(string, ",");
  free(pointStr);

  strcat(string, "\"len\":");
  float rtLength = getRouteLen(rt);
  rtLength = round10(rtLength);
  char *lengthStr = malloc(15);
  sprintf(lengthStr, "%.1f", rtLength);
  strcat(string, lengthStr);
  strcat(string, ",");
  free(lengthStr);

  int tf = 0;
  strcat(string, "\"loop\":");
  tf = isLoopRoute(rt, 10);
  if(tf == 1){
    strcat(string, "true}\0");
  }
  else{
    strcat(string, "false}\0");
  }

  return string;
}

char* routeListToJSON(const List *list){
  if(list == NULL) return NULL;
  char *string = malloc(2);
  strcpy(string, "[");

  Node * routeNode = (Node *)list->head;
  //get route then route attribute toString
  while(list->head != NULL){
    char *tempString = routeToJSON(routeNode->data);
    string = realloc(string, strlen(string) + strlen(tempString) + 5);
    strcat(string, tempString);
    free(tempString);

    if(routeNode->next != NULL){
      string = realloc(string, strlen(string) + 5);
      strcat(string, ",");
      routeNode = routeNode->next;
    }
    else{
      break;
    }
  }

  string = realloc(string, strlen(string) + 5);
  strcat(string, "]");

  return string;
}

char* trackListToJSON(const List *list){
  if(list == NULL) return NULL;
  char *string = malloc(3);
  strcpy(string, "[");
  printf("%s\n",string);
  Node * trackNode = (Node *)list->head;
  //get route then route attribute toString
  while(list->head != NULL){
    char *tempString = trackToJSON(trackNode->data);
    string = realloc(string, strlen(string) + strlen(tempString) + 5);
    strcat(string, tempString);
    free(tempString);

    if(trackNode->next != NULL){
      string = realloc(string, strlen(string) + 5);
      strcat(string, ",");
      trackNode = trackNode->next;
    }
    else{
      break;
    }
  }

  string = realloc(string, strlen(string) + 5);
  strcat(string, "]");

  return string;
}

char* GPXtoJSON(const GPXdoc* gpx){
  char *string = malloc(1000);

  if(gpx == NULL){
    strcpy(string, "{}");
    return string;
  }
  else{
    strcpy(string, "{");
  }

  char *versionStr = malloc(5);
  strcat(string, "\"version\":");
  sprintf(versionStr, "%.1f", gpx->version);
  strcat(string, versionStr);
  strcat(string, ",");
  free(versionStr);

  strcat(string, "\"creator\":\"");
  strcat(string, gpx->creator);
  strcat(string, "\",");

  char *wptString = malloc(5);
  int numWaypoints = getNumWaypoints(gpx);
  strcat(string, "\"numWaypoints\":");
  sprintf(wptString, "%d", numWaypoints);
  strcat(string, wptString);
  strcat(string, ",");
  free(wptString);

  char *rteString = malloc(5);
  int numRoutes = getNumRoutes(gpx);
  strcat(string, "\"numRoutes\":");
  sprintf(rteString, "%d", numRoutes);
  strcat(string, rteString);
  strcat(string, ",");
  free(rteString);

  char *trkString = malloc(5);
  int numTracks = getNumTracks(gpx);
  strcat(string, "\"numTracks\":");
  sprintf(trkString, "%d", numTracks);
  strcat(string, trkString);
  strcat(string, "}");
  free(trkString);

  return string;
}
//Bouns functions, should all work correctly
void addWaypoint(Route *rt, Waypoint *pt){
  if(rt != NULL && pt != NULL){
    insertBack(rt->waypoints, pt);
  }
}

void addRoute(GPXdoc* doc, Route* rt){
  if(doc != NULL && rt != NULL){
    insertBack(doc->routes, rt);
  }
}
//ran with val, no leaks
GPXdoc* JSONtoGPX(const char* gpxString){
  if(gpxString == NULL) return NULL;

  int i = 0;
  int j = 0;
  GPXdoc *newDoc = malloc(sizeof(GPXdoc));
  newDoc->creator = malloc(1);
  strcpy(newDoc->creator, "");
  newDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
  newDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
  newDoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

  int m = 0;

  for(i = 0; i < strlen(gpxString); i++){
    if(gpxString[i] == 'v' && gpxString[i+1] == 'e'){
      char *valueString = (char *)malloc(10);
      strcpy(valueString, "");
      j = i + 9;
      while(isdigit(gpxString[j]) || gpxString[j] == '.'){
        if(gpxString[j] == ',') break;
        valueString[m] = gpxString[j];

        j++;
        m++;
      }
      valueString[m] = '\0';
      newDoc->version = atof(valueString);
      free(valueString);
    }
    m = 0;
    if(gpxString[i] == 'c' && gpxString[i+1] == 'r'){
      char *valueString2 = (char *)malloc(200);
      strcpy(valueString2, "");
      j = i + 10;
      while(gpxString[j] != '"'){
        valueString2[m] = gpxString[j];

        j++;
        m++;
        if(gpxString[j] == '"') break;
      }
      valueString2[m] = '\0';
      newDoc->creator = realloc(newDoc->creator, strlen(valueString2) + 1);
      strcpy(newDoc->creator, valueString2);
      free(valueString2);
    }
  }
  strcpy(newDoc->namespace, "http://www.topografix.com/GPX/1/1");
  //printf("Version = %.1f, Creator = %s, namespace = %s\n", newDoc->version, newDoc->creator, newDoc->namespace);

  return newDoc;
}
//ran with val, no leaks
Waypoint* JSONtoWaypoint(const char* gpxString){
  if(gpxString == NULL) return NULL;

  Waypoint *wpt = malloc(sizeof(Waypoint));
  wpt->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
  wpt->name = malloc(1);
  strcpy(wpt->name, "");

  int j = 0;
  int i = 0;
  int m = 0;

  for(i = 0; i < strlen(gpxString); i++){
    if(gpxString[i] == 'l' && gpxString[i+1] == 'a'){
      char *valueString = (char *)malloc(20);
      strcpy(valueString, "");
      j = i + 5;
      while(isdigit(gpxString[j]) || gpxString[j] == '.'){
        if(gpxString[j] == ',') break;
        valueString[m] = gpxString[j];

        j++;
        m++;
      }
      valueString[m] = '\0';
      wpt->latitude = atof(valueString);
      free(valueString);
    }
    if(gpxString[i] == 'l' && gpxString[i+1] == 'o'){
      char *valueString2 = (char *)malloc(20);
      strcpy(valueString2, "");
      j = i + 5;
      m = 0;
      while(isdigit(gpxString[j]) || gpxString[j] == '.' || gpxString[j] == '-'){
        if(gpxString[j] == ',') break;
        valueString2[m] = gpxString[j];

        j++;
        m++;
      }
      valueString2[m] = '\0';
      wpt->longitude = atof(valueString2);
      free(valueString2);
    }
  }

  return wpt;
}
//ran with val, no leaks
Route* JSONtoRoute(const char* gpxString){
  if(gpxString == NULL) return NULL;

  Route *rte = malloc(sizeof(Route));
  rte->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
  rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);

  rte->name = malloc(1);
  strcpy(rte->name, "");

  int j = 0;
  int i = 0;
  int m = 0;

  for(i = 0; i < strlen(gpxString); i++){
    if(gpxString[i] == 'n' && gpxString[i+1] == 'a'){
      char *valueString = (char *)malloc(200);
      strcpy(valueString, "");
      j = i + 7;
      while(gpxString[j] != '"'){
        if(gpxString[j] == '"') break;
        valueString[m] = gpxString[j];

        j++;
        m++;
      }
      valueString[m] = '\0';
      rte->name = realloc(rte->name, strlen(valueString) + 1);
      strcpy(rte->name, valueString);
      free(valueString);
    }
  }

  return rte;
}
//My GPX wrapper function
char *parseGPX(char *fileName){
  char *returnString = NULL;
  char *fileDirectory = malloc(1000);

  strcpy(fileDirectory,"./uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *ptr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  returnString = GPXtoJSON(ptr);

  deleteGPXdoc(ptr);

  return returnString;
}
//My Route list wrapper function
char *parseRoute(char *fileName){
  char *returnString = NULL;
  char *fileDirectory = malloc(1000);

  strcpy(fileDirectory,"./uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *ptr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  returnString = routeListToJSON(ptr->routes);

  deleteGPXdoc(ptr);

  return returnString;
}
//My track list wrapper function
char *parseTrack(char *fileName){
  char *returnString = NULL;
  char *fileDirectory = malloc(1000);

  strcpy(fileDirectory,"./uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *ptr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  returnString = trackListToJSON(ptr->tracks);

  deleteGPXdoc(ptr);

  return returnString;
}
//Wrapper function for writing newly created .gpx doc to file
int writeWrapper(char *fileName, float version, char *creator){
  GPXdoc *GPXdocPtr = malloc(sizeof(GPXdoc));

  strcpy(GPXdocPtr->namespace, "http://www.topografix.com/GPX/1/1");

  GPXdocPtr->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
  GPXdocPtr->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
  GPXdocPtr->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

  GPXdocPtr->creator = malloc(strlen(creator) + 1);
  strcpy(GPXdocPtr->creator, creator);

  GPXdocPtr->version = version;

  int myWrite = 0;
  int myValidate = validateGPXDoc(GPXdocPtr, "./parser/gpx.xsd");
  if(myValidate == 1){
    myWrite = writeGPXdoc(GPXdocPtr, fileName);

    deleteGPXdoc(GPXdocPtr);
    return myWrite;
  }
  else{
    deleteGPXdoc(GPXdocPtr);
    printf("Failed to validate\n");
    return 0;
  }
}
//Wrapper function for adding route
int addRouteWrapper(char *fileName, char *routeName){
  char *fileDirectory = malloc(1000);
  //FIX BELOW
  strcpy(fileDirectory,"./uploads/");
  // strcpy(fileDirectory,"../uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *ptr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  Route *rte = malloc(sizeof(Route));
  rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
  rte->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

  if(strlen(routeName) > 0){
    rte->name = malloc(strlen(routeName) + 5);
    strcpy(rte->name, routeName);
    strcat(rte->name, "!#/");
  }
  else{
    rte->name = malloc(4);
    strcpy(rte->name, "!#/");
  }

  addRoute(ptr, rte);

  int myWrite = 0;
  //FIX BELOW
  int myValidate = validateGPXDoc(ptr, "./parser/gpx.xsd");
  // int myValidate = validateGPXDoc(ptr, "gpx.xsd");
  if(myValidate == 1){
    // printf("Validates\n");
    myWrite = writeGPXdoc(ptr, fileName);

    deleteGPXdoc(ptr);
    //deleteRoute(rte);
    return myWrite;
  }
  else{
    printf("Did not validate in Add Route Wrapper\n");
    deleteGPXdoc(ptr);
    //deleteRoute(rte);

    return 0;
  }
}
//Wrapper function for adding waypoints to the route from addRouteWrapper
void addWaypointWrapper(char *fileName, char *waypointName, float lat, float lon, int numWpts, int i){
  char *fileDirectory = malloc(1000);
  int lastWptOfLoop = numWpts-i;

  //FIX BELOW
  strcpy(fileDirectory,"./uploads/");
  // strcpy(fileDirectory,"../uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *doc = createGPXdoc(fileDirectory);
  free(fileDirectory);

  Waypoint *wpt = malloc(sizeof(Waypoint));
  wpt->attributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

  if(strlen(waypointName) > 0){
    wpt->name = malloc(strlen(waypointName) + 1);
    strcpy(wpt->name, waypointName);
  }
  else{
    wpt->name = malloc(2);
    strcpy(wpt->name, "");
  }

  wpt->longitude = lon;
  wpt->latitude = lat;

  List * routeList = (List *)doc->routes;
  Node * routeNode = (Node *)routeList->head;
  //get route then route attribute toString
  while(routeList->head != NULL){
    Route *rte = (Route *)routeNode->data;
      if(rte->name[strlen(rte->name) - 1] == '/' && rte->name[strlen(rte->name) - 2] == '#' && rte->name[strlen(rte->name) - 3] == '!'){
        addWaypoint(rte, wpt);
      }
    if(routeNode->next != NULL){
      routeNode = routeNode->next;
    }
    else{
      break;
    }
  }

  if(lastWptOfLoop == 0){
    List * routeList2 = (List *)doc->routes;
    Node * routeNode2 = (Node *)routeList->head;
    //get route then route attribute toString
    while(routeList2->head != NULL){
      Route *rte = (Route *)routeNode2->data;
        if(rte->name[strlen(rte->name) - 1] == '/' && rte->name[strlen(rte->name) - 2] == '#' && rte->name[strlen(rte->name) - 3] == '!'){
          rte->name[strlen(rte->name) - 3] = '\0';
        }
      if(routeNode2->next != NULL){
        routeNode2 = routeNode2->next;
      }
      else{
        break;
      }
    }
  }

  int myWrite = writeGPXdoc(doc, fileName);
  if(myWrite == 1){

  }
  else{
    printf("Failed to write Route Waypoints\n");
  }
  deleteGPXdoc(doc);
}
//function to validate files uploaded by user
int validateUploads(char *fileName){
  char *fileDirectory = malloc(1000);

  strcpy(fileDirectory,"./uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *GPXdocPtr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  int myValidate = validateGPXDoc(GPXdocPtr, "./parser/gpx.xsd");
  if(myValidate == 1){
    deleteGPXdoc(GPXdocPtr);
    return myValidate;
  }
  else{
    deleteGPXdoc(GPXdocPtr);
    // printf("Failed to validate\n");
    return 0;
  }
}
//function to find routes matching the user's specified endpoints
char *findBetweenRouteWrapper(char *fileName, float latSrc, float lonSrc, float latDest, float lonDest, int delta){
  char *returnString = NULL;

  // printf("filename = %s, lat = %f, lon = %f, delta = %d\n", fileName, lat, lon, delta);
  char *fileDirectory = malloc(1000);

  //FIX BELOW
  // strcpy(fileDirectory,"../uploads/");
  strcpy(fileDirectory,"./uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *GPXdocPtr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  List *routesList = getRoutesBetween(GPXdocPtr, latSrc, lonSrc, latDest, lonDest, delta);
  returnString = routeListToJSON(routesList);

  deleteGPXdoc(GPXdocPtr);
  freeList(routesList);

  return returnString;
}
//function to find tracks matching user's specified endpoints
char *findBetweenTrackWrapper(char *fileName, float latSrc, float lonSrc, float latDest, float lonDest, int delta){
  char *returnString = NULL;

  char *fileDirectory = malloc(1000);

  //FIX BELOW
  // strcpy(fileDirectory,"../uploads/");
  strcpy(fileDirectory,"./uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *GPXdocPtr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  List *tracksList = getTracksBetween(GPXdocPtr, latSrc, lonSrc, latDest, lonDest, delta);
  returnString = trackListToJSON(tracksList);

  deleteGPXdoc(GPXdocPtr);
  freeList(tracksList);

  return returnString;
}
//Function to rename specified route
int renameRoute(char *fileName, char *oldName, char *newName){
  char *fileDirectory = malloc(1000);
  //FIX BELOW
  strcpy(fileDirectory,"./uploads/");
  // strcpy(fileDirectory,"../uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *GPXdocPtr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  List * routeList = (List *)GPXdocPtr->routes;
  Node * routeNode = (Node *)routeList->head;
  //get route then route attribute toString
  while(routeList->head != NULL){
    Route *rte = (Route *)routeNode->data;
    if(strcmp(rte->name, oldName) == 0){
      rte->name = realloc(rte->name, strlen(newName) + 5);
      strcpy(rte->name, newName);

      int myWrite = 0;
      //FIX BELOW
      int myValidate = validateGPXDoc(GPXdocPtr, "./parser/gpx.xsd");
      // int myValidate = validateGPXDoc(GPXdocPtr, "gpx.xsd");
      if(myValidate == 1){
        // printf("Validates\n");
        myWrite = writeGPXdoc(GPXdocPtr, fileName);
      }
      if(myWrite == 1){
        deleteGPXdoc(GPXdocPtr);
        return 1;
      }
    }
    if(routeNode->next != NULL){
      routeNode = routeNode->next;
    }
    else{
      break;
    }
  }

  deleteGPXdoc(GPXdocPtr);

  return 0;
}
//Function to rename specified track
int renameTrack(char *fileName, char *oldName, char *newName){
  char *fileDirectory = malloc(1000);
  //FIX BELOW
  strcpy(fileDirectory,"./uploads/");
  // strcpy(fileDirectory,"../uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *GPXdocPtr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  List * trackList = (List *)GPXdocPtr->tracks;
  Node * trackNode = (Node *)trackList->head;
  //get route then route attribute toString
  while(trackList->head != NULL){
    Track *trk = (Track *)trackNode->data;
    if(strcmp(trk->name, oldName) == 0){
      trk->name = realloc(trk->name, strlen(newName) + 5);
      strcpy(trk->name, newName);

      int myWrite = 0;
      //FIX BELOW
      int myValidate = validateGPXDoc(GPXdocPtr, "./parser/gpx.xsd");
      // int myValidate = validateGPXDoc(GPXdocPtr, "gpx.xsd");
      if(myValidate == 1){
        // printf("Validates\n");
        myWrite = writeGPXdoc(GPXdocPtr, fileName);
      }
      if(myWrite == 1){
        deleteGPXdoc(GPXdocPtr);
        return 1;
      }
    }
    if(trackNode->next != NULL){
      trackNode = trackNode->next;
    }
    else{
      break;
    }
  }

  deleteGPXdoc(GPXdocPtr);

  return 0;
}
//returns string of route attributes
char *routeAttrListToJSON(char *fileName, char *routeName){
  char *string = malloc(3000);
  strcpy(string, "[");

  char *fileDirectory = malloc(1000);
  //FIX BELOW
  strcpy(fileDirectory,"./uploads/");
  // strcpy(fileDirectory,"../uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *GPXdocPtr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  List * routeList = (List *)GPXdocPtr->routes;
  Node * routeNode = (Node *)routeList->head;
  //get route then route attribute toString
  while(routeList->head != NULL){
    Route *rte = (Route *)routeNode->data;
    if(strcmp(rte->name, routeName) == 0){
      List *attributeList = (List *)rte->attributes;
      Node *attributeNode = (Node *)attributeList->head;
      if(getLength(attributeList) > 0){
        while(attributeList->head != NULL){
          Attribute *attr = (Attribute *)attributeNode->data;
          strcat(string, "{");
          strcat(string, "\"name\":\"");
          strcat(string, attr->name);
          strcat(string, "\",");
          strcat(string, "\"value\":\"");
          strcat(string, attr->value);
          strcat(string, "\"}");
          if(attributeNode->next != NULL){
            strcat(string, ",");
            attributeNode = attributeNode->next;
          }
          else{
            break;
          }
        }
      }
      else{
        strcat(string, "]");
        deleteGPXdoc(GPXdocPtr);
        return string;
      }
    }
    if(routeNode->next != NULL){
      // strcat(string, ",");
      routeNode = routeNode->next;
    }
    else{
      break;
    }
  }

  deleteGPXdoc(GPXdocPtr);
  strcat(string, "]");

  return string;
}
//returns a string of track attributes
char *trackAttrListToJSON(char *fileName, char *trackName){
  char *string = malloc(3000);
  strcpy(string, "[");

  char *fileDirectory = malloc(1000);
  //FIX BELOW
  strcpy(fileDirectory,"./uploads/");
  // strcpy(fileDirectory,"../uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *GPXdocPtr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  List * trackList = (List *)GPXdocPtr->tracks;
  Node * trackNode = (Node *)trackList->head;
  //get route then route attribute toString
  while(trackList->head != NULL){
    Track *trk = (Track *)trackNode->data;
    if(strcmp(trk->name, trackName) == 0){
      List *attributeList = (List *)trk->attributes;
      Node *attributeNode = (Node *)attributeList->head;
      if(getLength(attributeList) > 0){
        while(attributeList->head != NULL){
          Attribute *attr = (Attribute *)attributeNode->data;
          strcat(string, "{");
          strcat(string, "\"name\":\"");
          strcat(string, attr->name);
          strcat(string, "\",");
          strcat(string, "\"value\":\"");
          strcat(string, attr->value);
          strcat(string, "\"}");
          if(attributeNode->next != NULL){
            strcat(string, ",");
            attributeNode = attributeNode->next;
          }
          else{
            break;
          }
        }
      }
      else{
        strcat(string, "]");
        deleteGPXdoc(GPXdocPtr);
        return string;
      }
    }
    if(trackNode->next != NULL){
      // strcat(string, ",");
      trackNode = trackNode->next;
    }
    else{
      break;
    }
  }

  deleteGPXdoc(GPXdocPtr);
  strcat(string, "]");

  return string;
}

char *wptTableInfoJson(char *fileName, int i){
  char *string = malloc(3000);
  char *floatString = NULL;
  int j = 0;
  strcpy(string, "[");

  char *fileDirectory = malloc(1000);
  //FIX BELOW
  strcpy(fileDirectory,"./uploads/");
  // strcpy(fileDirectory,"../uploads/");
  strcat(fileDirectory, fileName);
  GPXdoc *GPXdocPtr = createGPXdoc(fileDirectory);
  free(fileDirectory);

  List * routeList = (List *)GPXdocPtr->routes;
  Node * routeNode = (Node *)routeList->head;
  while(j < i){
    routeNode = routeNode->next;
    j++;
  }

  Route *rte = (Route *)routeNode->data;
  List *rteWaypointList = (List *)rte->waypoints;
  Node *rteWaypointNode = (Node *)rteWaypointList->head;
  //loop through the wpt list
  while(rteWaypointList->head != NULL){
    Waypoint *rteWaypoint = (Waypoint *)rteWaypointNode->data;
    strcat(string, "{");

    strcat(string, "\"lon\":");
    floatString = malloc(15);
    sprintf(floatString, "%f", rteWaypoint->longitude);
    strcat(string, floatString);
    free(floatString);

    strcat(string, ",");

    strcat(string, "\"lat\":");
    floatString = malloc(15);
    sprintf(floatString, "%f", rteWaypoint->latitude);
    strcat(string, floatString);
    free(floatString);

    strcat(string, ",");

    strcat(string, "\"name\":\"");
    strcat(string, rteWaypoint->name);
    strcat(string, "\"}");
    if(rteWaypointNode->next != NULL){
      strcat(string, ",");
      rteWaypointNode = rteWaypointNode->next;
    }
    else{
      break;
    }
  }

  deleteGPXdoc(GPXdocPtr);
  strcat(string, "]");

  return string;
}


// char *string = GPXdocToString(GPXdocPtr);
// printf("string = \n%s\n", string);
// free(string);

/*
OLD MAKEFILE

all: LinkedListAPI.o LinkedListAPI GPXParser.o GPXParser main.o main

LinkedListAPI.o: ./src/LinkedListAPI.c ./include/LinkedListAPI.h
	gcc -Wall -std=c11 -g -c -fpic -Iinclude ./src/LinkedListAPI.c -o ./bin/LinkedListAPI.o

LinkedListAPI: LinkedListAPI.o
	gcc -shared ./bin/LinkedListAPI.o -o ./bin/libgpxparse.so

GPXParser.o: ./src/GPXParser.c ./include/GPXParser.h
	gcc -Wall -std=c11 -g -c -fpic -Iinclude -I/usr/include/libxml2/ ./src/GPXParser.c -o ./bin/GPXParser.o

GPXParser:	GPXParser.o
	gcc -shared ./bin/GPXParser.o -o ./bin/libgpxparse.so

main.o: main.c
	gcc -Wall -std=c11 -g -c -fpic -Iinclude -I/usr/include/libxml2/ main.c -o ./bin/main.o

main: ./bin/main.o ./bin/GPXParser.o ./bin/LinkedListAPI.o
	gcc -Wall -std=c11 -g  ./bin/main.o -o main -lxml2 -lm

clean:
	rm ./bin/.so ./bin/.o main


NEW MAKEFILE

all: parser

parser:
	gcc -Wall -std=c11 -g `xml2-config --cflags --libs` -Iinclude -fpic -c ./src/LinkedListAPI.c -o ./bin/LinkedListAPI.o
	gcc -Wall -std=c11 -g `xml2-config --cflags --libs` -Iinclude -fpic -c ./src/GPXParser.c -o ./bin/GPXParser.o
	gcc -Wall -std=c11 -g `xml2-config --cflags --libs` -Iinclude -fpic -shared ./bin/LinkedListAPI.o ./bin/GPXParser.o -o ./bin/libgpxparse.so

clean:
	rm ./bin/.so ./bin/.o


MAKEFILE FOR PUTTING libgpxparse.so IN A3STUB

all: parser

parser:
	gcc -Wall -std=c11 -g `xml2-config --cflags --libs` -Iinclude -fpic -c ./src/LinkedListAPI.c -o ./bin/LinkedListAPI.o
	gcc -Wall -std=c11 -g `xml2-config --cflags --libs` -Iinclude -fpic -c ./src/GPXParser.c -o ./bin/GPXParser.o
	gcc -Wall -std=c11 -g `xml2-config --cflags --libs` -Iinclude -fpic -shared ./bin/LinkedListAPI.o ./bin/GPXParser.o -o ../libgpxparse.so

clean:
	rm ../.so ./bin/.o

 */
