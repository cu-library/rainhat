<?php

/**
 * @file
 * Contains the utility class to connect to Fedora.
 *
 * This class uses the REST API, as described here:
 * https://wiki.duraspace.org/display/FCR30/REST+API
 *
 * The 'describeRepository', 'compareDatastreamChecksum', and 'getDatastreams'
 * methods are not implemented, as per the spec.
 *
 */

class FedoraConnection {

  /**
   * Constructor for the Fedora object.
   */
  function __construct($url = 'http://localhost:8080/fedora/admin', $username = 'fedoraAdmin', $password = 'fedoraAdmin') {
    $this->url = $url;
    $this->username = $username;
    $this->password = $password;
    $this->baseurl = $url . "/objects";
  }

  /**
   * findObjects and resumeFindObjects - API-A
   *
   * Query the repository. For more detailed information, see the API doc.
   * @code
   *   findObjects('demo', array('pid', 'subject', 'label'), array('resultFormat' => 'xml'));
   *   findObjects(array('title~rome', 'creator~staples'), array('pid', 'title', 'creator'));
   *   findObjects(array('pid~*1'), array('pid', 'title'), array('resultFormat' => 'xml', 'maxResults' => 50));
   *   findObjects('*', array('subject', 'label', 'pid'), array('resultFormat' => 'xml', 'sessionToken' => 'xyz\\\\'));
   * @endcode
   *
   * @param $term_or_query
   *   This parameter can either be a simple string or an array.
   *   If it is a string, it will be used as the search term which can include
   *   the ? and * wildcard characters. Fedora will search all object fields
   *   to find the term.
   *   If it is an array, it will be used to build the query for the search.
   *   The query will automatically be URL converted.
   *   The query is a sequence of conditions. A condition consists of a
   *   metadata element name followed directly by an operator, followed
   *   directly by a value.
   *   Valid element names are (pid, label, state, ownerId, cDate, mDate,
   *   dcmDate, title, creator, subject, description, publisher,
   *   contributor, date, type, format, identifier, source, language,
   *   relation, coverage, rights). Valid operators are: contains (),
   *   equals (=), greater than (>), less than (<), greater than or
   *   equals (>=), less than or equals (<=). The contains () operator may
   *   be used in combination with the ? and * wildcards to query for simple
   *   string patterns.
   *
   * @param array $select
   *   An array of strings which determines what fields are returned by the
   *   query.
   *   The possible values are:
   *   pid, label, state, ownerId, cDate, mDate, dcmDate, title, creator,
   *   subject, description, publisher, contributor, date, type, format,
   *   identifier, source, language, relation, coverage, rights
   *
   * @param array $options
   *   An array which contains the options which affect the returned results.
   *   This array should be in Key => Value format.
   *   'maxResults' determines the maximum number of results returned.
   *   'resultFormat' determines the output format: html or xml.
   *   'sessionToken' allows you to return the next page of results from a
   *   search: simply include the token included in the results of the last
   *   search.
   *
   * @return string
   *   The results of the query as a string, in html format unless
   *   specified otherwise by formattingOptions.
   *
   */
  function findObjects($term_or_query = '*', $select = array('pid', 'label'), $options = array('resultFormat' => 'xml')) {

    $finalOptionsArray = array();

    //First, determine if we're using a term or a query
    if (is_array($term_or_query)) {
      $term_or_query = http_build_query($term_or_query, '', '%20');
      $finalOptionsArray['query'] = $term_or_query;
    }
    else {
      $finalOptionsArray['terms'] = $term_or_query;
    }

    //Add the selectOptions as key=>true
    foreach ($select as $field) {
      $finalOptionsArray[$field] = True;
    }

    //Add the formatting options
    if (!is_null($options)) {
      $finalOptionsArray = array_merge($finalOptionsArray, $options);
    }

    $url = $this->buildURL($finalOptionsArray);
    $result = $this->rest_helper($url);

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }

  }

  /**
   * getDatastreamDissemination - API-A
   *
   * Get the contents of a datastream.
   * @code
   *   getDatastreamDissemination('demo:29', 'DC');
   *   getDatastreamDissemination('demo:29', 'DC', '2008-01-01');
   * @endcode
   *
   * @param string $pid
   *   The $pid of the object.
   *
   * @param string $dsID
   *   The datastream identifier of the requested datastream.
   *
   * @param array $options
   *   The options array.
   *   'asOfTimeDate' => ISO_8601 encoded UTC datetime value.
   *   'download' => true to have the user download the contents of the
   *   datastream as opposed to returning them.
   */
  function getDatastreamDissemination($pid, $dsID, $options = NULL) {

    $url = $this->buildURL($options, $pid . "/datastreams/" . $dsID . "/content");
    $result = $this->rest_helper($url);

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    elseif ($result->getStatus() == 404) {
      return FALSE;
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * getDissemination - API-A
   *
   * Invoke a method of a Service defined in a Service Definition for a
   * particular object and return the result.
   * @code
   *   getDissemination('demo:29', 'demo:27', 'resizeImage', array('width' => 100));
   *   getDissemination('demo:SmileyEarring', 'demo:DualResolution', 'fullSize');
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param string $sdefPid
   *   Persistent identifier of the sDef defining the methods.
   *
   * @param string $method
   *   Method to invoke.
   *
   * @param array $parameters
   *   Parameters to pass to the method.
   */
  function getDissemination($pid, $sdefPid, $method, $parameters = NULL) {
    $url = $this->buildURL($options, $pid . "/datastreams/" . $dsID . "/content");
    $result = $this->rest_helper($url);

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }

  }

  /**
   * getObjectHistory - API-A
   *
   * Get a list of dates and times showing when an object was created and
   * modified in ISO_8601 encoded UTC datetimes.
   * @code
   *   getObjectHistory('demo:29');
   *   getObjectHistory('demo:29', array('format' => 'xml'));
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param array $options
   *   The options array.
   *   'format' => 'xml' or 'html'
   */
  function getObjectHistory($pid, $options = NULL) {
    $url = $this->buildURL($options, $pid . "/versions");
    $result = $this->rest_helper($url);

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * getObjectProfile - API-A
   *
   * Get a general overview of an object, which contains it's pid, Label, and
   * other info.
   * @code
   *   getObjectProfile('demo:29');
   *   getObjectProfile('demo:29', array('format' => 'xml'));
   *   getObjectProfile('demo:29', array('asOfDateTime' => '2008-01-01'));
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param array $options
   *   The options array.
   *   'format' => 'xml' or 'html'
   *   'asOfDateTime' => ISO_8601 encoded UTC datetime value.
   */
  function getObjectProfile($pid, $options = NULL) {
    $url = $this->buildURL($options, $pid);
    $result = $this->rest_helper($url);

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * listDatastreams - API-A
   *
   * Get a list of an object's datastreams.
   * @code
   *   listDatastreams('demo:35');
   *   listDatastreams('demo:35', array('format' => 'xml', 'asOfDateTime' => '2008-01-01T05:15:00Z'));
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param array $options
   *   The options array.
   *   'format' => 'xml' or 'html'
   *   'asOfDateTime' => ISO_8601 encoded UTC datetime value.
   */
  function listDatastreams($pid, $options = NULL) {
    $url = $this->buildURL($options, $pid . "/datastreams");
    $result = $this->rest_helper($url);

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * listMethods - API-A
   *
   * Get a list of an object's available methods.
   * A list can be generated of all the available methods, or of the methods
   * provided by a particular Service Definition.
   * @code
   *   listMethods('demo:29');
   *   listMethods('demo:29', NULL, array('format' => 'xml', 'asOfDateTime' => '2008-01-01T05:15:00Z'));
   *   listMethods('demo:29', 'demo:27');
   *   listMethods('demo:29', 'demo:27', array('format' => 'xml', 'asOfDateTime' => '2008-01-01T05:15:00Z'));
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param string $sdefPid
   *   Persistent identifier of the sDef defining the methods.
   *
   * @param array $options
   *   The options array.
   *   'format' => 'xml' or 'html'
   *   'asOfDateTime' => ISO_8601 encoded UTC datetime value.
   */
  function listMethods($pid, $sdefPid = NULL, $options = NULL) {
    if (!is_null($sdefPid)) {
      $url = $this->buildURL($options, $pid . "/methods/" . $sdefPid);
    }
    else {
      $url = $this->buildURL($options, $pid . "/methods");
    }

    $result = $this->rest_helper($url);

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * addDatastream - API-M
   *
   * Add a datastream to an object.
   * @code
   *   addDatastream('demo:29', 'NEWDS', array('controlGroup' => 'X', 'dsLabel' => 'New'), 'new_file.jpg', 'image/jpg');
   *   addDatastream('demo:29', 'NEWDS', array('controlGroup' => 'M', 'dsLocation' => 'http://example:80/newds', 'dsLabel' => 'New'));
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param string $dsID
   *   The new Datastream identifier.
   *
   * @param array $options
   *   The options array. See the docs
   *   https://wiki.duraspace.org/display/FCR30/REST+API#RESTAPI-addDatastream
   *   for the full list of options.
   *
   * @param string $fileLocation
   *   The location of the file to be uploaded.
   *
   * @param string $contentType
   *   The mime type of the new content, if the new datastream is managed.
   */
  function addDatastream($pid, $dsID, $options = NULL, $fileLocation = NULL, $contentType = NULL, $inlineFile = FALSE) {

    //Fix for https://jira.duraspace.org/browse/FCREPO-849
    // DC, RELS-EXT, and RELS-INT must be versionable.
    if ($dsID == 'DC' || $dsID == 'RELS-EXT' || $dsID == 'RELS-INT') {
      if (!is_null($options) && array_key_exists('versionable', $options)) {
        $options['versionable'] = True;
      }
    }

    krumo($fileLocation);
    if (!is_null($fileLocation)) {
      if (is_null($options)) {
        $options = array();
        $options['controlGroup'] = 'M';
        krumo($options);
      }      
    }

    $url = $this->buildURL($options, $pid . "/datastreams/" . $dsID);    
    
    $result = $this->rest_helper($url, 'POST', $fileLocation, $contentType, $inlineFile);

    if ($result->getStatus() == 201) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }

  }

  /**
   * addRelationship - API-M
   *
   * Add a relationship between two objects.
   * @code
   *   addRelationship('demo:29', array('subject' => 'info:fedora/demo:29', 'predicate' => 'http://www.example.org/rels/name', 'object' => 'dublin core', 'isLiteral' => 'true');
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param array $options
   *   The options array which describes the relationship. See the docs
   *   https://wiki.duraspace.org/display/FCR30/REST+API#RESTAPI-addRelationship
   *   the full list of options.
   */
  function addRelationship($pid, $options) {

    $url = $this->buildURL($options, $pid . "/relationships/new");
    $result = $this->rest_helper($url, 'POST');
    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }

  }

  /**
   * export - API-M
   *
   * Export an object as XML.
   * @code
   *   export('demo:29');
   *   export('demo:29', array('context' => 'migrate'));
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param array $options
   *   The options array. See the docs
   *   https://wiki.duraspace.org/display/FCR30/REST+API#RESTAPI-export
   *   the full list of options.
   */
  function export($pid, $options = NULL) {
    $url = $this->buildURL($options, $pid . "/relationships/new");
    $result = $this->rest_helper($url);
    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * getDatastream - API-M
   *
   * Get the contents of a datastream.
   * @code
   *   getDatastream('demo:29', 'DC');
   *   getDatastream('demo:29', 'DC', array('format' => 'xml'));
   *   getDatastream('demo:29', 'DC', array('format' => 'xml', 'validateChecksum' => 'true'));
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param string $dsID
   *   The Datastream identifier.
   *
   * @param array $options
   *   The options array. See the docs
   *   https://wiki.duraspace.org/display/FCR30/REST+API#RESTAPI-getDatastream
   *   the full list of options.
   */
  function getDatastream($pid, $dsID, $options = NULL) {
    $url = $this->buildURL($options, $pid . "/datastreams/" . $dsID);
    $result = $this->rest_helper($url);
    
    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    elseif ($result->getStatus() == 404) {
      return FALSE;
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * getDatastreamHistory - API-M
   *
   * Get the history of a datastream.
   * @code
   *   getDatastreamHistory('changeme:1', 'DC');
   *   getDatastreamHistory('changeme:1', 'DC', array('format' => 'xml'));
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param string $dsID
   *   The Datastream identifier.
   *
   * @param array $options
   *   The options array. See the docs
   *   https://wiki.duraspace.org/display/FCR30/REST+API#RESTAPI-getDatastreamHistory
   *   the full list of options.
   */
  function getDatastreamHistory($pid, $dsID, $options = NULL) {
    $url = $this->buildURL($options, $pid . "/datastreams/" . $dsID . "/versions");
    $result = $this->rest_helper($url);
    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * getNextPID - API-M
   *
   * Get the next available PID in a particular namespace or the default
   * namespace.
   * @code
   *   getNextPID();
   *   getNextPID(array('numPIDS' => 5, 'namespace' => 'test', 'format' => 'xml'));
   * @endcode
   *
   * @param array $options
   *   The options array. See the docs
   *   https://wiki.duraspace.org/display/FCR30/REST+API#RESTAPI-getNextPID
   *   the full list of options.
   */
  function getNextPID($options = NULL) {
    $url = $this->buildURL($options, 'nextPID');
    $result = $this->rest_helper($url, 'POST');
    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * getObjectXML - API-M
   *
   * Get the XML of an object.
   * @code
   *   getObjectXML('demo:29');
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   */
  function getObjectXML($pid) {
    $url = $this->buildURL($options, $pid . '/objectXML');
    $result = $this->rest_helper($url);
    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * getRelationships - API-M
   *
   * Get the relationships of an object.
   * @code
   *   getRelationships('demo:29');
   *   getRelationships('demo:29', array('subject' => 'info:fedora/demo:29/DC'));
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param array $options
   *   The options array. See the docs
   *   https://wiki.duraspace.org/display/FCR30/REST+API#RESTAPI-getRelationships
   *   the full list of options.
   */
  function getRelationships($pid, $options = NULL) {
    $url = $this->buildURL($options, $pid . "/relationships/");
    $result = $this->rest_helper($url);
    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * ingest - API-M
   *
   * Ingest an XML document into the repository as a new object.
   * @code
   *   ingest();
   *   ingest('new', array('namespace' => 'demo'));
   *   ingest('test:100', array('label' => 'Test'));
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param array $options
   *   The options array. See the docs
   *   https://wiki.duraspace.org/display/FCR30/REST+API#RESTAPI-ingest
   *   the full list of options.
   *
   * @param string $fileLocation
   *   The location of the XML file to be uploaded.
   */
  function ingest($pid = NULL, $options = NULL, $fileLocation = NULL) {
    if (is_null($pid)) {
      $pid = 'new';
    }
    $url = $this->buildURL($options, $pid);
    if (!is_null($fileLocation)) {
      $result = $this->rest_helper($url, 'POST', $fileLocation, 'text/xml');
    }
    else{
      $result = $this->rest_helper($url, 'POST');
    }

    if ($result->getStatus() == 201) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

 /**
   * modifyDatastream - API-M
   *
   * Modify a datastream of an object.
   * @code
   *   modifyDatastream('demo:35', 'HIGH', 'new_pic.jpg', 'image/jpg');
   *   modifyDatastream('demo:29', 'HIGH', array('dsLocation' => 'http://example:80/highDS', 'logMessage' => 'Update'));
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param string $dsID
   *   The new Datastream identifier.
   *
   * @param array $options
   *   The options array. See the docs
   *   https://wiki.duraspace.org/display/FCR30/REST+API#RESTAPI-modifyDatastream
   *   for the full list of options.
   *
   * @param string $fileLocation
   *   The location of the file to be uploaded.
   *
   * @param string $contentType
   *   The mime type of the new content, if the new datastream is managed.
   */
  function modifyDatastream($pid, $dsID, $options = NULL, $fileLocation = NULL, $contentType = NULL, $inlineFile = FALSE) {

    $url = $this->buildURL($options, $pid . "/datastreams/" . $dsID);

    $result = $this->rest_helper($url, 'PUT', $fileLocation, $contentType, $inlineFile);

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }

  }

  /**
   * modifyObject - API-M
   *
   * Modify an object.
   * @code
   *   modifyObject('demo:29', array('label' = 'Updated'));
   *   modifyObject('demo:29', array('state' = 'D', 'logMessage' = 'Deleted'));
   * @endcode
   *
   * @param string $pid
   *   Persistent identifier for the digital object.
   *
   * @param array $options
   *   The options array. See the docs
   *   https://wiki.duraspace.org/display/FCR30/REST+API#RESTAPI-modifyObject
   *   for the full list of options.
   */
  function modifyObject($pid, $options) {

    $url = $this->buildURL($options, $pid);

    $result = $this->rest_helper($url, 'PUT');

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * purgeObject - API-M
   *
   * Purge an object from the repository.
   * @code
   *   purgeObject('demo:29');
   * @endcode
   *
   * @param string $pid
   *   Persistent identifier for the digital object.
   *
   * @param string $logMessage
   *   A log message to attach to the purge action.
   */
  function purgeObject($pid, $logMessage = NULL) {
    $options = NULL;
    if (!is_null($logMessage)) {
      $options = array('logMessage' => $logMessage);
    }
    $url = $this->buildURL($options, $pid);

    $result = $this->rest_helper($url, 'DELETE');

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * purgeRelationship - API-M
   *
   * Remove a relationship from an object.
   * @code
   *   purgeRelationship('demo:29', array('subject' => 'info:fedora/demo:29/DC', 'predicate' => 'http://www.example.org/rels/name', 'object' =>'dublin core', 'isLiteral' => 'true'));
   * @endcode
   *
   * @param string $pid
   *   Persistent identifier for the digital object.
   *
   * @param array $options
   *   The options which describe the relationship. See the docs here:
   *   https://wiki.duraspace.org/display/FCR30/REST+API#RESTAPI-purgeRelationship
   */
  function purgeRelationship($pid, $options) {

    $url = $this->buildURL($options, $pid . 'relationships');

    $result = $this->rest_helper($url, 'DELETE');

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * setDatastreamState - API-M
   *
   * Modify a datastream of an object.
   * @code
   *   setDatastreamState('demo:35', 'HIGH', 'D');
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param string $dsID
   *   The new Datastream identifier.
   *
   * @param $dsState
   *   The new datastream state.
   */
  function setDatastreamState($pid, $dsID, $dsState) {

    $options = array('dsState' => $dsState);

    $url = $this->buildURL($options, $pid);

    $url = $this->buildURL($options, $pid . "/datastreams/" . $dsID);

    $result = $this->rest_helper($url, 'PUT', $fileLocation, $contentType);

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }

  }

  /**
   * setDatastreamVersionable - API-M
   *
   * Set if a datastream is versionable or not.
   * @code
   *   setDatastreamVersionable('demo:35', 'HIGH', 'false');
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param string $dsID
   *   The new Datastream identifier.
   *
   * @param $versionable
   *   Set versionable to this value.
   */
  function setDatastreamVersionable($pid, $dsID, $versionable) {

    if (is_string($versionable)) {
      if (drupal_strtolower($versionable) == 'true') {
        $versionable = True;
      }
      elseif (drupal_strtolower($versionable) == 'false') {
        $versionable = False;
      }
      else{
        $versionable = True;
      }
    }

    //Fix for https://jira.duraspace.org/browse/FCREPO-849
    // DC, RELS-EXT, and RELS-INT must be versionable.
    if (($dsID == 'DC' || $dsID == 'RELS-EXT' || $dsID == 'RELS-INT') && $verionable == False) {
      throw new Exception('Error: unable to set datastream state, DC, RELS-EXT, and RELS-INT must be versionable');
    }

    $options = array('versionable' => $versionable);
    $url = $this->buildURL($options, $pid . "/datastreams/" . $dsID);
    $result = $this->rest_helper($url, 'PUT');

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }

  }

  /**
   * validate - API-M
   *
   * Validate that an object is correctly formed.
   * @code
   *   validate('demo:fail);
   *   validate('demo:29', '2008-01-01');
   * @endcode
   *
   * @param string $pid
   *   Persistent Identifier of the object.
   *
   * @param string $asOfDateTime
   *  ISO_8601 encoded UTC datetime value.
   */
  function validate($pid, $asOfDateTime = NULL) {

    $options = NULL;
    if (!is_null($asOfDateTime)) {
      $options = array('asOfDateTime' => $asOfDateTime);
    }

    $url = $this->buildURL($options, $pid . '/validate');
    $result = $this->rest_helper($url, 'GET');

    if ($result->getStatus() == 200) {
      return $result->getBody();
    }
    else {
      throw new Exception(sprintf('Error: Could not connect to fedora: Request %s failed with response %s', $url, $result->getReasonPhrase()));
    }
  }

  /**
   * The REST Helper. This is where the magic happens. Uses the HTTP_Request2
   * PEAR module to send requests.
   * http://pear.php.net/manual/en/package.http.http-request2.php
   */
  protected function rest_helper($url, $verb = 'GET', $fileLocation = NULL,  $contenttype = NULL, $inlineFile = FALSE) {

    switch ($verb) {
      case 'GET':
        $verb = HTTP_Request2::METHOD_GET;
        break;
      case 'POST':
        $verb = HTTP_Request2::METHOD_POST;
        break;
      case 'PUT':
        $verb = HTTP_Request2::METHOD_PUT;
        break;
      case 'DELETE':
        $verb = HTTP_Request2::METHOD_DELETE;
        break;
    }
    $request = new HTTP_Request2($url, $verb);
    $request->setAuth($this->username, $this->password, HTTP_Request2::AUTH_BASIC);
    if (!is_null($fileLocation)) {
      if($inlineFile){
        $request->setBody($fileLocation, False);
      }
      else{
        $request->setBody($fileLocation, True);
      }
      if (!is_null($contenttype)) {
        $request->setHeader('Content-Type', $contenttype);
      }
      else{
        $fileinfo = new finfo(FILEINFO_MIME_TYPE);
        $request->setHeader('Content-Type', $fileinfo->file($fileLocation));
      }

    }

    $response = $request->send();
    return $response;
  }

  /**
   * Utility function to build some urls.
   */
  protected function buildURL($options=NULL, $methodorpid = NULL) {

    $url = $this->baseurl;
    if (!is_null($methodorpid)) {
      $url .= "/" . $methodorpid;
    }
    if (!is_null($options)) {
      foreach ($options as $key => $value) {
        if (is_bool($value)) {
          $string_value = ($value) ? 'true' : 'false';
          $options[$key] = $string_value;
        }
      }
      $url = $url . '?' . http_build_query($options, '', '&');
    }
    return $url;
  }
  
  /**
   * Utility function to see if an object exists.
   */
  function exists($pid) {
    $url = $this->buildURL(NULL, $pid);
    $result = $this->rest_helper($url);

    if ($result->getStatus() == 200) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
  
}


