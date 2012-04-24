<?php

/**
 * @file
 * Contains the utility class to connect to Fedora.
 *
 * This class uses the REST API, as described here:
 * https://wiki.duraspace.org/display/FEDORA35/REST+API
 *
 */

class FedoraConnection {

  /**
   * Constructor for the Fedora object.
   */
  function __construct($url = 'http://localhost:8080/fedora/admin', $username = 'fedoraAdmin', $password = 'fedoraAdmin') {
    $this->url = $url . "/objects";
    $this->username = $username;
    $this->password = $password;
    $this->baseurl = $url;
  }

  /**
   * Check to see if a fedora repository exists and is usable using the url and username/password provided.
   *
   * @return bool
   */
  function check() {

    //Ingest and Purge a temporary object.
    $request = $this->ingestNewEmptyObject();

    if ($request->errorno != "201") {
      return FALSE;
    }
    else{
      $this->purgeObject($request->data);
      return TRUE;
    }
  }

  function ingestNewEmptyObject($pid = "new", $namespace = "rainhattest", $label = "rainhattest") {    
    $options = array("namespace" => $namespace, "label" => $label);
    $url = $this->buildURL($options, $pid);   
    return $this->execCurl($url);    
  }

  function updateObject($pid, $options){
    $url = $this->buildURL($options, $pid);   
    return $this->execCurl($url, '', 'PUT');    
  }
  
  function purgeObject($pid) {
    $url = $this->buildURL(NULL, $pid);
    return $this->execCurl($url, '', 'DELETE');
  }

  function addDatastreamData($pid, $dsID, $controlGroup, $label, $mime_type, $data) {
    $options = array('controlGroup' => $controlGroup, 'dsLabel' => $label, );
    $url = $this->buildURL($options, $pid . '/datastreams/' . $dsID);
    return $this->execCurl($url, $mime_type, 'POST', $data); 
  }
  
  function addDatastreamLocation($pid, $dsID, $controlGroup, $label, $mime_type, $location) {
    $options = array('controlGroup' => $controlGroup, 'dsLabel' => $label, 'dsLocation' => $location);
    $url = $this->buildURL($options, $pid . '/datastreams/' . $dsID);
    return $this->execCurl($url, $mime_type); 
  }
  
  function searchByPIDPrefix($pid, $token = NULL) {
    $options = array('query' => 'pid'."~".$pid.'*', 'pid' => TRUE, 'resultFormat' => 'xml');
    if ($token != NULL){
      $options['sessionToken'] = $token;
    }
    
    $url = $this->buildURL($options, NULL);
    return $this->execCurl($url, '', 'GET'); 
  }
  
  function getObjectXML($pid) {
    $url = $this->buildURL(NULL, $pid . '/objectXML');
    return $this->execCurl($url, '', 'GET'); 
  }
  
  function getDatastreamsList($pid){
    $options = array('format' => 'xml');
    $url = $this->buildURL($options, $pid . '/datastreams');
    return $this->execCurl($url, '', 'GET'); 
  }
  
  function getDatastreamDissemination($pid, $dsid){
    $url = $this->buildURL(NULL, $pid . '/datastreams/' . $dsid . '/content');
    return $this->execCurl($url, '', 'GET'); 
  }
  
  function downloadDatastreamDissemination($pid, $dsid, $file){
    $options = array('download' => TRUE);
    $url = $this->buildURL($options, $pid . '/datastreams/' . $dsid . '/content');
    return $this->execCurlDownload($url, $file); 
  }
  
  


  /**
  * Utility function to build urls.
  */
  protected function buildURL($options=NULL, $method_or_pid = NULL) {

    $url = $this->url;
    if (!is_null($method_or_pid)) {
      $url .= "/" . $method_or_pid;
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
  
  protected function execCurl($url, $mime = '', $verb = 'POST', $post = array()){
    $ch = curl_init();
    if($verb == 'POST'){
      curl_setopt($ch, CURLOPT_POSTFIELDS, $post);
    }
    else{
      curl_setopt($ch, CURLOPT_CUSTOMREQUEST, $verb);      
    }    
    if(!is_array($post)){
      curl_setopt($ch, CURLOPT_HTTPHEADER, Array("Content-Type: ".$mime));
    }
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC); 
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
    curl_setopt($ch, CURLOPT_USERPWD, $this->username . ":" . $this->password);
   
    $data = curl_exec($ch);
    $errorno = curl_getinfo($ch, CURLINFO_HTTP_CODE);    
    curl_close ($ch);
    
    $return = new stdClass();
    $return->data = $data;
    $return->errorno = $errorno;
    return $return;    
  }
  
  protected function execCurlDownload($url, $file){
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'GET');      
    $fp = fopen($file, 'w');
    curl_setopt($ch, CURLOPT_URL, $url);
    curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_BASIC); 
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
    curl_setopt($ch, CURLOPT_USERPWD, $this->username . ":" . $this->password);
    curl_setopt($ch, CURLOPT_FILE, $fp);
   
    $data = curl_exec($ch);
    $errorno = curl_getinfo($ch, CURLINFO_HTTP_CODE);    
    curl_close ($ch);
    
    $return = new stdClass();
    $return->data = $data;
    $return->errorno = $errorno;
    return $return;    
  }

}


